#include "Effect_Shader_Defines.hlsli"

vector g_vCamPosition;
bool g_bLocal;


struct ParticleParam
{
    float4 Right;
    float4 Up;
    float4 Look;
    float4 Translation;

    float3 vInitOffset;
    float _pad0;
    
    float4 Direction; // normalized dir (w=unused)
    float4 VelocityDir; // 실제 이동한 방향 벡터, w = length

    float2 LifeTime; // x=max, y=acc
    float Speed;
    float RotationSpeed; // degrees/sec

    float OrbitSpeed; // degrees/sec
    float fAccel; // 가속도 (+면 가속, -면 감속)
    float fMaxSpeed; // 최대 속도 (옵션)
    float fMinSpeed; // 최소 속도 (옵션, 감속 시 멈춤 방지)
};

StructuredBuffer<ParticleParam> Particle_SRV : register(t0);

struct VS_OUT
{
    /* SV_ : ShaderValue약자 */
    /* 내가 해야할 연산은 다 했으니 이제 니(장치)가 알아서 추가적인 연산을 해라. */     
    float4 vPosition : POSITION;
    float2 vPSize : PSIZE;
    float2 vLifeTime : TEXCOORD0;
    float fSpeed : TEXCOORD1;
    float4 vDir : TEXCOORD2;
};

VS_OUT VS_MAIN_CS(uint instanceID : SV_InstanceID)
{
    VS_OUT Out;
    
    matrix matWV, matWVP;
    
    ParticleParam particle = Particle_SRV[instanceID];
    
    row_major float4x4 TransformMatrix = float4x4(
        particle.Right,
        particle.Up,
        particle.Look,
        particle.Translation
    );
    
    // 이부분에서 transform 곱하는거로 로컬월드분기.......
    // 로컬쓸려면 곱하고, 각자 월드상태 가지려면 처음부터 월드 상태의 좌표 든 채로 이부분스킵
    vector vPosition = mul(vector(0.f, 0.f, 0.f, 1.f), TransformMatrix);
    
    //if (g_bLocal == 0)
    //    Out.vPosition = mul(vPosition, g_WorldMatrix);
    //else
        Out.vPosition = vPosition;
    
    Out.vPSize = float2(length(particle.Right.xyz), length(particle.Up.xyz));

    Out.vLifeTime = particle.LifeTime;
    Out.fSpeed = particle.Speed;
    Out.vDir = float4(normalize(particle.VelocityDir.xyz), particle.VelocityDir.w);
    
    return Out;
}


/* 그리는 형태에 따라서 호출된다. */ 

struct GS_IN
{
    float4 vPosition : POSITION;
    
    float2 vPSize : PSIZE;
    
    float2 vLifeTime : TEXCOORD0;
    float fSpeed : TEXCOORD1;
    float4 vDir : TEXCOORD2;
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
    Out[0].vProjPos = float4(length(In[0].vPosition.xyz - g_vCamPosition.xyz), 0.f, 0.f, 0.f);
        
    Out[1].vPosition = mul(float4(In[0].vPosition.xyz - vRight + vUp, 1.f), matVP);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vLifeTime = In[0].vLifeTime;
    Out[1].vProjPos = float4(length(In[0].vPosition.xyz - g_vCamPosition.xyz), 0.f, 0.f, 0.f);
    
    Out[2].vPosition = mul(float4(In[0].vPosition.xyz - vRight - vUp, 1.f), matVP);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vLifeTime = In[0].vLifeTime;
    Out[2].vProjPos = float4(length(In[0].vPosition.xyz - g_vCamPosition.xyz), 0.f, 0.f, 0.f);
    
    Out[3].vPosition = mul(float4(In[0].vPosition.xyz + vRight - vUp, 1.f), matVP);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vLifeTime = In[0].vLifeTime;
    Out[3].vProjPos = float4(length(In[0].vPosition.xyz - g_vCamPosition.xyz), 0.f, 0.f, 0.f);
    
    Triangles.Append(Out[0]);
    Triangles.Append(Out[1]);
    Triangles.Append(Out[2]);
    Triangles.RestartStrip();
    
    Triangles.Append(Out[0]);
    Triangles.Append(Out[2]);
    Triangles.Append(Out[3]);
    Triangles.RestartStrip();
}


[maxvertexcount(6)]
void GS_MAIN_VSTRETCH(point GS_IN In[1], inout TriangleStream<GS_OUT> Triangles)
{
    GS_OUT Out[4];
    // === 카메라 → 파티클 벡터 ===
    float3 vLook = normalize(g_vCamPosition.xyz - In[0].vPosition.xyz);

    // === 카메라 빌보드 기준 벡터 ===
    float3 vRight = normalize(cross(float3(0, 1, 0), vLook));
    float3 vUp = normalize(cross(vLook, vRight));

    // === 기본 사이즈 반영 ===
    vRight *= In[0].vPSize.x * 0.5f;
    vUp *= In[0].vPSize.y * 0.5f;

    // === 속도 방향 Stretch (길이만 늘림) ===
    float3 vDir = normalize(In[0].vDir);
    float stretchFactor = 0.015f * In[0].fSpeed; // 튜닝값
    float3 vStretch = vDir * stretchFactor;

    // 최종 Up에 더해줌 (Y축 Up + 속도 꼬리)
    float3 upStretch = vUp + vStretch;

    // === VP 변환 ===
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
  
    Out[0].vPosition = mul(float4(In[0].vPosition.xyz + vRight + upStretch, 1.f), matVP);
    Out[0].vTexcoord = float2(0.f, 0.f);
    Out[0].vLifeTime = In[0].vLifeTime;
    Out[0].vProjPos = float4(length(In[0].vPosition.xyz - g_vCamPosition.xyz), 0.f, 0.f, 0.f);
  
    Out[1].vPosition = mul(float4(In[0].vPosition.xyz - vRight + upStretch, 1.f), matVP);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vLifeTime = In[0].vLifeTime;
    Out[1].vProjPos = float4(length(In[0].vPosition.xyz - g_vCamPosition.xyz), 0.f, 0.f, 0.f);
  
    Out[2].vPosition = mul(float4(In[0].vPosition.xyz - vRight - upStretch, 1.f), matVP);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vLifeTime = In[0].vLifeTime;
    Out[2].vProjPos = float4(length(In[0].vPosition.xyz - g_vCamPosition.xyz), 0.f, 0.f, 0.f);
  
    Out[3].vPosition = mul(float4(In[0].vPosition.xyz + vRight - upStretch, 1.f), matVP);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vLifeTime = In[0].vLifeTime;
    Out[3].vProjPos = float4(length(In[0].vPosition.xyz - g_vCamPosition.xyz), 0.f, 0.f, 0.f);
  
    Triangles.Append(Out[0]);
    Triangles.Append(Out[1]);
    Triangles.Append(Out[2]);
    Triangles.RestartStrip();
  
    Triangles.Append(Out[0]);
    Triangles.Append(Out[2]);
    Triangles.Append(Out[3]);
    Triangles.RestartStrip();
}



//[maxvertexcount(6)]
//void GS_MAIN_VSTRETCH(point GS_IN In[1], inout TriangleStream<GS_OUT> Tri)
//{
//    GS_OUT Out[4];

//    float3 camToP = g_vCamPosition.xyz - In[0].vPosition.xyz;
//    float3 viewDir = normalize(camToP);

//    // 카메라 빌보드 기본 축
//    float3 right = normalize(cross(float3(0, 1, 0), viewDir));
//    float3 up = normalize(cross(viewDir, right));

//    right *= In[0].vPSize.x * 0.5f;
//    up *= In[0].vPSize.y * 0.5f;

//    float3 v = In[0].vDir.xyz;
//    float vLen = max(In[0].vDir.w, 1e-6);
//    v /= vLen;
//    v = normalize(v - viewDir * dot(v, viewDir));

//    float StretchScale = 1.f;
//    float3 vStretch = v * (StretchScale * In[0].vDir.w);
    
//    float speed = In[0].vDir.w;
//    if (speed < 1e-3f)
//    {
//    // 그냥 빌보드처럼 Up만 씀
//        vStretch = 0;
//    }
//    else
//    {
//        float3 v = normalize(In[0].vDir.xyz);
//        float stretchFactor = 0.02f * speed; // 튜닝값
//        vStretch = v * stretchFactor;
//    }
//    float3 upStretch = up + vStretch;

//    matrix VP = mul(g_ViewMatrix, g_ProjMatrix);

//    Out[0].vPosition = mul(float4(In[0].vPosition.xyz + right + upStretch, 1), VP);
//    Out[0].vTexcoord = float2(0, 0);
//    Out[0].vLifeTime = In[0].vLifeTime;
//    Out[0].vProjPos = float4(length(camToP), 0, 0, 0);

//    Out[1].vPosition = mul(float4(In[0].vPosition.xyz - right + upStretch, 1), VP);
//    Out[1].vTexcoord = float2(1, 0);
//    Out[1].vLifeTime = In[0].vLifeTime;
//    Out[1].vProjPos = Out[0].vProjPos;

//    Out[2].vPosition = mul(float4(In[0].vPosition.xyz - right - upStretch, 1), VP);
//    Out[2].vTexcoord = float2(1, 1);
//    Out[2].vLifeTime = In[0].vLifeTime;
//    Out[2].vProjPos = Out[0].vProjPos;

//    Out[3].vPosition = mul(float4(In[0].vPosition.xyz + right - upStretch, 1), VP);
//    Out[3].vTexcoord = float2(0, 1);
//    Out[3].vLifeTime = In[0].vLifeTime;
//    Out[3].vProjPos = Out[0].vProjPos;

//    Tri.Append(Out[0]);
//    Tri.Append(Out[1]);
//    Tri.Append(Out[2]);
//    Tri.RestartStrip();
//    Tri.Append(Out[0]);
//    Tri.Append(Out[2]);
//    Tri.Append(Out[3]);
//    Tri.RestartStrip();
//}

//[maxvertexcount(6)]
//void GS_MAIN_VSTRETCH(point GS_IN In[1], inout TriangleStream<GS_OUT> Tri)
//{
//    GS_OUT Out[4];

//    float3 pos = In[0].vPosition.xyz;
//    float3 camToP = normalize(g_vCamPosition.xyz - pos);

//    // === 1. 속도 방향 ===
//    float3 vDir = normalize(In[0].vDir.xyz);
//    float speed = In[0].vDir.w;

//    // === 2. Quad 가로축: 속도와 카메라벡터의 외적을 이용해 직교 벡터 생성 ===
//    float3 right = normalize(cross(vDir, camToP));
//    // 혹시 속도와 카메라가 평행하면 right=0 되니, 방어코드 필요할 수도 있음
//    if (all(abs(right) < 1e-6))
//        right = float3(1, 0, 0);

//    // === 3. 크기 적용 ===
//    float stretchLen = 0.1f * speed; // 튜닝값: 속도가 클수록 더 길게
//    float3 vStretch = vDir * stretchLen;

//    right *= In[0].vPSize.x * 0.5f; // 가로폭 (파티클 크기 기반)

//    // === 4. Quad 꼭짓점 계산 ===
//    float3 p0 = pos - right; // 시작-왼쪽
//    float3 p1 = pos + right; // 시작-오른쪽
//    float3 p2 = pos + vStretch + right; // 끝-오른쪽
//    float3 p3 = pos + vStretch - right; // 끝-왼쪽

//    matrix VP = mul(g_ViewMatrix, g_ProjMatrix);

//    // === 5. 출력 ===
//    Out[0].vPosition = mul(float4(p0, 1), VP);
//    Out[0].vTexcoord = float2(0, 1);
//    Out[1].vPosition = mul(float4(p1, 1), VP);
//    Out[1].vTexcoord = float2(1, 1);
//    Out[2].vPosition = mul(float4(p2, 1), VP);
//    Out[2].vTexcoord = float2(1, 0);
//    Out[3].vPosition = mul(float4(p3, 1), VP);
//    Out[3].vTexcoord = float2(0, 0);

//    [unroll]
//    for (int i = 0; i < 4; ++i)
//    {
//        Out[i].vLifeTime = In[0].vLifeTime;
//        Out[i].vProjPos = float4(length(camToP), 0, 0, 0);
//    }

//    // 두 삼각형 스트립 출력
//    Tri.Append(Out[0]);
//    Tri.Append(Out[1]);
//    Tri.Append(Out[2]);
//    Tri.RestartStrip();
//    Tri.Append(Out[0]);
//    Tri.Append(Out[2]);
//    Tri.Append(Out[3]);
//    Tri.RestartStrip();
//}


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
    
    if (Out.vColor.a < 0.003f)
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
    
    Out.vColor.a *= saturate(In.vLifeTime.x - In.vLifeTime.y);


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
    //vColor.a *= saturate(In.vLifeTime.x - In.vLifeTime.y);
    float lifeRatio = saturate(In.vLifeTime.y / In.vLifeTime.x); // 0 ~ 1
    float fade = smoothstep(0.8, 1.0, lifeRatio); // 0.8 이후부터 서서히 1.0으로
    fade = 1.0 - fade; // 남은 생명에 비례해 감소
    vColor.a *= fade;
    
    
    ////float fDepth = In.vProjPos.z / In.vProjPos.w;
    //float fDepth = In.vPosition.z;
    //
    //// 1 - 깊이 = 멀 수록 연하게
    //float fWeight = pow(saturate(1 - fDepth), 0.5f);
    ////float fWeight = 1.f;
    //float3 vPremulRGB = vColor.rgb * vColor.a;
    //
    //// rgb에 Premul * weight, a에 a * weight
    //Out.vAccumulation = float4(vPremulRGB * fWeight, vColor.a * fWeight);
    //
    //Out.fRevealage = vColor.a;
    //Out.vEmissive = float4(vPremulRGB * fWeight * g_fEmissiveIntensity, 0.f);
    ////Out.vEmissive = float4(fDepth, fWeight,0.f, 1.f);
    ////Out.vEmissive = float4(0.f, 0.f, 0.f, 0.f);
    
    
    /********************************************************************/
    //float fDepth = In.vProjPos.x / 1000.f;
    //float fWeight = pow(saturate(1 - fDepth), 2.f);
    //float3 vPremulRGB = vColor.rgb * vColor.a;
    //Out.vAccumulation = float4(vPremulRGB * fWeight, vColor.a * fWeight);
    //Out.fRevealage = vColor.a;
    //Out.vEmissive = float4(vPremulRGB, vColor.a);
    /********************************************************************/

    float3 vPremulRGB = vColor.rgb * vColor.a;
    Out.vAccumulation = float4(vPremulRGB, vColor.a);
    Out.fRevealage = vColor.a;
    Out.vEmissive = float4(vPremulRGB * g_fEmissiveIntensity, 0.f);
    
    
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
        

        VertexShader = compile vs_5_0 VS_MAIN_CS();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass MaskOnly //1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_OneBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN_CS();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_MASKONLY();
    }
    pass MaskOnly_WBGlow //2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_WBOIT, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN_CS();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_MASKONLY_WBGLOW();
    }
    pass MaskOnly_WBGlow_VStretch //3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnlyDepth, 0);
        SetBlendState(BS_WBOIT, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        

        VertexShader = compile vs_5_0 VS_MAIN_CS();
        GeometryShader = compile gs_5_0 GS_MAIN_VSTRETCH();
        PixelShader = compile ps_5_0 PS_MAIN_MASKONLY_WBGLOW();
    }
 
}
