#include "UI_Button.h"
#include "GameInstance.h"
#include "UI_Feature.h"

CUI_Button::CUI_Button(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CDynamic_UI{pDevice, pContext}
{
}

CUI_Button::CUI_Button(const CUI_Button& Prototype)
	:CDynamic_UI{Prototype}
{
}

HRESULT CUI_Button::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Button::Initialize(void* pArg)
{
	
	BUTTON_UI_DESC* pDesc = static_cast<BUTTON_UI_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(m_strTextureTag)))
		return E_FAIL;



	m_strCaption = pDesc->strCaption;

	m_fPadding = pDesc->fPadding;

	return S_OK;
}

void CUI_Button::Priority_Update(_float fTimeDelta)
{
}

void CUI_Button::Update(_float fTimeDelta)
{
	// 업데이트 할꺼? 잇나

	
}

void CUI_Button::Late_Update(_float fTimeDelta)
{
	if (!m_isVignetting)
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI, this);
	else
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI_DEFERRED, this);
}

HRESULT CUI_Button::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(D_UI_BUTTON)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	m_pGameInstance->Draw_Font(TEXT("Font_Bold"), m_strCaption.c_str(), { m_fX - 0.5f * m_fSizeX , m_fY - 0.5f * m_fSizeY + m_fPadding.y * 0.5f }, XMLoadFloat4(&m_vColor), 0.f, { 0.f,0.f }, 1.f);


	return S_OK;
}

HRESULT CUI_Button::Ready_Components(const wstring& strTextureTag)
{
	
	

	if (strTextureTag != L"")
	{

		m_isHasTexture = true;
	}

	// 마스크, 하이라이트 텍스처 있으면 좋겠는데? 
	
		/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Button_Hover"),
		TEXT("Com_Texture_Hover"), reinterpret_cast<CComponent**>(&m_pHoverTextureCom))))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Button_Highlight"),
		TEXT("Com_Texture_Highlight"), reinterpret_cast<CComponent**>(&m_pHighlightTextureCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_Button::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;


	if (nullptr != m_pTextureCom)
	{
		if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureIndex)))
			return E_FAIL;
	}

	// 텍스처 쓸지 말지 플래그 던지고, 텍스처 던지기

	_float4 flag = { _float(m_isHasTexture), _float(m_isMouseHover), _float(m_isMouseHover) , 0.f };

	if (m_isHighlight)
	{
		m_vColor = { 1.f,0.f,0.f,1.f };
	}
	else 
	{
		m_vColor = { 1.f,1.f,1.f,1.f };
	
	}
	if (FAILED(m_pShaderCom->Bind_RawValue("g_Color", &m_vColor, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_ButtonFlag", &flag, sizeof(_float4))))
		return E_FAIL;

	// 기본값을 던지고, 추가로 덮어쓰자
	if (FAILED(m_pHighlightTextureCom->Bind_ShaderResource(m_pShaderCom, "g_HighlightTexture", m_iTextureIndex)))
		return E_FAIL;

	if (FAILED(m_pHoverTextureCom->Bind_ShaderResource(m_pShaderCom, "g_HoverTexture", m_iTextureIndex)))
		return E_FAIL;

	_float fAlpha = 1.f;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Alpha", &fAlpha, sizeof(_float))))
		return E_FAIL;


	for (auto& pFeature : m_pUIFeatures)
	{
		if (nullptr != pFeature)
			pFeature->Bind_ShaderResources(m_pShaderCom);
	}



	return S_OK;
}

_bool CUI_Button::Check_MousePos()
{
	POINT ptScreen;
	GetCursorPos(&ptScreen);

	// 항상 내 게임 윈도우 기준으로 좌표 변환해야 함
	ScreenToClient(g_hWnd, &ptScreen);


	if (ptScreen.x < m_fX - 0.5f * m_fSizeX || ptScreen.x >m_fX + 0.5f * m_fSizeX)
		return false;

	if (ptScreen.y < m_fY - 0.5f * m_fSizeY || ptScreen.y > m_fY + 0.5f * m_fSizeY)
		return false;


	return true;
}

_bool CUI_Button::Check_MouseHover()
{

		m_isMouseHover = Check_MousePos();

		return m_isMouseHover;
}

_bool CUI_Button::Check_Click()
{
	m_isHighlight = Check_MousePos();

	return m_isHighlight;
}

CUI_Button* CUI_Button::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Button* pInstance = new CUI_Button(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Button");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Button::Clone(void* pArg)
{
	CUI_Button* pInstance = new CUI_Button(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Button");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Button::Free()
{
	__super::Free();

	Safe_Release(m_pHoverTextureCom);
	Safe_Release(m_pHighlightTextureCom);
	
}
