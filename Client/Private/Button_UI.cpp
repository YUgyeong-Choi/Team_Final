#include "Button_UI.h"
#include "GameInstance.h"


CButton_UI::CButton_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIObject{pDevice, pContext}
{
}

CButton_UI::CButton_UI(const CButton_UI& Prototype)
	:CUIObject{ Prototype }
{
}

HRESULT CButton_UI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CButton_UI::Initialize(void* pArg)
{
	BUTTON_UI_DESC* pDesc = static_cast<BUTTON_UI_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_strTextureTag = pDesc->strTextureTag;

	return S_OK;
}

void CButton_UI::Priority_Update(_float fTimeDelta)
{
}

void CButton_UI::Update(_float fTimeDelta)
{
}

void CButton_UI::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI, this);
}

HRESULT CButton_UI::Render()
{
	return S_OK;
}

HRESULT CButton_UI::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_UI"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CButton_UI::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	return S_OK;
}

_bool CButton_UI::Check_Click()
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

void CButton_UI::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);

}
