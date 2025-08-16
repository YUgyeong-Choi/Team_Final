

/********************************************************/

/* [ InstanceBuffer ] */
struct ParticleInst
{
    float4 Right;
    float4 Up;
    float4 Look;
    float4 Translation;
    float2 LifeTime; // x=max, y=acc
};

/* [ PARTICLEDESC ] */
struct ParticleParam
{
    float4 Direction; // normalized dir (w=unused)
    float Speed;
    float RotationSpeed; // degrees/sec
    float OrbitSpeed; // degrees/sec
    float3 _pad;
};

/* [ ParticleInitData ] */


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

// SRV/UAV
StructuredBuffer<ParticleParam> gParam : register(t0); // t�� SRV, u�� UAV, b�� Constant Buffer
StructuredBuffer<ParticleInst> gInitInst : register(t1); // �ʱ� ����
RWStructuredBuffer<ParticleInst> gInst : register(u0); // ���� ���ڴ� ���°�� ���ε� �ߴ����� ����

//static const float PI = 3.14159265f;

float3 rotateAroundAxis(float3 v, float3 axis, float angleRad)
{
    float s = sin(angleRad), c = cos(angleRad);
    return v * c + cross(axis, v) * s + axis * dot(axis, v) * (1.0f - c);
}
[numthreads(128, 1, 1)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint i = dispatchThreadId.x;
    ParticleInst inst = gInst[i];
    ParticleParam pp = gParam[i];

    // === �ð� ��� ===
    float t;
    if (IsTool != 0)
    {
        t = TrackTime;
        if (IsLoop != 0)
            t = fmod(t, inst.LifeTime.x);
        inst.LifeTime.y = t;
    }
    else
    {
        inst.LifeTime.y += DeltaTime;
        t = inst.LifeTime.y;
    }

    // === �⺻ ��ġ �̵� ===
    float3 pos = inst.Translation.xyz;

    if (ParticleType == 0)
    {
        // SPREAD: pivot -> start����
        float3 dir = normalize(inst.Translation.xyz - Pivot);
        pos += dir * pp.Speed * (IsTool != 0 ? t : DeltaTime);
    }
    else if (ParticleType == 1)
    {
        // DIRECTIONAL: center - pivot ����(����)
        float3 dir = normalize(Center - Pivot);
        pos += dir * pp.Speed * (IsTool != 0 ? t : DeltaTime);
    }

    // === ���� ó�� ===

    // �߷�
    if (UseGravity != 0)
    {
        float dt = (IsTool != 0 ? t : DeltaTime);
        pos.y -= 0.5f * Gravity * dt * dt;
    }

    // ����(���� basis ȸ��)
    if (UseSpin != 0)
    {
        float angle = radians(pp.RotationSpeed) * (IsTool != 0 ? t : DeltaTime);
        inst.Right.xyz = rotateAroundAxis(inst.Right.xyz, RotationAxis, angle);
        inst.Up.xyz = rotateAroundAxis(inst.Up.xyz, RotationAxis, angle);
        inst.Look.xyz = rotateAroundAxis(inst.Look.xyz, RotationAxis, angle);
    }

    // ����(���� ���� ������ ȸ��)
    if (UseOrbit != 0)
    {
        float angle = radians(pp.OrbitSpeed) * (IsTool != 0 ? t : DeltaTime);
        float3 offset = pos - Center;
        offset = rotateAroundAxis(offset, OrbitAxis, angle);
        pos = Center + offset;
    }

    // === ���� ó�� ===
    if (IsTool == 0 && IsLoop != 0 && inst.LifeTime.y >= inst.LifeTime.x)
    {
        inst.LifeTime.y = 0.0f;
        inst = gInitInst[i]; // �� �ʱⰪ ����
    }

    inst.Translation = float4(pos, 1.0f);
    gInst[i] = inst;
}
