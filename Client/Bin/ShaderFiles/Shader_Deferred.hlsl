#include "Engine_Shader_Defines.hlsli"

/* ������̺� ConstantTable */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_LightViewMatrixA, g_LightProjMatrixA;
matrix g_LightViewMatrixB, g_LightProjMatrixB;
matrix g_LightViewMatrixC, g_LightProjMatrixC;
matrix g_ViewMatrixInv, g_ProjMatrixInv;
Texture2D g_RenderTargetTexture;
Texture2D g_NormalTexture;
Texture2D g_DiffuseTexture;
Texture2D g_ShadeTexture;
Texture2D g_DepthTexture;
Texture2D g_SpecularTexture;
Texture2D g_ShadowTexture;

/* [ ĳ�����̵� ���� ] */
Texture2D g_ShadowTextureA;
Texture2D g_ShadowTextureB;
Texture2D g_ShadowTextureC;


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
vector g_vLightDiffuse;
vector g_vLightSpecular;
float  g_fLightAmbient;
float  g_fLightRange;
float  g_fLightIntencity = 1.f;

float  g_fMtrlAmbient = 1.f;
vector g_vMtrlSpecular = 1.f;

vector g_vCamPosition;

Texture2D g_MaskTexture;
Texture2D g_UITexture;

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
    
    // [ ViewPos ���� ]
    float2 vUV = In.vTexcoord;
    float z_ndc = vDepthDesc.x;
    float viewZ = vDepthDesc.y * 500.0f;
    
    float4 ndcPos;
    ndcPos.x = vUV.x * 2.0f - 1.0f;
    ndcPos.y = (1.0f - vUV.y * 2.0f);
    ndcPos.z = z_ndc;
    ndcPos.w = 1.0f;
    
    float4 viewPos = mul(ndcPos, g_ProjMatrixInv);
    viewPos /= viewPos.w; // Perspective divide
    viewPos *= viewZ / viewPos.z; // View Z ����

    float4 worldPos = mul(viewPos, g_ViewMatrixInv);
    
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
    radiance *= g_fLightIntencity;
    radiance *= 3.5f;

    /* [ ���� PBR ���� ��� ] */
    float3 FinalColor = (Diffuse + Specular) * radiance * NdotL * AO + Ambient;
    float3 Specalur = Specular * radiance;

    Out.vFinal = float4(FinalColor, vDiffuseDesc.a);
    Out.vSpecular = float4(Specular, 1.0f);
    
    
    
    
    /* [ Volumetric Raymarching ��� ] */
    //float4 ViewSpacePosition = viewPos;
    //float3 ViewSpaceCamPos = float3(0, 0, 0);
    //float3 ViewSpacePixelDir = normalize(viewPos.xyz);
    //
    //float StepSize = 0.2f;
    //int NumStep = 64;
    //
    //float3 SamplePos = ViewSpaceCamPos;
    //float Accumulated = 0.0f;
    
    
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
    
    float4 ndcPos;
    ndcPos.x = vUV.x * 2.0f - 1.0f;
    ndcPos.y = (1.0f - vUV.y * 2.0f);
    ndcPos.z = z_ndc;
    ndcPos.w = 1.0f;
    
    float4 viewPos = mul(ndcPos, g_ProjMatrixInv);
    viewPos /= viewPos.w; // Perspective divide
    viewPos *= viewZ / viewPos.z; // View Z ����

    float4 worldPos = mul(viewPos, g_ViewMatrixInv);

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
    radiance *= g_fLightIntencity;
    radiance *= 3.5f;

    // [ ���� ���� ]
    float3 FinalColor = (Diffuse + Specular) * radiance * NdotL * AO * fAtt + Ambient;
    float3 Specalur = Specular * radiance;

    Out.vFinal = float4(FinalColor, vDiffuseDesc.a);
    Out.vSpecular = float4(Specular, 1.0f);
    return Out;
}


PS_OUT PS_MAIN_DEFERRED(PS_IN In)
{
    PS_OUT Out;
    
    /* [ ���� VTXMesh ] */
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vSpecular = g_SpecularTexture.Sample(DefaultSampler, In.vTexcoord);
    Out.vBackBuffer = vDiffuse * vShade + vSpecular;   
    
    /* [ PBR �Ž� ] */
    vector vPBRFinal = g_PBR_Final.Sample(DefaultSampler, In.vTexcoord);
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 500.f;
    if (vPBRFinal.a > 0.01f)
        Out.vBackBuffer = vPBRFinal;
    
    if (vDiffuse.a < 0.1f && vPBRFinal.a < 0.1f)
        discard;
    
    
    /* [ ����Ʈ���� ���̰� ���� ] */
    vector vPosition;

    vPosition.x = In.vTexcoord.x * 2.f - 1.f;
    vPosition.y = In.vTexcoord.y * -2.f + 1.f;
    vPosition.z = vDepthDesc.x;
    vPosition.w = 1.f;

    vPosition = vPosition * fViewZ;
    
    vPosition = mul(vPosition, g_ProjMatrixInv);
    vPosition = mul(vPosition, g_ViewMatrixInv);
    
    // 1. Cascade A
    vector vLightPosA;
    vLightPosA = mul(vPosition, g_LightViewMatrixA);
    vLightPosA = mul(vLightPosA, g_LightProjMatrixA);
    
    float2 uvA;
    uvA.x = vLightPosA.x / vLightPosA.w * 0.5f + 0.5f;
    uvA.y = vLightPosA.y / vLightPosA.w * -0.5f + 0.5f;
    
    float4 vDepthA = g_ShadowTextureA.Sample(DefaultSampler, uvA);
    float fShadowViewZA = vDepthA.y * 500.f;
    
    // 2. Cascade B
    vector vLightPosB;
    vLightPosB = mul(vPosition, g_LightViewMatrixB);
    vLightPosB = mul(vLightPosB, g_LightProjMatrixB);
    
    float2 uvB;
    uvB.x = vLightPosB.x / vLightPosB.w * 0.5f + 0.5f;
    uvB.y = vLightPosB.y / vLightPosB.w * -0.5f + 0.5f;
    
    float4 vDepthB = g_ShadowTextureB.Sample(DefaultSampler, uvB);
    float fShadowViewZB = vDepthB.y * 500.f;

    // 3. Cascade C
    vector vLightPosC;
    vLightPosC = mul(vPosition, g_LightViewMatrixC);
    vLightPosC = mul(vLightPosC, g_LightProjMatrixC);
    
    float2 uvC;
    uvC.x = vLightPosC.x / vLightPosC.w * 0.5f + 0.5f;
    uvC.y = vLightPosC.y / vLightPosC.w * -0.5f + 0.5f;
    
    float4 vDepthC = g_ShadowTextureC.Sample(DefaultSampler, uvC);
    float fShadowViewZC = vDepthC.y * 500.f;

    // --- ���� �� ---
    float fBias = 0.1f;
    if (fShadowViewZA + fBias < vLightPosA.w)
        Out.vBackBuffer *= 0.5f;
    else if (fShadowViewZB + fBias < vLightPosB.w)
        Out.vBackBuffer *= 0.5f;
    else if (fShadowViewZC + fBias < vLightPosC.w)
        Out.vBackBuffer *= 0.5f;
        
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


PS_OUT PS_MAIN_VIGNETTING(PS_IN In)
{
    PS_OUT Out;

    Out.vBackBuffer = g_UITexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vMask = g_MaskTexture.Sample(DefaultSampler, In.vTexcoord);
  
    
    float2 uv = In.vTexcoord * 2.f - 1.f;
 
    
    
    float vignette = 1.0f - 0.8f * smoothstep(0.05f, 1.5f, length(uv));
    
    Out.vBackBuffer.rgb *= (vignette);
    
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

    pass Vignetting //8
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_VIGNETTING();
    }
  
}
