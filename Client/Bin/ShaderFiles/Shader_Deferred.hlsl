#include "Engine_Shader_Defines.hlsli"

/* ������̺� ConstantTable */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_LightViewMatrix, g_LightProjMatrix;
matrix g_ViewMatrixInv, g_ProjMatrixInv;
Texture2D g_RenderTargetTexture;
Texture2D g_NormalTexture;
Texture2D g_DiffuseTexture;
Texture2D g_ShadeTexture;
Texture2D g_DepthTexture;
Texture2D g_SpecularTexture;
Texture2D g_ShadowTexture;

Texture2D g_FinalTexture;
Texture2D g_BlurXTexture;


/* [ PBR ���� ] */
Texture2D g_PBR_Diffuse;
Texture2D g_PBR_Normal;
Texture2D g_PBR_ARM;
Texture2D g_PBR_Depth;
Texture2D g_PBR_Final;

float PI = 3.14159265358979323846f;


vector g_vLightDir;
vector g_vLightPos;
float g_fLightRange;
vector g_vLightDiffuse;
float  g_fLightAmbient;
vector g_vLightSpecular;

float  g_fMtrlAmbient = 1.f;
vector g_vMtrlSpecular = 1.f;

vector g_vCamPosition;

Texture2D g_MaskTexture;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    matrix matWV, matWVP;
    
    /* mul : ��� ����� ���ϱ⸦ �����Ѵ�. /w������ �������� �ʴ´�. */
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;    
};

struct PS_OUT
{
    vector vBackBuffer : SV_TARGET0;
};

PS_OUT PS_MAIN_DEBUG(PS_IN In)
{
    PS_OUT Out;
    
    Out.vBackBuffer = g_RenderTargetTexture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

struct PS_OUT_LIGHT
{
    vector vShade : SV_TARGET0;
    vector vSpecular : SV_TARGET1;
    
};

struct PS_OUT_PBR
{
    vector vSpecular    : SV_TARGET0;
    vector vFinal       : SV_TARGET1;
};

PS_OUT_LIGHT PS_MAIN_LIGHT_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out;
    
    vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);
    
    float fShade = max(dot(normalize(g_vLightDir) * -1.f, vNormal), 0.f) + (g_fLightAmbient * g_fMtrlAmbient);
    
    Out.vShade = g_vLightDiffuse * saturate(fShade);
    
    vector  vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);    
    float fViewZ = vDepthDesc.y * 500.f;
    
    vector vWorldPos;
    
    /* ������ġ * ���� *�� * ���� / w : ������������ ��ġ. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
     /* ������ġ * ���� *�� * ����  */
    vWorldPos = vWorldPos * fViewZ;
    
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);          
    
    vector vReflect = reflect(normalize(g_vLightDir), vNormal);
    vector vLook = vWorldPos - g_vCamPosition;
    
    Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * pow(max(dot(normalize(vLook) * -1.f, normalize(vReflect)), 0.f), 50.f);
    
    
    return Out;
}
PS_OUT_LIGHT PS_MAIN_LIGHT_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out;
    
    vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);
    
        
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 500.f;
    
    vector vWorldPos;
    
    /* ������ġ * ���� *�� * ���� / w : ������������ ��ġ. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
     /* ������ġ * ���� *�� * ����  */
    vWorldPos = vWorldPos * fViewZ;
    
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);
    
    vector vLightDir = vWorldPos - g_vLightPos;
    
    float fAtt = (g_fLightRange - length(vLightDir)) / g_fLightRange;
    
    float fShade = max(dot(normalize(vLightDir) * -1.f, vNormal), 0.f) + (g_fLightAmbient * g_fMtrlAmbient);
    
    Out.vShade = g_vLightDiffuse * saturate(fShade) * fAtt;
    
    vector vReflect = reflect(normalize(vLightDir), vNormal);
    vector vLook = vWorldPos - g_vCamPosition;
    
    Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * pow(max(dot(normalize(vLook) * -1.f, normalize(vReflect)), 0.f), 50.f) * fAtt;
    
    
    return Out;
}

PS_OUT_PBR PS_PBR_LIGHT_DIRECTIONAL(PS_IN In)
{
    PS_OUT_PBR Out;
    
    /* [ �ؽ�ó ����Ʈ ] */
    vector vDiffuseDesc = g_PBR_Diffuse.Sample(DefaultSampler, In.vTexcoord);
    vector vNormalDesc = g_PBR_Normal.Sample(DefaultSampler, In.vTexcoord);
    vector vARMDesc = g_PBR_ARM.Sample(DefaultSampler, In.vTexcoord);
    vector vDepthDesc = g_PBR_Depth.Sample(DefaultSampler, In.vTexcoord);
    
    /* [ Ȱ���� ���� ���� ] */
    float3 Albedo = vDiffuseDesc.rgb;
    float3 Normal = normalize(vNormalDesc.rgb * 2.0f - 1.0f);
    float AO = vARMDesc.r;
    float Roughness = vARMDesc.g;
    float Metallic = vARMDesc.b;
    float3 Ambient = Albedo * g_fLightAmbient * AO;
    
    /* [ ViewPos ���� ] */
    float2 vUV = In.vTexcoord;
    float z_ndc = vDepthDesc.x;
    float viewZ = vDepthDesc.y * 500.0f;

    /* [ NDC �������� ���̺��� ] */
    float4 clipPos;
    clipPos.x = vUV.x * 2.0f - 1.0f;
    clipPos.y = vUV.y * -2.0f + 1.0f;
    clipPos.z = z_ndc;
    clipPos.w = 1.0f;
    clipPos *= viewZ;
    
    /* [ ����� �������� ] */
    float4 worldPos = mul(clipPos, g_ProjMatrixInv);
    worldPos = mul(worldPos, g_ViewMatrixInv);
    
    /* [ �� ��� ] */
    float3 V = normalize(g_vCamPosition.xyz - worldPos.xyz);
    float3 L = normalize(g_vLightDir.xyz);
    float3 H = normalize(V + L);
    
    /* [ �ʿ��� ���� ���� ] */
    float NdotL = saturate(dot(Normal, L));
    float NdotV = saturate(dot(Normal, V));
    float NdotH = saturate(dot(Normal, H));
    float VdotH = saturate(dot(V, H));

    /* [ ������ �ݻ���(�ݼ�) ] */
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), Albedo, Metallic);
    float3 F = F0 + (1.0f - F0) * pow(1.0f - VdotH, 5.0f);
    F *= vDepthDesc.z;

    /* [ GGX �������ݻ� ���� ] */
    float a = Roughness * Roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH * (a2 - 1.0f) + 1.0f);
    float D = a2 / (PI * denom * denom + 0.001f);

    /* [ ������Ʈ�� �Լ� ���� ] */
    float k = (Roughness + 1.0f);
    k = (k * k) / 8.0f;
    float G_V = NdotV / (NdotV * (1.0f - k) + k);
    float G_L = NdotL / (NdotL * (1.0f - k) + k);
    float G = G_V * G_L;

    /* [ ������ ��� ����ŧ�� ] */
    float3 Specular = D * G * F / (4.0f * NdotV * NdotL + 0.001f);
    Specular *= g_vLightSpecular.rgb;
    Specular *= vDepthDesc.w;

    /* [ ��ǻ�� ���� ���� ] */
    float3 kD = (1.0f - F) * (1.0f - Metallic);
    float3 Diffuse = kD * Albedo / PI;

    /* [ ����Ʈ�� ���� ] */
    float3 radiance = g_vLightDiffuse.rgb;
    radiance *= 3.5f;

    /* [ ���� PBR ���� ��� ] */
    float3 FinalColor = (Diffuse + Specular) * radiance * NdotL * AO + Ambient;
    float3 Specalur = Specular * radiance;

    Out.vFinal = float4(FinalColor, 1.0f);
    Out.vSpecular = float4(Specular, 1.0f);
    return Out;
}
PS_OUT_PBR PS_PBR_LIGHT_POINT(PS_IN In)
{
    PS_OUT_PBR Out;

    // [ �ؽ�ó ���ø� ]
    vector vDiffuseDesc = g_PBR_Diffuse.Sample(DefaultSampler, In.vTexcoord);
    vector vNormalDesc = g_PBR_Normal.Sample(DefaultSampler, In.vTexcoord);
    vector vARMDesc = g_PBR_ARM.Sample(DefaultSampler, In.vTexcoord);
    vector vDepthDesc = g_PBR_Depth.Sample(DefaultSampler, In.vTexcoord);

    // [ ���� ���� ]
    float3 Albedo = vDiffuseDesc.rgb;
    float3 Normal = normalize(vNormalDesc.rgb * 2.0f - 1.0f);
    float AO = vARMDesc.r;
    float Roughness = vARMDesc.g;
    float Metallic = vARMDesc.b;
    float3 Ambient = Albedo * g_fLightAmbient * AO;

    // [ ViewPos ���� ]
    float2 vUV = In.vTexcoord;
    float z_ndc = vDepthDesc.x;
    float viewZ = vDepthDesc.y * 500.0f;

    float4 clipPos;
    clipPos.x = vUV.x * 2.0f - 1.0f;
    clipPos.y = vUV.y * -2.0f + 1.0f;
    clipPos.z = z_ndc;
    clipPos.w = 1.0f;
    clipPos *= viewZ;
    
    float4 worldPos = mul(clipPos, g_ProjMatrixInv);
    worldPos = mul(worldPos, g_ViewMatrixInv);

    // [ ����Ʈ ���� �� ���� ]
    //float3 L_unormalized = g_vLightPos.xyz - worldPos.xyz;
    float3 L_unormalized = worldPos.xyz - g_vLightPos.xyz;
    float distance = length(L_unormalized);
    float3 L = normalize(L_unormalized);

    float fAtt = saturate((g_fLightRange - distance) / g_fLightRange);

    // [ ��, ���� ���� ]
    float3 V = normalize(g_vCamPosition.xyz - worldPos.xyz);
    float3 H = normalize(V + L);

    float NdotL = saturate(dot(Normal, L));
    float NdotV = saturate(dot(Normal, V));
    float NdotH = saturate(dot(Normal, H));
    float VdotH = saturate(dot(V, H));

    // [ ������ ]
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), Albedo, Metallic);
    float3 F = F0 + (1.0f - F0) * pow(1.0f - VdotH, 5.0f);

    // [ GGX ]
    float a = Roughness * Roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH * (a2 - 1.0f) + 1.0f);
    float D = a2 / (PI * denom * denom + 0.001f);

    // [ Geometry ]
    float k = (Roughness + 1.0f);
    k = (k * k) / 8.0f;
    float G_V = NdotV / (NdotV * (1.0f - k) + k);
    float G_L = NdotL / (NdotL * (1.0f - k) + k);
    float G = G_V * G_L;

    // [ Specular ]
    float3 Specular = D * G * F / (4.0f * NdotV * NdotL + 0.001f);
    Specular *= g_vLightSpecular.rgb;

    // [ Diffuse ]
    float3 kD = (1.0f - F) * (1.0f - Metallic);
    float3 Diffuse = kD * Albedo / PI;

    // [ ����Ʈ ���� ]
    float3 radiance = g_vLightDiffuse.rgb;
    radiance *= 3.5f;

    // [ ���� ���� ]
    float3 FinalColor = (Diffuse + Specular) * radiance * NdotL * AO * fAtt + Ambient;
    float3 Specalur = Specular * radiance;

    Out.vFinal = float4(FinalColor, 1.0f);
    Out.vSpecular = float4(Specular, 1.0f);
    return Out;
}


PS_OUT PS_MAIN_DEFERRED(PS_IN In)
{
    PS_OUT Out;
    
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    //if (all(vDiffuse.rgb == 0.f))
    if(vDiffuse.a == 0.f)
        discard;
    
    vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vSpecular = g_SpecularTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vPBRFinal = g_PBR_Final.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vBackBuffer = vDiffuse * vShade + vSpecular;
    if (vPBRFinal.a > 0.01f)
        Out.vBackBuffer = vPBRFinal;
    
    
    
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 500.f;
    
    vector vPosition;

    vPosition.x = In.vTexcoord.x * 2.f - 1.f;
    vPosition.y = In.vTexcoord.y * -2.f + 1.f;
    vPosition.z = vDepthDesc.x;
    vPosition.w = 1.f;

    vPosition = vPosition * fViewZ;
    
    vPosition = mul(vPosition, g_ProjMatrixInv);
    vPosition = mul(vPosition, g_ViewMatrixInv);
    
    vPosition = mul(vPosition, g_LightViewMatrix);
    vPosition = mul(vPosition, g_LightProjMatrix);
    
    float2 vTexcoord;
    
    /* (-1, 1 ~ 1, -1) -> (0, 0 ~ 1, 1) */
    vTexcoord.x = vPosition.x / vPosition.w * 0.5f + 0.5f;
    vTexcoord.y = vPosition.y / vPosition.w * -0.5f + 0.5f;    
    
    float4  vOldDepthDesc = g_ShadowTexture.Sample(DefaultSampler, vTexcoord);
    float fOldViewZ = vOldDepthDesc.y * 500.f;
    
    if (fOldViewZ + 0.1f < vPosition.w)
        Out.vBackBuffer = Out.vBackBuffer * 0.5f;
    
    return Out;    
}


float g_fWeights[13] =
{
    0.0561, 0.1353, 0.278, 0.4868, 0.7261, 0.9231, 1.f, 0.9231, 0.7261, 0.4868, 0.278, 0.1353, 0.0561
};

struct PS_OUT_BLUR
{
    vector vColor : SV_TARGET0;
};

PS_OUT_BLUR PS_MAIN_BLURX(PS_IN In)
{
    PS_OUT_BLUR Out;

    float2 vTexcoord;
    
    vector vColor;
    
    for (int i = -6; i < 7; ++i)
    {
        vTexcoord.x = In.vTexcoord.x + i / 1920.f;
        vTexcoord.y = In.vTexcoord.y;
  
        Out.vColor += g_fWeights[i + 6] * g_FinalTexture.Sample(LinearClampSampler, vTexcoord);
    }

    Out.vColor /= 6.0f;
    
    return Out;
}

PS_OUT PS_MAIN_BLURY(PS_IN In)
{
    PS_OUT Out;

    float2 vTexcoord;
    
    vector vColor;
    
    for (int i = -6; i < 7; ++i)
    {
        vTexcoord.x = In.vTexcoord.x;
        vTexcoord.y = In.vTexcoord.y + i / 1080.f;
  
        Out.vBackBuffer += g_fWeights[i + 6] * g_BlurXTexture.Sample(LinearClampSampler, vTexcoord);
    }

    Out.vBackBuffer /= 6.0f;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass Debug //0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
    }

    pass Light_Directional //1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_LIGHT_DIRECTIONAL();
    }

    pass Light_Point //2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_LIGHT_POINT();
    }

    pass Deferred //3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEFERRED();
    }

    pass BlurX //4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);        

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLURX();
    }

    pass BlurY //5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLURY();
    }

    pass PBRLight_Point //6
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_PBR_LIGHT_POINT();
    }

    pass PBRLight_Direction //7
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_PBR_LIGHT_DIRECTIONAL();
    }
  
}
