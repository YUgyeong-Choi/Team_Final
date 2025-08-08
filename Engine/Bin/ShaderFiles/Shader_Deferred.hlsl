#include "Engine_Shader_Defines.hlsli"

/* 상수테이블 ConstantTable */
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

//데칼 텍스쳐
Texture2D g_DecalTexture;

/* [ Blur ] */
Texture2D g_PreBlurTexture;
Texture2D g_BlurXTexture;
Texture2D g_BlurYTexture;

/* [ 캐스케이드 전용 ] */
Texture2D g_ShadowTextureA;
Texture2D g_ShadowTextureB;
Texture2D g_ShadowTextureC;

/* [ 볼륨메트리 포그 ] */
float g_fFogSpeed = 0.1f;
float g_fFogPower = 1.5f;
float g_fFogCutoff = 15.f;
float g_fTime;
int g_iPointNum = 1;
bool g_bVolumetricFog = false;

/* [ PBR 전용 ] */
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

/* [ 볼륨메트리 포그 함수 ] */
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
    float3 uvw = worldPos * 0.08f; // 타일링 조절
    uvw.z += time * g_fFogSpeed;

    float noise = ValueNoise3D(uvw);
    //float density = saturate(noise * 1.5f);
    float density = saturate(pow(noise, 1.2f) * 1.2f);

    return density;
}
float SampleShadowMap(float3 worldPos)
{
    float shadow = 1.0f;

    // A 캐스케이드
    float4 lightPosA = mul(float4(worldPos, 1.0f), g_LightViewMatrixA * g_LightProjMatrixA);
    lightPosA.xyz /= lightPosA.w;
    float2 uvA = lightPosA.xy * 0.5f + 0.5f;
    float depthA = lightPosA.z;
    float mapDepthA = g_ShadowTextureA.SampleLevel(DefaultSampler, uvA, 0.0f).r;
    if (depthA + 0.005f < mapDepthA)
        shadow = min(shadow, 0.5f);

    // B 캐스케이드
    float4 lightPosB = mul(float4(worldPos, 1.0f), g_LightViewMatrixB * g_LightProjMatrixB);
    lightPosB.xyz /= lightPosB.w;
    float2 uvB = lightPosB.xy * 0.5f + 0.5f;
    float depthB = lightPosB.z;
    float mapDepthB = g_ShadowTextureB.SampleLevel(DefaultSampler, uvB, 0.0f).r;
    if (depthB + 0.005f < mapDepthB)
        shadow = min(shadow, 0.5f);

    // C 캐스케이드
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
    // NDC 좌표 계산
    float z_ndc = g_PBR_Depth.SampleLevel(DefaultSampler, uv, 0.0f).x;
    float z_view = g_PBR_Depth.SampleLevel(DefaultSampler, uv, 0.0f).y * 1000.0f;

    float4 ndcPos;
    ndcPos.x = uv.x * 2.0f - 1.0f;
    ndcPos.y = (1.0f - uv.y * 2.0f);
    ndcPos.z = z_ndc;
    ndcPos.w = 1.0f;

    // Projection 역변환
    float4 viewPos = mul(ndcPos, g_ProjMatrixInv);
    viewPos /= viewPos.w;

    // ViewZ 보정
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
    
    /* mul : 모든 행렬의 곱하기를 수행한다. /w연산을 수행하지 않는다. */
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
    
    /* 로컬위치 * 월드 *뷰 * 투영 / w : 투영공간상의 위치. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
     /* 로컬위치 * 월드 *뷰 * 투영  */
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
    
    /* 로컬위치 * 월드 *뷰 * 투영 / w : 투영공간상의 위치. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
     /* 로컬위치 * 월드 *뷰 * 투영  */
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
    
    /* [ 텍스처 임포트 ] */
    vector vDiffuseDesc = g_PBR_Diffuse.Sample(DefaultSampler, In.vTexcoord);
    vector vNormalDesc = g_PBR_Normal.Sample(DefaultSampler, In.vTexcoord);
    vector vARMDesc = g_PBR_ARM.Sample(DefaultSampler, In.vTexcoord);
    vector vDepthDesc = g_PBR_Depth.Sample(DefaultSampler, In.vTexcoord);
    
    /* [ 활용할 변수 정리 ] */
    float3 Albedo = vDiffuseDesc.rgb;
    float3 Normal = normalize(vNormalDesc.rgb * 2.0f - 1.0f);
    float AO = vARMDesc.r;
    float Roughness = vARMDesc.g;
    float Metallic = vARMDesc.b;
    float3 Ambient = Albedo * 0.1f * AO;
    
    // [ ViewPos 복원 ]
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
    viewPos *= viewZ / viewPos.z; // View Z 보정

    float4 worldPos = mul(viewPos, g_ViewMatrixInv);
    
    /* [ 빛 계산 ] */
    float3 V = normalize(g_vCamPosition.xyz - worldPos.xyz);
    float3 L = normalize(g_vLightDir.xyz);
    float3 H = normalize(V + L);
    
    /* [ 필요한 내적 공식 ] */
    float NdotL = saturate(dot(Normal, L));
    float NdotV = saturate(dot(Normal, V));
    float NdotH = saturate(dot(Normal, H));
    float VdotH = saturate(dot(V, H));

    /* [ 프레넬 반사율(금속) ] */
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), Albedo, Metallic);
    float3 F = F0 + (1.0f - F0) * pow(1.0f - VdotH, 5.0f);
    F *= vDepthDesc.z;

    /* [ GGX 에너지반사 공식 ] */
    float a = Roughness * Roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH * (a2 - 1.0f) + 1.0f);
    float D = a2 / (PI * denom * denom + 0.001f);

    /* [ 지오메트리 함수 공식 ] */
    float k = (Roughness + 1.0f);
    k = (k * k) / 8.0f;
    float G_V = NdotV / (NdotV * (1.0f - k) + k);
    float G_L = NdotL / (NdotL * (1.0f - k) + k);
    float G = G_V * G_L;

    /* [ 공식의 결과 스펙큘러 ] */
    float3 Specular = D * G * F / (4.0f * NdotV * NdotL + 0.001f);
    Specular *= g_vLightSpecular.rgb;
    Specular *= vDepthDesc.w;

    /* [ 디퓨즈 색상 결정 ] */
    float3 kD = (1.0f - F) * (1.0f - Metallic);
    float3 Diffuse = kD * Albedo / PI;

    /* [ 라이트의 색상 ] */
    float3 radiance = g_vLightDiffuse.rgb;
    radiance *= g_fLightIntencity;
    radiance *= 3.5f;

    /* [ 최종 PBR 조명 계산 ] */
    float3 FinalColor = (Diffuse + Specular) * radiance * NdotL * AO + Ambient;
    float3 Specalur = Specular * radiance;

    Out.vFinal = float4(FinalColor, vDiffuseDesc.a);
    Out.vSpecular = float4(Specular, 1.0f);
    
    return Out;
}
PS_OUT_PBR PS_PBR_LIGHT_POINT(PS_IN In)
{
    PS_OUT_PBR Out;

    // [ 텍스처 샘플링 ]
    vector vDiffuseDesc = g_PBR_Diffuse.Sample(DefaultSampler, In.vTexcoord);
    vector vNormalDesc = g_PBR_Normal.Sample(DefaultSampler, In.vTexcoord);
    vector vARMDesc = g_PBR_ARM.Sample(DefaultSampler, In.vTexcoord);
    vector vDepthDesc = g_PBR_Depth.Sample(DefaultSampler, In.vTexcoord);

    // [ 변수 정리 ]
    float3 Albedo = vDiffuseDesc.rgb;
    float3 Normal = normalize(vNormalDesc.rgb * 2.0f - 1.0f);
    float AO = vARMDesc.r;
    float Roughness = vARMDesc.g;
    float Metallic = vARMDesc.b;
    float3 Ambient = Albedo * g_fLightAmbient * AO;

    // [ ViewPos 복원 ]
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
    viewPos *= viewZ / viewPos.z; // View Z 보정

    float4 worldPos = mul(viewPos, g_ViewMatrixInv);

    // [ 라이트 방향 및 감쇠 ]
    //float3 L_unormalized = g_vLightPos.xyz - worldPos.xyz;
    float3 L_unormalized = worldPos.xyz - g_vLightPos.xyz;
    float distance = length(L_unormalized);
    float3 L = normalize(L_unormalized);
    
    float fAtt = saturate(1.0 - pow(distance / g_fLightRange, 4.0));

    // [ 뷰, 하프 벡터 ]
    float3 V = normalize(g_vCamPosition.xyz - worldPos.xyz);
    float3 H = normalize(V + L);

    float NdotL = saturate(dot(Normal, L));
    float NdotV = saturate(dot(Normal, V));
    float NdotH = saturate(dot(Normal, H));
    float VdotH = saturate(dot(V, H));

    // [ 프레넬 ]
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

    // [ 라이트 색상 ]
    float3 radiance = g_vLightDiffuse.rgb;
    radiance *= g_fLightIntencity;
    radiance *= 3.5f;

    // [ 최종 조명 ]
    float3 FinalColor = (Diffuse + Specular) * radiance * NdotL * AO * fAtt;//    +Ambient;
    float3 Specalur = Specular * radiance;

    Out.vFinal = float4(FinalColor, vDiffuseDesc.a);
    Out.vSpecular = float4(Specular, 1.0f);
    
    return Out;
}
PS_OUT_PBR PS_PBR_LIGHT_SPOT(PS_IN In)
{
    PS_OUT_PBR Out;

    // [ 텍스처 샘플링 ]
    vector vDiffuseDesc = g_PBR_Diffuse.Sample(DefaultSampler, In.vTexcoord);
    vector vNormalDesc = g_PBR_Normal.Sample(DefaultSampler, In.vTexcoord);
    vector vARMDesc = g_PBR_ARM.Sample(DefaultSampler, In.vTexcoord);
    vector vDepthDesc = g_PBR_Depth.Sample(DefaultSampler, In.vTexcoord);

    // [ 변수 정리 ]
    float3 Albedo = vDiffuseDesc.rgb;
    float3 Normal = normalize(vNormalDesc.rgb * 2.0f - 1.0f);
    float AO = vARMDesc.r;
    float Roughness = vARMDesc.g;
    float Metallic = vARMDesc.b;
    float3 Ambient = Albedo * g_fLightAmbient * AO;

    // [ ViewPos 복원 ]
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
    viewPos *= viewZ / viewPos.z; // View Z 보정

    float4 worldPos = mul(viewPos, g_ViewMatrixInv);

    // [ 라이트 방향 및 감쇠 ]
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
    
    // [ 뷰, 하프 벡터 ]
    float3 V = normalize(g_vCamPosition.xyz - worldPos.xyz);
    float3 H = normalize(V + L);

    float NdotL = saturate(dot(Normal, L));
    float NdotV = saturate(dot(Normal, V));
    float NdotH = saturate(dot(Normal, H));
    float VdotH = saturate(dot(V, H));

    // [ 프레넬 ]
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

    // [ 라이트 색상 ]
    float3 radiance = g_vLightDiffuse.rgb;
    radiance *= g_fLightIntencity;
    radiance *= 3.5f;
    
    // [ 최종 조명 ]
    float3 FinalColor = (Diffuse + Specular) * radiance * NdotL * AO * fAtt;//    +Ambient;
    float3 Specalur = Specular * radiance;
    
    Out.vFinal = float4(FinalColor, vDiffuseDesc.a);
    Out.vSpecular = float4(Specular, 1.0f);
    return Out;
}

PS_OUT_VOLUMETRIC PS_VOLUMETRIC_POINT(PS_IN In)
{
    PS_OUT_VOLUMETRIC Out;

    // [ 텍스처 샘플링 ]
    vector vDepthDesc = g_PBR_Depth.Sample(DefaultSampler, In.vTexcoord);

    // [ 해당 뷰포트의 깊이값 포함 ViewPos ]
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
    // 카메라 정면 방향 * 최대 거리
        viewPos.xyz = normalize(viewPos.xyz) * 50.0f;
    }
    else
    {
        float scaleZ = viewZ / max(viewPos.z, 0.0001f);
        viewPos *= scaleZ;
    }   

    /* [ Volumetric Raymarching 기법 ] */
    float4 ViewSpacePosition = viewPos;
    
    //float StepSize = 0.5f;
    static const int NumStep = 100;
    float StepSize = viewZ / NumStep;
    // -----------------------------------------
    // 여기부터 Light 방향 볼륨광 추가
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
        // 레이 도달지점을 월드 좌표로 변환
        RayPos += RayDir * StepSize;
        float3 worldPos = mul(float4(RayPos, 1.0f), g_ViewMatrixInv).xyz;
        
        // 매쉬의 차폐 여부 확인
        if (!bNoMeshBehind)
        {
            if (!bBlocked && RayPos.z > viewZ + 0.01f)
                bBlocked = true;
        
            if (bBlocked)
                continue;
        }
        
        /* [ 포그 최대거리 컷팅 ] */
        float distanceToLight = length(g_vLightPos.xyz - worldPos);
        float softFalloff = saturate(1.0f - (distanceToLight / g_fFogCutoff));

        float density = 0.3f; //SampleFogDensity(worldPos, g_fTime);
        float shadow = SampleShadowMap(worldPos);
        
        float transmittance = 1.0f - shadow;
        int NumLights = max(g_iPointNum, 1);
        LightFog += density * transmittance * softFalloff * StepSize * 0.05f;
    }
    
    // -----------------------------------------
    // 결과 조합
    // -----------------------------------------

    float3 fogColor = g_vLightDiffuse.rgb;
    float3 finalFog = fogColor * (LightFog * g_fFogPower);
    
    Out.vVolumetric = float4(finalFog, 1.f);
    
    return Out;
}
PS_OUT_VOLUMETRIC PS_VOLUMETRIC_DIRECTIONAL(PS_IN In)
{
    PS_OUT_VOLUMETRIC Out;

    // [ 텍스처 샘플링 ]
    vector vDepthDesc = g_PBR_Depth.Sample(DefaultSampler, In.vTexcoord);

    // [ 해당 뷰포트의 깊이값 포함 ViewPos ]
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

    /* [ Volumetric Raymarching 기법 ] */
    float4 ViewSpacePosition = viewPos;

    float StepSize = 0.25f;
    static const int NumStep = 50;

    float LightFog = 0.0f;

    float3 PixelWorldPos = mul(float4(ViewSpacePosition.xyz, 1.0f), g_ViewMatrixInv).xyz;

    // 디렉셔널 라이트는 광원이 아닌 광선 방향만 있음
    float3 RayDirWorld = g_vLightDir.xyz;
    float3 RayOriginWorld = PixelWorldPos;
    //float3 RayOriginWorld = PixelWorldPos - g_vLightDir.xyz * 30.0f;
    float3 RayPosWorld = RayOriginWorld;

    bool bBlocked = false;
    for (int j = 0; j < NumStep; ++j)
    {
        RayPosWorld += RayDirWorld * StepSize;
        float3 sampleViewPos = mul(float4(RayPosWorld, 1.0f), g_ViewMatrix).xyz;

        // 메쉬 차폐
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

    // [ 텍스처 샘플링 ]
    vector vDepthDesc = g_PBR_Depth.Sample(DefaultSampler, In.vTexcoord);

    // [ 해당 뷰포트의 깊이값 포함 ViewPos ]
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

    /* [ Volumetric Raymarching 기법 ] */
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

        // [ 차폐 체크 유지 ]
        if (!bNoMeshBehind)
        {
            if (!bBlocked && RayPos.z > viewZ + 0.01f)
                bBlocked = true;

            if (bBlocked)
                continue;
        }

        // [ 스포트라이트 원뿔 감쇠 계산 ]
        float3 toPos = worldPos - g_vLightPos.xyz;
        float distanceToLight = length(toPos);
        float3 dirToPos = toPos / max(distanceToLight, 0.001f);

        float cosAngle = dot(-g_vLightDir.xyz, dirToPos);

        // [ 범위 바깥은 무시 ]
        float fOuterCosAngle = g_fOuterCosAngle;
        if (cosAngle < fOuterCosAngle)
            continue;

        // [ 원뿔 감쇠 ]
        float spotFalloff = saturate((cosAngle - fOuterCosAngle) / max(g_fInnerCosAngle - fOuterCosAngle, 0.001f));

        // [ 최대 거리 감쇠 ]
        float softFalloff = saturate(1.0f - (distanceToLight / g_fFogCutoff));

        // [ 밀도 + 그림자 ]
        float density = 0.3f; //SampleFogDensity(worldPos, g_fTime);
        float shadow = SampleShadowMap(worldPos);
        float transmittance = 1.0f - shadow;

        // [ 누적 ]
        LightFog += density * transmittance * softFalloff * spotFalloff * StepSize * 0.05f;        
        
    }
    
    
    // 정규화 적용
    float3 fogColor = g_vLightDiffuse.rgb;
    float3 finalFog = fogColor * (LightFog * g_fFogPower * 100.f / max(viewZ, 1.0f));
    
    Out.vVolumetric = float4(finalFog, 1.f);
    return Out;
}

PS_OUT PS_MAIN_DEFERRED(PS_IN In)
{
    PS_OUT Out;
    
    vector finalColor = vector(0.f, 0.f, 0.f, 0.f);
    
    /* [ 기존 VTXMesh ] */
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vSpecular = g_SpecularTexture.Sample(DefaultSampler, In.vTexcoord);
    Out.vBackBuffer = vDiffuse * vShade + vSpecular;   
    finalColor = Out.vBackBuffer;
    
    /* [ PBR 매쉬 ] */
    vector vPBRFinal = g_PBR_Final.Sample(DefaultSampler, In.vTexcoord);
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vVolumetric = g_VolumetricTexture.Sample(DefaultSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 1000.f;
    if (vPBRFinal.a > 0.01f)
        Out.vBackBuffer = vPBRFinal;
    finalColor = Out.vBackBuffer;
    
    
    /* [ 이펙트 + 글로우 ] */
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

    //데칼 입히기
    vector vDecal = g_DecalTexture.Sample(DefaultSampler, In.vTexcoord);
    finalColor.rgb = finalColor.rgb * (1 - vDecal.a) + vDecal.rgb * vDecal.a;
    
    Out.vBackBuffer = finalColor;

    
    // === 지수 감쇠 방식 적용 ===
    float fogDensity = 0.02f;
    float attenuation = exp(-fViewZ * fogDensity);
    float3 fogColor = vVolumetric.rgb * (1.0 - attenuation);

    Out.vBackBuffer.rgb += fogColor;

    
    /* [ 뷰포트상의 깊이값 복원 ] */
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

    // --- 깊이 비교 ---
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
    /* 기타 잡기술 */
    Out.vBackBuffer.rgb *= 1.5f;
    Out.vBackBuffer.a *= 1.2f;
    // 밖에서 값 변경해가면서 볼 수 있으면 좋겠는데 당장 안 될 것 같으므로 고정함
    
    return Out;
}

PS_OUT PS_WB_COMPOSITE(PS_IN In)
{
    PS_OUT Out;
    
    /* [ 이쪽이 맞는 것 같은데 나눗셈 연산이 쉽지 않음 ] */
    //vector vAccum = g_WB_Accumulation.Sample(DefaultSampler, In.vTexcoord);
    //float4 fReveal = g_WB_Revealage.Sample(DefaultSampler, In.vTexcoord);
    //
    //float3 vColor = vAccum.rgb / max(saturate(vAccum.a), 0.00001f); // 0 나누기 방지용
    //float fAlpha = 1 - saturate(fReveal.r);
    //Out.vBackBuffer = float4(vColor * fAlpha, fAlpha);
    
    /********************************************************************/

    /* [ 컴팩트 버전 ] */
    /* [ Reveal의 값에 이미 각 픽셀에 적용 될 알파의 상태가 계산되어 있으므로 거리 별 가중치를 고려하지 않고 사용 ] */
    /* [ 근데 이러면 Weighted Blend OIT라고 하긴 뭐하고 그냥 Order-Independent만 지킨 느낌이라 고쳐야 할 듯... 언젠가... ] */
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
    pass Downscale_Copy //14 다운스케일 용으로 만들었는데 그냥 카피만 해주는 패스임
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
