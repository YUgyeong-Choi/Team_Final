#include "Renderer.h"
#include "GameObject.h"

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
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Depth"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 1.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Specular"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PickPos"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Shadow"), g_iMaxWidth, g_iMaxHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.0f, 1.0f, 1.0f, 1.0f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Final"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 0.0f, 0.0f, 1.0f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_BlurX"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.0f, 1.0f, 1.0f, 1.0f))))
		return E_FAIL;	

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Diffuse"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Normal"))))
		return E_FAIL;		
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Depth"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_PickPos"))))
		return E_FAIL;




#pragma region RenderPBR
	/* [ PBR ����Ÿ�� ] */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_Diffuse"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_Normal"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.5f, 0.5f, 1.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_ARM"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.0f, 0.5f, 0.0f, 1.0f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_PBR_Depth"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 0.f, 0.f, 0.f))))
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

	/* [ PBR ��Ƽ����Ÿ�� ] */
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_Diffuse"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_Normal"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_ARM"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_Depth"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_Specular"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_AO"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRGameObjects"), TEXT("Target_PBR_Roughness"))))

		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRFinal"), TEXT("Target_PBR_Metallic"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_PBRFinal"), TEXT("Target_PBR_Final"))))
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
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_BlurX"), TEXT("Target_BlurX"))))
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

	if (FAILED(Ready_DepthStencilView(g_iMaxWidth, g_iMaxHeight)))
		return E_FAIL;

#ifdef _DEBUG
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Diffuse"), 100.0f, 100.0f, 200.0f, 200.0f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Normal"), 100.0f, 300.0f, 200.0f, 200.0f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Depth"), 100.0f, 500.0f, 200.0f, 200.0f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Shade"), 300.f, 100.0f, 200.0f, 200.0f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Specular"), 300.f, 300.0f, 200.0f, 200.0f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_Shadow"), ViewportDesc.Width - 150.f, 150.0f, 300.0f, 300.0f)))
		return E_FAIL;

	/* [ PBR ����� ] */
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
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_Specular"), GetTargetX(4), GetTargetY(4), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_AO"), GetTargetX(4), GetTargetY(3), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_Roughness"), GetTargetX(4), GetTargetY(2), fSizeX, fSizeY)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_Metallic"), GetTargetX(4), GetTargetY(1), fSizeX, fSizeY)))
		return E_FAIL;

	fSizeX = 1600.f / 2;
	fSizeY = 900.f / 2;
	if (FAILED(m_pGameInstance->Ready_RT_Debug(TEXT("Target_PBR_Final"), fSizeX, fSizeY, fSizeX, fSizeY)))
		return E_FAIL;
	
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
		return E_FAIL;	
	if (FAILED(Render_Shadow()))
		return E_FAIL;
	
	if (FAILED(Render_NonBlend()))
		return E_FAIL;

	if (FAILED(Render_PBRMesh()))
		return E_FAIL;

	if (FAILED(Render_Lights()))
		return E_FAIL;

	if (FAILED(Render_PBRLights()))
		return E_FAIL;

	if (FAILED(Render_BackBuffer()))
		return E_FAIL;

	//if (FAILED(Render_Blur()))
	//	return E_FAIL;

	if (FAILED(Render_NonLight()))
		return E_FAIL;



	/* ���������� ����۸� �ϼ���Ű��.  */
	if (FAILED(Render_Blend()))
		return E_FAIL;
	if (FAILED(Render_UI()))
		return E_FAIL;

#ifdef _DEBUG


	if (FAILED(Render_Debug()))
		return E_FAIL;

#endif
	

	return S_OK;
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
	m_pGameInstance->Begin_MRT(TEXT("MRT_ShadowObjects"), m_pShadowDSV, true, true);

	if (FAILED(Change_ViewportDesc(g_iMaxWidth, g_iMaxHeight)))
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
HRESULT CRenderer::Render_PBRMesh()
{
	/* [ PBR ������ ] */
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


//_bool Compare(CGameObject* pSour, CGameObject* pDest)
//{
//	return dynamic_cast<CBlendObject*>(pSour)->Get_Depth() > dynamic_cast<CBlendObject*>(pDest)->Get_Depth();
//}

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
	for (auto& pGameObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_UI)])
	{
		if (nullptr != pGameObject)
			pGameObject->Render();

		Safe_Release(pGameObject);
	}
	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_UI)].clear();

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

	/* ��ġ�� ����۷� �����Ѵ�. */
	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_PBRLights()
{
	/* Shade */
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

	m_iCurrentRenderLevel = m_pGameInstance->GetCurrentLevelIndex();
	m_pGameInstance->Render_PBR_Lights(m_pShader, m_pVIBuffer, m_iCurrentRenderLevel);

	/* ��ġ�� ����۷� �����Ѵ�. */
	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

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
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Shadow"), m_pShader, "g_ShadowTexture")))
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

	if (FAILED(m_pShader->Bind_Matrix("g_LightViewMatrix", m_pGameInstance->Get_Light_ViewMatrix())))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_LightProjMatrix", m_pGameInstance->Get_Light_ProjMatrix())))
		return E_FAIL;

	m_pShader->Begin(3);

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

HRESULT CRenderer::Render_Blur()
{
	m_pGameInstance->Begin_MRT(TEXT("MRT_BlurX"));

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Final"), m_pShader, "g_FinalTexture")))
		return E_FAIL;

	m_pShader->Begin(4);
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	
	/* ����ۿ� ��´�. */
	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_BlurX"), m_pShader, "g_BlurXTexture")))
		return E_FAIL;

	m_pShader->Begin(5);
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	return S_OK;
}

HRESULT CRenderer::Ready_DepthStencilView(_uint iWidth, _uint iHeight)
{
	ID3D11Texture2D* pDepthStencilTexture = nullptr;

	D3D11_TEXTURE2D_DESC	TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	/* ���� ������ �ȼ��� ������� �ȼ��� ������ �����ؾ߸� ���� �ؽ�Ʈ�� ����������. */
	/* �ȼ��� ���� �ٸ��� �ƿ� �������� ����. */
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

		m_pGameInstance->Render_MRT_Debug(TEXT("MRT_GameObjects"), m_pShader, m_pVIBuffer);
		m_pGameInstance->Render_MRT_Debug(TEXT("MRT_Lights"), m_pShader, m_pVIBuffer);
		m_pGameInstance->Render_MRT_Debug(TEXT("MRT_ShadowObjects"), m_pShader, m_pVIBuffer);
		m_pGameInstance->Render_MRT_Debug(TEXT("MRT_ShadowObjects"), m_pShader, m_pVIBuffer);
		m_pGameInstance->Render_MRT_Debug(TEXT("MRT_PBRGameObjects"), m_pShader, m_pVIBuffer);
		m_pGameInstance->Render_MRT_Debug(TEXT("MRT_PBRFinal"), m_pShader, m_pVIBuffer);
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
