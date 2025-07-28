#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
//Texture2D g_NormalTexture;

Texture2D g_DiffuseTexture;

Texture2D g_MaskTexture1;
Texture2D g_MaskTexture2;

Texture2D g_DepthTexture;

vector g_vColor = { 1.f, 1.f, 1.f, 1.f };
float g_fThreshold, g_fIntensity;
vector g_vCenterColor;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{      
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;    
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    matrix matWV, matWVP;
    
    /* mul : 모든 행렬의 곱하기를 수행한다. /w연산을 수행하지 않는다. */
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);    
    Out.vNormal = normalize(mul(vector(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTangent = normalize(mul(vector(In.vTangent, 0.f), g_WorldMatrix));
    Out.vBinormal = normalize(cross(Out.vNormal.xyz, Out.vTangent.xyz));
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(vector(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

struct PS_OUT
{
    vector vDiffuse : SV_TARGET0;
    vector vNormal : SV_TARGET1;
    vector vDepth : SV_TARGET2;
    //vector vPickPos : SV_TARGET3;
};

float4 SoftEffect(float4 vOrigColor, float4 vProjPos)
{
    float2 vTexcoord;
    
    vTexcoord.x = vProjPos.x / vProjPos.w;
    vTexcoord.y = vProjPos.y / vProjPos.w;
    
    vTexcoord.x = vTexcoord.x * 0.5f + 0.5f;
    vTexcoord.y = vTexcoord.y * -0.5f + 0.5f;
    
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, vTexcoord);
    
    if (vDepthDesc.y != 0.f)
    {
        float fOldViewZ = vDepthDesc.y * 1000.f;
        float fDiff = (fOldViewZ - vProjPos.w) / vProjPos.w;
        vOrigColor.a = vOrigColor.a * saturate(fDiff);
    }
    
    return vOrigColor;
}


struct PS_OUT_EFFECT
{
    vector vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;    
    
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.3f)
        discard;
    
    //vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    //float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;
    
    //float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz, In.vNormal.xyz);
    
    //vNormal = mul(vNormal, WorldMatrix);    
    
   
    Out.vDiffuse = vMtrlDiffuse;
    //Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.0f, 0.f, 0.f);
    //Out.vPickPos = In.vWorldPos;
    
    return Out;
}

PS_OUT_EFFECT PS_MAIN_MASKONLY(PS_IN In)
{
    PS_OUT_EFFECT Out;
    //샘플러좀보고오자
    // 마스크의 밝기 정보 (흑백 텍스처)
    float mask = g_MaskTexture1.Sample(LinearClampSampler, In.vTexcoord).r;
    
    
    // 중심부 기준 값과 비교해서 색상 결정
    float4 color;
    float lerpFactor = saturate((mask - g_fThreshold) / (1.f - g_fThreshold));

    // 외곽은 보라색, 중심부는 흰색
    color = lerp(g_vColor, g_vCenterColor, lerpFactor);

    // 밝기 조절
    Out.vColor.rgb = color.rgb * mask * g_fIntensity;
    Out.vColor.a = color.a * mask;
    
    return Out;
}

// 나중에 노이즈 텍스쳐도 포함해서 만들기
PS_OUT_EFFECT PS_MAIN_MASK_NOISE(PS_IN In)
{
    PS_OUT_EFFECT Out;
    // 마스크의 밝기 정보 (흑백 텍스처)
    float mask = g_MaskTexture1.Sample(LinearClampSampler, In.vTexcoord).r;
    
    
    // 중심부 기준 값과 비교해서 색상 결정
    float4 color;
    float lerpFactor = saturate((mask - g_fThreshold) / (1.f - g_fThreshold));

    // 외곽은 보라색, 중심부는 흰색
    color = lerp(g_vColor, g_vCenterColor, lerpFactor);

    // 밝기 조절
    Out.vColor.rgb = color.rgb * mask * g_fIntensity;
    Out.vColor.a = color.a * mask;
    
    return Out;
}


technique11 DefaultTechnique
{   
    pass Default        // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();      
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();      
    }
   
    pass MaskOnly       // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();      
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_MASKONLY();
    }
   
}
