#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_ARMTexture;

/* [ 조절용 파라미터 ] */
float g_fDiffuseIntensity = 1;
float g_fNormalIntensity = 1;
float g_fAOIntensity = 1;
float g_fAOPower = 1;
float g_fRoughnessIntensity = 1;
float g_fMetallicIntensity = 1;
float g_fReflectionIntensity = 1;
float g_fSpecularIntensity = 1;
vector g_vDiffuseTint = { 1.f, 1.f, 1.f, 1.f };


float g_fID;


struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float2 vTexcoord : TEXCOORD0;
    
    row_major float4x4 TransformMatrix : WORLD;
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

    // 인스턴스별 월드 행렬 계산
    matrix matInstanceWorld = mul(In.TransformMatrix, g_WorldMatrix);

    // 위치 계산
    float4 vWorldPos = mul(float4(In.vPosition, 1.f), matInstanceWorld);
    float4 vViewPos = mul(vWorldPos, g_ViewMatrix);
    float4 vProjPos = mul(vViewPos, g_ProjMatrix);

    Out.vPosition = vProjPos;
    Out.vProjPos = vProjPos;

    // 정규벡터 변환 (회전만 적용)
    matrix matNormalTransform = matInstanceWorld;
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), matNormalTransform));
    Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), matNormalTransform));
    Out.vBinormal = normalize(cross(Out.vNormal.xyz, Out.vTangent.xyz));

    // 텍스처 좌표 및 월드 위치
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = vWorldPos;

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

struct PS_OUT_TOOL
{
    vector vDiffuse : SV_TARGET0;
    vector vNormal : SV_TARGET1;
    vector vDepth : SV_TARGET2;
    vector vPickPos : SV_TARGET3;
};

struct PS_OUT
{
    vector vDiffuse : SV_TARGET0;
    vector vNormal : SV_TARGET1;
    vector vARM : SV_TARGET2;
    vector vProjPos : SV_TARGET3;
    vector vAO : SV_TARGET4;
    vector vRoughness : SV_TARGET5;
    vector vMetallic : SV_TARGET6;
};

struct PS_SKY_OUT
{
    vector vDiffuse : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;    
    
    // 디퓨즈 텍스처
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.3f)
        discard;
    
    // 노말 텍스처
    vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;
    
    float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz, In.vNormal.xyz);
    float3 vNormalSample = vNormalDesc.xyz * 2.f - 1.f;
    float3 vNormalTS = normalize(lerp(float3(0, 0, 1), vNormalSample, g_fNormalIntensity));
    float3x3 TBN = float3x3(In.vTangent.xyz, In.vBinormal.xyz, In.vNormal.xyz);
    float3 vWorldNormal = mul(vNormalTS, TBN);
    
    // ARM 텍스처
    float3 vARM = g_ARMTexture.Sample(DefaultSampler, In.vTexcoord).rgb;
    float AO = pow(vARM.r, g_fAOPower) * g_fAOIntensity;
    float Roughness = vARM.g * g_fRoughnessIntensity;
    float Metallic = vARM.b * g_fMetallicIntensity;
   
    Out.vDiffuse = float4(vMtrlDiffuse.rgb * g_fDiffuseIntensity * g_vDiffuseTint.rgb, vMtrlDiffuse.a);
    Out.vNormal = float4(normalize(vWorldNormal) * 0.5f + 0.5f, 1.f);
    Out.vARM = float4(AO, Roughness, Metallic, 1.f);
    Out.vProjPos = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, g_fReflectionIntensity, g_fSpecularIntensity);
    Out.vAO = float4(AO, AO, AO, 1.f);
    Out.vRoughness = float4(Roughness, Roughness, Roughness, 1.0f);
    Out.vMetallic = float4(Metallic, Metallic, Metallic, 1.0f);
    return Out;
}

PS_OUT_TOOL PS_MAPTOOLOBJECT(PS_IN In)
{
    PS_OUT_TOOL Out;
    
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.3f)
        discard;
    
    vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;
    
    float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz, In.vNormal.xyz);
    
    vNormal = mul(vNormal, WorldMatrix);
    
   
    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.f, g_fID); //w값에다가 아이디를 저장하겠음
    Out.vPickPos = In.vWorldPos;
    
    return Out;
}

PS_SKY_OUT PS_SKY_MAIN(PS_IN In)
{
    PS_SKY_OUT Out;
    
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.3f)
        discard;
    
    vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;
    
    float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz, In.vNormal.xyz);
    
    vNormal = mul(vNormal, WorldMatrix);
    
   
    Out.vDiffuse = vMtrlDiffuse;
    
    return Out;
}

technique11 DefaultTechnique
{   
//0
    pass Default
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();      
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();      
    }
//1
    pass SkyBox
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SKY_MAIN();
    }
//2
    pass MAPTOOL_OBJECT
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAPTOOLOBJECT();
    }
//3
    pass PREVIEW_OBJECT
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
   
}
