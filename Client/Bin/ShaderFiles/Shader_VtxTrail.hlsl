#include "Effect_Shader_Defines.hlsli"
vector g_vCamPosition;

struct VS_IN
{
    float3 vOuterPos : POSITION0;
    float3 vInnerPos : POSITION1;
    float2 vLifeTime : TEXCOORD0;
};

struct VS_OUT
{
    float3 vOuterPos : POSITION0;
    float3 vInnerPos : POSITION1;
    float2 vLifeTime : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    // 지금 월드 상태 가정하고 적음 아니 근데 아 로컬주고 여기서 곱하면 플레이어 무조건 따라다니잖아 진짜 고민한보람없네
    Out.vOuterPos = In.vOuterPos;
    Out.vInnerPos = In.vInnerPos;
    Out.vLifeTime = In.vLifeTime;   
    
    return Out;
}   

struct GS_IN
{
    float3 vOuterPos : POSITION0;
    float3 vInnerPos : POSITION1;
    float2 vLifeTime : TEXCOORD0;
};

struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float fFade : TEXCOORD1; //??
};

[maxvertexcount(4)] 
void GS_MAIN(line VS_OUT input[2], inout TriangleStream<GS_OUT> Triangles)
{
    float3 Outer0 = input[0].vOuterPos;
    float3 Inner0 = input[0].vInnerPos;

    float3 Outer1 = input[1].vOuterPos;
    float3 Inner1 = input[1].vInnerPos;

    float fade0 = 1.0 - saturate(input[0].vLifeTime.y / input[0].vLifeTime.x); // 이걸왜여기서함성격이상하네
    float fade1 = 1.0 - saturate(input[1].vLifeTime.y / input[1].vLifeTime.x);

    // 최종 변환 행렬
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);

    // UVs (좌→우, 상→하)
    float2 uv[4] =
    {
        float2(0, 0), // Outer0
        float2(1, 0), // Inner0
        float2(1, 1), // Inner1
        float2(0, 1) // Outer1
    };

    float3 verts[4] =
    {
        Outer0,
        Inner0,
        Inner1,
        Outer1
    };

    float fades[4] =
    {
        fade0,
        fade0,
        fade1,
        fade1
    };

    // 출력: TriangleStrip 사용하지 않음 → 직접 TriangleList 생성
    GS_OUT outVert[4];
    
    for (int i = 0; i < 4; i++)
    {
        outVert[i].vPosition = mul(float4(verts[i], 1.f), matVP);
        outVert[i].vTexcoord = uv[i];
        outVert[i].fFade = fades[i];
    }
    
    Triangles.Append(outVert[0]);
    Triangles.Append(outVert[1]);
    Triangles.Append(outVert[2]);
    
    Triangles.Append(outVert[0]);
    Triangles.Append(outVert[2]);
    Triangles.Append(outVert[3]);
}



struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float fFade : TEXCOORD1; //??
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

technique11 DefaultTechnique
{
    pass Default // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    //pass SoftEffectOnly // 1
    //{
    //    SetRasterizerState(RS_Cull_None);
    //    SetDepthStencilState(DSS_Default, 0);
    //    SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

    //    VertexShader = compile vs_5_0 VS_MAIN_BLEND();
    //    GeometryShader = NULL;
    //    PixelShader = compile ps_5_0 PS_MAIN_SOFTEFFECT();
    //}

}