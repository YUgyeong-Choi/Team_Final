#include "Effect_Shader_Defines.hlsli"

vector  g_vCamPosition;
bool    g_bLocal;
float   g_fEmissiveIntensity;
float   g_fWeightPower = 3.f;

//bool    g_size 

struct VS_IN
{
    float3 vPosition : POSITION;       
    
    row_major float4x4 TransformMatrix : WORLD;
    
    float2 vLifeTime : TEXCOORD0;    
    //float4 vDirection : TEXCOORD1;
};

struct VS_OUT
{
    /* SV_ : ShaderValue약자 */
    /* 내가 해야할 연산은 다 했으니 이제 니(장치)가 알아서 추가적인 연산을 해라. */     
    float4 vPosition : POSITION;
    
    float2 vPSize : PSIZE;
    
    float2 vLifeTime : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    matrix matWV, matWVP;
   
    // 이부분에서 transform 곱하는거로 로컬월드분기.......
    // 로컬쓸려면 곱하고, 각자 월드상태 가지려면 처음부터 월드 상태의 좌표 든 채로 이부분스킵
    vector vPosition = mul(vector(In.vPosition, 1.f), In.TransformMatrix);    
    
    if (g_bLocal == 0)
        Out.vPosition = mul(vPosition, g_WorldMatrix);
    else
        Out.vPosition = vPosition;
    
    Out.vPSize = float2(length(In.TransformMatrix._11_12_13), length(In.TransformMatrix._21_22_23));
    
    Out.vLifeTime = In.vLifeTime;
    return Out;
}

/* 그리는 형태에 따라서 호출된다. */ 

struct GS_IN
{
    float4 vPosition : POSITION;
    
    float2 vPSize : PSIZE;
    
    float2 vLifeTime : TEXCOORD0;
};

struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

[maxvertexcount(6)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> Triangles)
{
    GS_OUT Out[4];
    
    float3 vLook = g_vCamPosition.xyz - In[0].vPosition.xyz;
    float3 vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook)) * In[0].vPSize.x * 0.5f;
    float3 vUp = normalize(cross(vLook, vRight)) * In[0].vPSize.y * 0.5f;
    
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
    
    Out[0].vPosition = mul(float4(In[0].vPosition.xyz + vRight + vUp, 1.f), matVP);    
    Out[0].vTexcoord = float2(0.f, 0.f);
    Out[0].vLifeTime = In[0].vLifeTime;
    Out[0].vProjPos = Out[0].vPosition;
    
    Out[1].vPosition = mul(float4(In[0].vPosition.xyz - vRight + vUp, 1.f), matVP);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vLifeTime = In[0].vLifeTime;
    Out[1].vProjPos = Out[1].vPosition;
    
    Out[2].vPosition = mul(float4(In[0].vPosition.xyz - vRight - vUp, 1.f), matVP);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vLifeTime = In[0].vLifeTime;
    Out[2].vProjPos = Out[2].vPosition;
    
    Out[3].vPosition = mul(float4(In[0].vPosition.xyz + vRight - vUp, 1.f), matVP);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vLifeTime = In[0].vLifeTime;
    Out[3].vProjPos = Out[3].vPosition;
    
    Triangles.Append(Out[0]);
    Triangles.Append(Out[1]);
    Triangles.Append(Out[2]);
    Triangles.RestartStrip();
    
    Triangles.Append(Out[0]);
    Triangles.Append(Out[2]);
    Triangles.Append(Out[3]);
    Triangles.RestartStrip();
}



struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

struct PS_OUT
{
    vector vColor : SV_TARGET0;
};


PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;    
    
    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset));
    
    if(Out.vColor.a < 0.003f)
        discard;
    
    // 이부분도 변수로 받을 수 있으면??
    Out.vColor.a = saturate(In.vLifeTime.x - In.vLifeTime.y);
    

    if (In.vLifeTime.y >= In.vLifeTime.x)
        discard;
    
    return Out;
}

PS_OUT PS_MAIN_MASKONLY(PS_IN In)
{
    PS_OUT Out;
    
    float mask = g_MaskTexture1.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset)).r;
    if (mask < 0.003f)
        discard;
    float4 color;
    float lerpFactor = saturate((mask - g_fThreshold) / (1.f - g_fThreshold));
    
    color = lerp(g_vColor, g_vCenterColor, lerpFactor);
    
    Out.vColor.rgb = color.rgb * mask * g_fIntensity;
    Out.vColor.a = color.a * mask;
    



    Out.vColor.a = saturate(In.vLifeTime.x - In.vLifeTime.y);



    if (In.vLifeTime.y >= In.vLifeTime.x)
        discard;
    
    return Out;
}


struct PS_OUT_WB
{
    vector vAccumulation : SV_TARGET0;
    vector fRevealage : SV_TARGET1;
    vector vEmissive : SV_TARGET2;
};

PS_OUT_WB PS_MAIN_MASKONLY_WBGLOW(PS_IN In)
{
    PS_OUT_WB Out;
    
    float mask = g_MaskTexture1.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset)).r;
    if (mask < 0.003f)
        discard;
    float4 vPreColor;
    float lerpFactor = saturate((mask - g_fThreshold) / (1.f - g_fThreshold));
    
    vPreColor = lerp(g_vColor, g_vCenterColor, lerpFactor);
    
    vector vColor;
    
    vColor.rgb = vPreColor.rgb * mask * g_fIntensity;
    vColor.a = vPreColor.a * mask;
    vColor.a = saturate(In.vLifeTime.x - In.vLifeTime.y);
    
    
    float fDepth = In.vProjPos.z / In.vProjPos.w;
    

    // 1 - 깊이 = 멀 수록 연하게
    float fWeight = pow(saturate(1 - fDepth), g_fWeightPower);
    float3 vPremulRGB = vColor.rgb * vColor.a;
    
    // rgb에 Premul * weight, a에 a * weight
    Out.vAccumulation = float4(vPremulRGB * fWeight, vColor.a * fWeight);
    
    Out.fRevealage = vColor.a;
    Out.vEmissive = float4(vPremulRGB * fWeight, 0.f);
  
    
    
    if (In.vLifeTime.y >= In.vLifeTime.x)
        discard;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass Default // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();    
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();      
    }
    pass MaskOnly //1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();    
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_MASKONLY();
    }
    pass MaskOnly_WBGlow //2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_WBOIT, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();    
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_MASKONLY_WBGLOW();
    }
 
}
