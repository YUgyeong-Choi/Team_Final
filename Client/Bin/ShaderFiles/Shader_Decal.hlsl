#include "Engine_Shader_Defines.hlsli"

matrix g_ViewMatrix, g_ProjMatrix, g_WorldMatrix;

matrix g_ProjMatrixInv;
matrix g_ViewWorldMatrixInv;

Texture2D g_DepthTexture;

Texture2D g_ARMT;
Texture2D g_N;
Texture2D g_BC;

Texture2D g_MASK; //�븻 �¸��� ����ũ �ؽ���

float g_Far = 1000.f;
float2 g_ScreenSize = float2(1600.f, 900.f);




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
    
    /* mul : ��� ����� ���ϱ⸦ �����Ѵ�. /w������ �������� �ʴ´�. */
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

struct DECAL_SAMPLE
{
    float2 vTexcoord;
    float3 vLocalPos;
    float3 vWorldPos;
};

DECAL_SAMPLE SampleDecal(PS_IN In)
{
    DECAL_SAMPLE Out;

    // ȭ�� ��ǥ �� UV
    float2 vUV = In.vPosition.xy / g_ScreenSize;
    
    // Depth
    vector vDepthDesc = g_DepthTexture.Sample(PointSampler, vUV);
    float fViewZ = vDepthDesc.y * g_Far;

    // NDC �� View Space
    float4 vPosition;
    vPosition.x = vUV.x * 2.f - 1.f;
    vPosition.y = vUV.y * -2.f + 1.f;
    vPosition.z = vDepthDesc.x;
    vPosition.w = 1.f;

    vPosition *= fViewZ;

    float4 vWorldPos4 = mul(vPosition, g_ProjMatrixInv);
    Out.vWorldPos = vWorldPos4.xyz;

    // Local space
    Out.vLocalPos = mul(float4(Out.vWorldPos, 1.f), g_ViewWorldMatrixInv).xyz;

    // Clip: �����޽� ���� ����
    clip(0.5f - abs(Out.vLocalPos));

    // Texcoord
    Out.vTexcoord = Out.vLocalPos.xz + 0.5f;

    return Out;
}


struct PS_OUT
{
    float4 vARMT : SV_Target0;
    float4 vN : SV_Target1;
    float4 vBC : SV_Target2;
};

PS_OUT PS_DECAL(PS_IN In)
{
    PS_OUT Out;

    DECAL_SAMPLE tagDecalSample = SampleDecal(In);

    vector vARMT = g_ARMT.Sample(DefaultSampler, tagDecalSample.vTexcoord);
    if (vARMT.a < 0.3f)
        discard;
    
    vector vN = g_N.Sample(DefaultSampler, tagDecalSample.vTexcoord);
    vector vBC = g_BC.Sample(DefaultSampler, tagDecalSample.vTexcoord);

    Out.vARMT = vARMT;
    Out.vN = vN * vARMT.a;
    Out.vBC = vBC * vARMT.a;

    return Out;
}

struct PS_OUT_NORMALONLY
{
    float4 vN : SV_Target1;
};

PS_OUT_NORMALONLY PS_DECAL_NORMALONLY(PS_IN In)
{
    PS_OUT_NORMALONLY Out;

    DECAL_SAMPLE tagDecalSample = SampleDecal(In);

    vector vN = g_N.Sample(DefaultSampler, tagDecalSample.vTexcoord);
    vector vMask = g_MASK.Sample(DefaultSampler, tagDecalSample.vTexcoord);

    Out.vN = vN * vMask.r;

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

float4 PS_DEBUG(VS_OUT_DEBUG In) : SV_Target3
{
    return float4(1.f, 0.f, 0.f, 1.f); // ���� ��
}



technique11 DefaultTechnique
{
    pass Decal //0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_DECAL();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DECAL();
    }

    pass DebugCube //1 ������ ť��
    {
        SetRasterizerState(RS_Wireframe); // ���̾����������� ���̰�
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_DEBUG();
        PixelShader = compile ps_5_0 PS_DEBUG();
    }

    pass Decal_NormalOnly //2 �븻�� ������ ��Į
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_DECAL();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DECAL_NORMALONLY();
    }
}
