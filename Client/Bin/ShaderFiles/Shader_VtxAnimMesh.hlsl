
#include "Engine_Shader_Defines.hlsli"
#pragma pack_matrix(row_major)
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_WorldMatrixInvTrans;
float4 g_CamposWS;

matrix g_BoneMatrices[512];
matrix g_BoneMatrices2[256];

//
//// 모든 뼈 컴바인드 매트릭스
//StructuredBuffer<matrix> g_FinalBoneMatrices : register(t0);
//
//// 메시별 로컬 뼈 인덱스
//StructuredBuffer<uint>     g_LocalToGlobal    : register(t1);
//
//// 지금 메시의 뼈 오프셋들
//StructuredBuffer<matrix> g_Offsets          : register(t2);
Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_ARMTexture;
Texture2D g_Emissive;
Texture2D g_NoiseMap;

/* [ 조절용 파라미터 ] */
float g_fDiffuseIntensity = 1;
float g_fNormalIntensity = 1;
float g_fAOIntensity = 1;
float g_fAOPower = 1;
float g_fRoughnessIntensity = 1;
float g_fMetallicIntensity = 1;
float g_fReflectionIntensity = 1;
float g_fSpecularIntensity = 1;
float g_fEmissiveIntensity = 1;
float g_fLimLightIntensity = 0;
vector g_vDiffuseTint = { 1.f, 1.f, 1.f, 1.f };

/* [ BurnTextures ] */
Texture2D g_Burn;
Texture2D g_BurnMask;
Texture2D g_BurnMask2;
float g_fBurnPhase;
float g_fBurnTime;

/* [ LimLight ] */
float4 g_fLimLightColor = { 1.f, 0.f, 0.f, 1.f };
bool g_bUseLimLight = false;
float g_RimPower = 2.f;
float g_fBandStart = 0.75f;
float g_fBandEnd = 0.8f;

/* [ 피킹변수 ] */
float g_fID;


/* [ 공용 디졸브 함수 ] */
Texture2D g_MaskTexture;
bool g_bDissolve = false;
float g_fDissolveAmount = 0.5f;
float g_fDissolveRange = 0.03f;
float3 g_vDissolveGlowColor = float3(1.0f, 0.8f, 0.2f);

struct SDissolveResult
{
    float fClip;
    float fEdge;
    float fGlow;
    float3 vAddColor;
};

static SDissolveResult DoDissolve_NoArgs(float2 vTexcoord)
{   
    const float fSafeRange = max(g_fDissolveRange, 1e-6f);

    SDissolveResult t;
   
    const float fMask = g_MaskTexture.Sample(DefaultSampler, vTexcoord * 0.15f).r;
    
    const float fClipTerm = (g_fDissolveAmount + fSafeRange) - fMask;
    t.fClip = fClipTerm;
    
    if (g_fDissolveAmount < 0.03f)
        t.fClip = -1.0f;
    
    const float fCenter = g_fDissolveAmount;
    t.fEdge = 1.0f - smoothstep(fCenter - fSafeRange, fCenter + fSafeRange, fMask);
    
    t.fGlow = saturate(1.0f - abs(fMask - g_fDissolveAmount) / fSafeRange);
    t.vAddColor = g_vDissolveGlowColor * t.fGlow;

    return t;
}

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

struct VS_OUT_PBR
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

VS_OUT_PBR VS_MAIN(VS_IN In)
{
    VS_OUT_PBR Out;
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
    Out.vTangent = normalize(mul(vector(In.vTangent, 0.f), g_WorldMatrix));
    Out.vBinormal = normalize(cross(Out.vNormal.xyz, Out.vTangent.xyz));
    
    return Out;
}


VS_OUT_PBR VS_LIMLIGHT(VS_IN In)
{
    VS_OUT_PBR Out = (VS_OUT_PBR) 0;

    // 1) 스킨 가중치
    float fWeightW = saturate(1.0f - (In.vBlendWeights.x + In.vBlendWeights.y + In.vBlendWeights.z));

    // 2) 스킨 매트릭스 (가중합)
    float4x4 mSkin =
          g_BoneMatrices[In.vBlendIndices.x] * In.vBlendWeights.x
        + g_BoneMatrices[In.vBlendIndices.y] * In.vBlendWeights.y
        + g_BoneMatrices[In.vBlendIndices.z] * In.vBlendWeights.z
        + g_BoneMatrices[In.vBlendIndices.w] * fWeightW;

    // 3) 스킨 적용
    float4 vPosSkinned = mul(float4(In.vPosition, 1.0f), mSkin);
    float3 vNrmSkinned = normalize(mul((float3x3) mSkin, In.vNormal));
    float3 vTanSkinned = normalize(mul((float3x3) mSkin, In.vTangent.xyz));

    // 4) 월드 변환
    float4 vPosWS = mul(vPosSkinned, g_WorldMatrix);

    float3x3 mN = (float3x3) g_WorldMatrixInvTrans;
    float3 vNrmWS = normalize(mul(vNrmSkinned, mN));
    float3 vTanWS = normalize(mul(vTanSkinned, mN));

    // 5) 클립 좌표
    float4 vPosH = mul(mul(vPosWS, g_ViewMatrix), g_ProjMatrix);

    // 6) 출력 (PS 림마스크가 쓰는 값들)
    Out.vPosition = vPosH;
    Out.vProjPos = vPosH;
    Out.vWorldPos = vPosWS;
    Out.vNormal = float4(vNrmWS, 1.f);
    Out.vTexcoord = In.vTexcoord;

    // (옵션) 기존 파이프 호환용
    Out.vTangent = float4(vTanWS, 1.f);
    Out.vBinormal = normalize(cross(Out.vNormal.xyz, Out.vTangent.xyz));

    return Out;
}

VS_OUT VS_INNERLINE(VS_IN In)
{
    /* 기타 변환들을 수행한다.*/
    VS_OUT Out = (VS_OUT) 0;
    
    float fWeightW = 1.f - (In.vBlendWeights.x + In.vBlendWeights.y + In.vBlendWeights.z);


    matrix BoneMatrix = g_BoneMatrices[In.vBlendIndices.x] * In.vBlendWeights.x +
        g_BoneMatrices[In.vBlendIndices.y] * In.vBlendWeights.y +
        g_BoneMatrices[In.vBlendIndices.z] * In.vBlendWeights.z +
        g_BoneMatrices[In.vBlendIndices.w] * fWeightW;

    vector vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);
    vector vNormal = mul(vector(In.vNormal, 0.f), BoneMatrix);
    
    vPosition -= normalize(vNormal) * 0.045f;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(vector(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
    
    return Out;
}


VS_OUT VS_PICK(VS_IN In)
{
    VS_OUT Out;
    
    float fWeightW = 1.f - (In.vBlendWeights.x + In.vBlendWeights.y + In.vBlendWeights.z);
        
    matrix BoneMatrix = g_BoneMatrices[In.vBlendIndices.x] * In.vBlendWeights.x +
        g_BoneMatrices[In.vBlendIndices.y] * In.vBlendWeights.y +
        g_BoneMatrices[In.vBlendIndices.z] * In.vBlendWeights.z +
        g_BoneMatrices[In.vBlendIndices.w] * fWeightW;
    
    vector vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);
    vector vNormal = mul(vector(In.vNormal, 0.f), BoneMatrix);
    
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

struct PS_IN_PBR
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

struct PS_OUT_PBR
{
    vector vDiffuse : SV_TARGET0;
    vector vNormal : SV_TARGET1;
    vector vARM : SV_TARGET2;
    vector vProjPos : SV_TARGET3;
    vector vAO : SV_TARGET4;
    vector vRoughness : SV_TARGET5;
    vector vMetallic : SV_TARGET6;
    vector vEmissive : SV_TARGET7;
};


struct PS_OUT_PICK
{
    vector vDiffuse : SV_TARGET0;
    vector vNormal : SV_TARGET1;
    vector vARM : SV_TARGET2;
    vector vDepth : SV_TARGET3;
    
};

struct PS_OUT_LINE
{
    vector vLimLight : SV_TARGET0;
    vector vInnerLine : SV_TARGET1;
};

PS_OUT_PBR PS_MAIN(PS_IN_PBR In)
{
    PS_OUT_PBR Out;
    
    SDissolveResult tD = (SDissolveResult) 0;
    if (g_bDissolve)
    {
        tD = DoDissolve_NoArgs(In.vTexcoord);
        clip(tD.fClip); // 디스카드 처리
    }
    
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
    float AO = pow(abs(vARM.r), g_fAOPower) * g_fAOIntensity;
    float Roughness = vARM.g * g_fRoughnessIntensity;
    float Metallic = vARM.b * g_fMetallicIntensity;
    
    // 이미시브 텍스처
    vector vEmissive = g_Emissive.Sample(DefaultSampler, In.vTexcoord);
   
    float fIsUnit = 0.f;
    Out.vDiffuse = float4(vMtrlDiffuse.rgb * g_fDiffuseIntensity * g_vDiffuseTint.rgb, vMtrlDiffuse.a);
    if (g_bDissolve)
        Out.vDiffuse.rgb += tD.vAddColor;
    Out.vNormal = float4(normalize(vWorldNormal) * 0.5f + 0.5f, 1.f);
    Out.vARM = float4(AO, Roughness, Metallic, fIsUnit);
    Out.vProjPos = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, g_fReflectionIntensity, g_fSpecularIntensity);
    Out.vAO = float4(AO, AO, AO, 1.f);
    Out.vRoughness = float4(Roughness, Roughness, Roughness, 1.0f);
    Out.vMetallic = float4(Metallic, Metallic, Metallic, 1.0f);
    Out.vEmissive = float4(vEmissive.rgb * g_fEmissiveIntensity, 0.f);
    if (g_bDissolve)
        Out.vEmissive.rgb += tD.vAddColor;
    
    return Out;
}

PS_OUT_PICK PS_PICK(PS_IN In)
{
    PS_OUT_PICK Out;
    
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.3f)
        discard;
    
    Out.vDiffuse = vMtrlDiffuse;
    
    /* -1.f -> 0.f, 1.f -> 1.f */    
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
     
    // ARM 텍스처
    float3 vARM = g_ARMTexture.Sample(DefaultSampler, In.vTexcoord).rgb;
    float AO = pow(abs(vARM.r), g_fAOPower) * g_fAOIntensity;
    float Roughness = vARM.g * g_fRoughnessIntensity;
    float Metallic = vARM.b * g_fMetallicIntensity;
    
    Out.vARM = float4(AO, Roughness, Metallic, 1.f);
    
    //Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.f, g_fID);
    
    return Out;
}

struct PS_IN_SHADOW_OLD
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
};

struct PS_OUT_SHADOW_OLD
{
    vector vShadow : SV_TARGET0;
};

PS_OUT_SHADOW_OLD PS_MAIN_SHADOW(PS_IN_SHADOW_OLD In)
{
    PS_OUT_SHADOW_OLD Out;
    
    Out.vShadow = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.f, 0.f);
    
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

float4 PS_Cascade0(VS_OUT_SHADOW In) : SV_TARGET0
{
    float depthZ = In.vProjPos.z / In.vProjPos.w;
    float depthW = In.vProjPos.w / 1000.0f;
    return float4(depthZ, depthW, 0.f, 0.f);
}

// SV_TARGET1에만 쓰는 버전
float4 PS_Cascade1(VS_OUT_SHADOW In) : SV_TARGET1
{
    float depthZ = In.vProjPos.z / In.vProjPos.w;
    float depthW = In.vProjPos.w / 1000.0f;
    return float4(depthZ, depthW, 0.f, 0.f);
}

// SV_TARGET2에만 쓰는 버전
float4 PS_Cascade2(VS_OUT_SHADOW In) : SV_TARGET2
{
    float depthZ = In.vProjPos.z / In.vProjPos.w;
    float depthW = In.vProjPos.w / 1000.0f;
    //return float4(depthZ, depthW, 0.f, 0.f);
    return float4(1.f, 0.f, 0.f, 1.f);
}


float4 PS_LIMLIGHT(PS_IN_PBR In) : SV_Target0
{
    float4 Out;
    // --- 기본 파라미터 ---
    //float4 vEdgeColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
    float4 vEdgeColor = g_fLimLightColor;
    float fNoiseScale = 0.5f;
    float fNoiseContrast = 0.9f;
    float fEdgeSoftness = 3.12f;
    float fScrollSpeed = 0.0f; 
    
    float fTimeSeconds = 0.0f; 
    float fRimPower = g_RimPower;
    float fBandStart = g_fBandStart;
    float fBandEnd = g_fBandEnd;
    
    // WS 노말 → VS, 위치도 VS로 변환해서 수치 안정
    float3 vPosVS = mul(In.vWorldPos, g_ViewMatrix).xyz;
    float3 vViewVS = normalize(-vPosVS);

    float3 vNrmWS = normalize(In.vNormal.xyz);
    float3 vNrmVS = normalize(mul(float4(vNrmWS, 0.0f), g_ViewMatrix).xyz);

    // 지오메트리 노말(면 노말) 섞어서 고주파 완화
    float3 vNgWS = normalize(cross(ddx(In.vWorldPos.xyz), ddy(In.vWorldPos.xyz)));
    float3 vNgVS = normalize(mul(float4(vNgWS, 0.0f), g_ViewMatrix).xyz);
    vNrmVS = normalize(lerp(vNgVS, vNrmVS, 0.15f)); // 0.4~0.7 사이 튜닝

    // 동일(뷰) 공간에서 림 계산
    float fDotRaw = dot(vNrmVS, vViewVS);
    float fRimRaw = 1.0f - saturate(abs(fDotRaw));
    float fRim = pow(fRimRaw, fRimPower);
    
    float fBandWidth = 0.14f;

    // dot / rimRaw의 파생폭도 함께 고려 + 최소폭 보장
    float fWdot = max(fwidth(fDotRaw) * 1.0f, 1e-3f);
    float fWraw = max(fwidth(fRimRaw) * 2.0f, 1e-3f);
    float fW = max(fwidth(fRim) * 2.0f + fWdot + fWraw, 0.01f);

    float fUp = smoothstep(fBandStart - fW, fBandEnd + fW, fRim);
    float fOut = smoothstep(fBandEnd + fW, fBandEnd + fBandWidth + 2.0f * fW, fRim);
    float fRimBand = saturate(fUp - fOut);
    
    
    if (g_bUseLimLight)
    {
        // --- 노이즈 마스크 --- 
        float2 vNoiseUV = In.vTexcoord * fNoiseScale + fTimeSeconds * fScrollSpeed * float2(0.17f, -0.09f);
        float fNoise = g_NoiseMap.Sample(DefaultSampler, vNoiseUV).r;
        fNoise = pow(saturate(fNoise), max(0.001f, fNoiseContrast));
        float fNoiseMask = smoothstep(0.5f - fEdgeSoftness, 0.5f + fEdgeSoftness, fNoise);
        float fMask = saturate(fRimBand * fNoiseMask);
        float fA = vEdgeColor.a * fMask;
    
        float fAmbientStrength = 0.2f;
        float3 vAmbient = vEdgeColor.rgb * fAmbientStrength;
    
        Out = float4(vEdgeColor.rgb * g_fLimLightIntensity, fMask * g_fLimLightIntensity);
        Out += float4(vAmbient, 0.2f);
    }
    else
    {
        Out = float4(vEdgeColor.rgb * g_fLimLightIntensity, fRimBand * g_fLimLightIntensity);
    }
    
    //Out = float4(fRimBand.xxx, 1);
    return Out;
    
}

float4 PS_INNERLINE(PS_IN In) : SV_Target1
{
    float4 Out;
    
    float4 vEdgeColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
    float fNoiseScale = 0.5f;
    float fNoiseContrast = 3.0f;
    float fEdgeSoftness = 0.06f;
    float fScrollSpeed = 2.0f;
    float fGlowBoost = 0.0f;
    float fTimeSeconds = 0.0f;
    
    float2 vNoiseUV = In.vTexcoord * fNoiseScale
                    + fTimeSeconds * fScrollSpeed * float2(0.17f, -0.09f);
    
    float fNoise = g_NoiseMap.Sample(DefaultSampler, vNoiseUV).r;
    fNoise = pow(saturate(fNoise), max(0.001f, fNoiseContrast));
    
    float fMask = smoothstep(0.5f - fEdgeSoftness, 0.5f + fEdgeSoftness, fNoise);
    
    float3 vEdgeRGB = vEdgeColor.rgb * (1.0f + fGlowBoost * fMask);
    float fAlpha = vEdgeColor.a * fMask;

    Out = float4(vEdgeRGB, fAlpha);
    Out *= g_fLimLightIntensity;
    
    return Out;
    
}

struct PS_OUT_BURN
{
    float4 vBurn : SV_TARGET0;
    float4 vBlack : SV_TARGET1;
};
PS_OUT_BURN PS_BURN(PS_IN_PBR In)
{
    PS_OUT_BURN Out;
    float4 vBurnTexture = g_Burn.Sample(DefaultSampler, In.vTexcoord);
    float fTimeSeconds = g_fBurnTime;
    
    float fMaskTiling = 3.1f;
    float fMaskScrollU = 0.07f;
    float fMaskScrollV = 0.07f;
    
    float fWobbleAmp = 0.015f;
    float fWobbleFreq = 9.0f;
    float fWobbleSpeed = 1.7f; 
    
    float fNoiseTiling = 3.0f;
    float fNoiseAmp = 0.010f;
    float fNoiseScroll = 0.4f;
    
    float fEdgeWidth = 0.08f;
    float fBurnPhase = g_fBurnPhase;
    float fBurnPhaseNormalize = saturate(fBurnPhase / 0.25f);
    
    float4 vBlackMask = g_BurnMask2.Sample(DefaultSampler, In.vTexcoord * 1.f);
    float4 vBlackInverse = (1.0f - vBlackMask) * fBurnPhaseNormalize;
    
    float2 vMaskUV = In.vTexcoord * fMaskTiling
               + float2(fMaskScrollU, fMaskScrollV) * fTimeSeconds;
    
    float2 vWobble;
    vWobble.x = sin(vMaskUV.y * fWobbleFreq + fTimeSeconds * fWobbleSpeed);
    vWobble.y = cos(vMaskUV.x * fWobbleFreq - fTimeSeconds * fWobbleSpeed);
    vMaskUV += vWobble * fWobbleAmp;
    
    if (fNoiseAmp > 0.0f)
    {
        float fNoise = g_Burn.Sample(DefaultSampler,
                                     In.vTexcoord * fNoiseTiling + fTimeSeconds * fNoiseScroll).b;
        vMaskUV += (fNoise - 0.5f) * fNoiseAmp;
    }
    
    float4 vBurnMask = g_BurnMask.Sample(DefaultSampler, vMaskUV);
    float fSeed = vBurnMask.r;
    
    float fReveal = 1.0f - smoothstep(fBurnPhase - fEdgeWidth, fBurnPhase + fEdgeWidth, fSeed);
    fReveal += vBlackInverse.r;
    
    Out.vBurn.rgb = vBurnTexture.rgb * fReveal;
    Out.vBurn.a = fReveal;
    
    
    const float fSootStrength = 1.f;
    float fEdgeMask = saturate(1.0f - abs(fSeed - fBurnPhase) / 0.5f);
    
    
    float4 vBlackTexture = g_BurnMask2.Sample(DefaultSampler, In.vTexcoord * 3.f);
   
    Out.vBlack = lerp(float4(1.0f, 1.0f, 1.0f, 1.0f), vBlackTexture, fBurnPhaseNormalize);
    Out.vBlack.rgb *= (1.0f - fEdgeMask * fSootStrength * fBurnPhaseNormalize);
    
    return Out;
}


technique11 DefaultTechnique
{   
    pass Default //0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();   
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();      
    }

    pass MonsterToolObject //1 (피킹 해서 아이디 얻어올 수 있다.)
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_PICK();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_PICK();
    }

    pass Shadow // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN_SHADOW();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
    }
    pass ShadowPass0 //3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_SHADOW();
        PixelShader = compile ps_5_0 PS_Cascade0(); // SV_TARGET0 전용
    }

    pass ShadowPass1 //4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_SHADOW();
        PixelShader = compile ps_5_0 PS_Cascade1(); // SV_TARGET1 전용
    }

    pass ShadowPass2 //5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_SHADOW();
        PixelShader = compile ps_5_0 PS_Cascade2(); // SV_TARGET2 전용
    }
   
    pass RimLight //6
    {
        SetRasterizerState(RS_Inner);
        SetDepthStencilState(DSS_Inner, 0);
        SetBlendState(BS_AlphaBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_LIMLIGHT();
        PixelShader = compile ps_5_0 PS_LIMLIGHT();
    }

    pass InnerLine //7
    { 
        SetRasterizerState(RS_Inner);
        SetDepthStencilState(DSS_Inner, 0);
        SetBlendState(BS_OneBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_INNERLINE();
        PixelShader = compile ps_5_0 PS_INNERLINE(); 
    }

    pass Burn //8
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BURN();
    }

    pass Default_CullNone //9
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
   
}
