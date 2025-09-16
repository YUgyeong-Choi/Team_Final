#include "Effect_Shader_Defines.hlsli"


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
    
    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;    
}

struct PS_IN_BLEND
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;    
};


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
    
    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, UVTexcoord(In.vTexcoord));
    
    Out.vColor = SoftEffect(Out.vColor, In.vProjPos);
    
    return Out;
}

PS_OUT PS_MAIN_GRID(PS_IN_BLEND In)
{
    PS_OUT Out;
    
    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset));
    if (Out.vColor.a <= 0.01f)
        discard;
    
    Out.vColor = SoftEffect(Out.vColor, In.vProjPos);
    
    return Out;
}

PS_OUT PS_MAIN_GRID_COLOR(PS_IN_BLEND In)
{
    PS_OUT Out;    
    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset));
    if (Out.vColor.a <= 0.0003f)
        discard;
    
    Out.vColor = ColorAdjustment_Multiply(Out.vColor, g_vColor);
    
    Out.vColor = SoftEffect(Out.vColor, In.vProjPos);
    
    return Out;
}

struct PS_OUT_EFFECT_WB
{
    vector vAccumulation : SV_TARGET0;
    vector fRevealage : SV_TARGET1;
    vector vEmissive : SV_TARGET2;
    vector vDistortion : SV_TARGET3;
};

PS_OUT_EFFECT_WB PS_MAIN_GRID_COLOR_WB(PS_IN_BLEND In)
{
    PS_OUT_EFFECT_WB Out = (PS_OUT_EFFECT_WB)0;
    vector vColor = g_DiffuseTexture.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset));
    if (vColor.a <= 0.003f)
        discard;

    vColor = ColorAdjustment_Multiply(vColor, g_vColor);

    vColor = SoftEffect(vColor, In.vProjPos);


    float3 vPremulRGB = vColor.rgb * vColor.a;
    Out.vAccumulation = float4(vPremulRGB, vColor.a);   
    Out.fRevealage = vColor.a;
    Out.vEmissive = float4(vPremulRGB * g_fEmissiveIntensity, 0.f);

    /*****/
    
    //float z_ndc = saturate(In.vProjPos.z / In.vProjPos.w); // D3D는 0..1
    //// 1 - 깊이 = 멀 수록 연하게
    //float fWeight = pow(1.0f - z_ndc, 0.5f);
    //
    //
    ////float fWeight = max(1.0 - z_ndc, 0.05f); // pow 대신 선형도 권장
    ////float fWeight = 1.f;
    //float3 vPremulRGB = vColor.rgb * vColor.a;
    //Out.vAccumulation = float4(vPremulRGB * fWeight, vColor.a * fWeight);
    //
    //Out.fRevealage = vColor.aaaa;
    //Out.vEmissive = float4(vPremulRGB * fWeight * g_fEmissiveIntensity, 0.f);
    
    return Out;
}

PS_OUT_EFFECT_WB PS_MAIN_MASKONLY_CWB(PS_IN_BLEND In)
{
    PS_OUT_EFFECT_WB Out = (PS_OUT_EFFECT_WB) 0;


    float mask = g_MaskTexture1.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset)).r;
    if (mask < 0.003f)
        discard;
    //float noise = g_MaskTexture2.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset)).r;
    float4 vPreColor;
    float lerpFactor = saturate((mask - g_fThreshold) / (1.f - g_fThreshold));
    
    vPreColor = lerp(g_vColor, g_vCenterColor, lerpFactor);
    
    vector vColor;
    
    vColor = SoftEffect(vPreColor, In.vProjPos);

    vColor.rgb = vPreColor.rgb * mask * g_fIntensity;
    vColor.a = g_vColor.a * mask;
    

    float3 vPremulRGB = vColor.rgb * vColor.a;
    Out.vAccumulation = float4(vPremulRGB, vColor.a);
    Out.fRevealage = vColor.a;
    Out.vEmissive = float4(vPremulRGB * g_fEmissiveIntensity, 0.f);

    return Out;
}

PS_OUT_EFFECT_WB PS_MAIN_MASKDISSOLVE_CWB(PS_IN_BLEND In)
{
    PS_OUT_EFFECT_WB Out = (PS_OUT_EFFECT_WB) 0;


    vector noise = g_MaskTexture2.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset));

    float2 dir = noise.rg * 2.0 - 1.0; // [-1,1]
    float mag = lerp(1.0, noise.b, 0.5); // B를 세기로 활용(옵션)
    
    float2 flowUV = saturate(UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset) + dir * frac(g_fTime * 0.5f) * mag);
    
    
    float mask = g_MaskTexture1.Sample(DefaultSampler, flowUV).r;
    if (mask < 0.003f)
        discard;
    
    float4 vPreColor;
    float lerpFactor = saturate((mask - g_fThreshold) / (1.f - g_fThreshold));
    
    vPreColor = lerp(g_vColor, g_vCenterColor, lerpFactor);
    
    vector vColor;
    
    vColor.rgb = vPreColor.rgb * mask * g_fIntensity;
    //vColor.a = vPreColor.a * mask;
    vColor.a = g_vColor.a * mask;


    float3 vPremulRGB = vColor.rgb * vColor.a;
    Out.vAccumulation = float4(vPremulRGB, vColor.a);
    Out.fRevealage = vColor.a;
    Out.vEmissive = float4(vPremulRGB * g_fEmissiveIntensity, 0.f);
    return Out;
}


PS_OUT_EFFECT_WB PS_MAIN_DISTORTIONONLY(PS_IN_BLEND In)
{
    PS_OUT_EFFECT_WB Out = (PS_OUT_EFFECT_WB) 0;

    vector vMask = g_MaskTexture1.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset));

    float2 dir = vMask.rg * 2.0 - 1.0; // [-1,1]
    //float mag = lerp(1.0, vMask.b, 0.5); // B를 세기로 활용(옵션)
    
    float2 flowUV = saturate(UVTexcoord(In.vTexcoord) + dir * g_fTime * 1.f);

    Out.vDistortion = g_MaskTexture2.Sample(DefaultSampler, flowUV);
    Out.vDistortion *= g_vColor;
    Out.vDistortion.b = g_fDistortionStrength / 255.f;
    return Out;
}

PS_OUT_EFFECT_WB PS_MAIN_DISTORTION(PS_IN_BLEND In)
{
    PS_OUT_EFFECT_WB Out = (PS_OUT_EFFECT_WB) 0;

    vector vDistort = g_MaskTexture2.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset));
    Out.vDistortion = vDistort;
    Out.vDistortion = saturate(Out.vDistortion * 2.0 - 1.0); // 0.5~1.0 -> 0.0~1.0
    Out.vDistortion *= g_vColor;
    Out.vDistortion.b = saturate(g_fDistortionStrength / 255.f * (Out.vDistortion.r + Out.vDistortion.g));
    Out.vDistortion.b *= g_vColor.a;
    return Out;
}

PS_OUT PS_MAIN_NONLIGHT(PS_IN_BLEND In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    // ==== 샘플 ====
    vector vColor = g_DiffuseTexture.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset));

    //vector vColor = g_DiffuseTexture.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, In.vTileOffset));

    if (vColor.a < 0.1f)
        discard;

    vColor *= g_vColor;

    vColor = SoftEffect(vColor, In.vProjPos);

    Out.vColor = vColor;
    return Out;
}



technique11 DefaultTechnique
{
    pass Default            // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();    
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();      
    }
    pass SoftEffectOnly     // 1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);        

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SOFTEFFECT();
    }
    pass UVSprite_Default   // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_GRID();
    }
    pass UVSprite_Coloring  // 3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_GRID_COLOR();
    }
    pass UVSprite_Coloring_WB // 4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_WBOIT, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_GRID_COLOR_WB();
    }
    pass MaskSprite_C_WB // 5
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_WBOIT, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_MASKONLY_CWB();
    }
    pass MaskDissolveSprite_C_WB // 6
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_WBOIT, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_MASKDISSOLVE_CWB();
    }
    pass DistortionOnly // 7
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_WBOIT, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DISTORTIONONLY();
    }
    pass DistortionNoMask // 8
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_WBOIT, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_BLEND();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DISTORTION();
    }
}
