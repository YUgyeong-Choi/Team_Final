#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix;
matrix g_ViewMatrix;
matrix g_ProjMatrix;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vTexcoord : TEXCOORD0; // 사용 안 하지만 구조 유지용
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float4 worldPos = mul(float4(In.vPosition, 1.0f), g_WorldMatrix);
    float4 viewPos = mul(worldPos, g_ViewMatrix);
    Out.vPosition = mul(viewPos, g_ProjMatrix);

    return Out;
}