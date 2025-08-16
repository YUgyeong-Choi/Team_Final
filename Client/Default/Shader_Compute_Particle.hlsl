struct Particle
{
    float fSpeed;
    // etc...
    /* TODO: 외부에서 넘겨주는 구조체 정의 */
};

/* TODO: register()에 대해 알아오기.. */

// UAV로 접근할 수 있는 Structured Buffer 선언

/* RWStructuredBuffer<T> : Read/Write StructuredBuffer, 이름이 이런 이유는 그냥 버퍼를 구조체단위로 전달하기 때문인듯.. */
RWStructuredBuffer<Particle> particles : register(u0); // 실제로 작성할 곳

/* StructuredBuffer<T> : Read only. */
StructuredBuffer<Particle> InitParticles : register(t0); // 원본참고용(피벗이나 그런거?)

// 이게 매프레임 넘겨주는 버퍼
cbuffer MovementBuffer : register(b0) // 받아온 걸 상수로 쓰기 위한 버퍼인듯?
{
    uint iNumInstance;
    uint iState;
    float2 pad_1; // 크기를 맞추기 위한 더미 변수 16바이트를 맞춰야함
    
    float4 vPivot;
    // etc... 
}

// numthreads의 xyz 배열은 꼭 cpu에서 정의한 수와 동일해야함 !! 
[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
}

/********************************************************/

//// === 공용 구조 ===
//struct ParticleInst
//{
//    float4 Right;
//    float4 Up;
//    float4 Look;
//    float4 Translation;
//    float2 LifeTime; // x=max, y=acc
//};

//struct ParticleParam
//{
//    float4 Direction; // normalized dir (w=unused)
//    float Speed;
//    float RotationSpeed; // degrees/sec
//    float OrbitSpeed; // degrees/sec
//    float3 _pad;
//};

//cbuffer ParticleCB : register(b0)
//{
//    float DeltaTime; // dt (tool이면 무시)
//    float TrackTime; // tool 절대시간(초) = curTrackPos/60.f
//    uint ParticleType; // 0:SPREAD, 1:DIRECTIONAL
//    uint IsTool;

//    uint IsLoop;
//    uint UseGravity;
//    uint UseSpin;
//    uint UseOrbit;

//    float Gravity; // e.g. 9.8
//    float3 Pivot; // vPivot

//    float3 Center; // vCenter
//    float _pad0;

//    float3 OrbitAxis; // normalized
//    float _pad1;

//    float3 RotationAxis; // normalized
//    float _pad2;
//};

//// SRV/UAV
//StructuredBuffer<ParticleParam> gParam : register(t0); // t는 SRV, u는 UAV, b는 Constant Buffer
//RWStructuredBuffer<ParticleInst> gInst : register(u0); // 뒤의 숫자는 몇번째로 바인딩 했는지를 뜻함

////static const float PI = 3.14159265f;

//float3 rotateAroundAxis(float3 v, float3 axis, float angleRad)
//{
//    float s = sin(angleRad), c = cos(angleRad);
//    return v * c + cross(axis, v) * s + axis * dot(axis, v) * (1.0f - c);
//}

//[numthreads(128, 1, 1)]
//void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
//{
//    uint i = dispatchThreadId.x;
//    ParticleInst inst = gInst[i];
//    ParticleParam pp = gParam[i]; // 파티클 하나의 연산 마다 하나의 쓰레드가 배정되는 식

//    // 시간 계산
//    float t;
//    if (IsTool != 0)
//    { // 툴: 절대시간으로 정위치
//        t = TrackTime;
//        if (IsLoop != 0)
//            t = fmod(t, inst.LifeTime.x);
//        inst.LifeTime.y = t;
//    }
//    else
//    { // 런타임: 누적
//        inst.LifeTime.y += DeltaTime;
//        t = inst.LifeTime.y;
//    }

//    // 이동
//    float3 pos = inst.Translation.xyz;
//    if (ParticleType == 0)
//    { // SPREAD: pivot -> start방향
//        float3 dir = normalize(inst.Translation.xyz - Pivot);
        
//        pos += dir * pp.Speed * (IsTool != 0 ? t : DeltaTime);
//    }
//    else if (ParticleType == 1)
//    { // DIRECTIONAL: center - pivot 방향(고정)
//        float3 dir = normalize(Center - Pivot);
//        pos += dir * pp.Speed * (IsTool != 0 ? t : DeltaTime);
//    }

//    // 중력
//    if (UseGravity != 0)
//    {
//        float dt = (IsTool != 0 ? t : DeltaTime);
//        pos.y -= 0.5f * Gravity * dt * dt;
//    }

//    // 자전(로컬 basis 회전)
//    if (UseSpin != 0)
//    {
//        float angle = radians(pp.RotationSpeed) * (IsTool != 0 ? t : DeltaTime);
//        inst.Right.xyz = rotateAroundAxis(inst.Right.xyz, RotationAxis, angle);
//        inst.Up.xyz = rotateAroundAxis(inst.Up.xyz, RotationAxis, angle);
//        inst.Look.xyz = rotateAroundAxis(inst.Look.xyz, RotationAxis, angle);
//    }

//    // 공전(센터 기준 오프셋 회전)
//    if (UseOrbit != 0)
//    {
//        float angle = radians(pp.OrbitSpeed) * (IsTool != 0 ? t : DeltaTime);
//        float3 offset = pos - Center;
//        offset = rotateAroundAxis(offset, OrbitAxis, angle);
//        pos = Center + offset;
//    }

//    // 루프 처리
//    if (IsTool == 0 && IsLoop != 0 && inst.LifeTime.y >= inst.LifeTime.x)
//    {
//        inst.LifeTime.y = 0.0f;
//        // 시작 위치로 리셋(초기 Translation은 초기화 때 기록되어 있어야 함)
//        // 여기서는 간단히 pivot/center 근처로 되돌리는 식 대신,
//        // 초기값을 별도 버퍼로 들고 오거나 LifeTime.y=0 만 두고 VS/PS에서 페이드 아웃/인 해도 됩니다.
//    }

//    inst.Translation = float4(pos, 1.0f);
//    gInst[i] = inst;
//}

