

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
StructuredBuffer<ParticleParam> gParam : register(t0); // t는 SRV, u는 UAV, b는 Constant Buffer
StructuredBuffer<ParticleInst> gInitInst : register(t1); // 초기 상태
RWStructuredBuffer<ParticleInst> gInst : register(u0); // 뒤의 숫자는 몇번째로 바인딩 했는지를 뜻함

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

    // === 시간 계산 ===
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

    // === 기본 위치 이동 ===
    float3 pos = inst.Translation.xyz;

    if (ParticleType == 0)
    {
        // SPREAD: pivot -> start방향
        float3 dir = normalize(inst.Translation.xyz - Pivot);
        pos += dir * pp.Speed * (IsTool != 0 ? t : DeltaTime);
    }
    else if (ParticleType == 1)
    {
        // DIRECTIONAL: center - pivot 방향(고정)
        float3 dir = normalize(Center - Pivot);
        pos += dir * pp.Speed * (IsTool != 0 ? t : DeltaTime);
    }

    // === 공통 처리 ===

    // 중력
    if (UseGravity != 0)
    {
        float dt = (IsTool != 0 ? t : DeltaTime);
        pos.y -= 0.5f * Gravity * dt * dt;
    }

    // 자전(로컬 basis 회전)
    if (UseSpin != 0)
    {
        float angle = radians(pp.RotationSpeed) * (IsTool != 0 ? t : DeltaTime);
        inst.Right.xyz = rotateAroundAxis(inst.Right.xyz, RotationAxis, angle);
        inst.Up.xyz = rotateAroundAxis(inst.Up.xyz, RotationAxis, angle);
        inst.Look.xyz = rotateAroundAxis(inst.Look.xyz, RotationAxis, angle);
    }

    // 공전(센터 기준 오프셋 회전)
    if (UseOrbit != 0)
    {
        float angle = radians(pp.OrbitSpeed) * (IsTool != 0 ? t : DeltaTime);
        float3 offset = pos - Center;
        offset = rotateAroundAxis(offset, OrbitAxis, angle);
        pos = Center + offset;
    }

    // === 루프 처리 ===
    if (IsTool == 0 && IsLoop != 0 && inst.LifeTime.y >= inst.LifeTime.x)
    {
        inst.LifeTime.y = 0.0f;
        inst = gInitInst[i]; // ★ 초기값 복원
    }

    inst.Translation = float4(pos, 1.0f);
    gInst[i] = inst;
}
