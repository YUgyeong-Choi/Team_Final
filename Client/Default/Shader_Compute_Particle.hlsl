struct Particle
{
    float fSpeed;
    // etc...
    /* TODO: �ܺο��� �Ѱ��ִ� ����ü ���� */
};

/* TODO: register()�� ���� �˾ƿ���.. */

// UAV�� ������ �� �ִ� Structured Buffer ����

/* RWStructuredBuffer<T> : Read/Write StructuredBuffer, �̸��� �̷� ������ �׳� ���۸� ����ü������ �����ϱ� �����ε�.. */
RWStructuredBuffer<Particle> particles : register(u0);  // ������ �ۼ��� ��

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
void main( uint3 DTid : SV_DispatchThreadID )
{
    
}