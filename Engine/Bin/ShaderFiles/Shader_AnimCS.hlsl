
cbuffer CSAnimDesc : register(b0)
{
    float blendWeight;
    float blendFactor;
    uint boneCount;
    uint currentLevel;

    uint isMasked;
    float3 padding;

    float4x4 preTransformMatrix;
};

StructuredBuffer<float4x4> g_LocalBoneMatrices_A : register(t0);
StructuredBuffer<float4x4> g_LocalBoneMatrices_B : register(t1);
StructuredBuffer<int> g_Parents : register(t2);
StructuredBuffer<float> g_BoneMask : register(t3);
StructuredBuffer<int> g_BoneLevels : register(t4);

RWStructuredBuffer<float4x4> g_FinalBoneMatrices : register(u0);

[numthreads(64, 1, 1)]
void BoneAnimationCS(uint3 id : SV_DispatchThreadID)
{
    uint boneIdx = id.x;

    if (boneIdx >= boneCount)
        return;

    if (g_BoneLevels[boneIdx] != currentLevel)
        return;

    float4x4 localMat = g_LocalBoneMatrices_A[boneIdx];
    int parentIdx = g_Parents[boneIdx];
    float4x4 finalMat;

    if (parentIdx >= 0)
    {
        // �� �켱: parent * local (CPU�� ������ ����)
        finalMat = mul(g_FinalBoneMatrices[parentIdx], localMat);
    }
    else
    {
        // �� �켱: preTransform * local
        finalMat = mul(preTransformMatrix,localMat);
    }

    g_FinalBoneMatrices[boneIdx] = (finalMat);
}