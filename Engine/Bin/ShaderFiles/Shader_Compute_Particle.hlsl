
/* [ ��ƼŬ ���� �Ӽ� ���պ� ] */
struct ParticleParam
{
    float4      Right;
    float4      Up;
    float4      Look;
    float4      Translation;
    
    float3      vInitOffset;
    uint        bFirstLoopDiscard;
    
    float4      Direction;  // normalized dir (w=unused)
    float4      VelocityDir; // ���� �̵��� ���� ����, w = length
    
    float2      LifeTime;   // x=max, y=acc
    float       Speed;
    float       fTileIdx;

    float       OrbitSpeed; // degrees/sec
    float       fAccel;     // ���ӵ� (+�� ����, -�� ����)
    float       fMaxSpeed;  // �ִ� �ӵ� (�ɼ�)
    float       fMinSpeed;  // �ּ� �ӵ� (�ɼ�, ���� �� ���� ����)
    
    float2      vTileOffset;
    float       RotationSpeed; // degrees/sec
    float       RotationAngle;
};


/* [ Constant Buffer ] */
cbuffer ParticleCB : register(b0)
{
    float DeltaTime; // dt (tool�̸� ����)
    float TrackTime; // tool ����ð�(��) = curTrackPos/60.f
    uint ParticleType; // 0:SPREAD, 1:DIRECTIONAL
    uint NumInstances;

    uint isTool; // uint == bool
    uint isLoop;
    uint UseGravity;
    uint UseSpin;

    uint UseOrbit;
    uint isFirst;
    uint isTileLoop;
    float fGravity; // e.g. 9.8

    float2 vTileCnt;
    float fTileTickPerSec;
    float fAccTime;

    float3 Pivot; // vPivot
    uint EffectSeed;
    
    float3 Center; // vCenter
    float ShrinkThreshold;
    
    float3 OrbitAxis; // normalized
    uint LoopInSet;

    float3 Range; // vRange
    float LoopInSet_LoopDelay;
    
    float3 CircleNormal;
    uint isCircleRange;

    float4 vSocketRot;

    row_major float4x4 g_CombinedMatrix;
};

// t�� SRV, u�� UAV, b�� Constant Buffer
// ���� ���ڴ� ���°�� ���ε� �ߴ����� ����
// RWStructuredBuffer - Read / Write
// StructuredBuffer - Read only.
StructuredBuffer<ParticleParam> gInitInst : register(t0); 
RWStructuredBuffer<ParticleParam> gInst : register(u0);

//static const float PI = 3.14159265f;

float3 rotateAroundAxis(float3 v, float3 axis, float angleRad)
{
    float s = sin(angleRad), c = cos(angleRad);
    return v * c + cross(axis, v) * s + axis * dot(axis, v) * (1.0f - c);
}


float3 RotateByQuat(float3 v, float4 q)
{
    // q * v * q^-1
    float3 t = 2.0f * cross(q.xyz, v);
    return v + q.w * t + cross(q.xyz, t);
}

float Hash(uint n)
{
    n = (n << 13u) ^ n;
    return ((n * (n * n * 15731u + 789221u) + 1376312589u) & 0x7fffffffu)
           / 2147483648.0; // 2^31
}

// iSeed + EffectSeed + �ð�(fAccTime)�� ���� ����
float Random(uint iSeed, float fMin, float fMax)
{
    // �ð��� float �� uint ��ȯ �������� aliasing�� �� ������ �����ϸ��ؼ� ����ȭ
    uint timeSeed = asuint(fAccTime * 1000.0f); // �и��� ���� ����ȭ

    // �� ������ XOR/�� �������� ����
    uint mixedSeed = iSeed ^ (EffectSeed * 1664525u) ^ (timeSeed * 1013904223u);

    float rnd = Hash(mixedSeed); // [0,1)

    return lerp(fMin, fMax, rnd);
}

// �� �ѷ� �� ���� ��ǥ
//float4 RandomOnCircle(uint iSeed, float3 Center, float Radius)
//{
//    float angle = Random(iSeed, 0.0f, 6.2831853f);
//    float x = cos(angle) * Radius;
//    float z = sin(angle) * Radius;

//    return float4(Center + float3(x, 0.0f, z), 1.f);
//}

// ���ϴ� ������ ��� �� �� �ѷ� ���� ��ǥ                  (����)
// EllipseScale�� ���ؼ� Ÿ��ó��. range z�� �־�����ϱ� �⺻���� 1�� �ֱ�...
float4 RandomOnCircle(uint iSeed, float3 Center, float Radius, float EllipseScale /*, float3 Normal*/)
{
    // --- ������ ���� ---
    float3 Normal = normalize(CircleNormal);

    // --- ������ �����ϴ� ���� �ϳ� ��� ---
    float3 ref = (abs(Normal.y) < 0.999f) ? float3(0, 1, 0) : float3(1, 0, 0);
    float3 tangent = normalize(cross(Normal, ref));

    // --- �� ��° ���� ���� ---
    float3 bitangent = cross(Normal, tangent);

    // --- ���� ���� ---
    float angle = Random(iSeed, 0.0f, 6.2831853f);
    float c = cos(angle) * Radius;
    float s = sin(angle) * Radius * EllipseScale;

    // --- ��� �� �� ---
    float3 pos = Center + tangent * c + bitangent * s;

    return float4(pos, 1.f);
}


float4 RandomInRange(uint iSeed, float3 Center, float3 Range)
{
    return float4(
        Random(iSeed, Center.x - Range.x * 0.5f, Center.x + Range.x * 0.5f),
        Random(iSeed + 1, Center.y - Range.y * 0.5f, Center.y + Range.y * 0.5f),
        Random(iSeed + 2, Center.z - Range.z * 0.5f, Center.z + Range.z * 0.5f),
        1.f );
}

[numthreads(128, 1, 1)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint i = dispatchThreadId.x;
    if (i >= NumInstances)
        return;

    ParticleParam pp = gInst[i];
    float3 prevPos = pp.Translation.xyz;
    const bool TOOL = (isTool != 0);

    if (TOOL)
    {
        // --- �� ���: ����ð����� "�ʱⰪ ����" �籸�� ---
        pp.bFirstLoopDiscard = 0;
        float t = TrackTime / 60.f;
        if (isLoop != 0)
            t = fmod(t, pp.LifeTime.x);
        pp.fTileIdx = TrackTime / 60.f / fTileTickPerSec;
        if (isTileLoop != 0)
            pp.fTileIdx = fmod(pp.fTileIdx, vTileCnt.x * vTileCnt.y);
        uint iTileIdx = (uint) pp.fTileIdx;
        float fTileSizeX = 1.0f / float(vTileCnt.x);
        float fTileSizeY = 1.0f / float(vTileCnt.y);
        pp.vTileOffset = float2((iTileIdx % uint(vTileCnt.x)) * fTileSizeX, (iTileIdx / uint(vTileCnt.x)) * fTileSizeY);

        const ParticleParam init = gInitInst[i]; // �׻� �ʱⰪ ����
        float3 pos = init.Translation.xyz;
        
        float3 dir = normalize(pp.Direction.xyz);
        float v0 = init.Speed;
        float accel = pp.fAccel;
        float vMin = pp.fMinSpeed;
        float vMax = pp.fMaxSpeed;

        float travel = 0.0f;

        if (accel != 0.0f)
        {
            // vMin�� �����ϴ� �ð�
            float tStop = (vMin - v0) / accel;

            if ((accel < 0 && t < tStop) || (accel > 0 && v0 < vMin))
            {
            // ���� vMin�� ���� �� �� �� ����/���� ����
                travel = v0 * t + 0.5f * accel * t * t;
            }
            else
            {
            // vMin ���Ϸ� �������ų�, vMin �̻����� �ö� �� �� vMin ���
                float stopDist = v0 * tStop + 0.5f * accel * tStop * tStop;
                float remain = max(t - tStop, 0.0f);
                travel = stopDist + vMin * remain;
            }
        }
        else
        {
            // ���ӵ� 0�̸� �׳� ���
            float curSpeed = clamp(v0, vMin, vMax);
            travel = curSpeed * t;
        }

        // ���� ��ġ
        pos += dir * travel;
        /*****/

        // �߷�: ����ð� ���� (s = 1/2 g t^2)
        if (UseGravity != 0)
            pos.y -= 0.5f * fGravity * t * t;

        // ����: �ʱ� basis�� "���� ����" ����
        else
        {
            pp.Right = init.Right;
            pp.Up = init.Up;
            pp.Look = init.Look;
        }

        // ����: (pos - Center)�� "���� ����"��ŭ ȸ��
        if (UseOrbit != 0)  
        {
            float3 axis = normalize(OrbitAxis);
            float angle = radians(pp.OrbitSpeed) * t;
            float3 offset = pos - Center;
            offset = rotateAroundAxis(offset, axis, angle);
            pos = Center + offset;
        }

        pp.Translation = float4(pos, 1.0f);
        pp.LifeTime.y = t; // ���� ����ð��� �״�� ����
        float3 velocity = pos - init.Translation.xyz;
        pp.VelocityDir = float4(normalize(velocity), length(velocity));
        // ����� ���������� ��� �� ���� �ִµ� �ϴ� �ӽ÷� ���� �ʱ���ġ���� ��
        gInst[i] = pp;
        return;
    }
    else
    {
        // --- ��Ÿ�� ���: ����(��t) ������Ʈ ---
        pp.LifeTime.y += DeltaTime;
        
        if (isFirst == 1)
        {
            /* ����Ʈ ���� ���� ���� ��(bake �ϰ� ���� ����) */
            //pp.Translation = float4(mul(float4(pp.Translation.xyz, 1.f), g_CombinedMatrix).xyz, 1.f);
            //
            //float3 localDir = gInitInst[i].Direction.xyz;
            //float3 worldDir = RotateByQuat(localDir, vSocketRot);
            //pp.Direction.xyz = normalize(worldDir);

            /******************************************************************************/

            /* ����Ʈ ���� ���� �� (bake �� �ϰ�) */

            float3 vCurPivot = Center + Pivot;
            
            if (isCircleRange == 0)
            {
                pp.Translation = RandomInRange(i * 3, Center.xyz, Range.xyz);
            }
            else
            {
                pp.Translation = RandomOnCircle(i * 3, Center.xyz, Range.x * 0.5f, Range.z);
            }
            
            if (ParticleType == 0) // spread
            {
                pp.Direction = float4(normalize(pp.Translation.xyz - vCurPivot.xyz), 0.f);
            }
            else if (ParticleType == 1) // directional
            {
                pp.Direction = float4(normalize(Center.xyz - vCurPivot.xyz), 0.f);
            }
            else if (ParticleType == 2) // random
            {
                pp.Direction = float4(
                    Random(i * 3    , -1.f, 1.f), 
                    Random(i * 3 + 1, -1.f, 1.f),
                    Random(i * 3 + 2, -1.f, 1.f),
                    0.f
                );
            }
            else if (ParticleType == 3) // to pivot
            {
                pp.Direction = float4(normalize(vCurPivot.xyz - pp.Translation.xyz), 0.f);
            }
            
            
            //pp.Translation = float4(mul(float4(pp.Translation.xyz, 1.f), g_CombinedMatrix).xyz, 1.f);
            float3 worldDir = RotateByQuat(pp.Direction.xyz, vSocketRot);
            pp.Direction.xyz = normalize(worldDir);
            
            pp.vInitOffset = float3(
			    pp.Translation.x,
			    pp.Translation.y,
			    pp.Translation.z
		    );
            
            
            if (pp.RotationSpeed < 0.0001f)
                pp.RotationAngle = 0.f;
            else
                pp.RotationAngle = Random(pp.RotationSpeed, 0.f, 3.14f);
            // MeshEmitter ��� �� �Լ� ȣ�� �϶�µ�
            // Spawn();
        }
        
        float t = pp.LifeTime.y;
        float tPrev = max(t - DeltaTime, 0.0f);

        float3 pos = pp.Translation.xyz;
        float3 dir = normalize(pp.Direction.xyz);

        //// �̵�: ��s = v * ��t
        //pos += dir * pp.Speed * DeltaTime;
        
        // ���� �߰� ����
        // �ӵ� ����: v = v + a*��t (�ּ�/�ִ� �ӵ� ���� ����)
        pp.Speed = clamp(pp.Speed + pp.fAccel * DeltaTime, pp.fMinSpeed, pp.fMaxSpeed);

        // �̵�: ��s = v * ��t 
        pos += dir * pp.Speed * DeltaTime;

        // �߷�: ��s = 1/2 g (t^2 - (t-��t)^2)  = g*(t-��t/2)*��t
        if (UseGravity != 0)
        {
            float gravNow = 0.5f * fGravity * t * t;
            float gravPrev = 0.5f * fGravity * tPrev * tPrev;
            pos.y -= (gravNow - gravPrev);
        }

        // ����: ���� basis�� �İ�����ŭ ȸ��
        if (UseSpin != 0)
        {
            float angle = radians(pp.RotationSpeed) * DeltaTime;
        }

        // ����: �İ�����ŭ ������ ȸ��
        if (UseOrbit != 0)
        {
            float3 axis = normalize(OrbitAxis);
            float angle = radians(pp.OrbitSpeed) * DeltaTime;
            float3 offset = pos - Center;
            offset = rotateAroundAxis(offset, axis, angle);
            pos = Center + offset;
        }
        
        pp.fTileIdx += DeltaTime * fTileTickPerSec;
        
        float3 velocity = pos - prevPos;
        if (isFirst)
            pp.VelocityDir = float4(0.f, 0.f, 0.f, 0.f);
        else
            pp.VelocityDir = float4(normalize(velocity), length(velocity));

        const bool firstLoop = (pp.bFirstLoopDiscard != 0);
        const bool lifeEnded = pp.LifeTime.y >= pp.LifeTime.x;
        const bool tileEnded = pp.fTileIdx >= vTileCnt.x * vTileCnt.y;
        const bool SetLifeEnded = pp.LifeTime.y >= LoopInSet_LoopDelay;

        //bool needReset = firstLoop ? lifeEnded 
        //                : ((isTileLoop != 0) ? tileEnded
        //                                        : ((isLoop != 0) && lifeEnded));
        //// ���� ���׻��׿����ڿ� ������
        //bool needReset = false;
        //if (firstLoop)
        //    needReset = lifeEnded;
        //else if (isTileLoop != 0)
        //    needReset = tileEnded; // TileLoop �켱
        //else if (isLoop != 0)
        //    needReset = lifeEnded;
        
        bool needReset = false;

        if (isTileLoop != 0) // true
        {
            if (firstLoop)
                needReset = lifeEnded; // ù ������ lifeTime���θ� ������
            else
                needReset = tileEnded; // ���ĺ��ʹ� tileLoop�� üũ
        }
        else
        {
            if (firstLoop)
                needReset = lifeEnded; // ���� ���� �״��
            else if (isLoop != 0)
                needReset = lifeEnded;
        }
        if (LoopInSet != 0) // true
        {
            needReset = SetLifeEnded;
        }
        if (ParticleType == 3)
        {
            float3 vCurPivot = Center + Pivot;
            //float fDist = length(vCurPivot - pos);
            float fPivotRadius = 0.01f;
            float3 prevPos = pp.Translation.xyz;
            
            // === ����-��ü ���� üũ ===
            float3 seg = pos - prevPos;
            float3 toPivot = vCurPivot - prevPos;

            // t = [0,1] ���̸� ���� �� ����
            float t = dot(toPivot, seg) / dot(seg, seg);
            t = saturate(t); // 0~1 clamp

            // �ֱ��� ���� pivot �Ÿ�
            float3 closest = prevPos + seg * t;
            float fDist = length(vCurPivot - closest);

            if (fDist < ShrinkThreshold)
            {
                if (isLoop != 0)
                    needReset = true;
                else
                    pp.LifeTime.y = pp.LifeTime.x;
            }
        }

        if (needReset)
        {
            pp.LifeTime.y = 0.0f;
            pp.Speed = gInitInst[i].Speed; // �ӵ� �ʱ�ȭ�� ����
            pp.bFirstLoopDiscard = 0;
            pp.fTileIdx = 0.f;
            
            // initoffset�� ����ؼ� �ʱ� ��� �״�� �����Ұ���..
            //float3 offset0 = gInitInst[i].vInitOffset;
            //pos = Center + offset0; // << ������
            
            //// === �� ȸ�� ����� �������� �ٽ� ���� ===
            //float3 localDir = gInitInst[i].Direction.xyz;
            //float3 worldDir = RotateByQuat(localDir, vSocketRot);
            //pp.Direction.xyz = normalize(worldDir);


            float3 vCurPivot = Center + Pivot;

            // range�� �ٽ� ����ؼ� ���Ӱ� �ٽ� ���� ����..
                        
            if (isCircleRange == 0)
            {
                pos = RandomInRange(i * 3, Center.xyz, Range.xyz).xyz;
            }
            else
            {
                pos = RandomOnCircle(i * 3, Center.xyz, Range.x * 0.5f, Range.z).xyz;
            }
            

            if (ParticleType == 0)
            {
                pp.Direction = float4(normalize(pos.xyz - vCurPivot.xyz), 0.f);
            }
            else if (ParticleType == 1)
            {
                pp.Direction = float4(normalize(Center.xyz - vCurPivot.xyz), 0.f);
            }
            else if (ParticleType == 2) // random
            {
                pp.Direction = float4(
                    Random(i * 3, -1.f, 1.f),
                    Random(i * 3 + 1, -1.f, 1.f),
                    Random(i * 3 + 2, -1.f, 1.f),
                    0.f
                );
            }
             else if (ParticleType == 3) // to center
             {
                 pp.Direction = float4(normalize(vCurPivot.xyz - pos.xyz), 0.f);
             }
            
            pp.vInitOffset = float3(
			    pos.x - Center.x,
			    pos.y - Center.y,
			    pos.z - Center.z
		    );
            
            //float3 localDir = gInitInst[i].Direction.xyz;
            float3 worldDir = RotateByQuat(pp.Direction.xyz, vSocketRot);
            pp.Direction.xyz = normalize(worldDir);
            pp.VelocityDir = float4(0.f, 0.f, 0.f, 0.f);
        }
        pp.RotationAngle += pp.RotationSpeed;

        if (pp.RotationAngle > 6.28318530718f)
            pp.RotationAngle -= 6.28318530718f;
        
        uint iTileIdx = (uint) pp.fTileIdx;
        float fTileSizeX = 1.0f / float(vTileCnt.x);
        float fTileSizeY = 1.0f / float(vTileCnt.y);
        pp.vTileOffset = float2((iTileIdx % uint(vTileCnt.x)) * fTileSizeX, (iTileIdx / uint(vTileCnt.x)) * fTileSizeY);

        pp.Translation = float4(pos, 1.0f);

        gInst[i] = pp;
        return;
    }
}
