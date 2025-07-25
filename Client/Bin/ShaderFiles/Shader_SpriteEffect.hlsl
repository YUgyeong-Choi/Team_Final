#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_Texture;
Texture2D g_DepthTexture;
Texture2D g_MaskTexture;

float4  g_vColor = { 1.f, 1.f, 1.f, 1.f };
float   g_fOpacity = 1.f;
float   g_fPercentage = 1.f;

/******  Sprite Grid variables  ******/
float2  g_fTileSize;     // Ÿ�� �ϳ� ������
float2  g_fTileOffset;   // Ÿ�� ��ġ



struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    /* SV_ : ShaderValue���� */
    /* ���� �ؾ��� ������ �� ������ ���� ��(��ġ)�� �˾Ƽ� �߰����� ������ �ض�. */     
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

struct VS_OUT_BLEND
{  
    float4 vPosition : SV_POSITION;    
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

VS_OUT_BLEND VS_MAIN_BLEND(VS_IN In)
{
    VS_OUT_BLEND Out;
    
    matrix matWV, matWVP;
    
    /* mul : ��� ����� ���ϱ⸦ �����Ѵ�. /w������ �������� �ʴ´�. */
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = Out.vPosition;
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
    vector vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;    
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;    
}

struct PS_IN_BLEND
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;    
};

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


PS_OUT PS_MAIN_SOFTEFFECT(PS_IN_BLEND In)
{
    PS_OUT Out;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vColor = SoftEffect(Out.vColor, In.vProjPos);
    
    return Out;
}

PS_OUT PS_MAIN_GRID(PS_IN_BLEND In)
{
    PS_OUT Out;

    float2 baseUV = In.vTexcoord * g_fTileSize;
    
    float2 finalUV = baseUV + g_fTileOffset.xy;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, finalUV);
    
    Out.vColor = SoftEffect(Out.vColor, In.vProjPos);
    
    return Out;
}

technique11 DefaultTechnique
{
    pass Default    // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();    
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();      
    }
    pass SoftEffect // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);        

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SOFTEFFECT();
    }
    pass UVSprite   // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_GRID();
    }
}
