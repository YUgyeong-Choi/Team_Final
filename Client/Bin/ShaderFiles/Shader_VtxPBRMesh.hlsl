#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix;
matrix g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_ARMTexture;
Texture2D g_Emissive;

/* [ 조절용 파라미터 ] */
float g_fDiffuseIntensity = 1;
float g_fNormalIntensity = 1;
float g_fAOIntensity = 1;
float g_fAOPower = 1;
float g_fRoughnessIntensity = 1;
float g_fMetallicIntensity = 1;
float g_fReflectionIntensity = 1;
float g_fSpecularIntensity = 1;
float g_fEmissiveIntensity = 0;
vector g_vDiffuseTint = { 1.f, 1.f, 1.f, 1.f };

/* [ 피킹변수 ] */
float g_fID;


/* [ 타일링 전용 ] */
bool g_bTile = false;
float2 g_TileDensity = float2(1.0f, 1.0f);


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

struct VS_SHADOW_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
};

VS_SHADOW_OUT VS_SHADOW_MAIN(VS_IN In)
{
    VS_SHADOW_OUT Out;
    
    matrix matWV, matWVP;
    
    /* mul : 모든 행렬의 곱하기를 수행한다. /w연산을 수행하지 않는다. */
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vProjPos = Out.vPosition;
    return Out;
}

struct PS_IN
{
    float4 vPosition    : SV_POSITION;
    float4 vNormal      : NORMAL;
    float4 vTangent     : TANGENT;
    float3 vBinormal    : BINORMAL;
    float2 vTexcoord    : TEXCOORD0;
    float4 vWorldPos    : TEXCOORD1;
    float4 vProjPos     : TEXCOORD2;
};

struct PS_OUT
{
    vector vDiffuse     : SV_TARGET0;
    vector vNormal      : SV_TARGET1;
    vector vARM         : SV_TARGET2;
    vector vDepth       : SV_TARGET3;
    vector vAO          : SV_TARGET4;
    vector vRoughness   : SV_TARGET5;
    vector vMetallic    : SV_TARGET6;
    vector vEmissive    : SV_TARGET7;
    //vector vWorldPos : SV_TARGET7;
};

struct PS_SKY_OUT
{
    vector vDiffuse : SV_TARGET0;
};

float4 Sample_TriplanarTexture(Texture2D tex, PS_IN In, out float2 dummyUV, out float3 blend)
{
    if (g_bTile)
    {
        float3 worldNormal = normalize(In.vNormal.xyz);
        blend = abs(worldNormal);
        blend = pow(blend, 3.0);
        blend /= (blend.x + blend.y + blend.z); // normalize

        float2 uvXZ = In.vWorldPos.zx * g_TileDensity; // Y축 평면
        float2 uvXY = In.vWorldPos.xy * g_TileDensity; // Z축 평면
        float2 uvYZ = In.vWorldPos.zy * g_TileDensity; // X축 평면

        float4 sampleXZ = tex.Sample(DefaultSampler, uvXZ);
        float4 sampleXY = tex.Sample(DefaultSampler, uvXY);
        float4 sampleYZ = tex.Sample(DefaultSampler, uvYZ);

        dummyUV = uvXZ; // 아무거나 반환 (디버깅용)
        return sampleXZ * blend.y + sampleXY * blend.z + sampleYZ * blend.x;
    }
    else
    {
        dummyUV = In.vTexcoord;
        blend = float3(0, 0, 0); // unused
        return tex.Sample(DefaultSampler, In.vTexcoord);
    }
}
PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;    
    
    float2 vTriplanarUV;
    float3 vTriplanarBlend;
    
    // 디퓨즈 텍스처
    float4 vMtrlDiffuse = Sample_TriplanarTexture(g_DiffuseTexture, In, vTriplanarUV, vTriplanarBlend);
    if (vMtrlDiffuse.a < 0.3f)
        discard;
    
    // 노말 텍스처
    vector vNormalDesc = Sample_TriplanarTexture(g_NormalTexture, In, vTriplanarUV, vTriplanarBlend);
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
    
    // 이미시브 텍스처
    vector vEmissive = g_Emissive.Sample(DefaultSampler, In.vTexcoord);
   
    Out.vDiffuse = float4(vMtrlDiffuse.rgb * g_fDiffuseIntensity * g_vDiffuseTint.rgb, vMtrlDiffuse.a);
    Out.vNormal = float4(normalize(vWorldNormal) * 0.5f + 0.5f, 1.f);
    Out.vARM = float4(AO, Roughness, Metallic, 1.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, g_fReflectionIntensity, g_fSpecularIntensity);
    Out.vAO = float4(AO, AO, AO, 1.f);
    Out.vRoughness = float4(Roughness, Roughness, Roughness, 1.0f);
    Out.vMetallic = float4(Metallic, Metallic, Metallic, 1.0f);
    Out.vEmissive = float4(vEmissive.rgb * g_fEmissiveIntensity, vEmissive.a);
    //Out.vWorldPos = In.vWorldPos; //테스트(영웅)
    
    return Out;
}

PS_OUT PS_TOOL_MAIN(PS_IN In)
{
    PS_OUT Out;
    
    float2 vTriplanarUV;
    float3 vTriplanarBlend;
    
    // 디퓨즈 텍스처
    float4 vMtrlDiffuse = Sample_TriplanarTexture(g_DiffuseTexture, In, vTriplanarUV, vTriplanarBlend);
    if (vMtrlDiffuse.a < 0.3f)
        discard;
    
    // 노말 텍스처
    vector vNormalDesc = Sample_TriplanarTexture(g_NormalTexture, In, vTriplanarUV, vTriplanarBlend);
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
    
    // 이미시브 텍스처
    vector vEmissive = g_Emissive.Sample(DefaultSampler, In.vTexcoord);
   
    Out.vDiffuse = float4(vMtrlDiffuse.rgb * g_fDiffuseIntensity * g_vDiffuseTint.rgb, vMtrlDiffuse.a);
    Out.vNormal = float4(normalize(vWorldNormal) * 0.5f + 0.5f, 1.f);
    Out.vARM = float4(AO, Roughness, Metallic, 1.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.f, g_fID);
    Out.vAO = float4(AO, AO, AO, 1.f);
    Out.vRoughness = float4(Roughness, Roughness, Roughness, 1.0f);
    Out.vMetallic = float4(Metallic, Metallic, Metallic, 1.0f);
    Out.vEmissive = float4(vEmissive.rgb * g_fEmissiveIntensity, vEmissive.a);
    
    return Out;
}

struct PS_IN_SHADOW
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
};
struct PS_OUT_SHADOW
{
    float4 vShadowA : SV_TARGET0;
    float4 vShadowB : SV_TARGET1;
    float4 vShadowC : SV_TARGET2;
};

float4 PS_Cascade0(VS_SHADOW_OUT In) : SV_TARGET0
{
    float depthZ = In.vProjPos.z / In.vProjPos.w;
    float depthW = In.vProjPos.w / 1000.0f;
    return float4(depthZ, depthW, 0.f, 0.f);
}

// SV_TARGET1에만 쓰는 버전
float4 PS_Cascade1(VS_SHADOW_OUT In) : SV_TARGET1
{
    float depthZ = In.vProjPos.z / In.vProjPos.w;
    float depthW = In.vProjPos.w / 1000.0f;
    return float4(depthZ, depthW, 0.f, 0.f);
}

// SV_TARGET2에만 쓰는 버전
float4 PS_Cascade2(VS_SHADOW_OUT In) : SV_TARGET2
{
    float depthZ = In.vProjPos.z / In.vProjPos.w;
    float depthW = In.vProjPos.w / 1000.0f;
    return float4(depthZ, depthW, 0.f, 0.f);
}



technique11 DefaultTechnique
{   
    pass Default //0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();      
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();      
    }
    pass ToolMesh //1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TOOL_MAIN();
    }

    pass ShadowPass0 //2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_SHADOW_MAIN();
        PixelShader = compile ps_5_0 PS_Cascade0(); // SV_TARGET0 전용
    }

    pass ShadowPass1 //3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_SHADOW_MAIN();
        PixelShader = compile ps_5_0 PS_Cascade1(); // SV_TARGET1 전용
    }

    pass ShadowPass2 //4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_SHADOW_MAIN();
        PixelShader = compile ps_5_0 PS_Cascade2(); // SV_TARGET2 전용
    }

}
