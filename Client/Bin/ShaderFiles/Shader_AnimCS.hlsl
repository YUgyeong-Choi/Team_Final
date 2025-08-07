cbuffer CSAnimDesc : register(b0)
{
    float blendWeight;
    float blendFactor;
    uint boneCount;
    uint isMasked; 
}

// 입력 StructuredBuffer (C++에서 SetShaderResources로 전달)
// t0: 단일 애니메이션의 로컬 행렬 또는 하체 애니메이션의 로컬 행렬
StructuredBuffer<matrix> g_LocalBoneMatrices_A : register(t0);
// t1: 상체 애니메이션의 로컬 행렬
StructuredBuffer<matrix> g_LocalBoneMatrices_B : register(t1);

// t2: 각 뼈대의 부모 인덱스
StructuredBuffer<int> g_Parents : register(t2);

// t3: 마스킹 가중치 배열. 0.0이면 하체, 1.0이면 상체
StructuredBuffer<float> g_BoneMask : register(t3);

// 출력 RWStructuredBuffer (C++에서 SetUnorderedAccessViews로 전달)
// u0: 최종 계산된 결합 변환 행렬
RWStructuredBuffer<matrix> g_FinalBoneMatrices : register(u0);

// [numthreads(64, 1, 1)]는 스레드 그룹당 64개의 스레드를 실행
// X,Y,Z CPU에서 정해서 넘겨주면 된다.
// 64개로 쓰는 이유는 독립적으로 계산이라 동기화나 그룹 공유 메모리를 사용할 필요가 없음
[numthreads(64, 1, 1)]
void BoneAnimationCS(uint3 id : SV_DispatchThreadID)
{
    // 현재 스레드의 인덱스가 총 뼈대 개수를 초과하면 작업을 중단합니다.
    if (id.x >= boneCount)
    {
        return;
    }
    
    uint boneIndex = id.x;

    // 로컬 행렬을 블렌딩합니다.
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
        // 단일 애니메이션인 경우
        LocalMatrix = g_LocalBoneMatrices_A[boneIndex];
    }
    
    // 부모-자식 곱해서 컴바인드 매트릭스 구하기
    // 이 작업은 GPU에서 병렬로 이루어지므로, 부모 뼈대의 행렬이 이미
    // 계산되었음을 가정하고 진행합니다. (C++에서 뼈대 계층 순으로
    // Dispatch를 호출하는 경우 안전하지만, 단일 호출 시 쓰기 경쟁이 발생할 수 있습니다.)
    matrix CombinedMatrix = LocalMatrix;
    int iParentIndex = g_Parents[boneIndex];

    while (iParentIndex != -1)
    {
        // 부모의 최종 행렬을 읽어와서 곱합니다.
        CombinedMatrix = mul(CombinedMatrix, g_FinalBoneMatrices[iParentIndex]);
        iParentIndex = g_Parents[iParentIndex];
    }

    g_FinalBoneMatrices[boneIndex] = CombinedMatrix;
}