

/********************************************************/

/* [ ��ƼŬ ���� �Ӽ� ���պ� ] */
struct ParticleParam
{
    float4      Right;
    float4      Up;
    float4      Look;
    float4      Translation;

    float4      Direction;  // normalized dir (w=unused)

    float2      LifeTime;   // x=max, y=acc
    float       Speed;
    float       RotationSpeed; // degrees/sec

    float       OrbitSpeed; // degrees/sec
    float       fAccel;     // ���ӵ� (+�� ����, -�� ����)
    float       fMaxSpeed;  // �ִ� �ӵ� (�ɼ�)
    float       fMinSpeed;  // �ּ� �ӵ� (�ɼ�, ���� �� ���� ����)
};





/* [ ParticleInitData ] */
/* [ InstanceBuffer ] */
//struct ParticleInst
//{
//    float4 Right;
//    float4 Up;
//    float4 Look;
//    float4 Translation;
//    float2 LifeTime; // x=max, y=acc
//    float2 _pad;
//};

/* [ PARTICLEDESC ] */
//struct ParticleParam
//{
//    float4 Direction; // normalized dir (w=unused)

//    float Speed;
//    float RotationSpeed; // degrees/sec
//    float OrbitSpeed; // degrees/sec
//    float fAccel; // ���ӵ� (+�� ����, -�� ����)

//    float fMaxSpeed; // �ִ� �ӵ� (�ɼ�)
//    float fMinSpeed; // �ּ� �ӵ� (�ɼ�, ���� �� ���� ����)
//    float2 _pad;
//};



/* [ Constant Buffer ] */
cbuffer ParticleCB : register(b0)
{
    float DeltaTime;
    float TrackTime;
    uint ParticleType;
    uint NumInstances;

    uint IsTool;
    uint IsLoop;
    uint UseGravity;
    uint UseSpin;

    uint UseOrbit;
    float Gravity;
    float2 _pad0;

    float3 Pivot;
    float _pad1;

    float3 Center;
    float _pad2;

    float3 OrbitAxis;
    float _pad3;

    float3 RotationAxis;
    float _pad4;
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



[numthreads(128, 1, 1)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint i = dispatchThreadId.x;
    if (i >= NumInstances)
        return;

    ParticleParam pp = gInst[i];

    const bool TOOL = (IsTool != 0);

    if (TOOL)
    {
        // --- �� ���: ����ð����� "�ʱⰪ ����" �籸�� ---
        float t = TrackTime;
        if (IsLoop != 0)
            t = fmod(t, pp.LifeTime.x);

        const ParticleParam init = gInitInst[i]; // �׻� �ʱⰪ ����
        float3 pos = init.Translation.xyz;

        // �̵�: �ʱ���ġ + (���� * �ӵ� * t)
        //float3 dir = normalize(pp.Direction.xyz);
        //pos += dir * pp.Speed * t;
        
        /*****/
        //// �̵�: �ʱ���ġ + v0*t + 0.5*a*t^2 (���� �߰� ����)
        //float3 dir = normalize(pp.Direction.xyz);
        //float v0 = init.Speed;
        //float accel = pp.fAccel;
        
        //// ���� �ӵ� (clamp �ݿ�)
        //float curSpeed = clamp(v0 + accel * t, pp.fMinSpeed, pp.fMaxSpeed);
        //pos += dir * (curSpeed * t);
        /*****/
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
            pos.y -= 0.5f * Gravity * t * t;

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
        gInst[i] = pp;
        return;
    }
    else
    {
        // --- ��Ÿ�� ���: ����(��t) ������Ʈ ---
        pp.LifeTime.y += DeltaTime;
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
            float gravNow = 0.5f * Gravity * t * t;
            float gravPrev = 0.5f * Gravity * tPrev * tPrev;
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

        // ����: ��ġ/�������� �ʱ�ȭ(���� ������ �����ϰ� ������ �Ʒ�ó�� Translation�� ����)
        if (IsLoop != 0 && pp.LifeTime.y >= pp.LifeTime.x)
        {
            pp.LifeTime.y = 0.0f;
            pos = gInitInst[i].Translation.xyz; // ��ġ�� ����
            // �ʿ� �� basis�� �ʱ�ȭ ���ϸ� �Ʒ� �ּ� ����
            // pp.Right = gInitInst[i].Right;
            // pp.Up    = gInitInst[i].Up;
            // pp.Look  = gInitInst[i].Look;
        }

        pp.Translation = float4(pos, 1.0f);
        gInst[i] = pp;
        return;
    }
}