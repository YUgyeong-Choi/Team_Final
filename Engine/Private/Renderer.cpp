#include "Renderer.h"
#include "GameObject.h"
#include "UIObject.h"
#include "BlendObject.h"
#include "GameInstance.h"


CRenderer::CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::Get_Instance()}
{

	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	
}

HRESULT CRenderer::Initialize()
{
	_uint				iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Diffuse"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Normal"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.0f, 1.0f, 1.0f, 1.0f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Shade"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Depth"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 1.f, 0.f, -1.f)))) //w에  -1.f를채워 id가 -1인것 표현
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Specular"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PickPos"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Shadow"), g_iMaxWidth, g_iMaxHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.0f, 1.0f, 1.0f, 1.0f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Final"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 0.0f, 0.0f, 1.0f))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Diffuse"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Normal"))))
		return E_FAIL;		
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Depth"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_PickPos"))))
		return E_FAIL;


#pragma region 데칼
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Decal"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Decals"), TEXT("Target_Decal"))))
		return E_FAIL;
#pragma endregion






#pragma region RenderPBR
	/* [ PBR 렌더타겟 ] */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_Diffuse"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_Normal"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.5f, 0.5f, 1.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_ARM"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.0f, 0.5f, 0.0f, 1.0f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_Depth"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 0.f, 0.f, -1.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_AO"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_Roughness"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_Metallic"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_Specular"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_Final"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* [ 케스케이드 쉐도우 ] */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_ShadowA"), g_iMiddleWidth, g_iMiddleHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.0f, 1.0f, 1.0f, 1.0f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_ShadowB"), g_iMiddleWidth, g_iMiddleHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.0f, 1.0f, 1.0f, 1.0f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_ShadowC"), g_iMiddleWidth, g_iMiddleHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.0f, 1.0f, 1.0f, 1.0f))))
		return E_FAIL;

	/* [ 볼륨메트릭 포그 ] */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Volumetric"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* [ PBR 멀티렌더타겟 ] */
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_Diffuse"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_Normal"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_ARM"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_Depth"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_AO"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_Roughness"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_Metallic"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRFinal"), TEXT("Target_PBR_Specular"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRFinal"), TEXT("Target_PBR_Final"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRShadow"), TEXT("Target_PBR_ShadowA"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRShadow"), TEXT("Target_PBR_ShadowB"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRShadow"), TEXT("Target_PBR_ShadowC"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Volumetric"), TEXT("Target_Volumetric"))))
		return E_FAIL;
#pragma endregion


#pragma region 이펙트용 MRT
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_EffectBlend_Diffuse"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EffectBlendObjects"), TEXT("Target_EffectBlend_Diffuse"))))
		return E_FAIL;

	// 다른 곳에서도 블러 사용할 수도 있으니까 블러용 렌더타겟은 별도로 두고 글로우 용 렌더타겟 따로 두겠습니다
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_EffectBlend_Glow"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EffectBlend_Glow"), TEXT("Target_EffectBlend_Glow"))))
		return E_FAIL;

	// WB용 글로우 나눠두고 테스트 함 
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_EffectBlend_WBGlow"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EffectBlend_WBGlow"), TEXT("Target_EffectBlend_WBGlow"))))
		return E_FAIL;

	// downscale 배율
	//m_fDownscaledRatio = 0.0625f; // 1/16
	//m_fDownscaledRatio = 0.125f;	// 1/8
	m_fDownscaledRatio = 0.25f;		// 1/4

	_uint iDownscaledWidth = static_cast<_uint>(ViewportDesc.Width * m_fDownscaledRatio);
	_uint iDownscaledHeight = static_cast<_uint>(ViewportDesc.Height * m_fDownscaledRatio);

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Effect_Downscaled"), iDownscaledWidth, iDownscaledHeight, DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Effectblend_Downscaled"), TEXT("Target_Effect_Downscaled"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_DownscaledBlurX"), iDownscaledWidth, iDownscaledHeight, DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_DownscaledBlurY"), iDownscaledWidth, iDownscaledHeight, DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_BlurX"), TEXT("Target_DownscaledBlurX"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_BlurY"), TEXT("Target_DownscaledBlurY"))))
		return E_FAIL;


	if (FAILED(Ready_DepthStencilView_Blur(iDownscaledWidth, iDownscaledHeight)))
		return E_FAIL;


	/* [ Weighted Blend ] */
	// 색상 합산을 위한 렌더타겟.
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Effect_WB_Accumulation"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	// 알파 연산을 위한 렌더타겟.
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Effect_WB_Revealage"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(1.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	// 글로우가 필요한 이펙트를 따로 모아두는 렌더타겟.
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Effect_WB_Emissive"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_FLOAT , _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Effect_WeightedBlend"), TEXT("Target_Effect_WB_Accumulation"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Effect_WeightedBlend"), TEXT("Target_Effect_WB_Revealage"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Effect_WeightedBlend"), TEXT("Target_Effect_WB_Emissive"))))
		return E_FAIL;

	// Weighted Blend 연산이 끝난 결과물 담는 렌더타겟.
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Effect_WB_Composite"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Effect_WeightedBlend_Composite"), TEXT("Target_Effect_WB_Composite"))))
		return E_FAIL;


	// 트레일 적용 후 디스토션 렌더타겟에 넣고 나면 주석을 푸세요 // 
	//if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Effect_Distort"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 1.f, 0.f))))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EffectBlendObjects"), TEXT("Target_Effect_Distort"))))
	//	return E_FAIL;

#pragma endregion


#pragma region RenderUI

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_UI_Diffuse"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_UIObjects"), TEXT("Target_UI_Diffuse"))))
		return E_FAIL;

#pragma endregion


	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Lights"), TEXT("Target_Shade"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Lights"), TEXT("Target_Specular"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Final"), TEXT("Target_Final"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_ShadowObjects"), TEXT("Target_Shadow"))))
		return E_FAIL;




	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Deferred.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(ViewportDesc.Width, ViewportDesc.Height, 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.0f, 1.f));

	m_iOriginalViewportWidth = static_cast<_uint>(ViewportDesc.Width);
	m_iOriginalViewportHeight = static_cast<_uint>(ViewportDesc.Height);

	if (FAILED(Ready_DepthStencilView_Shadow(g_iMiddleWidth, g_iMiddleHeight)))
		return E_FAIL;

#ifdef _DEBUG

	/* [ PBR 디버깅 ] */
	_float fSizeX = 1600.f / 5;
	_float fSizeY = 900.f / 5;
	_float fOffset = 3.f;

	const _float fStartX = fSizeX / 2.f;
	const _float fStartY = fSizeY / 2.f;

	auto GetTargetY = [&](int index) {
		return fStartY + index * (fSizeY + fOffset);
		};

	auto GetTargetX = [&](int index) {
		return fStartX + index * (fSizeX + fOffset);
		};

	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_Diffuse"), GetTargetX(0), GetTargetY(4), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_Normal"), GetTargetX(1), GetTargetY(4), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_ARM"), GetTargetX(2), GetTargetY(4), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_Depth"), GetTargetX(3), GetTargetY(4), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_AO"), GetTargetX(4), GetTargetY(4), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_Roughness"), GetTargetX(4), GetTargetY(3), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_Metallic"), GetTargetX(4), GetTargetY(2), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_Specular"), GetTargetX(4), GetTargetY(1), fSizeX, fSizeY)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_ShadowA"), GetTargetX(0), GetTargetY(3), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_ShadowB"), GetTargetX(0), GetTargetY(2), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_ShadowC"), GetTargetX(0), GetTargetY(1), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Volumetric"), GetTargetX(0), GetTargetY(0), fSizeX, fSizeY)))
		return E_FAIL;

#pragma region CY Debug
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Effect_WB_Composite"), GetTargetX(0), GetTargetY(0), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_EffectBlend_WBGlow"), GetTargetX(0), GetTargetY(1), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Effect_WB_Accumulation"), GetTargetX(0), GetTargetY(2), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Effect_WB_Revealage"), GetTargetX(0), GetTargetY(3), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Effect_WB_Emissive"), GetTargetX(0), GetTargetY(4), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_EffectBlend_Diffuse"), GetTargetX(4), GetTargetY(0), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_EffectBlend_Glow"), GetTargetX(4), GetTargetY(1), fSizeX, fSizeY)))
		return E_FAIL;

#pragma endregion

#pragma region YW Debug

	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Diffuse"), GetTargetX(0), GetTargetY(0), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Normal"), GetTargetX(0), GetTargetY(1), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Depth"), GetTargetX(0), GetTargetY(2), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Shade"), GetTargetX(0), GetTargetY(3), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Decal"), GetTargetX(1), GetTargetY(0), fSizeX, fSizeY)))
		return E_FAIL;

#pragma endregion

	m_StartTime = std::chrono::steady_clock::now();
#endif

	return S_OK;
}

HRESULT CRenderer::Add_RenderGroup(RENDERGROUP eRenderGroup, CGameObject* pRenderObject)
{
	if (eRenderGroup >=	RENDERGROUP::RG_END ||
		nullptr == pRenderObject)
		return E_FAIL;

	m_RenderObjects[ENUM_CLASS(eRenderGroup)].push_back(pRenderObject);

	Safe_AddRef(pRenderObject);

	return S_OK;
}

HRESULT CRenderer::Draw()
{

	
	
	if (FAILED(Render_Priority()))
	{
		MSG_BOX("Render Priority Failed");
		return E_FAIL;
	}

	if (FAILED(Render_Shadow()))
	{
		MSG_BOX("Render_Shadow Failed");
		return E_FAIL;
	}
	
	if (FAILED(Render_NonBlend()))
	{
		MSG_BOX("Render_NonBlend Failed");
		return E_FAIL;
	}

	if (FAILED(Render_PBRMesh()))
	{
		MSG_BOX("Render_PBRMesh Failed");
		return E_FAIL;
	}

	if (FAILED(Render_Decal()))
	{
		MSG_BOX("Render_Decal Failed");
		return E_FAIL;
	}

	if (FAILED(Render_Lights()))
	{
		MSG_BOX("Render_Lights Failed");
		return E_FAIL;
	}

	if (FAILED(Render_PBRLights()))
	{
		MSG_BOX("Render_PBRLights Failed");
		return E_FAIL;
	}

	if (FAILED(Render_Volumetric()))
	{
		MSG_BOX("Render_Volumetric Failed");
		return E_FAIL;
	}

	if (FAILED(Render_Effect_Blend()))
	{
		MSG_BOX("Render_Effect_Blend Failed");
		return E_FAIL;
	}
	if (FAILED(Render_Effect_WB()))
	{
		MSG_BOX("Render_Effect_Blend Failed");
		return E_FAIL;
	}
	
	if (FAILED(Render_Effect_WB_Composite()))
	{
		MSG_BOX("Render_Effect_WB_Composite Failed");
		return E_FAIL;
	}

	//if (FAILED(Render_Blur(TEXT("Target_EffectBlend_Diffuse"))))
	if (FAILED(Render_Blur(TEXT("Target_Effect_WB_Emissive"))))
	{
		MSG_BOX("Render_Blur - Target_Effect_WB_Emissive Failed");
		return E_FAIL;
	}

	if (FAILED(Render_Effect_WBGlow()))
	{
		MSG_BOX("Render_Effect_WBGlow Failed");
		return E_FAIL;
	}

	if (FAILED(Render_Blur(TEXT("Target_EffectBlend_Diffuse"))))
	{
		MSG_BOX("Render_Blur - Target_EffectBlend_Diffuse Failed");
		return E_FAIL;
	}

	if (FAILED(Render_Effect_Glow()))
	{
		MSG_BOX("Render_Effect_Glow Failed");
		return E_FAIL;
	}


	//if (FAILED(Render_Effect_NonLight()))
	//	return E_FAIL;

	if (FAILED(Render_BackBuffer()))
	{
		MSG_BOX("Render_BackBuffer Failed");
		return E_FAIL;
	}

	if (FAILED(Render_NonLight()))
	{
		MSG_BOX("Render_NonLight Failed");
		return E_FAIL;
	}


	/* 블렌딩이전에 백버퍼를 완성시킨다.  */
	if (FAILED(Render_Blend()))
	{
		MSG_BOX("Render_Blend Failed");
		return E_FAIL;
	}

	
	if (FAILED(Render_UI_Deferred()))
	{
		MSG_BOX("Render_UI_Deferred Failed");
		return E_FAIL;
	}
	
	
	if (FAILED(Render_UI()))
	{
		MSG_BOX("Render_UI Failed");
		return E_FAIL;
	}



#ifdef _DEBUG


	if (FAILED(Render_Debug()))
	{
		MSG_BOX("Render_Debug Failed");
		return E_FAIL;
	}

#endif
	

	return S_OK;
}

void CRenderer::ClearRenderObjects()
{
	for (int i = 0; i < static_cast<int>(RENDERGROUP::RG_END); ++i)
	{
		for (CGameObject* pObj : m_RenderObjects[i])
		{
			Safe_Release(pObj);
		}
		m_RenderObjects[i].clear();
	}
}

#ifdef _DEBUG

HRESULT CRenderer::Add_DebugComponent(CComponent* pDebugCom)
{
	m_DebugComponent.push_back(pDebugCom);

	Safe_AddRef(pDebugCom);

	return S_OK;
}

#endif

HRESULT CRenderer::Render_Priority()
{
	//m_pGameInstance->Begin_MRT(TEXT("MRT_Final"));

	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_PRIORITY)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_PRIORITY)].clear();

	//m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_Shadow()
{
	m_pGameInstance->Begin_MRT(TEXT("MRT_PBRShadow"), m_pShadowDSV, true, true);

	if (FAILED(Change_ViewportDesc(g_iMiddleWidth, g_iMiddleHeight)))
		return E_FAIL;

	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_SHADOW)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render_Shadow();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_SHADOW)].clear();

	m_pGameInstance->End_MRT();

	if (FAILED(Change_ViewportDesc(m_iOriginalViewportWidth, m_iOriginalViewportHeight)))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_NonBlend()
{
	/* Diffuse + Normal */
	m_pGameInstance->Begin_MRT(TEXT("MRT_GameObjects"));

	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_NONBLEND)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_NONBLEND)].clear();

	m_pGameInstance->End_MRT();

	return S_OK;
}
HRESULT CRenderer::Render_Decal()
{
	m_pGameInstance->Begin_MRT(TEXT("MRT_Decals"));

	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_DECAL)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_DECAL)].clear();

	m_pGameInstance->End_MRT();

	return S_OK;
}
HRESULT CRenderer::Render_PBRMesh()
{
	/* [ PBR 렌더링 ] */
	m_pGameInstance->Begin_MRT(TEXT("MRT_PBRGameObjects"));

	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_PBRMESH)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_PBRMESH)].clear();

	m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_Blend()
{
	/*m_RenderObjects[RG_BLEND].sort([](CGameObject* pSour, CGameObject* pDest)->_bool
	{
		return dynamic_cast<CBlendObject*>(pSour)->Get_Depth() > dynamic_cast<CBlendObject*>(pDest)->Get_Depth();
	});*/

	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_BLEND)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_BLEND)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_UI()
{
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_UI)].sort([](CGameObject* pSour, CGameObject* pDest)->_bool
	{
		return dynamic_cast<CUIObject*>(pSour)->Get_Depth() > dynamic_cast<CUIObject*>(pDest)->Get_Depth();
	});


	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_UI)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_UI)].clear();


	//

	

	

	return S_OK;
}

HRESULT CRenderer::Render_Lights()
{
	/* Shade */
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Lights"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")))
		return E_FAIL;	
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_Transform_Float4x4_Inv(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_Transform_Float4x4_Inv(D3DTS::PROJ))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	m_pGameInstance->Render_Lights(m_pShader, m_pVIBuffer);

	/* 장치에 백버퍼로 복구한다. */
	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_PBRLights()
{
	m_iCurrentRenderLevel = m_pGameInstance->GetCurrentLevelIndex();

	/* [ PBR 최종(라이팅에서 끝남) ] */
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_PBRFinal"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_Diffuse"), m_pShader, "g_PBR_Diffuse")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_Normal"), m_pShader, "g_PBR_Normal")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_ARM"), m_pShader, "g_PBR_ARM")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_Depth"), m_pShader, "g_PBR_Depth")))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_Transform_Float4x4_Inv(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_Transform_Float4x4_Inv(D3DTS::PROJ))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL; 

	m_pGameInstance->Render_PBR_Lights(m_pShader, m_pVIBuffer, m_iCurrentRenderLevel);

	/* 장치에 백버퍼로 복구한다. */
	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Volumetric()
{
	/* [ 렌더타겟 클리어 x , 뎁스 클리어 x ] */
	m_pGameInstance->Begin_MRT(TEXT("MRT_Volumetric"));
		
	/* [ 볼륨메트리를 위한 매핑 ] */
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_Depth"), m_pShader, "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_ShadowA"), m_pShader, "g_ShadowTextureA")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_ShadowB"), m_pShader, "g_ShadowTextureB")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_ShadowC"), m_pShader, "g_ShadowTextureC")))
		return E_FAIL;

	/* [ 포인트라이트 개수 추가 ] */
	_uint iPointNum = m_pGameInstance->Get_LightCount(2, m_iCurrentRenderLevel);
	m_pShader->Bind_RawValue("g_iPointNum", &iPointNum, sizeof(_uint));

	/* [ 포그 애니메이션 추가 ] */
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsed = now - m_StartTime;

	_float g_fTime = elapsed.count();
	if (FAILED(m_pShader->Bind_RawValue("g_fTime", &g_fTime, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_Transform_Float4x4_Inv(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_Transform_Float4x4_Inv(D3DTS::PROJ))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	m_pGameInstance->Render_Volumetric_Lights(m_pShader, m_pVIBuffer, m_iCurrentRenderLevel);
	m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_BackBuffer()
{
	//m_pGameInstance->Begin_MRT(TEXT("MRT_Final"), nullptr, false);

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Shade"), m_pShader, "g_ShadeTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Specular"), m_pShader, "g_SpecularTexture")))
		return E_FAIL;

	//데칼 텍스쳐
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Decal"), m_pShader, "g_DecalTexture")))
		return E_FAIL;

	/* [ PBR 렌더링용 ] */
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_Final"), m_pShader, "g_PBR_Final")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_Depth"), m_pShader, "g_DepthTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_ShadowA"), m_pShader, "g_ShadowTextureA")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_ShadowB"), m_pShader, "g_ShadowTextureB")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_ShadowC"), m_pShader, "g_ShadowTextureC")))
		return E_FAIL;

	/* [ Effect 렌더링용 ]*/
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_EffectBlend_Diffuse"), m_pShader, "g_EffectBlend_Diffuse")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Effect_WB_Composite"), m_pShader, "g_EffectBlend_WBComposite")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_EffectBlend_Glow"), m_pShader, "g_EffectBlend_Glow")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_EffectBlend_WBGlow"), m_pShader, "g_EffectBlend_WBGlow")))
		return E_FAIL;

	// 트레일 적용 후 디스토션 렌더타겟에 넣고 나면 주석을 푸세요 // 
	//if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Effect_Distort"), m_pShader, "g_Effect_Distort")))
	//	return E_FAIL;

	/* [ 볼륨메트릭 포그 ] */
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Volumetric"), m_pShader, "g_VolumetricTexture")))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_Transform_Float4x4_Inv(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_Transform_Float4x4_Inv(D3DTS::PROJ))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_LightViewMatrixA", m_pGameInstance->Get_Light_ViewMatrix(SHADOW::SHADOWA))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_LightProjMatrixA", m_pGameInstance->Get_Light_ProjMatrix(SHADOW::SHADOWA))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_LightViewMatrixB", m_pGameInstance->Get_Light_ViewMatrix(SHADOW::SHADOWB))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_LightProjMatrixB", m_pGameInstance->Get_Light_ProjMatrix(SHADOW::SHADOWB))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_LightViewMatrixC", m_pGameInstance->Get_Light_ViewMatrix(SHADOW::SHADOWC))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_LightProjMatrixC", m_pGameInstance->Get_Light_ProjMatrix(SHADOW::SHADOWC))))
		return E_FAIL;

	m_pShader->Begin(ENUM_CLASS(DEFEREDPASS::DEFERRED));

	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	//m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_NonLight()
{
	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_NONLIGHT)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_NONLIGHT)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_UI_Deferred()
{
	if (m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_UI_DEFERRED)].empty())
		return S_OK;

	m_pGameInstance->Begin_MRT(TEXT("MRT_UIObjects"));

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_UI_DEFERRED)].sort([](CGameObject* pSour, CGameObject* pDest)->_bool
		{
			return dynamic_cast<CUIObject*>(pSour)->Get_Depth() > dynamic_cast<CUIObject*>(pDest)->Get_Depth();
		});


	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_UI_DEFERRED)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_UI_DEFERRED)].clear();


	m_pGameInstance->End_MRT();

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_UI_Diffuse"), m_pShader, "g_UITexture")))
		return E_FAIL;

	m_pShader->Begin(ENUM_CLASS(DEFEREDPASS::VIGNETTING));
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	return S_OK;
}

HRESULT CRenderer::Render_Effect_Blend()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_EffectBlendObjects"))))
		return E_FAIL;

	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_EFFECT_GLOW)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_EFFECT_GLOW)].clear();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Blur(const _wstring& strTargetTag)
{
	_float fCurVPSizeX = { static_cast<_float>(m_iOriginalViewportWidth) };
	_float fCurVPSizeY = { static_cast<_float>(m_iOriginalViewportHeight) };

	m_pGameInstance->Begin_MRT(TEXT("MRT_BlurX"), m_pBlurDSV, true, true);
	// 뷰포트
	if (FAILED(Change_ViewportDesc(static_cast<_uint>(m_iOriginalViewportWidth * m_fDownscaledRatio), static_cast<_uint>(m_iOriginalViewportHeight * m_fDownscaledRatio))))
		return E_FAIL;

	fCurVPSizeX *= m_fDownscaledRatio;
	fCurVPSizeY *= m_fDownscaledRatio;


	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_fViewportSizeX", &fCurVPSizeX, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(strTargetTag, m_pShader, "g_PreBlurTexture")))
		return E_FAIL;

	m_pShader->Begin(ENUM_CLASS(DEFEREDPASS::BLURX));
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

		m_pGameInstance->Begin_MRT(TEXT("MRT_BlurY"), m_pBlurDSV, true, true);

	/* 백버퍼에 찍는다. */
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_DownscaledBlurX"), m_pShader, "g_BlurXTexture")))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fViewportSizeY", &fCurVPSizeY, sizeof(_float))))
		return E_FAIL;

	m_pShader->Begin(ENUM_CLASS(DEFEREDPASS::BLURY));
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();
	m_pGameInstance->End_MRT();

	if (FAILED(Change_ViewportDesc(m_iOriginalViewportWidth, m_iOriginalViewportHeight)))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Effect_Glow()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_EffectBlend_Glow"))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_DownscaledBlurY"), m_pShader, "g_BlurYTexture")))
		return E_FAIL;

	m_pShader->Begin(ENUM_CLASS(DEFEREDPASS::EFFECT_GLOW));
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_Effect_WBGlow()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_EffectBlend_WBGlow"))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_DownscaledBlurY"), m_pShader, "g_BlurYTexture")))
		return E_FAIL;

	m_pShader->Begin(ENUM_CLASS(DEFEREDPASS::EFFECT_GLOW));
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	return S_OK;
}

HRESULT CRenderer::Render_Effect_NonLight()
{
	// 당장 안 쓰고 상황 봐서 사용할 것
	// 예상가는사용처: 블렌드 필요 없는 SubUV SE/PE
	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_EFFECT_NL)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_EFFECT_NL)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_Effect_WB()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Effect_WeightedBlend"))))
		return E_FAIL;

	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_EFFECT_WB)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_EFFECT_WB)].clear();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Effect_WB_Composite()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Effect_WeightedBlend_Composite"))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Effect_WB_Accumulation"), m_pShader, "g_WB_Accumulation")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Effect_WB_Revealage"), m_pShader, "g_WB_Revealage")))
		return E_FAIL;

	m_pShader->Begin(ENUM_CLASS(DEFEREDPASS::WB_COMPOSITE));
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();
	return S_OK;
}

HRESULT CRenderer::Ready_DepthStencilView_Shadow(_uint iWidth, _uint iHeight)
{
	ID3D11Texture2D* pDepthStencilTexture = nullptr;

	D3D11_TEXTURE2D_DESC	TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	/* 깊이 버퍼의 픽셀은 백버퍼의 픽셀과 갯수가 동일해야만 깊이 텍스트가 가능해진다. */
	/* 픽셀의 수가 다르면 아에 렌더링을 못함. */
	TextureDesc.Width = iWidth;
	TextureDesc.Height = iHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;	
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pDepthStencilTexture)))
		return E_FAIL;


	if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pShadowDSV)))
		return E_FAIL;

	Safe_Release(pDepthStencilTexture);

	return S_OK;
}

HRESULT CRenderer::Ready_DepthStencilView_Blur(_uint iWidth, _uint iHeight)
{
	ID3D11Texture2D* pDepthStencilTexture = nullptr;

	D3D11_TEXTURE2D_DESC	TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	TextureDesc.Width = iWidth;
	TextureDesc.Height = iHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pDepthStencilTexture)))
		return E_FAIL;


	if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pBlurDSV)))
		return E_FAIL;

	Safe_Release(pDepthStencilTexture);

	return S_OK;
}

HRESULT CRenderer::Change_ViewportDesc(_uint iWidth, _uint iHeight)
{
	D3D11_VIEWPORT			ViewportDesc{};
	_uint					iNumViewports = { 1 };

	ViewportDesc.TopLeftX = 0;
	ViewportDesc.TopLeftY = 0;
	ViewportDesc.Width = static_cast<_float>(iWidth);
	ViewportDesc.Height = static_cast<_float>(iHeight);
	ViewportDesc.MinDepth = 0.f;
	ViewportDesc.MaxDepth = 1.f;	

	m_pContext->RSSetViewports(iNumViewports, &ViewportDesc);

	return S_OK;
}

HRESULT CRenderer::Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg)
{
	CComponent* pComponent = static_cast<CComponent*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_COMPONENT, iPrototypeLevelIndex, strPrototypeTag, pArg));
	if (nullptr == pComponent)
		return E_FAIL;

	*ppOut = pComponent;

	return S_OK;
}


#ifdef _DEBUG

HRESULT CRenderer::Render_Debug()
{
	if (m_bRenderCollider)
	{
		for (auto& pDebugCom : m_DebugComponent)
		{
			pDebugCom->Render();
			Safe_Release(pDebugCom);
		}
		m_DebugComponent.clear();
	}

	if (m_bRenderTarget)
	{

		m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix);
		m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix);

		switch (m_eDebugRT)
		{
		case Engine::CRenderer::DEBUGRT_DH:
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_PBRFinal"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_GameObjects"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_Lights"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_ShadowObjects"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_ShadowObjects"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_PBRGameObjects"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_PBRShadow"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_Volumetric"), m_pShader, m_pVIBuffer);
			break;
		case Engine::CRenderer::DEBUGRT_YW:
			/* 여기에 MRT 입력 */
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_GameObjects"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_Lights"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_Decals"), m_pShader, m_pVIBuffer);

			break;
		case Engine::CRenderer::DEBUGRT_CY:
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_EffectBlendObjects"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_EffectBlend_Glow"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_Effect_WeightedBlend"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_Effect_WeightedBlend_Composite"), m_pShader, m_pVIBuffer);
			m_pGameInstance->Render_MRT_Debug(TEXT("MRT_EffectBlend_WBGlow"), m_pShader, m_pVIBuffer);
			break;
		default:
			break;
		}
	}

	return S_OK;
}

#endif

CRenderer* CRenderer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRenderer* pInstance = new CRenderer(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CRenderer");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CRenderer::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);

	Safe_Release(m_pShadowDSV);
	Safe_Release(m_pBlurDSV);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pShader);

	for (auto& ObjectList : m_RenderObjects)
	{
		for (auto& pGameObject : ObjectList)
			Safe_Release(pGameObject);
		ObjectList.clear();
	}		
}
