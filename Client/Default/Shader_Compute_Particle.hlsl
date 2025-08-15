struct Particle
{
    float fSpeed;
    // etc...
    /* TODO: 외부에서 넘겨주는 구조체 정의 */
};

/* TODO: register()에 대해 알아오기.. */

// UAV로 접근할 수 있는 Structured Buffer 선언

/* RWStructuredBuffer<T> : Read/Write StructuredBuffer, 이름이 이런 이유는 그냥 버퍼를 구조체단위로 전달하기 때문인듯.. */
RWStructuredBuffer<Particle> particles : register(u0);  // 실제로 작성할 곳

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
void main( uint3 DTid : SV_DispatchThreadID )
{
    
}