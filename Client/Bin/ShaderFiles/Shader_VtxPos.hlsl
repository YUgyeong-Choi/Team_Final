#include "Engine_Shader_Defines.hlsli"

matrix g_ViewMatrix, g_ProjMatrix, g_WorldMatrix;

matrix g_WorldMatrixInv;
matrix g_ProjMatrixInv;
matrix g_ViewMatrixInv;

Texture2D g_DepthTexture;

Texture2D g_Texture;

struct VS_IN
{
    float3 vPosition : POSITION;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
};

VS_OUT VS_DECAL(VS_IN In)
{
    VS_OUT Out;
    
    matrix matWV, matWVP;
    
    /* mul : 모든 행렬의 곱하기를 수행한다. /w연산을 수행하지 않는다. */
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    float4 ProjPos = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vPosition = ProjPos;
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
};

struct PS_OUT
{
    float4 vDecal : SV_Target0;
};

PS_OUT PS_DECAL(PS_IN In)
{
    PS_OUT Out;

    float2 vUV = In.vPosition.xy / float2(1600.f, 900.f); //이것으로 해결했음 으아아악
    
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, vUV);
    float fViewZ = vDepthDesc.y * 1000.f; //(Near~Far)
    
    vector vPosition;

    vPosition.x = vUV.x * 2.f - 1.f;
    vPosition.y = vUV.y * -2.f + 1.f;
    vPosition.z = vDepthDesc.x; //투영 스페이스의 깊이(0~1)
    vPosition.w = 1.f;

    vPosition = vPosition * fViewZ; //w나누던 연산을 역으로 곱하는 부분
    
    vPosition = mul(vPosition, g_ProjMatrixInv); //투영 역행렬
    vPosition = mul(vPosition, g_ViewMatrixInv); //뷰 역행렬
    
    //월드로 왔음
    
    // 데칼 로컬 공간으로 변환
    float3 vLocalPos = mul(float4(vPosition.xyz, 1.f), g_WorldMatrixInv).xyz;
    
    //일정 각도이상 기울어진 표면은 데칼을 적용하지 않음 
    
    clip(0.5 - abs(vLocalPos.xyz));

    // 텍스처 좌표 계산 (0 ~ 1)
    float2 vTexcoord = vLocalPos.xz + 0.5f;
    float4 vColor = g_Texture.Sample(DefaultSampler, vTexcoord);
    Out.vDecal = vColor;
    
    return Out;
}

struct VS_OUT_DEBUG
{
    float4 vPosition : SV_POSITION;
};

VS_OUT_DEBUG VS_DEBUG(VS_IN In)
{ 
    VS_OUT_DEBUG Out;
    matrix matWV, matWVP;
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    float4 ProjPos = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vPosition = ProjPos;
    return Out;
}

float4 PS_DEBUG(VS_OUT_DEBUG In) : SV_Target
{
    return float4(1.f, 0.f, 0.f, 1.f); // 빨간 선
}



technique11 DefaultTechnique
{
    pass Decal
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_DECAL();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DECAL();
    }

    pass DebugCube // 디버깅용 큐브
    {
        SetRasterizerState(RS_Wireframe); // 와이어프레임으로 보이게
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_DEBUG();
        PixelShader = compile ps_5_0 PS_DEBUG();
    }
}
