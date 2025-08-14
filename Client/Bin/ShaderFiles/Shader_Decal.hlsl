#include "Engine_Shader_Defines.hlsli"

matrix g_ViewMatrix, g_ProjMatrix, g_WorldMatrix;

//matrix g_WorldMatrixInv;
matrix g_ProjMatrixInv;
//matrix g_ViewMatrixInv;
matrix g_ViewWorldMatrixInv;

Texture2D g_DepthTexture;
//Texture2D g_WorldPosTexture;

Texture2D g_ARMT;
Texture2D g_N;
Texture2D g_BC;

//float g_Near = 0.001f;
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

struct PS_OUT
{
    float4 vARMT : SV_Target0;
    float4 vN : SV_Target1;
    float4 vBC : SV_Target2;
};

PS_OUT PS_DECAL(PS_IN In)
{
    PS_OUT Out;

    float2 vUV = In.vPosition.xy / g_ScreenSize; //�̰����� �ذ����� ���ƾƾ�(�ְ�Ǵ°�)
    
    vector vDepthDesc = g_DepthTexture.Sample(PointSampler, vUV);
    float fViewZ = vDepthDesc.y * g_Far; //(Near~Far)
    
    vector vPosition;

    vPosition.x = vUV.x * 2.f - 1.f;
    vPosition.y = vUV.y * -2.f + 1.f;
    vPosition.z = vDepthDesc.x; //���� �����̽��� ����(0~1)
    vPosition.w = 1.f;

    vPosition = vPosition * fViewZ; //w������ ������ ������ ���ϴ� �κ�
    
    vPosition = mul(vPosition, g_ProjMatrixInv); //���� �����
    //vPosition = mul(vPosition, g_ViewMatrixInv); //�� �����
    
    //����� ����
    //vector vWorldPosDesc = g_WorldPosTexture.Sample(PointSampler, vUV); //�׽�Ʈ
    
    // ��Į ���� �������� ��ȯ
    float3 vLocalPos = mul(float4(vPosition.xyz, 1.f), g_ViewWorldMatrixInv).xyz; //��*���� ����ķ� �ѹ濡�ϴϱ� ������ �������!!!!!!!
    
    //���� �����̻� ������ ǥ���� ��Į�� �������� ���� 
    
    clip(0.5f - abs(vLocalPos.xyz));

    // �ؽ�ó ��ǥ ��� (0 ~ 1)
    float2 vTexcoord = vLocalPos.xz + 0.5f;
    
    Out.vARMT = g_ARMT.Sample(DefaultSampler, vTexcoord);
    Out.vN = g_N.Sample(DefaultSampler, vTexcoord);
    Out.vBC = g_BC.Sample(DefaultSampler, vTexcoord);
    
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
    pass Decal
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_DECAL();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DECAL();
    }

    pass DebugCube // ������ ť��
    {
        SetRasterizerState(RS_Wireframe); // ���̾����������� ���̰�
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_DEBUG();
        PixelShader = compile ps_5_0 PS_DEBUG();
    }
}
