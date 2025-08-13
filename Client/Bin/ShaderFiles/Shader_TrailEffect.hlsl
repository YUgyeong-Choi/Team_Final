#include "Effect_Shader_Defines.hlsli"
vector g_vCamPosition;

struct VS_IN
{
    float3 vOuterPos : POSITION0;
    float3 vInnerPos : POSITION1;
    float2 vLifeTime : TEXCOORD0;
    float fVCoord : TEXCOORD1;
};

struct VS_OUT
{
    float3 vOuterPos : POSITION0;
    float3 vInnerPos : POSITION1;
    float2 vLifeTime : TEXCOORD0;
    float fVCoord : TEXCOORD1;
};


VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    // 지금 월드 상태 가정하고 적음 아니 근데 아 로컬주고 여기서 곱하면 플레이어 무조건 따라다니잖아 진짜 고민한보람없네
    Out.vOuterPos = In.vOuterPos;
    Out.vInnerPos = In.vInnerPos;
    Out.vLifeTime = In.vLifeTime;   
    Out.fVCoord = In.fVCoord;
    return Out;
}   

VS_OUT VS_MAIN_DROP(VS_IN In)
{
    VS_OUT Out;
    
    // 지금 월드 상태 가정하고 적음 아니 근데 아 로컬주고 여기서 곱하면 플레이어 무조건 따라다니잖아 진짜 고민한보람없네
    Out.vOuterPos = In.vOuterPos;
    Out.vInnerPos = In.vInnerPos;
    Out.vLifeTime = In.vLifeTime;
    Out.fVCoord = In.fVCoord;
    
    float lifeRatio = saturate(In.vLifeTime.y / In.vLifeTime.x);
    
    float3 vOuterPos = In.vOuterPos;
    float3 vInnerPos = In.vInnerPos;

    if (lifeRatio >= 0.5f)
    {
        float fade = smoothstep(0.5, 1.0, lifeRatio); // 마지막 20%에서만 스르륵

        vOuterPos = float3(In.vOuterPos.x, In.vOuterPos.y - fade * 0.8f, In.vOuterPos.z);
        vInnerPos = float3(In.vInnerPos.x, In.vInnerPos.y - fade * 0.8f, In.vInnerPos.z);
    }

    Out.vOuterPos = vOuterPos;
    Out.vInnerPos = vInnerPos;
    
    
    return Out;
}

struct GS_IN
{
    float3 vOuterPos : POSITION0;
    float3 vInnerPos : POSITION1;
    float2 vLifeTime : TEXCOORD0;
    float fVCoord : TEXCOORD1;
};

struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
};

[maxvertexcount(6)] 
void GS_MAIN(line VS_OUT input[2], inout TriangleStream<GS_OUT> Triangles)
{
    float3 Outer0 = input[0].vOuterPos;
    float3 Inner0 = input[0].vInnerPos;

    float3 Outer1 = input[1].vOuterPos;
    float3 Inner1 = input[1].vInnerPos;
    
    float v0 = input[0].fVCoord;
    float v1 = input[1].fVCoord;
    
    
    //float fade0 = 1.0 - saturate(input[0].vLifeTime.y / input[0].vLifeTime.x); // 이걸왜여기서함성격이상하네
    //float fade1 = 1.0 - saturate(input[1].vLifeTime.y / input[1].vLifeTime.x);
    
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);

    // UVs (좌→우, 상→하)
    float2 uv[4] =
    {
        float2(0, v0), // Outer0
        float2(1, v0), // Inner0
        float2(1, v1), // Inner1
        float2(0, v1) // Outer1
    };

    
    float3 verts[4] =
    {
        Outer0,
        Inner0,
        Inner1,
        Outer1
    };

    // 출력: TriangleStrip 사용하지 않음 → 직접 TriangleList 생성
    GS_OUT outVert[4];
    
    //for (int i = 0; i < 4; i++)
    //{
    //    outVert[i].vPosition = mul(float4(verts[i], 1.f), matVP);
    //    outVert[i].vTexcoord = uv[i];
    //    outVert[i].fFade = fades[i];
    //}
    // FXC에서 for문에 대해 겁나게 보수적으로 작동해서 중간에 끊기는지에 대해 확신을 못하는 이슈로 빌드 시에 경고 뜸.. 
    // 웬만하면 명시적으로 적기
    
    outVert[0].vPosition = mul(float4(verts[0], 1.f), matVP);
    outVert[0].vTexcoord = uv[0];
    outVert[0].vLifeTime = input[0].vLifeTime;
    
    outVert[1].vPosition = mul(float4(verts[1], 1.f), matVP);
    outVert[1].vTexcoord = uv[1];
    outVert[1].vLifeTime = input[0].vLifeTime;
    
    outVert[2].vPosition = mul(float4(verts[2], 1.f), matVP);
    outVert[2].vTexcoord = uv[2];
    outVert[2].vLifeTime = input[1].vLifeTime;
    
    outVert[3].vPosition = mul(float4(verts[3], 1.f), matVP);
    outVert[3].vTexcoord = uv[3];
    outVert[3].vLifeTime = input[1].vLifeTime;
    
    
    
    Triangles.Append(outVert[0]);
    Triangles.Append(outVert[1]);
    Triangles.Append(outVert[2]);
    Triangles.RestartStrip();
    
    Triangles.Append(outVert[0]);
    Triangles.Append(outVert[2]);
    Triangles.Append(outVert[3]);
    Triangles.RestartStrip();
}



struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
};

struct PS_OUT
{
    vector vAccumulation : SV_TARGET0;
    vector fRevealage : SV_TARGET1;
    vector vEmissive : SV_TARGET2;
    vector vDistortion : SV_TARGET3;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    vector vColor = g_DiffuseTexture.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset));
    vColor = ColorAdjustment_Multiply(vColor, g_vColor);
    float fMask = g_MaskTexture1.Sample(DefaultSampler, UVTexcoord(In.vTexcoord, g_fTileSize, g_fTileOffset)).r;
    float lifeRatio = saturate(In.vLifeTime.y / In.vLifeTime.x);
    float fade = 1.0 - smoothstep(0.8, 1.0, lifeRatio); // 마지막 20%에서만 스르륵
    vColor.a *= fade * fMask;

    
    float3 vPremulRGB = vColor.rgb * vColor.a;
    Out.vAccumulation = float4(vPremulRGB, vColor.a);
    Out.fRevealage = vColor.a;
    Out.vEmissive = float4(vPremulRGB * g_fEmissiveIntensity, 0.f);
    
    //float2 vInvertUV = float2(1.f - In.vTexcoord.x, In.vTexcoord.y);
    float2 vInvertUV = float2(In.vTexcoord.x, In.vTexcoord.y);
    Out.vDistortion = g_MaskTexture2.Sample(DefaultSampler, UVTexcoord(vInvertUV, g_fTileSize, g_fTileOffset));
    Out.vDistortion.a *= fade * fMask;
    return Out;
}

technique11 DefaultTechnique
{
    pass Default // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_WBOIT, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass Drop // 1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_WBOIT, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_DROP();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }

}

