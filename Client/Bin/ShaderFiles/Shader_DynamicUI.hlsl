#include "Engine_Shader_Defines.hlsli"

/* ������̺� ConstantTable */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_Texture;

texture2D g_BackgroundTexture;
texture2D g_GradationTexture;

texture2D g_HoverTexture;
texture2D g_HighlightTexture;

float2 g_fTexcoord;
float2 g_fTileSize;
float g_Alpha;
float4 g_Color;

float4 g_ButtonFlag;

float g_CurrentBarRatio;
float g_BarRatio;
float4 g_ManaDesc;

texture2D g_ItemTexture;
texture2D g_InputTexture;
float4 g_ItemDesc;

float g_Groggy;
float g_UVTime;


float g_IsPlayer;
float g_IsHpBar;
float g_UseGradation;
float g_fHpEffectTime;

float g_IsDurablityBar;
float g_IsIncrease;


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
    
    In.vTexcoord.x += g_UVTime;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vProjPos = In.vProjPos;
    float4 vPos = In.vPosition;
    
    Out.vColor *= g_Color;
   
    Out.vColor.a *= g_Alpha;
    
    return Out;
}

PS_OUT PS_MAIN_DISCARD_DARK(PS_IN In)
{
    PS_OUT Out;
    
    In.vTexcoord.x -= g_UVTime;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    float brightness = dot(Out.vColor.rgb, float3(0.2126, 0.7152, 0.0722));

   // 0.2 ���ϸ� ����, 0.3 �̻��̸� �״��
    float alphaFactor = smoothstep(0.2f, 0.5f, brightness);

    // ���� ���� �״��
    Out.vColor.rgb *= g_Color.rgb;

    // ���ĸ� ���̵� ����
    Out.vColor.a *= g_Alpha * alphaFactor * 0.4f;

    return Out;
    
}

PS_OUT PS_MAIN_SPRITE(PS_IN In)
{
    PS_OUT Out;
    
    
    float2 finalUV = g_fTexcoord + In.vTexcoord * g_fTileSize;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, finalUV);
    
    if (Out.vColor.a < 0.1f)
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
    
    if (g_ButtonFlag.x == 1.f)
    {
        vTexture = g_Texture.Sample(DefaultSampler, In.vTexcoord);

    }
    
    if (g_ButtonFlag.y == 1.f)
    {
        float2 vTexcoord = In.vTexcoord;
        vTexcoord.x *= 2.5f;
        
        vHover = g_HoverTexture.Sample(DefaultSampler, vTexcoord);
        vHover.a = 0.2f;

    }
    
    if (g_ButtonFlag.z == 1.f)
    {
        if (In.vTexcoord.y > 0.95f)
        {
            vHighlight = g_HighlightTexture.Sample(DefaultSampler, In.vTexcoord - float2(0.f, 0.4f));
           
        }

    }
    
    Out.vColor = vTexture + vHighlight;
    
    if (Out.vColor.a < 0.001f)
    {
        if (length(vHover.rgb) > 0.5f)
        {
            
            if (In.vTexcoord.x < 0.4f)
            {
                Out.vColor += vHover;

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
    Out.vColor = float4(0, 0, 0, 0);

    float4 vBorder = g_Texture.Sample(PointSampler, In.vTexcoord);
    float2 highlightUV;
// ����ϰ� ��� ����
    highlightUV.x = (In.vTexcoord.x - 0.5f) * 0.9f + 0.5f;
    highlightUV.y = (In.vTexcoord.y - 0.5f) * 0.5f + 0.5f;

 

    float fMarginX = 0.085f;
    float fMarginY = 0.25f;

// ä���� �� �ִ� X ����
    float minX = fMarginX * 1.15f;
    float maxX = 1 - 1.15f * fMarginX;

// ������ ���� ���� ä������ ��ġ
    float filledX = 0.f;
    
    if(g_BarRatio > 0.f)
    {
        float minWidth = 0.f;
        if (g_IsHpBar == 1.f)
        {
            minWidth = 0.05f;
        }
        
        filledX = minX + max((maxX - minX) * g_BarRatio , minWidth);
    }
    float filledCurX = minX + max((maxX - minX) * g_CurrentBarRatio, 0.05f);
 

    bool isInsideX = 0;
    bool isInsideY = In.vTexcoord.y >= fMarginY && In.vTexcoord.y <= 1 - fMarginY ;

 
    if (vBorder.a > 0.001f)
    {
        Out.vColor += vBorder * 0.8f;
        return Out;
    }
    
    vector vGradation = g_GradationTexture.Sample(PointSampler, In.vTexcoord);
    
    if (g_IsPlayer == 1.f && g_IsHpBar == 1.f)
    {
        float pixelSize = 1 / 192.f; // ���� ü�� �� �ȼ� ũ��
        // ü�� ���� ��
        if (filledX > filledCurX)
        {
            bool isInsideXMain = In.vTexcoord.x >= minX && In.vTexcoord.x <= filledCurX;
            bool isInsideXSub = In.vTexcoord.x >= filledCurX && In.vTexcoord.x <= filledX;


            
            if (isInsideXMain && isInsideY)
            {
                Out.vColor = g_Color * (length(vGradation.rgb) * 0.5 + 0.5f);
                if (g_fHpEffectTime > 0.f && In.vTexcoord.x > filledCurX - pixelSize * 0.6f )
                    Out.vColor = float4(1.f, 1.f, 1.f, 1.f);
            }
            else if (isInsideXSub && isInsideY)
            {
                Out.vColor = g_Color * 0.5f; 
            }

          
                
        }
        // ü�� ���� ��
        else if (filledX < filledCurX)
        {
            bool isInsideXMain = In.vTexcoord.x >= minX && In.vTexcoord.x <= filledX;
            bool isInsideXSub = In.vTexcoord.x >= filledX && In.vTexcoord.x <= filledCurX;
            
          
              
            if (isInsideXMain && isInsideY)
            {
                Out.vColor = g_Color * (length(vGradation.rgb) * 0.5 + 0.5f);
                
                if (g_fHpEffectTime > 0.f && In.vTexcoord.x > filledX - pixelSize * 0.6f)
                    Out.vColor = float4(1.f, 1.f, 1.f, 1.f);
            }
            else if (isInsideXSub && isInsideY)
            {
                if(In.vTexcoord.x < minX)
                    discard;
                
                Out.vColor = g_Color * (length(vGradation.rgb) * 0.5 + 0.5f) * 0.5f; // ��ο� ����
            }

          
            
            
           

        }
        else
        {
            isInsideX = In.vTexcoord.x >= minX && In.vTexcoord.x <= filledX;
            
            if (isInsideX && isInsideY)
            {
                Out.vColor = g_Color * (length(vGradation.rgb) * 0.5 + 0.5f);
                
                
                if (g_fHpEffectTime > 0.f)
                {
                    if (In.vTexcoord.x > filledX - pixelSize * 0.6f)       
                        Out.vColor = float4(1.f, 1.f, 1.f, 1.f);
                }
            }

        }
      
            
          
        return Out;
    }
    
    isInsideX = In.vTexcoord.x >= minX && In.vTexcoord.x <= filledX;

    if (isInsideX && isInsideY)
    {
     
         
            Out.vColor = g_Color * (length(vGradation.rgb) * 0.5 + 0.5f);
        
        
           
    }

    return Out;
}

PS_OUT PS_MAIN_MANABAR(PS_IN In)
{
    PS_OUT Out;

    float unitCount = g_ManaDesc.x; // �� ĭ ��
    float margin = g_ManaDesc.y * 1.2f; // ĭ ���� ���� (0~0.5)
    int fullUnits = (int) g_ManaDesc.z; // �� �� ĭ ����
    float remainRatio = g_ManaDesc.w; // ������ ĭ�� ���� (0~1)

    float fMarginY = 0.3f;

// Y ���� ���̸� ����
    if (In.vTexcoord.y < fMarginY || In.vTexcoord.y > 1.0f - fMarginY)
        discard;

// �� ĭ �ʺ�
    float unitWidth = 1.0 / unitCount;

// ���� �ȼ��� ���� ĭ �ε���
    int unitIndex = (int) (In.vTexcoord.x / unitWidth);

// ĭ �� ��� ��ġ (0~1)
    float localX = (In.vTexcoord.x - unitIndex * unitWidth) / unitWidth;
    float localY = (In.vTexcoord.y - fMarginY) / (1.0f - 2.0f * fMarginY);

// ���� ���� ������
    if (localX < margin || localX > 1.0f - margin)
        discard;

// ���� ���ø�
    float2 borderUV = float2(localX, localY);
    vector vBorder = g_Texture.Sample(DefaultSampler, borderUV);
    if (vBorder.a > 0.1f)
    {
        Out.vColor = vBorder;
        return Out;
    }

// �׵θ� ���� ���� ���
    float borderThickness = 0.1f;

    float fillStartX = borderThickness * 0.7f;
    float fillEndX = 1.0f - borderThickness * 0.7f;

    float fillStartY = borderThickness * 3.f;
    float fillEndY = 1.0f - borderThickness * 3.f;

// ���� ������ �ƴϸ� ����
    if (localX < fillStartX || localX > fillEndX ||
    localY < fillStartY || localY > fillEndY)
    {
        discard;
    }

// ĭ�� ä�� ����
    float fillAmount = 0.0f;
    if (unitIndex < fullUnits)
    {
        fillAmount = fillEndX;
    }
    else if (unitIndex == fullUnits)
    {
        fillAmount = fillStartX + remainRatio * (fillEndX - fillStartX) * 0.9f;
    }
    else
    {
        fillAmount = fillStartX;
    }

// ä�� ���� �Ѿ�� ����
    if (localX > fillAmount)
        discard;

// ä��� �� ���
   
    vector vGradation = g_GradationTexture.Sample(DefaultSampler, borderUV);
    if (unitIndex < fullUnits)
    {
    // �� �� ĭ�� �׶��̼� ���ϰ� (��: 1.0 ~ 1.5 ��)
        vector vGradation = g_GradationTexture.Sample(DefaultSampler, borderUV);
        
        Out.vColor = g_Color * (length(vGradation.rgb) * 0.3 + 0.7f);

    }
    else
    {
        Out.vColor = g_Color * 0.6f;

    }
   

    return Out;
}

PS_OUT PS_MAIN_ITEM_ICON(PS_IN In)
{
    PS_OUT Out;
    
    vector vBack = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    vector vHighlight = g_HighlightTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vInput = g_InputTexture.Sample(DefaultSampler, In.vTexcoord);

    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    if (vBack.a < 0.01f)
        discard;
    
  
    
    if (g_ItemDesc.x == 1.f)
    {
        Out.vColor = vBack + vHighlight * float4(1.f, 0.f, 0.f, 0.5f);

    }
    
    if (g_ItemDesc.y == 1.f)
    {
        float2 center = float2(0.5f, 0.5f); // �ؽ�ó �߾�
        float scale = 0.65f; // 0.5�� ��� 
    
        float2 localUV = (In.vTexcoord - center) / scale + center;

        if (localUV.x >= 0.f && localUV.x <= 1.f &&
        localUV.y >= 0.f && localUV.y <= 1.f)
        {
            vector vSmallItem = g_ItemTexture.Sample(DefaultSampler, localUV);

            if (vSmallItem.a > 0.01f)
            {
                float glowStrength = smoothstep(0.15f, 0.6f, vSmallItem.a); // �ε巴�� �ö�
                vector glow = vSmallItem * glowStrength;

                Out.vColor += glow;
            }
        }
    }
    
    if (g_ItemDesc.z == 1.f)
    {
        Out.vColor += vInput * (1.f - g_ItemDesc.w) * 0.7f;
    }
  
    
    Out.vColor *= g_Color;
  
    
    return Out;
}


PS_OUT PS_MAIN_DURABILITYBAR(PS_IN In)
{
    PS_OUT Out;
    Out.vColor = float4(0.f, 0.f, 0.f, 0.f);

   
    vector vBorder = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    vector vBack = g_BackgroundTexture.Sample(DefaultSampler, In.vTexcoord);

    
    float fMarginX = 0.097f;
    float fMarginY = 0.097f;

// ä���� �� �ִ� X ����
    float minX = fMarginX;
    float maxX = 1 - fMarginX * 0.97f;

    // Y ���� ���̸� ����
    if (In.vTexcoord.y < fMarginY * 1.5f  || In.vTexcoord.y > 1.0f - fMarginY * 1.07f )
        discard;
    
    if (In.vTexcoord.x < fMarginX * 0.9f || In.vTexcoord.x > 1.0f - fMarginX *  0.9f)
        discard;
    
    if (vBorder.a > 0.01f)
    {
        Out.vColor = vBorder * 0.9f;
        return Out;
    }
    
    float filledX = minX + (maxX - minX) * g_BarRatio;

    bool isInsideX = In.vTexcoord.x >= minX && In.vTexcoord.x <= filledX;
    bool isInsideY = In.vTexcoord.y >= fMarginY && In.vTexcoord.y <= 1 - fMarginY;

    float borderThickness = 0.1f;

    float fillStartY = borderThickness * 2.75f;
    float fillEndY = 1.0f - borderThickness * 2.7f;


    if (!isInsideX ||
    In.vTexcoord.y < fillStartY || In.vTexcoord.y > fillEndY)
    {
        Out.vColor = float4(0.f, 0.f, 0.f, 0.7f);

    }
    else
    {
        
        if (isInsideX)
        {
            if (g_UseGradation == 1.f)
            {
                float2 vTexcoord;
                vTexcoord.x = In.vTexcoord.x;
                vTexcoord.y = In.vTexcoord.y * 0.5f + 0.5f;
                vector vGradation = g_GradationTexture.Sample(DefaultSampler, vTexcoord);
                Out.vColor = g_Color * (length(vGradation.rgb)   + 0.5f);
            }
            else
            {
                if (g_IsIncrease == 1.f && g_IsDurablityBar == 1.f)
                {
                    // ���κп� �׶��̼��� �־��
                    if (In.vTexcoord.x >= filledX - 0.075f)
                    {
                        if (In.vTexcoord.x >= filledX - 0.01f)
                            Out.vColor = float4(1.f, 1.f, 1.f, 1.f);
                        else
                        {
                            float t = saturate((In.vTexcoord.x - filledX + 0.1f) / 0.15f);
                            Out.vColor = lerp(float4(0.8f, 0.5f, 0.5f, 1.f), float4(1.f, 1.f, 1.f, 1.f), t);
                            Out.vColor.a *= 1.5f;
                            
                        }
                        
                        
             
                        
                    }
                    else
                    {
                        
                        
                        Out.vColor = g_Color;
                    }

                }
                else
                {
                    Out.vColor = g_Color;
                }
            }
                
        }
          
        else
            Out.vColor = float4(0.f, 0.f, 0.f, 0.7f);
    }

    return Out;
}

PS_OUT PS_MAIN_HPBAR_MONSTER(PS_IN In)
{
    PS_OUT Out;
    Out.vColor = float4(0, 0, 0, 0);

    float4 vBorder = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    float2 highlightUV;
// ����ϰ� ��� ����
    highlightUV.x = saturate((In.vTexcoord.x - 0.46f)   + 0.5f);
    highlightUV.y = saturate((In.vTexcoord.y - 0.5f) * 0.2f + 0.5f);

    float4 vHighlight = (g_Groggy == 1.f) ? g_HighlightTexture.Sample(DefaultSampler, highlightUV) : float4(0, 0, 0, 0);

    float fMarginX = 0.09f;
    float fMarginY = 0.25f;

// ä���� �� �ִ� X ����
    float minX = fMarginX;
    float maxX = 1 - 1.05f * fMarginX;

// ������ ���� ���� ä������ ��ġ
    
    float filledX = 0.f;
    
    if (g_BarRatio > 0.f)
    {
        float minWidth = 0.03f;
        
        
        filledX = minX + max((maxX - minX) * g_BarRatio, minWidth);
    }

    bool isInsideX = In.vTexcoord.x >= minX && In.vTexcoord.x <= filledX;
    bool isInsideY = In.vTexcoord.y >= fMarginY && In.vTexcoord.y <= 1 - fMarginY;


    if (vHighlight.a > 0.001f)
    {
       
        Out.vColor += vHighlight * 2.f;
    }
    
    if (vBorder.a > 0.1f)
    {
        if ((In.vTexcoord.x >= minX * 1.0375f) && (In.vTexcoord.x <= (maxX + 0.0015f )))
            Out.vColor = float4(0.5f,0.5f,0.5f,0.3f);
        
        return Out;
    }

    if (isInsideX && isInsideY)
    {
        vector vGradation = g_GradationTexture.Sample(DefaultSampler, In.vTexcoord);
        Out.vColor = g_Color * (length(vGradation.rgb) * 0.5 + 0.5f);
    }
  

    return Out;
}

PS_OUT PS_MAIN_DISCARD_ALPAH_REVERSE(PS_IN In)
{
    PS_OUT Out;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
   
    
    if(length(Out.vColor.rgb) > 0.4f)
        discard;

    Out.vColor.rgb = float3(0.f, 0.f, 0.f);
    Out.vColor.a = 1.f;
    
    
    Out.vColor *= g_Color;
    Out.vColor.a *= g_Alpha;
    
    return Out;
}


PS_OUT PS_MAIN_ACTION_ICON(PS_IN In)
{
    PS_OUT Out;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    if (Out.vColor.a < 0.001f)
        discard;
    
    vector vHighlight = g_HighlightTexture.Sample(DefaultSampler, In.vTexcoord);

    

  

    if (g_ItemDesc.x == 1.f)
    {
     
       

        Out.vColor.rgb *= vHighlight.a;

    // y ��ǥ ��� �׶��̼� (0~1)
        float redFactor = saturate((In.vTexcoord.y - 0.4f) * 2.0f);
    
        redFactor *= 0.2f;

    // ���� ���� ������ �����ֱ�
        float3 red = float3(0.2f, 0.f, 0.f);
        Out.vColor.rgb = lerp(Out.vColor.rgb , red, redFactor);
   
    }
    else
    {
        // ���� ���
        Out.vColor.rgb *= vHighlight.a;
    }

    Out.vColor *= g_Color;
    
    return Out;
}

PS_OUT PS_MAIN_REVIVE(PS_IN In)
{
    PS_OUT Out;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vColor *= g_Alpha;
   
    
    if (length(Out.vColor.rgb) > 0.5f)
        discard;

    Out.vColor.rgb = float3(0.f, 0.f, 0.f);
    Out.vColor.a = 1.f;
    
    
    Out.vColor *= g_Color;
    
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
    pass Default /* ��� + ����ŧ�� + �׸��� + ssao + ������Ʈ */
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
    pass ManaBar
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);


        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_MANABAR();
    }
    pass IconItem
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ITEM_ICON();

    }
    pass Durability
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DURABILITYBAR();
    }
   
    pass Monster_HP_Bar
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_HPBAR_MONSTER();
    }

    pass Back
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);


        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass BackGround_Reverse
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);


        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DISCARD_ALPAH_REVERSE();
    }

    pass Action_Icon
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);


        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ACTION_ICON();
    }

    pass Revive
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);


        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_REVIVE();
    }
    pass Discard_Dark
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);


        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DISCARD_DARK();
    }

 }
