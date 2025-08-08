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

//��Į �ؽ���
Texture2D g_DecalTexture;

/* [ Blur ] */
Texture2D g_PreBlurTexture;
Texture2D g_BlurXTexture;
Texture2D g_BlurYTexture;

/* [ ĳ�����̵� ���� ] */
Texture2D g_ShadowTextureA;
Texture2D g_ShadowTextureB;
Texture2D g_ShadowTextureC;

/* [ ������Ʈ�� ���� ] */
float g_fFogSpeed = 0.1f;
float g_fFogPower = 1.5f;
float g_fFogCutoff = 15.f;
float g_fTime;
int g_iPointNum = 1;
bool g_bVolumetricFog = false;

/* [ PBR ���� ] */
Texture2D g_PBR_Diffuse;
Texture2D g_PBR_Normal;
Texture2D g_PBR_ARM;
Texture2D g_PBR_Depth;
Texture2D g_PBR_Final;

Texture2D g_VolumetricTexture;

/* [ Effect ] */
Texture2D g_EffectBlend_Diffuse;
Texture2D g_EffectBlend_WBComposite;
Texture2D g_EffectBlend_Glow;
Texture2D g_EffectBlend_WBGlow;
// Texture2D g_Effect_Distort;

/* [ WeightedBlend Composite ] */
Texture2D g_WB_Accumulation;
Texture2D g_WB_Revealage;



float PI = 3.14159265358979323846f;


vector g_vLightDir;
vector g_vLightPos;
vector g_vLightDiffuse;
vector g_vLightSpecular;
float  g_fLightAmbient;
float  g_fLightRange;
float  g_fInnerCosAngle;
float  g_fOuterCosAngle;
float  g_fFalloff;
float  g_fLightIntencity = 1.f;

float  g_fMtrlAmbient = 1.f;
vector g_vMtrlSpecular = 1.f;

vector g_vCamPosition;

Texture2D g_MaskTexture;
Texture2D g_UITexture;

float g_fViewportSizeX, g_fViewportSizeY;

/* [ ������Ʈ�� ���� �Լ� ] */
float Hash(float3 p)
{
    p = frac(p * 0.3183099f + 0.1f);
    p *= 17.0f;
    return frac(p.x * p.y * p.z * (p.x + p.y + p.z));
}
float ValueNoise3D(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);

    // Trilinear interpolation weights
    float3 w = f * f * (3.0f - 2.0f * f); // smootherstep

    // 8-corner hash values
    float n000 = Hash(i + float3(0, 0, 0));
    float n100 = Hash(i + float3(1, 0, 0));
    float n010 = Hash(i + float3(0, 1, 0));
    float n110 = Hash(i + float3(1, 1, 0));
    float n001 = Hash(i + float3(0, 0, 1));
    float n101 = Hash(i + float3(1, 0, 1));
    float n011 = Hash(i + float3(0, 1, 1));
    float n111 = Hash(i + float3(1, 1, 1));

    // Trilinear interpolation
    float nx00 = lerp(n000, n100, w.x);
    float nx10 = lerp(n010, n110, w.x);
    float nx01 = lerp(n001, n101, w.x);
    float nx11 = lerp(n011, n111, w.x);

    float nxy0 = lerp(nx00, nx10, w.y);
    float nxy1 = lerp(nx01, nx11, w.y);

    return lerp(nxy0, nxy1, w.z);
}

float SampleFogDensity(float3 worldPos, float time)
{
    float3 uvw = worldPos * 0.08f; // Ÿ�ϸ� ����
    uvw.z += time * g_fFogSpeed;

    float noise = ValueNoise3D(uvw);
    //float density = saturate(noise * 1.5f);
    float density = saturate(pow(noise, 1.2f) * 1.2f);

    return density;
}
float SampleShadowMap(float3 worldPos)
{
    float shadow = 1.0f;

    // A ĳ�����̵�
    float4 lightPosA = mul(float4(worldPos, 1.0f), g_LightViewMatrixA * g_LightProjMatrixA);
    lightPosA.xyz /= lightPosA.w;
    float2 uvA = lightPosA.xy * 0.5f + 0.5f;
    float depthA = lightPosA.z;
    float mapDepthA = g_ShadowTextureA.SampleLevel(DefaultSampler, uvA, 0.0f).r;
    if (depthA + 0.005f < mapDepthA)
        shadow = min(shadow, 0.5f);

    // B ĳ�����̵�
    float4 lightPosB = mul(float4(worldPos, 1.0f), g_LightViewMatrixB * g_LightProjMatrixB);
    lightPosB.xyz /= lightPosB.w;
    float2 uvB = lightPosB.xy * 0.5f + 0.5f;
    float depthB = lightPosB.z;
    float mapDepthB = g_ShadowTextureB.SampleLevel(DefaultSampler, uvB, 0.0f).r;
    if (depthB + 0.005f < mapDepthB)
        shadow = min(shadow, 0.5f);

    // C ĳ�����̵�
    float4 lightPosC = mul(float4(worldPos, 1.0f), g_LightViewMatrixC * g_LightProjMatrixC);
    lightPosC.xyz /= lightPosC.w;
    float2 uvC = lightPosC.xy * 0.5f + 0.5f;
    float depthC = lightPosC.z;
    float mapDepthC = g_ShadowTextureC.SampleLevel(DefaultSampler, uvC, 0.0f).r;
    if (depthC + 0.005f < mapDepthC)
        shadow = min(shadow, 0.5f);

    return shadow;
}
float3 ReconstructViewPosFromDepth(float2 uv)
{
    // NDC ��ǥ ���
    float z_ndc = g_PBR_Depth.SampleLevel(DefaultSampler, uv, 0.0f).x;
    float z_view = g_PBR_Depth.SampleLevel(DefaultSampler, uv, 0.0f).y * 1000.0f;

    float4 ndcPos;
    ndcPos.x = uv.x * 2.0f - 1.0f;
    ndcPos.y = (1.0f - uv.y * 2.0f);
    ndcPos.z = z_ndc;
    ndcPos.w = 1.0f;

    // Projection ����ȯ
    float4 viewPos = mul(ndcPos, g_ProjMatrixInv);
    viewPos /= viewPos.w;

    // ViewZ ����
    viewPos *= z_view / viewPos.z;

    return viewPos.xyz;
}

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

struct PS_OUT_VOLUMETRIC
{
    vector vVolumetric : SV_TARGET0;
};

PS_OUT_LIGHT PS_MAIN_LIGHT_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out;
    
    vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);
    
    float fShade = max(dot(normalize(g_vLightDir) * -1.f, vNormal), 0.f) + (g_fLightAmbient * g_fMtrlAmbient);
    
    Out.vShade = g_vLightDiffuse * saturate(fShade);
    
    vector  vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);    
    float fViewZ = vDepthDesc.y * 1000.f;
    
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
    float fViewZ = vDepthDesc.y * 1000.f;
    
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
    float3 Ambient = Albedo * 0.1f * AO;
    
    // [ ViewPos ���� ]
    float2 vUV = In.vTexcoord;
    float z_ndc = vDepthDesc.x;
    float viewZ = vDepthDesc.y * 1000.0f;
    
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
    float viewZ = vDepthDesc.y * 1000.0f;
    
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
    
    float fAtt = saturate(1.0 - pow(distance / g_fLightRange, 4.0));

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
    float3 FinalColor = (Diffuse + Specular) * radiance * NdotL * AO * fAtt;//    +Ambient;
    float3 Specalur = Specular * radiance;

    Out.vFinal = float4(FinalColor, vDiffuseDesc.a);
    Out.vSpecular = float4(Specular, 1.0f);
    
    return Out;
}
PS_OUT_PBR PS_PBR_LIGHT_SPOT(PS_IN In)
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
    float viewZ = vDepthDesc.y * 1000.0f;
    
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
    
    float fAtt = saturate(1.0 - distance / g_fLightRange);
    
    float3 SpotDir = normalize(-g_vLightDir.xyz);
    float3 ToPixel = normalize(worldPos.xyz - g_vLightPos.xyz);

    float cosAngle = dot(SpotDir, ToPixel);
    float spotAtt = saturate((cosAngle - g_fOuterCosAngle) / (g_fInnerCosAngle - g_fOuterCosAngle));
    spotAtt = pow(spotAtt, g_fFalloff);

    fAtt *= spotAtt;
    
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
    float3 FinalColor = (Diffuse + Specular) * radiance * NdotL * AO * fAtt;//    +Ambient;
    float3 Specalur = Specular * radiance;
    
    Out.vFinal = float4(FinalColor, vDiffuseDesc.a);
    Out.vSpecular = float4(Specular, 1.0f);
    return Out;
}

PS_OUT_VOLUMETRIC PS_VOLUMETRIC_POINT(PS_IN In)
{
    PS_OUT_VOLUMETRIC Out;

    // [ �ؽ�ó ���ø� ]
    vector vDepthDesc = g_PBR_Depth.Sample(DefaultSampler, In.vTexcoord);

    // [ �ش� ����Ʈ�� ���̰� ���� ViewPos ]
    float2 vUV = In.vTexcoord;
    float z_ndc = vDepthDesc.x;
    float viewZ = vDepthDesc.y * 1000.0f;
    
    bool bNoMeshBehind = (z_ndc >= 0.999f || viewZ <= 0.001f);
    
    float4 ndcPos;
    ndcPos.x = vUV.x * 2.0f - 1.0f;
    ndcPos.y = 1.0f - vUV.y * 2.0f;
    ndcPos.z = z_ndc;
    ndcPos.w = 1.0f;
    
    float4 viewPos = mul(ndcPos, g_ProjMatrixInv);
    viewPos /= viewPos.w;
    
    if (bNoMeshBehind)
    {
    // ī�޶� ���� ���� * �ִ� �Ÿ�
        viewPos.xyz = normalize(viewPos.xyz) * 50.0f;
    }
    else
    {
        float scaleZ = viewZ / max(viewPos.z, 0.0001f);
        viewPos *= scaleZ;
    }   

    /* [ Volumetric Raymarching ��� ] */
    float4 ViewSpacePosition = viewPos;
    
    //float StepSize = 0.5f;
    static const int NumStep = 100;
    float StepSize = viewZ / NumStep;
    // -----------------------------------------
    // ������� Light ���� ������ �߰�
    // -----------------------------------------

    float LightFog = 0.0f;

    float3 PixelWorldPos = mul(float4(ViewSpacePosition.xyz, 1.0f), g_ViewMatrixInv).xyz;
    float3 LightSamplePos = PixelWorldPos;
    float3 RayOrigin = float3(0.f,0.f,0.f);
    float3 RayDir = normalize(ViewSpacePosition.xyz);
    float3 RayPos = RayOrigin;
    
    bool bBlocked = false;
    for (int j = 0; j < NumStep; ++j)
    {
        // ���� ���������� ���� ��ǥ�� ��ȯ
        RayPos += RayDir * StepSize;
        float3 worldPos = mul(float4(RayPos, 1.0f), g_ViewMatrixInv).xyz;
        
        // �Ž��� ���� ���� Ȯ��
        if (!bNoMeshBehind)
        {
            if (!bBlocked && RayPos.z > viewZ + 0.01f)
                bBlocked = true;
        
            if (bBlocked)
                continue;
        }
        
        /* [ ���� �ִ�Ÿ� ���� ] */
        float distanceToLight = length(g_vLightPos.xyz - worldPos);
        float softFalloff = saturate(1.0f - (distanceToLight / g_fFogCutoff));

        float density = 0.3f; //SampleFogDensity(worldPos, g_fTime);
        float shadow = SampleShadowMap(worldPos);
        
        float transmittance = 1.0f - shadow;
        int NumLights = max(g_iPointNum, 1);
        LightFog += density * transmittance * softFalloff * StepSize * 0.05f;
    }
    
    // -----------------------------------------
    // ��� ����
    // -----------------------------------------

    float3 fogColor = g_vLightDiffuse.rgb;
    float3 finalFog = fogColor * (LightFog * g_fFogPower);
    
    Out.vVolumetric = float4(finalFog, 1.f);
    
    return Out;
}
PS_OUT_VOLUMETRIC PS_VOLUMETRIC_DIRECTIONAL(PS_IN In)
{
    PS_OUT_VOLUMETRIC Out;

    // [ �ؽ�ó ���ø� ]
    vector vDepthDesc = g_PBR_Depth.Sample(DefaultSampler, In.vTexcoord);

    // [ �ش� ����Ʈ�� ���̰� ���� ViewPos ]
    float2 vUV = In.vTexcoord;
    float z_ndc = vDepthDesc.x;
    float viewZ = vDepthDesc.y * 1000.0f;
    
    float4 ndcPos;
    ndcPos.x = vUV.x * 2.0f - 1.0f;
    ndcPos.y = 1.0f - vUV.y * 2.0f;
    ndcPos.z = z_ndc;
    ndcPos.w = 1.0f;
    
    float4 viewPos = mul(ndcPos, g_ProjMatrixInv);
    viewPos /= viewPos.w;
    
    float scaleZ = viewZ / max(viewPos.z, 0.0001f);
    viewPos *= scaleZ;

    /* [ Volumetric Raymarching ��� ] */
    float4 ViewSpacePosition = viewPos;

    float StepSize = 0.25f;
    static const int NumStep = 50;

    float LightFog = 0.0f;

    float3 PixelWorldPos = mul(float4(ViewSpacePosition.xyz, 1.0f), g_ViewMatrixInv).xyz;

    // �𷺼ų� ����Ʈ�� ������ �ƴ� ���� ���⸸ ����
    float3 RayDirWorld = g_vLightDir.xyz;
    float3 RayOriginWorld = PixelWorldPos;
    //float3 RayOriginWorld = PixelWorldPos - g_vLightDir.xyz * 30.0f;
    float3 RayPosWorld = RayOriginWorld;

    bool bBlocked = false;
    for (int j = 0; j < NumStep; ++j)
    {
        RayPosWorld += RayDirWorld * StepSize;
        float3 sampleViewPos = mul(float4(RayPosWorld, 1.0f), g_ViewMatrix).xyz;

        // �޽� ����
        if (!bBlocked && sampleViewPos.z > viewZ + 0.01f)
            bBlocked = true;

        if (bBlocked)
            continue;

        float density = SampleFogDensity(RayPosWorld, g_fTime);
        float shadow = SampleShadowMap(RayPosWorld);
        
        float transmittance = 1.0f - shadow;
        int NumLights = max(g_iPointNum, 1);
        LightFog += density * transmittance * StepSize * 0.1f;
    }
    
    float3 fogColor = g_vLightDiffuse.rgb;
    float3 finalFog = fogColor * (LightFog * g_fFogPower);
    
    float fadeRatio = saturate(viewZ / 1000.0f);
    float3 fadeFog = fogColor * fadeRatio * g_fFogPower;
    
    finalFog = lerp(finalFog, fadeFog, fadeRatio);

    Out.vVolumetric = float4(finalFog, 1.f);
    return Out;
}
PS_OUT_VOLUMETRIC PS_VOLUMETRIC_SPOT(PS_IN In)
{
    PS_OUT_VOLUMETRIC Out;

    // [ �ؽ�ó ���ø� ]
    vector vDepthDesc = g_PBR_Depth.Sample(DefaultSampler, In.vTexcoord);

    // [ �ش� ����Ʈ�� ���̰� ���� ViewPos ]
    float2 vUV = In.vTexcoord;
    float z_ndc = vDepthDesc.x;
    float viewZ = vDepthDesc.y * 1000.0f;
    
    bool bNoMeshBehind = (z_ndc >= 0.999f || viewZ <= 0.001f);
    
    float4 ndcPos;
    ndcPos.x = vUV.x * 2.0f - 1.0f;
    ndcPos.y = 1.0f - vUV.y * 2.0f;
    ndcPos.z = z_ndc;
    ndcPos.w = 1.0f;
    
    float4 viewPos = mul(ndcPos, g_ProjMatrixInv);
    viewPos /= viewPos.w;
    
    if (bNoMeshBehind)
    {
        viewPos.xyz = normalize(viewPos.xyz) * 50.0f;
    }
    else
    {
        float scaleZ = viewZ / max(viewPos.z, 0.0001f);
        viewPos *= scaleZ;
    }

    /* [ Volumetric Raymarching ��� ] */
    float4 ViewSpacePosition = viewPos;

    //float StepSize = 0.5f;
    static const int NumStep = 100;
    float StepSize = viewZ / NumStep;

    float LightFog = 0.0f;

    float3 PixelWorldPos = mul(float4(ViewSpacePosition.xyz, 1.0f), g_ViewMatrixInv).xyz;
    float3 RayOrigin = float3(0.f, 0.f, 0.f);
    float3 RayDir = normalize(ViewSpacePosition.xyz);
    float3 RayPos = RayOrigin;

    bool bBlocked = false;
    for (int j = 0; j < NumStep; ++j)
    {
        RayPos += RayDir * StepSize;
        float3 worldPos = mul(float4(RayPos, 1.0f), g_ViewMatrixInv).xyz;

        // [ ���� üũ ���� ]
        if (!bNoMeshBehind)
        {
            if (!bBlocked && RayPos.z > viewZ + 0.01f)
                bBlocked = true;

            if (bBlocked)
                continue;
        }

        // [ ����Ʈ����Ʈ ���� ���� ��� ]
        float3 toPos = worldPos - g_vLightPos.xyz;
        float distanceToLight = length(toPos);
        float3 dirToPos = toPos / max(distanceToLight, 0.001f);

        float cosAngle = dot(-g_vLightDir.xyz, dirToPos);

        // [ ���� �ٱ��� ���� ]
        float fOuterCosAngle = g_fOuterCosAngle;
        if (cosAngle < fOuterCosAngle)
            continue;

        // [ ���� ���� ]
        float spotFalloff = saturate((cosAngle - fOuterCosAngle) / max(g_fInnerCosAngle - fOuterCosAngle, 0.001f));

        // [ �ִ� �Ÿ� ���� ]
        float softFalloff = saturate(1.0f - (distanceToLight / g_fFogCutoff));

        // [ �е� + �׸��� ]
        float density = 0.3f; //SampleFogDensity(worldPos, g_fTime);
        float shadow = SampleShadowMap(worldPos);
        float transmittance = 1.0f - shadow;

        // [ ���� ]
        LightFog += density * transmittance * softFalloff * spotFalloff * StepSize * 0.05f;        
        
    }
    
    
    // ����ȭ ����
    float3 fogColor = g_vLightDiffuse.rgb;
    float3 finalFog = fogColor * (LightFog * g_fFogPower * 100.f / max(viewZ, 1.0f));
    
    Out.vVolumetric = float4(finalFog, 1.f);
    return Out;
}

PS_OUT PS_MAIN_DEFERRED(PS_IN In)
{
    PS_OUT Out;
    
    vector finalColor = vector(0.f, 0.f, 0.f, 0.f);
    
    /* [ ���� VTXMesh ] */
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vSpecular = g_SpecularTexture.Sample(DefaultSampler, In.vTexcoord);
    Out.vBackBuffer = vDiffuse * vShade + vSpecular;   
    finalColor = Out.vBackBuffer;
    
    /* [ PBR �Ž� ] */
    vector vPBRFinal = g_PBR_Final.Sample(DefaultSampler, In.vTexcoord);
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vVolumetric = g_VolumetricTexture.Sample(DefaultSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 1000.f;
    if (vPBRFinal.a > 0.01f)
        Out.vBackBuffer = vPBRFinal;
    finalColor = Out.vBackBuffer;
    
    
    /* [ ����Ʈ + �۷ο� ] */
    //float fGlowIntensity = 3.f;
    
    //vector EffectBlendDiffuse = g_EffectBlend_Diffuse.Sample(DefaultSampler, In.vTexcoord);
    //vector EffectBlendGlow = g_EffectBlend_Glow.Sample(DefaultSampler, In.vTexcoord);
    ////EffectBlendGlow.rgb *= fGlowIntensity;
    //// EffectBlendGlow.a *= 1.5f;
    //EffectBlendDiffuse += EffectBlendGlow;
    //finalColor += EffectBlendDiffuse;
    
     //if (vDiffuse.a < 0.1f && vPBRFinal.a < 0.1f && EffectDiffuse.a < 0.1f)
     //   discard;   
    //if (finalColor.a < 0.003f)
    //    discard;

    //��Į ������
    vector vDecal = g_DecalTexture.Sample(DefaultSampler, In.vTexcoord);
    finalColor.rgb = finalColor.rgb * (1 - vDecal.a) + vDecal.rgb * vDecal.a;
    
    Out.vBackBuffer = finalColor;

    
    // === ���� ���� ��� ���� ===
    float fogDensity = 0.02f;
    float attenuation = exp(-fViewZ * fogDensity);
    float3 fogColor = vVolumetric.rgb * (1.0 - attenuation);

    Out.vBackBuffer.rgb += fogColor;

    
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
    
    float4 vDepthA = g_ShadowTextureA.Sample(LinearClampSampler, uvA);
    float fShadowViewZA = vDepthA.y * 1000.f;
    
    // 2. Cascade B
    vector vLightPosB;
    vLightPosB = mul(vPosition, g_LightViewMatrixB);
    vLightPosB = mul(vLightPosB, g_LightProjMatrixB);
    
    float2 uvB;
    uvB.x = vLightPosB.x / vLightPosB.w * 0.5f + 0.5f;
    uvB.y = vLightPosB.y / vLightPosB.w * -0.5f + 0.5f;
    
    float4 vDepthB = g_ShadowTextureB.Sample(LinearClampSampler, uvB);
    float fShadowViewZB = vDepthB.y * 1000.f;

    // 3. Cascade C
    vector vLightPosC;
    vLightPosC = mul(vPosition, g_LightViewMatrixC);
    vLightPosC = mul(vLightPosC, g_LightProjMatrixC);
    
    float2 uvC;
    uvC.x = vLightPosC.x / vLightPosC.w * 0.5f + 0.5f;
    uvC.y = vLightPosC.y / vLightPosC.w * -0.5f + 0.5f;
    
    float4 vDepthC = g_ShadowTextureC.Sample(LinearClampSampler, uvC);
    float fShadowViewZC = vDepthC.y * 1000.f;

    // --- ���� �� ---
    float fBias = 0.1f;
    if (fShadowViewZA + fBias < vLightPosA.w)
        Out.vBackBuffer *= 0.5f;
    else if (fShadowViewZB + fBias < vLightPosB.w)
        Out.vBackBuffer *= 0.5f;
    else if (fShadowViewZC + fBias < vLightPosC.w)
        Out.vBackBuffer *= 0.5f;
    
    vector EffectBlendDiffuse = g_EffectBlend_Diffuse.Sample(DefaultSampler, In.vTexcoord);
    vector EffectBlendGlow = g_EffectBlend_Glow.Sample(DefaultSampler, In.vTexcoord);
    EffectBlendDiffuse += EffectBlendGlow;
    Out.vBackBuffer += EffectBlendDiffuse;
    //EffectBlendGlow.rgb *= fGlowIntensity;
    // EffectBlendGlow.a *= 1.5f;
    
    vector EffectBlendWBComposite = g_EffectBlend_WBComposite.Sample(DefaultSampler, In.vTexcoord);
    vector EffectBlendWBGlow = g_EffectBlend_WBGlow.Sample(DefaultSampler, In.vTexcoord);
    EffectBlendWBComposite += EffectBlendWBGlow;
    Out.vBackBuffer += EffectBlendWBComposite;
    
    /*****************/
    //vector vAccum = g_WB_Accumulation.Sample(DefaultSampler, In.vTexcoord);
    //float fReveal = g_WB_Revealage.Sample(DefaultSampler, In.vTexcoord).r;
    //vector Final;
    
    //Final = vAccum.rgb + Out.vBackBuffer.rgb * fReveal;
    //Final.a = 1 - fReveal;

    //Out.vBackBuffer += Final;
       
    if (Out.vBackBuffer.a < 0.003f)
        discard;
    
    return Out;    
}

float g_f7Weights[7] =
{
    0.0702, 0.1315, 0.1900, 0.2166, 0.1900, 0.1315, 0.0702
};

float g_f13Weights[13] =
{
    0.01855, 0.03416, 0.05634, 0.08316, 0.10971, 0.12962,
    0.13703,
    0.12962, 0.10971, 0.08316, 0.05634, 0.03416, 0.01855 
};

float g_f21Weights[21] =
{
    0.00202, 0.00413, 0.00761, 0.01320, 0.02145,
    0.03264, 0.04630, 0.06134, 0.07619, 0.08921,
    0.09810,
    0.10177, 0.09810, 0.08921, 0.07619, 0.06134,
    0.04630, 0.03264, 0.02145, 0.01320, 0.00761
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
        vTexcoord.x = In.vTexcoord.x + i / g_fViewportSizeX;
        vTexcoord.y = In.vTexcoord.y;
  
        Out.vColor += g_f13Weights[i + 6] * g_PreBlurTexture.Sample(LinearClampSampler, vTexcoord);
    }
    
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
        vTexcoord.y = In.vTexcoord.y + i / g_fViewportSizeY;
  
        Out.vBackBuffer += g_f13Weights[i + 6] * g_BlurXTexture.Sample(LinearClampSampler, vTexcoord);
    }
    
    return Out;
}

PS_OUT PS_EFFECT_GLOW(PS_IN In)
{   
    PS_OUT Out;
    
    Out.vBackBuffer = g_BlurYTexture.Sample(DefaultSampler, In.vTexcoord);
    /* ��Ÿ ���� */
    Out.vBackBuffer.rgb *= 1.5f;
    Out.vBackBuffer.a *= 1.2f;
    // �ۿ��� �� �����ذ��鼭 �� �� ������ ���ڴµ� ���� �� �� �� �����Ƿ� ������
    
    return Out;
}

PS_OUT PS_WB_COMPOSITE(PS_IN In)
{
    PS_OUT Out;
    
    /* [ ������ �´� �� ������ ������ ������ ���� ���� ] */
    //vector vAccum = g_WB_Accumulation.Sample(DefaultSampler, In.vTexcoord);
    //float4 fReveal = g_WB_Revealage.Sample(DefaultSampler, In.vTexcoord);
    //
    //float3 vColor = vAccum.rgb / max(saturate(vAccum.a), 0.00001f); // 0 ������ ������
    //float fAlpha = 1 - saturate(fReveal.r);
    //Out.vBackBuffer = float4(vColor * fAlpha, fAlpha);
    
    /********************************************************************/

    /* [ ����Ʈ ���� ] */
    /* [ Reveal�� ���� �̹� �� �ȼ��� ���� �� ������ ���°� ���Ǿ� �����Ƿ� �Ÿ� �� ����ġ�� ������� �ʰ� ��� ] */
    /* [ �ٵ� �̷��� Weighted Blend OIT��� �ϱ� ���ϰ� �׳� Order-Independent�� ��Ų �����̶� ���ľ� �� ��... ������... ] */
    vector vAccum = g_WB_Accumulation.Sample(DefaultSampler, In.vTexcoord);
    float fReveal = g_WB_Revealage.Sample(DefaultSampler, In.vTexcoord).r;
    float fAlpha = 1 - saturate(fReveal);
    Out.vBackBuffer = float4(vAccum.rgb, fAlpha);

    /********************************************************************/

    
    
    return Out;
}

PS_OUT PS_COPYONLY(PS_IN In)
{
    PS_OUT Out;
    
    Out.vBackBuffer = g_EffectBlend_Diffuse.Sample(DefaultSampler, In.vTexcoord);
    
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
  
    pass VolumetricPoint //9
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_VOLUMETRIC_POINT();
    }
    pass VolumetricDirection //10
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_VOLUMETRIC_DIRECTIONAL();
    }
    pass PBRLight_Spot //11
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_PBR_LIGHT_SPOT();
    }
    pass VolumetricSpot //12
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_VOLUMETRIC_SPOT();
    }
    pass Effect_Glow //13 
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_EFFECT_GLOW();
    }   
    pass Downscale_Copy //14 �ٿ���� ������ ������µ� �׳� ī�Ǹ� ���ִ� �н���
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_COPYONLY();
    }
    pass WB_Composite //15
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_SoftAdd, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_WB_COMPOSITE();
    }
}
