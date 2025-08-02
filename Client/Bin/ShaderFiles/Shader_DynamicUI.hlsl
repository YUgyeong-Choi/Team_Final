#include "Engine_Shader_Defines.hlsli"

/* ������̺� ConstantTable */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_Texture;

texture2D g_BackgroundTexture;

texture2D g_HoverTexture;
texture2D g_HighlightTexture;

float2   g_fTexcoord;
float2   g_fTileSize;
float    g_Alpha;
float4   g_Color;

float4   g_ButtonFlag;

float    g_BarRatio;
/* ������ �������� ��ȯ (���庯ȯ, ��, ������ȯ) */ 
/* ������ ���� ������ ������ �� �ִ�. */ 

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


//POSITION�ø�ƽ�� ����
//��������� ���ؼ�

/* W������ ������ �����Ѵ�. */
/* ����Ʈ�� ��ȯ�Ѵ�. */
/* ��� ���п� ���ؼ� �����Ͷ�����. -> �ȼ��� �����Ѵ�. */


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
    
    Out.vColor *= g_Color;
    
    return Out;    
}

struct PS_IN_BLEND
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;    
};


PS_OUT PS_MAIN_BLEND(PS_IN_BLEND In)
{
    PS_OUT Out;
    
   
    
    return Out;
}

PS_OUT PS_MAIN_DISCARD_DARK(PS_IN In)
{
    PS_OUT Out;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    if(length(Out.vColor.rgb) < 0.2f)
        discard;
    
    Out.vColor *= g_Color;
    
    return Out;
}

PS_OUT PS_MAIN_SPRITE(PS_IN In)
{
    PS_OUT Out;
    
    
    float2 finalUV = g_fTexcoord + In.vTexcoord * g_fTileSize;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, finalUV);
    
    if(Out.vColor.a <0.1f)
        discard;
   
    Out.vColor.a = g_Alpha;
    
    Out.vColor *= g_Color;
    
    return Out;
}

PS_OUT PS_MAIN_FADE(PS_IN In)
{
    PS_OUT Out;
    
    Out.vColor = g_Texture.Sample(LinearClampSampler, In.vTexcoord);
    
    if (Out.vColor.a < 0.001f)
        discard;
   
    Out.vColor.a = g_Alpha;
    
    Out.vColor *= g_Color;
    
    return Out;
}


PS_OUT PS_MAIN_BUTTON(PS_IN In)
{
    PS_OUT Out;
    
    float4 vTexture = { 0.f, 0.f, 0.f, 0.f };
    float4 vHover = { 0.f, 0.f, 0.f, 0.f };
    float4 vHighlight = { 0.f, 0.f, 0.f, 0.f };
    
    if(g_ButtonFlag.x == 1.f)
    {
        vTexture = g_Texture.Sample(DefaultSampler, In.vTexcoord);

    }
    
    if(g_ButtonFlag.y == 1.f)
    {
        float2 vTexcoord = In.vTexcoord;
        vTexcoord.x *= 2.5f;
        
        vHover = g_HoverTexture.Sample(DefaultSampler, vTexcoord);
        vHover.a = 0.2f;

    }
    
    if(g_ButtonFlag.z == 1.f)
    {
        if (In.vTexcoord.y > 0.95f)
        {
            vHighlight = g_HighlightTexture.Sample(DefaultSampler, In.vTexcoord - float2(0.f, 0.4f));
           
        }

    }
    
    Out.vColor  = vTexture + vHighlight ;
    
    if (Out.vColor.a < 0.001f)
    {
        if (length(vHover.rgb) > 0.5f)
        {
            
            if(In.vTexcoord.x < 0.4f)
            {
                Out.vColor += vHover ;

            }
              
           
        }
       
    }
    else
    {
        Out.vColor.a *= g_Alpha;
    
        Out.vColor *= g_Color;
    }
    
   
   
    
  
    
    return Out;
}

PS_OUT PS_MAIN_HPBAR(PS_IN In)
{
    PS_OUT Out;
 
    vector vBorder = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    vector vBack = g_BackgroundTexture.Sample(DefaultSampler, In.vTexcoord);
    
    
  
    if (vBorder.a > 0.1f)
    {
        Out.vColor = vBorder;
        return Out;
    }
    
    float fMarginX = 0.06f;
    float fMarginY = 0.3f;
    bool isInsideX;
    
    if (g_BarRatio > 1 - 1.2 * fMarginX)
        isInsideX = In.vTexcoord.x >= fMarginX && In.vTexcoord.x <= 1 - 1.2 * fMarginX;
    else
        isInsideX = In.vTexcoord.x >= fMarginX && In.vTexcoord.x <= g_BarRatio;
    bool isInsideY = In.vTexcoord.y >= fMarginY && In.vTexcoord.y <= 1 - fMarginY;
  
    if (isInsideX && isInsideY)
    {
        Out.vColor = g_Color;

        Out.vColor * g_Alpha;

    }
    else
    {
        discard;
    }

    return Out;
}



technique11 DefaultTechnique
{ 
    /* �н��� �����ϴ� ������ ����? */ 
    /* ���� ���� �׸��� ���� �ٸ� �������������� �Կ����ϰų�. 
    ������ �ٸ� ���̵� ����� �����ؾ��ϰų� */ 

    /* �� ���̴��� � �������� �����Ұ���.  */ 
    /* � ���̴��� ����Ұ���? */ 
    /* ������ �Լ� ���� */
    /* �������������� ���� ����*/ 
    pass Default/* ��� + ����ŧ�� + �׸��� + ssao + ������Ʈ */ 
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();    
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();      
    }
    pass SoftEffect
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);        

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLEND();
    }

    pass Sprite
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SPRITE();
    }


    pass Fade
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FADE();
    }

    pass Button
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BUTTON();
    }
    pass HPBar
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);


        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_HPBAR();
    }
   
}
