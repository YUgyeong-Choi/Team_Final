
cbuffer CSAnimDesc : register(b0)
{
    float blendWeight;
    float blendFactor;
    uint boneCount;
    uint currentLevel;
    
    uint isMasked;
    float3 padding;
    
    matrix preTransformMatrix;
};




StructuredBuffer<matrix> g_LocalBoneMatrices_A : register(t0);
StructuredBuffer<matrix> g_LocalBoneMatrices_B : register(t1);
StructuredBuffer<int> g_Parents : register(t2);
StructuredBuffer<float> g_BoneMask : register(t3);
StructuredBuffer<int> g_BoneLevels : register(t4);

RWStructuredBuffer<matrix> g_FinalBoneMatrices : register(u0);

[numthreads(64, 1, 1)]
void BoneAnimationCS(uint3 id : SV_DispatchThreadID)
{
    uint boneIdx = id.x; // 계산하는 인덱스
    if (boneIdx >= boneCount) 
        return;

   if (g_BoneLevels[boneIdx] != currentLevel) 
       return;

    matrix localMat = g_LocalBoneMatrices_A[boneIdx];

    int parentIdx = g_Parents[boneIdx];
    matrix finalMat;

    if (parentIdx >= 0)
    {
        finalMat = mul(localMat, g_FinalBoneMatrices[parentIdx]);
    }
    else
    {
        finalMat = mul(localMat, preTransformMatrix);
    }

    g_FinalBoneMatrices[boneIdx] =finalMat;
}
