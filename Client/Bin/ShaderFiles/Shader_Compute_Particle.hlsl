
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
    float       RotationSpeed; // degrees/sec

    float       OrbitSpeed; // degrees/sec
    float       fAccel;     // ���ӵ� (+�� ����, -�� ����)
    float       fMaxSpeed;  // �ִ� �ӵ� (�ɼ�)
    float       fMinSpeed;  // �ּ� �ӵ� (�ɼ�, ���� �� ���� ����)
    
    float       fTileIdx;
    float2      vTileOffset;
    float       _pad0;
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
    float _pad2;

    float3  Pivot; // vPivot
    float _pad3;
    
    float3 Center; // vCenter
    float _pad4;
    
    float3 OrbitAxis; // normalized
    float _pad5;
    
    float3 RotationAxis; // normalized
    float _pad6;

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
        pp.fTileIdx = TrackTime / fTileTickPerSec;
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
        if (UseSpin != 0)
        {
            float3 axis = normalize(RotationAxis);
            float angle = radians(pp.RotationSpeed) * t;
            pp.Right.xyz = rotateAroundAxis(init.Right.xyz, axis, angle);
            pp.Up.xyz = rotateAroundAxis(init.Up.xyz, axis, angle);
            pp.Look.xyz = rotateAroundAxis(init.Look.xyz, axis, angle);
        }
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
            pp.Translation = float4(mul(float4(pp.Translation.xyz, 1.f), g_CombinedMatrix).xyz, 1.f);

            float3 localDir = gInitInst[i].Direction.xyz;
            float3 worldDir = RotateByQuat(localDir, vSocketRot);
            pp.Direction.xyz = normalize(worldDir);
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
            float3 axis = normalize(RotationAxis);
            float angle = radians(pp.RotationSpeed) * DeltaTime;
            pp.Right.xyz = rotateAroundAxis(pp.Right.xyz, axis, angle);
            pp.Up.xyz = rotateAroundAxis(pp.Up.xyz, axis, angle);
            pp.Look.xyz = rotateAroundAxis(pp.Look.xyz, axis, angle);
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
        
        const bool firstLoop = (pp.bFirstLoopDiscard != 0);
        const bool lifeEnded = pp.LifeTime.y >= pp.LifeTime.x;
        const bool tileEnded = isTileLoop != 0 && (pp.fTileIdx >= vTileCnt.x * vTileCnt.y);
        
        //bool needReset = (pp.bFirstLoopDiscard != 0 && (lifeEnded != 0.0f)) ||
        //             (pp.bFirstLoopDiscard == 0 && (tileEnded || (isLoop != 0 && lifeEnded)));
        
        bool needReset = firstLoop ? lifeEnded
                           : ((isTileLoop != 0) ? tileEnded
                                                : ((isLoop != 0) && lifeEnded));

        if (needReset)
        {
            pp.LifeTime.y = 0.0f;
            float3 offset0 = gInitInst[i].vInitOffset;
            pos = Center + offset0; // << ������
            pp.Speed = gInitInst[i].Speed; // �ӵ� �ʱ�ȭ�� ����
            
                    // === �� ȸ�� ����� �������� �ٽ� ���� ===
            float3 localDir = gInitInst[i].Direction.xyz;
            float3 worldDir = RotateByQuat(localDir, vSocketRot);
            pp.Direction.xyz = normalize(worldDir);
            pp.bFirstLoopDiscard = false;
            pp.fTileIdx = 0.f;
        }
        
        
        
        
        //// ����: ��ġ/�������� �ʱ�ȭ(���� ������ �����ϰ� ������ �Ʒ�ó�� Translation�� ����)
        //if ((isLoop != 0 && pp.LifeTime.y >= pp.LifeTime.x)
        //    || (isTileLoop != 0 && pp.fTileIdx >= vTileCnt.x * vTileCnt.y)) // tileloop�� ���� or lifetimeloop�� ���� �� �� �ϳ��� �ɷ��� �ʱ�ȭ �ǵ��� ��
        //{
        //    pp.LifeTime.y = 0.0f;
        //    //float3 offset0 = float3(pp.Right.w, pp.Up.w, pp.Look.w);
        //    float3 offset0 = gInitInst[i].vInitOffset;
        //    pos = Center + offset0; // << ������
        //    pp.Speed = gInitInst[i].Speed; // �ӵ� �ʱ�ȭ�� ����
        //    
        //    // === �� ȸ�� ����� �������� �ٽ� ���� ===
        //    float3 localDir = gInitInst[i].Direction.xyz;
        //    float3 worldDir = RotateByQuat(localDir, vSocketRot);
        //    pp.Direction.xyz = normalize(worldDir);
        //    pp.bFirstLoopDiscard = false;
        //    pp.fTileIdx = 0.f;
        //}
        
        uint iTileIdx = (uint) pp.fTileIdx;
        float fTileSizeX = 1.0f / float(vTileCnt.x);
        float fTileSizeY = 1.0f / float(vTileCnt.y);
        pp.vTileOffset = float2((iTileIdx % uint(vTileCnt.x)) * fTileSizeX, (iTileIdx / uint(vTileCnt.x)) * fTileSizeY);

        pp.Translation = float4(pos, 1.0f);
        float3 velocity = pos - prevPos;
        pp.VelocityDir = float4(normalize(velocity), length(velocity));
        gInst[i] = pp;
        return;
    }
}
