#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;

float g_fID;

bool g_bTile = false;
float2 g_TileDensity = float2(1.0f, 1.0f); // 1m당 1회 반복

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
    vector vPickPos : SV_TARGET3;
};

struct PS_SKY_OUT
{
    vector vDiffuse : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;

    vector vMtrlDiffuse;
    vector vNormalDesc;

    if (g_bTile)
    {
        // Triplanar Mapping 적용
        float3 worldNormal = normalize(In.vNormal);
        float3 blend = abs(worldNormal);
        blend = pow(blend, 3.0);
        blend /= (blend.x + blend.y + blend.z); // Normalize

        float2 uvXZ = In.vWorldPos.zx * g_TileDensity; // Y축 수직 면 (바닥)
        float2 uvXY = In.vWorldPos.xy * g_TileDensity; // Z축 수직 면 (정면)
        float2 uvYZ = In.vWorldPos.zy * g_TileDensity; // X축 수직 면 (측면)

        float4 diffXZ = g_DiffuseTexture.Sample(DefaultSampler, uvXZ);
        float4 diffXY = g_DiffuseTexture.Sample(DefaultSampler, uvXY);
        float4 diffYZ = g_DiffuseTexture.Sample(DefaultSampler, uvYZ);

        float4 normXZ = g_NormalTexture.Sample(DefaultSampler, uvXZ);
        float4 normXY = g_NormalTexture.Sample(DefaultSampler, uvXY);
        float4 normYZ = g_NormalTexture.Sample(DefaultSampler, uvYZ);

        vMtrlDiffuse = diffXZ * blend.y + diffXY * blend.z + diffYZ * blend.x;
        vNormalDesc = normXZ * blend.y + normXY * blend.z + normYZ * blend.x;
    }
    else // 기본 UV 방식
    {
        vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    }

    // 알파 테스트
    if (vMtrlDiffuse.a < 0.3f)
        discard;

    // Normal Map 변환 [-1, 1]
    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    // Tangent Space → World Space
    float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz, In.vNormal.xyz);
    vNormal = mul(vNormal, WorldMatrix);

    // 출력
    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f); // encode to [0,1]
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.0f, 0.f, 0.f);
    Out.vPickPos = In.vWorldPos;

    return Out;
}


PS_OUT PS_MAPTOOLOBJECT(PS_IN In)
{
    PS_OUT Out;

    vector vMtrlDiffuse;
    vector vNormalDesc;

    if (g_bTile)
    {
        // Triplanar Mapping: 법선 방향 기반 타일링
        float3 worldNormal = normalize(In.vNormal);
        float3 blend = abs(worldNormal);
        blend = pow(blend, 3.0f); // 부드러운 블렌딩을 위해 거듭제곱
        blend /= (blend.x + blend.y + blend.z); // 정규화

        float2 uvXZ = In.vWorldPos.zx * g_TileDensity; // 바닥
        float2 uvXY = In.vWorldPos.xy * g_TileDensity; // 천장
        float2 uvYZ = In.vWorldPos.zy * g_TileDensity; // 벽

        float4 diffXZ = g_DiffuseTexture.Sample(DefaultSampler, uvXZ);
        float4 diffXY = g_DiffuseTexture.Sample(DefaultSampler, uvXY);
        float4 diffYZ = g_DiffuseTexture.Sample(DefaultSampler, uvYZ);

        float4 normXZ = g_NormalTexture.Sample(DefaultSampler, uvXZ);
        float4 normXY = g_NormalTexture.Sample(DefaultSampler, uvXY);
        float4 normYZ = g_NormalTexture.Sample(DefaultSampler, uvYZ);

        // 블렌딩
        vMtrlDiffuse = diffXZ * blend.y + diffXY * blend.z + diffYZ * blend.x;
        vNormalDesc = normXZ * blend.y + normXY * blend.z + normYZ * blend.x;
    }
    else
    {
        // 기본 UV 기반 텍스처 샘플링
        vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    }

    // 알파 테스트
    if (vMtrlDiffuse.a < 0.3f)
        discard;

    // 노멀 맵 [-1, 1]로 변환
    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    // Tangent → World 변환
    float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz, In.vNormal.xyz);
    vNormal = mul(vNormal, WorldMatrix);

    // 출력
    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f); // Encode to [0,1]
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 500.0f, 0.f, g_fID); // w: ID 저장
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
