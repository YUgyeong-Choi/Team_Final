

/********************************************************/

/* [ ParticleInitData ] */
/* [ InstanceBuffer ] */
struct ParticleInst
{
    float4 Right;
    float4 Up;
    float4 Look;
    float4 Translation;
    float2 LifeTime; // x=max, y=acc
    float2 _pad;
};

/* [ PARTICLEDESC ] */
struct ParticleParam
{
    float4 Direction; // normalized dir (w=unused)

    float Speed;
    float RotationSpeed; // degrees/sec
    float OrbitSpeed; // degrees/sec
    float fAccel; // 가속도 (+면 가속, -면 감속)

    float fMaxSpeed; // 최대 속도 (옵션)
    float fMinSpeed; // 최소 속도 (옵션, 감속 시 멈춤 방지)
    float2 _pad;
};



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
    const uint i = dispatchThreadId.x;
    if (i >= NumInstances)
        return;

    ParticleInst inst = gInst[i];
    ParticleParam pp = gParam[i];

    const bool TOOL = (IsTool != 0);

    if (TOOL)
    {
        // --- 툴 모드: 절대시간으로 "초기값 기준" 재구성 ---
        float t = TrackTime;
        if (IsLoop != 0)
            t = fmod(t, inst.LifeTime.x);

        const ParticleInst init = gInitInst[i]; // 항상 초기값 기준
        float3 pos = init.Translation.xyz;

        // 이동: 초기위치 + (방향 * 속도 * t)
        //float3 dir = normalize(pp.Direction.xyz);
        //pos += dir * pp.Speed * t;

        // 이동: 초기위치 + v0*t + 0.5*a*t^2 (가속 추가 버전)
        float3 dir = normalize(pp.Direction.xyz);
        float v0 = pp.Speed;
        float accel = pp.fAccel;
        pos += dir * (v0 * t + 0.5f * accel * t * t);

        // 중력: 절대시간 적용 (s = 1/2 g t^2)
        if (UseGravity != 0)
            pos.y -= 0.5f * Gravity * t * t;

        // 자전: 초기 basis에 "절대 각도" 적용
        if (UseSpin != 0)
        {
            float3 axis = normalize(RotationAxis);
            float angle = radians(pp.RotationSpeed) * t;
            inst.Right.xyz = rotateAroundAxis(init.Right.xyz, axis, angle);
            inst.Up.xyz = rotateAroundAxis(init.Up.xyz, axis, angle);
            inst.Look.xyz = rotateAroundAxis(init.Look.xyz, axis, angle);
        }
        else
        {
            inst.Right = init.Right;
            inst.Up = init.Up;
            inst.Look = init.Look;
        }

        // 공전: (pos - Center)를 "절대 각도"만큼 회전
        if (UseOrbit != 0)
        {
            float3 axis = normalize(OrbitAxis);
            float angle = radians(pp.OrbitSpeed) * t;
            float3 offset = pos - Center;
            offset = rotateAroundAxis(offset, axis, angle);
            pos = Center + offset;
        }

        inst.Translation = float4(pos, 1.0f);
        inst.LifeTime.y = t; // 툴은 절대시간을 그대로 보관
        gInst[i] = inst;
        return;
    }
    else
    {
        // --- 런타임 모드: 증분(Δt) 업데이트 ---
        inst.LifeTime.y += DeltaTime;
        float t = inst.LifeTime.y;
        float tPrev = max(t - DeltaTime, 0.0f);

        float3 pos = inst.Translation.xyz;
        float3 dir = normalize(pp.Direction.xyz);

        //// 이동: Δs = v * Δt
        //pos += dir * pp.Speed * DeltaTime;
        
        // 가속 추가 버전
        // 속도 갱신: v = v + a*Δt (최소/최대 속도 제한 포함)
        pp.Speed = clamp(pp.Speed + pp.fAccel * DeltaTime, pp.fMinSpeed, pp.fMaxSpeed);

        // 이동: Δs = v * Δt
        pos += dir * pp.Speed * DeltaTime;

        // 중력: Δs = 1/2 g (t^2 - (t-Δt)^2)  = g*(t-Δt/2)*Δt
        if (UseGravity != 0)
        {
            float gravNow = 0.5f * Gravity * t * t;
            float gravPrev = 0.5f * Gravity * tPrev * tPrev;
            pos.y -= (gravNow - gravPrev);
        }

        // 자전: 직전 basis에 Δ각도만큼 회전
        if (UseSpin != 0)
        {
            float3 axis = normalize(RotationAxis);
            float angle = radians(pp.RotationSpeed) * DeltaTime;
            inst.Right.xyz = rotateAroundAxis(inst.Right.xyz, axis, angle);
            inst.Up.xyz = rotateAroundAxis(inst.Up.xyz, axis, angle);
            inst.Look.xyz = rotateAroundAxis(inst.Look.xyz, axis, angle);
        }

        // 공전: Δ각도만큼 오프셋 회전
        if (UseOrbit != 0)
        {
            float3 axis = normalize(OrbitAxis);
            float angle = radians(pp.OrbitSpeed) * DeltaTime;
            float3 offset = pos - Center;
            offset = rotateAroundAxis(offset, axis, angle);
            pos = Center + offset;
        }

        // 루프: 위치/라이프만 초기화(기하 기준은 유지하고 싶으면 아래처럼 Translation만 복원)
        if (IsLoop != 0 && inst.LifeTime.y >= inst.LifeTime.x)
        {
            inst.LifeTime.y = 0.0f;
            pos = gInitInst[i].Translation.xyz; // 위치만 리셋
            // 필요 시 basis도 초기화 원하면 아래 주석 해제
            // inst.Right = gInitInst[i].Right;
            // inst.Up    = gInitInst[i].Up;
            // inst.Look  = gInitInst[i].Look;
        }

        inst.Translation = float4(pos, 1.0f);
        gInst[i] = inst;
        return;
    }
}