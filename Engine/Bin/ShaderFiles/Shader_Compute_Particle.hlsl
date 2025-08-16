struct Particle
{
    float fSpeed;
    // etc...
    /* TODO: �ܺο��� �Ѱ��ִ� ����ü ���� */
};

/* TODO: register()�� ���� �˾ƿ���.. */

// UAV�� ������ �� �ִ� Structured Buffer ����

/* RWStructuredBuffer<T> : Read/Write StructuredBuffer, �̸��� �̷� ������ �׳� ���۸� ����ü������ �����ϱ� �����ε�.. */
RWStructuredBuffer<Particle> particles : register(u0); // ������ �ۼ��� ��

/* StructuredBuffer<T> : Read only. */
StructuredBuffer<Particle> InitParticles : register(t0); // ���������(�ǹ��̳� �׷���?)

// �̰� �������� �Ѱ��ִ� ����
cbuffer MovementBuffer : register(b0) // �޾ƿ� �� ����� ���� ���� �����ε�?
{
    uint iNumInstance;
    uint iState;
    float2 pad_1; // ũ�⸦ ���߱� ���� ���� ���� 16����Ʈ�� �������
    
    float4 vPivot;
    // etc... 
}

// numthreads�� xyz �迭�� �� cpu���� ������ ���� �����ؾ��� !! 
[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
}

/********************************************************/

//// === ���� ���� ===
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
//    float DeltaTime; // dt (tool�̸� ����)
//    float TrackTime; // tool ����ð�(��) = curTrackPos/60.f
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
//StructuredBuffer<ParticleParam> gParam : register(t0); // t�� SRV, u�� UAV, b�� Constant Buffer
//RWStructuredBuffer<ParticleInst> gInst : register(u0); // ���� ���ڴ� ���°�� ���ε� �ߴ����� ����

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
//    ParticleParam pp = gParam[i]; // ��ƼŬ �ϳ��� ���� ���� �ϳ��� �����尡 �����Ǵ� ��

//    // �ð� ���
//    float t;
//    if (IsTool != 0)
//    { // ��: ����ð����� ����ġ
//        t = TrackTime;
//        if (IsLoop != 0)
//            t = fmod(t, inst.LifeTime.x);
//        inst.LifeTime.y = t;
//    }
//    else
//    { // ��Ÿ��: ����
//        inst.LifeTime.y += DeltaTime;
//        t = inst.LifeTime.y;
//    }

//    // �̵�
//    float3 pos = inst.Translation.xyz;
//    if (ParticleType == 0)
//    { // SPREAD: pivot -> start����
//        float3 dir = normalize(inst.Translation.xyz - Pivot);
        
//        pos += dir * pp.Speed * (IsTool != 0 ? t : DeltaTime);
//    }
//    else if (ParticleType == 1)
//    { // DIRECTIONAL: center - pivot ����(����)
//        float3 dir = normalize(Center - Pivot);
//        pos += dir * pp.Speed * (IsTool != 0 ? t : DeltaTime);
//    }

//    // �߷�
//    if (UseGravity != 0)
//    {
//        float dt = (IsTool != 0 ? t : DeltaTime);
//        pos.y -= 0.5f * Gravity * dt * dt;
//    }

//    // ����(���� basis ȸ��)
//    if (UseSpin != 0)
//    {
//        float angle = radians(pp.RotationSpeed) * (IsTool != 0 ? t : DeltaTime);
//        inst.Right.xyz = rotateAroundAxis(inst.Right.xyz, RotationAxis, angle);
//        inst.Up.xyz = rotateAroundAxis(inst.Up.xyz, RotationAxis, angle);
//        inst.Look.xyz = rotateAroundAxis(inst.Look.xyz, RotationAxis, angle);
//    }

//    // ����(���� ���� ������ ȸ��)
//    if (UseOrbit != 0)
//    {
//        float angle = radians(pp.OrbitSpeed) * (IsTool != 0 ? t : DeltaTime);
//        float3 offset = pos - Center;
//        offset = rotateAroundAxis(offset, OrbitAxis, angle);
//        pos = Center + offset;
//    }

//    // ���� ó��
//    if (IsTool == 0 && IsLoop != 0 && inst.LifeTime.y >= inst.LifeTime.x)
//    {
//        inst.LifeTime.y = 0.0f;
//        // ���� ��ġ�� ����(�ʱ� Translation�� �ʱ�ȭ �� ��ϵǾ� �־�� ��)
//        // ���⼭�� ������ pivot/center ��ó�� �ǵ����� �� ���,
//        // �ʱⰪ�� ���� ���۷� ��� ���ų� LifeTime.y=0 �� �ΰ� VS/PS���� ���̵� �ƿ�/�� �ص� �˴ϴ�.
//    }

//    inst.Translation = float4(pos, 1.0f);
//    gInst[i] = inst;
//}

