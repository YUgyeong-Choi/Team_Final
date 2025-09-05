#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix;
matrix g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;


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
    vector vPuddle : SV_TARGET0;
};

float3 UnpackNormal(float3 n)
{
    return normalize(n * 2.0f - 1.0f);
}

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    float fMask = vMtrlDiffuse.r;
    
    if (fMask <= 0.001f)
        discard;
    
    vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    float3 vN_ts = UnpackNormal(vNormalDesc.xyz);
    
    float3x3 tbn = float3x3(normalize(In.vTangent.xyz),
                            normalize(In.vBinormal.xyz),
                            normalize(In.vNormal.xyz));
    float3 vN_ws = normalize(mul(vN_ts, tbn));

    // 화면 왜곡 오프셋 계산 0.003 ~ 0.015
    const float fRefractionScale = 0.008f;
    float2 vDistort = vN_ws.xy * fRefractionScale;

    // 5) 최종 불투명도(마스크 * 전역 투명도)
    const float fOpacity = 0.75f;

    // 6) 출력 패킹: RG=왜곡, B=알파, A=예비
    Out.vPuddle = vector(vDistort.x, vDistort.y, fMask * fOpacity, 0.0f);
    return Out;
}


technique11 DefaultTechnique
{
    pass Default //0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}