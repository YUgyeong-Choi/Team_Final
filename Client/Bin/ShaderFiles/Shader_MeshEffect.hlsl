#include "Effect_Shader_Defines.hlsli"


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
    //vector vPickPos : SV_TARGET3;
};


struct PS_OUT_EFFECT
{
    vector vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;    
    
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vMtrlDiffuse.a < 0.3f)
        discard;
    
    //vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    //float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;
    
    //float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz, In.vNormal.xyz);
    
    //vNormal = mul(vNormal, WorldMatrix);    
    
   
    Out.vDiffuse = vMtrlDiffuse;
    //Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vNormal = vector(0.f, 0.f, 0.f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.f, 0.f);
    //Out.vPickPos = In.vWorldPos;
    
    return Out;
}

PS_OUT_EFFECT PS_MAIN_MASKONLY(PS_IN In)
{
    PS_OUT_EFFECT Out;
    
    float mask = g_MaskTexture1.Sample(DefaultSampler, UVTileScroll(In.vTexcoord, g_fTileSize, g_fTileOffset, 1.2f, g_fTime)).r;
    if (mask < 0.00001f)
        discard;
    
    float4 color;
    float lerpFactor = saturate((mask - g_fThreshold) / (1.f - g_fThreshold));
    
    color = lerp(g_vColor, g_vCenterColor, lerpFactor);
    
    Out.vColor.rgb = color.rgb * mask * g_fIntensity;
    Out.vColor.a = color.a * mask;
    
    return Out;
}

PS_OUT_EFFECT PS_MAIN_MASK_NOISE(PS_IN In)
{
    PS_OUT_EFFECT Out;
    
    float2 uv = In.vTexcoord;
    
    uv.x *= 3.0f; // 가로 타일링
    uv.x -= g_fTime * 0.8f; // 시간 기반 스크롤
    uv.y *= 0.5f; // 세로 압축 (패턴 길이 보정)
    

    float2 noiseUV = uv * /*g_fNoiseTile*/float2(4.0, 4.0) + g_fTime * float2(0.8, 0.5) /*g_fNoiseScrollSpeed*/;
    float noise = g_MaskTexture2.Sample(DefaultSampler, noiseUV).r; // grayscale 노이즈
    
    float noiseOffset = (noise - 0.5f) * /*g_fNoiseStrength*/0.03;
    uv.y += noiseOffset; // 또는 uv.x += noiseOffset; ← 방향 선택 가능
    
    float mask = g_MaskTexture1.Sample(DefaultSampler, uv).r;
    
    float lerpFactor = saturate((mask - g_fThreshold) / (1.f - g_fThreshold));
    float4 color = lerp(g_vColor, g_vCenterColor, lerpFactor);
    
    Out.vColor.rgb = color.rgb * mask * g_fIntensity;
    Out.vColor.a = color.a * mask;

    return Out;
}


PS_OUT_EFFECT PS_MAIN_UVMASKONLY(PS_IN In)
{
    PS_OUT_EFFECT Out;
    
    float mask = g_DiffuseTexture.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset)).r;
    
    float4 color;
    float lerpFactor = saturate((mask - g_fThreshold) / (1.f - g_fThreshold));
    
    color = lerp(g_vColor, g_vCenterColor, lerpFactor);
    
    Out.vColor.rgb = color.rgb * mask * g_fIntensity;
    Out.vColor.a = color.a * mask;
    
    return Out;
}

struct PS_OUT_EFFECT_WB
{
    vector vAccumulation : SV_TARGET0;
    vector fRevealage : SV_TARGET1;
    vector vEmissive : SV_TARGET2;
};

PS_OUT_EFFECT_WB PS_MAIN_MASKONLY_SCROLL_WB(PS_IN In)
{
    PS_OUT_EFFECT_WB Out;
    
    float mask = g_MaskTexture1.Sample(DefaultSampler, UVTileScroll(In.vTexcoord, g_fTileSize, g_fTileOffset, 1.2f, g_fTime)).r;
    if (mask < 0.003f)
        discard;
    float4 vPreColor;
    float lerpFactor = saturate((mask - g_fThreshold) / (1.f - g_fThreshold));
    
    vPreColor = lerp(g_vColor, g_vCenterColor, lerpFactor);
    
    vector vColor;
    
    vColor.rgb = vPreColor.rgb * mask * g_fIntensity;
    vColor.a = vPreColor.a * mask;
    

    float3 vPremulRGB = vColor.rgb * vColor.a;
    Out.vAccumulation = float4(vPremulRGB, vColor.a);
    Out.fRevealage = vColor.a;
    Out.vEmissive = float4(vPremulRGB * g_fEmissiveIntensity, 0.f);
        
    
    return Out;
}

PS_OUT_EFFECT_WB PS_MAIN_MASKONLY_WB(PS_IN In)
{
    PS_OUT_EFFECT_WB Out;
    
    float mask = g_MaskTexture1.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset)).r;
    if (mask < 0.003f)
        discard;
    float4 vPreColor;
    float lerpFactor = saturate((mask - g_fThreshold) / (1.f - g_fThreshold));
    
    vPreColor = lerp(g_vColor, g_vCenterColor, lerpFactor);
    
    vector vColor;
    
    vColor.rgb = vPreColor.rgb * mask * g_fIntensity;
    vColor.a = vPreColor.a * mask;
    

    float3 vPremulRGB = vColor.rgb * vColor.a;
    Out.vAccumulation = float4(vPremulRGB, vColor.a);
    Out.fRevealage = vColor.a;
    Out.vEmissive = float4(vPremulRGB * g_fEmissiveIntensity, 0.f);
        
    
    return Out;
}

technique11 DefaultTechnique
{   
    pass Default        // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();      
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();      
    }
   
    pass MaskOnly       // 1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();      
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_MASKONLY();
    }
   
    pass MaskNoise       // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();      
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_MASK_NOISE();
    }
   
    pass UVMask // 3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_UVMASKONLY();
    }
   
    pass MaskOnly_TileScroll_WB // 4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_WBOIT, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 /*PS_MAIN_MASKONLY_SCROLL_WB();*/PS_MAIN_MASKONLY_WB();

    }
    pass MaskOnly_WB // 5
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_WBOIT, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_MASKONLY_WB();
    }
   
}
