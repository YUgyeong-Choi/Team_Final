#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

/******  Sprite Grid variables  ******/
float2 g_fTileSize; // tile size(1/tilecnt)
float2 g_fTileOffset;
bool g_bFlipUV;


/******  Texture variables  ******/
Texture2D g_DiffuseTexture;
Texture2D g_MaskTexture1;
Texture2D g_MaskTexture2;
Texture2D g_MaskTexture3;

Texture2D g_DepthTexture;


/******  Color variables  ******/
vector g_vColor = { 1.f, 1.f, 1.f, 1.f }; // Default Color
float g_fThreshold; 
float g_fIntensity;
vector g_vCenterColor;


/******  WeightedBlend variables  ******/
float g_fEmissiveIntensity = 0.f;
float g_fWeightPower = 5.f;

float g_fTime;


/******  Basic Functions  ******/

float2 FlipUV_90(float2 vInTexcoord)
{
    if (g_bFlipUV != 0)
    {
    // 90도 회전 (시계 방향)
        float2 center = float2(0.5f, 0.5f);
        float2 offset = vInTexcoord - center;

    // Rotate 90 deg CW → [x, y] -> [-y, x]
        float2 rotatedUV;
        rotatedUV.x = -offset.y;
        rotatedUV.y = offset.x;

        vInTexcoord = rotatedUV + center;
    }
    return vInTexcoord;
}

float2 UVTexcoord(float2 vInTexcoord, float2 fTileSize, float2 fTileOffset)
{
    vInTexcoord = FlipUV_90(vInTexcoord);
    
    float2 baseUV = vInTexcoord * fTileSize;
    
    float2 finalUV = baseUV + fTileOffset.xy;
    
    return finalUV;
}

// if texture has no tileset, just put In.Texcoord only
float2 UVTexcoord(float2 vInTexcoord)
{
    vInTexcoord = FlipUV_90(vInTexcoord);
    
    return vInTexcoord;
}

// 타일링 + 오프셋 + 스크롤 포함
float2 UVTileScroll(float2 uv, float2 tile, float2 offset, float2 scrollSpeed, float time)
{
    uv = FlipUV_90(uv);
    float2 scrolled = uv + scrollSpeed * time;
    return scrolled * tile + offset;
}

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