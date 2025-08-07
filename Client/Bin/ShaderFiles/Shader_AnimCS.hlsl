cbuffer CSAnimDesc : register(b0)
{
    float blendWeight;
    float blendFactor;
    uint boneCount;
    uint isMasked; 
}

// �Է� StructuredBuffer (C++���� SetShaderResources�� ����)
// t0: ���� �ִϸ��̼��� ���� ��� �Ǵ� ��ü �ִϸ��̼��� ���� ���
StructuredBuffer<matrix> g_LocalBoneMatrices_A : register(t0);
// t1: ��ü �ִϸ��̼��� ���� ���
StructuredBuffer<matrix> g_LocalBoneMatrices_B : register(t1);

// t2: �� ������ �θ� �ε���
StructuredBuffer<int> g_Parents : register(t2);

// t3: ����ŷ ����ġ �迭. 0.0�̸� ��ü, 1.0�̸� ��ü
StructuredBuffer<float> g_BoneMask : register(t3);

// ��� RWStructuredBuffer (C++���� SetUnorderedAccessViews�� ����)
// u0: ���� ���� ���� ��ȯ ���
RWStructuredBuffer<matrix> g_FinalBoneMatrices : register(u0);

// [numthreads(64, 1, 1)]�� ������ �׷�� 64���� �����带 ����
// X,Y,Z CPU���� ���ؼ� �Ѱ��ָ� �ȴ�.
// 64���� ���� ������ ���������� ����̶� ����ȭ�� �׷� ���� �޸𸮸� ����� �ʿ䰡 ����
[numthreads(64, 1, 1)]
void BoneAnimationCS(uint3 id : SV_DispatchThreadID)
{
    // ���� �������� �ε����� �� ���� ������ �ʰ��ϸ� �۾��� �ߴ��մϴ�.
    if (id.x >= boneCount)
    {
        return;
    }
    
    uint boneIndex = id.x;

    // ���� ����� �����մϴ�.
    matrix LocalMatrix;
    if (isMasked)
    {
        matrix lowerMatrix = g_LocalBoneMatrices_A[boneIndex];
        matrix upperMatrix = g_LocalBoneMatrices_B[boneIndex];

        float finalWeight = g_BoneMask[boneIndex] * blendFactor;
        
        LocalMatrix = lerp(lowerMatrix, upperMatrix, finalWeight);
    }
    else
    {
        // ���� �ִϸ��̼��� ���
        LocalMatrix = g_LocalBoneMatrices_A[boneIndex];
    }
    
    // �θ�-�ڽ� ���ؼ� �Ĺ��ε� ��Ʈ���� ���ϱ�
    // �� �۾��� GPU���� ���ķ� �̷�����Ƿ�, �θ� ������ ����� �̹�
    // ���Ǿ����� �����ϰ� �����մϴ�. (C++���� ���� ���� ������
    // Dispatch�� ȣ���ϴ� ��� ����������, ���� ȣ�� �� ���� ������ �߻��� �� �ֽ��ϴ�.)
    matrix CombinedMatrix = LocalMatrix;
    int iParentIndex = g_Parents[boneIndex];

    while (iParentIndex != -1)
    {
        // �θ��� ���� ����� �о�ͼ� ���մϴ�.
        CombinedMatrix = mul(CombinedMatrix, g_FinalBoneMatrices[iParentIndex]);
        iParentIndex = g_Parents[iParentIndex];
    }

    g_FinalBoneMatrices[boneIndex] = CombinedMatrix;
}