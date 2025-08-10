
#include "Engine_Shader_Defines.hlsli"
#pragma pack_matrix(row_major)
 matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

matrix g_BoneMatrices[512];

//
//// 모든 뼈 컴바인드 매트릭스
//StructuredBuffer<matrix> g_FinalBoneMatrices : register(t0);
//
//// 메시별 로컬 뼈 인덱스
//StructuredBuffer<uint>     g_LocalToGlobal    : register(t1);
//
//// 지금 메시의 뼈 오프셋들
//StructuredBuffer<matrix> g_Offsets          : register(t2);
texture2D g_DiffuseTexture;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float2 vTexcoord : TEXCOORD0;
    uint4  vBlendIndices : BLENDINDEX;
    float4 vBlendWeights : BLENDWEIGHT;
};

struct VS_OUT
{      
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;    
    //
    float fWeightW = 1.f - (In.vBlendWeights.x + In.vBlendWeights.y + In.vBlendWeights.z);


    matrix BoneMatrix = g_BoneMatrices[In.vBlendIndices.x] * In.vBlendWeights.x +
        g_BoneMatrices[In.vBlendIndices.y] * In.vBlendWeights.y +
        g_BoneMatrices[In.vBlendIndices.z] * In.vBlendWeights.z +
        g_BoneMatrices[In.vBlendIndices.w] * fWeightW;

    vector vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);
    vector vNormal = mul(vector(In.vNormal, 0.f), BoneMatrix);


    //float4 w = In.vBlendWeights;
    //w.w = 1.0 - (w.x + w.y + w.z);

    //uint4 meshLocalIndices = In.vBlendIndices;  // 메시 내 로컬 본 인덱스

    //// CPU와 동일한 계산: 메시 로컬 인덱스 i에 대해
    //// offset[i] * globalBone[m_BoneIndices[i]]

    //// 각 메시 로컬 인덱스에 대해 해당하는 글로벌 인덱스 찾기
    //uint globalIndex0 = g_LocalToGlobal[meshLocalIndices.x];
    //uint globalIndex1 = g_LocalToGlobal[meshLocalIndices.y];
    //uint globalIndex2 = g_LocalToGlobal[meshLocalIndices.z];
    //uint globalIndex3 = g_LocalToGlobal[meshLocalIndices.w];

    //// CPU와 동일한 순서: offset * globalBone
    //matrix skinMatrix0 = mul(g_Offsets[meshLocalIndices.x], g_FinalBoneMatrices[globalIndex0]);
    //matrix skinMatrix1 = mul(g_Offsets[meshLocalIndices.y], g_FinalBoneMatrices[globalIndex1]);
    //matrix skinMatrix2 = mul(g_Offsets[meshLocalIndices.z], g_FinalBoneMatrices[globalIndex2]);
    //matrix skinMatrix3 = mul(g_Offsets[meshLocalIndices.w], g_FinalBoneMatrices[globalIndex3]);

    //// 가중합
    //matrix finalSkinMatrix = skinMatrix0 * w.x +
    //    skinMatrix1 * w.y +
    //    skinMatrix2 * w.z +
    //    skinMatrix3 * w.w;

    //// 스키닝 적용
    //vector vPosition = mul(vector(In.vPosition, 1.f), finalSkinMatrix);
    //vector vNormal = mul(vector(In.vNormal, 0.f), finalSkinMatrix);
  
    
    matrix matWV, matWVP;
    
    /* mul : 모든 행렬의 곱하기를 수행한다. /w연산을 수행하지 않는다. */
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;    
    Out.vWorldPos = mul(vPosition, g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
    return Out;
}

struct VS_OUT_SHADOW
{
    float4 vPosition : SV_POSITION;    
    float4 vProjPos : TEXCOORD0;
};


VS_OUT_SHADOW VS_MAIN_SHADOW(VS_IN In)
{
    VS_OUT_SHADOW Out;
    
    float fWeightW = 1.f - (In.vBlendWeights.x + In.vBlendWeights.y + In.vBlendWeights.z);
        
    matrix BoneMatrix = g_BoneMatrices[In.vBlendIndices.x] * In.vBlendWeights.x +
        g_BoneMatrices[In.vBlendIndices.y] * In.vBlendWeights.y +
        g_BoneMatrices[In.vBlendIndices.z] * In.vBlendWeights.z +
        g_BoneMatrices[In.vBlendIndices.w] * fWeightW;
    
    vector vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);    
    
    matrix matWV, matWVP;
    
    /* mul : 모든 행렬의 곱하기를 수행한다. /w연산을 수행하지 않는다. */
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(vPosition, matWVP);    
    Out.vProjPos = Out.vPosition;
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
    
};

struct PS_OUT
{
    vector vDiffuse : SV_TARGET0;
    vector vNormal : SV_TARGET1;
    vector vDepth : SV_TARGET2;
    vector vPickPos : SV_TARGET3;
};

struct PS_OUT_NONPICK
{
    vector vDiffuse : SV_TARGET0;
    vector vNormal : SV_TARGET1;
    vector vDepth : SV_TARGET2;
    
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;    
    
    vector  vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.3f)
        discard;  
    
    Out.vDiffuse = vMtrlDiffuse;
    
    /* -1.f -> 0.f, 1.f -> 1.f */    
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.0f, 0.f, 0.f);
    
    Out.vPickPos = In.vWorldPos;
    
    return Out;
}

PS_OUT_NONPICK PS_MAIN_NONPICK(PS_IN In)
{
    PS_OUT_NONPICK Out;
    
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.3f)
        discard;
    
    Out.vDiffuse = vMtrlDiffuse;
    
    /* -1.f -> 0.f, 1.f -> 1.f */    
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.0f, 0.f, 0.f);
    
    return Out;
}

struct PS_IN_SHADOW
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
};

struct PS_OUT_SHADOW
{
    vector vShadow : SV_TARGET0;    
};

PS_OUT_SHADOW PS_MAIN_SHADOW(PS_IN_SHADOW In)
{
    PS_OUT_SHADOW Out;
    
    Out.vShadow = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.0f, 0.f, 0.f);
    
    return Out;
}

technique11 DefaultTechnique
{   
    pass Default
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();   
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();      
    }

    pass NonPick
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NONPICK();
    }

    pass Shadow
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN_SHADOW();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
    }
   
   
   
}
