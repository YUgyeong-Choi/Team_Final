#include "Engine_Shader_Defines.hlsli"
#include "Effect_Shader_Defines.hlsli"


Texture2D g_Texture;

float   g_fOpacity = 1.f;
float   g_fPercentage = 1.f;


struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    /* SV_ : ShaderValue약자 */
    /* 내가 해야할 연산은 다 했으니 이제 니(장치)가 알아서 추가적인 연산을 해라. */     
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
    
    /* mul : 모든 행렬의 곱하기를 수행한다. /w연산을 수행하지 않는다. */
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



vector ColorAdjustment_Multiply(float4 vOrigColor, float4 vMultiplyColor)
{
    vector vResColor =  vOrigColor * vMultiplyColor;
    
    return vResColor;
}


vector ColorAdjustment_Hue(float4 vOrigColor, float4 vMultiplyColor)
{
    vector vResColor =  vOrigColor * vMultiplyColor;
    
    return vResColor;
}


float GetHue(float3 rgb)
{
    float cmax = max(rgb.r, max(rgb.g, rgb.b));
    float cmin = min(rgb.r, min(rgb.g, rgb.b));
    float delta = cmax - cmin;

    float hue = 0.0f;

    if (delta > 0.00001f)
    {
        if (cmax == rgb.r)
            hue = fmod((rgb.g - rgb.b) / delta, 6.0f);
        else if (cmax == rgb.g)
            hue = ((rgb.b - rgb.r) / delta) + 2.0f;
        else
            hue = ((rgb.r - rgb.g) / delta) + 4.0f;

        hue /= 6.0f;
        if (hue < 0)
            hue += 1.0f;
    }

    return hue; // 0.0 ~ 1.0
}




PS_OUT PS_MAIN_SOFTEFFECT(PS_IN_BLEND In)
{
    PS_OUT Out;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, UVTexcoord(In.vTexcoord));
    
    Out.vColor = SoftEffect(Out.vColor, In.vProjPos);
    
    return Out;
}

PS_OUT PS_MAIN_GRID(PS_IN_BLEND In)
{
    PS_OUT Out;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset));
    if (Out.vColor.a <= 0.01f)
        discard;
    
    Out.vColor = SoftEffect(Out.vColor, In.vProjPos);
    
    return Out;
}

PS_OUT PS_MAIN_GRID_COLOR(PS_IN_BLEND In)
{
    PS_OUT Out;    
    Out.vColor = g_Texture.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset));
    if (Out.vColor.a <= 0.01f)
        discard;
    
    Out.vColor = ColorAdjustment_Multiply(Out.vColor, g_vColor);
    
    Out.vColor = SoftEffect(Out.vColor, In.vProjPos);
    
    return Out;
}

technique11 DefaultTechnique
{
    pass Default            // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();    
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();      
    }
    pass SoftEffectOnly     // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);        

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SOFTEFFECT();
    }
    pass UVSprite_Default   // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_GRID();
    }
    pass UVSprite_Coloring  // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_GRID_COLOR();
    }
}
