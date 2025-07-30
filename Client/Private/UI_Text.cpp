#include "UI_Text.h"
#include "GameInstance.h"

CUI_Text::CUI_Text(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUIObject{pDevice, pContext}
{
}

CUI_Text::CUI_Text(const CUI_Text& Prototype)
    :CUIObject{Prototype}
{
}

HRESULT CUI_Text::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Text::Initialize(void* pArg)
{

	TEXT_UI_DESC* pDesc = static_cast<TEXT_UI_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strTextureTag = pDesc->strTextureTag;

	if (FAILED(Ready_Components(m_strTextureTag)))
		return E_FAIL;

	m_vColor = pDesc->vColor;

	m_strFontTag = pDesc->strFontTag;
	m_strCaption = pDesc->strCaption;

	m_fFontOffset = pDesc->fFontOffset;
	m_fFontScale = pDesc->fFontScale;

	m_isCenter = pDesc->isCenter;

	m_isDeferred = false;

	

	return S_OK;
   
}

void CUI_Text::Priority_Update(_float fTimeDelta)
{
}

void CUI_Text::Update(_float fTimeDelta)
{
	Fade(fTimeDelta);
}

void CUI_Text::Late_Update(_float fTimeDelta)
{
	if (!m_isDeferred)
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI, this);
	else
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI_DEFERRED, this);
}

HRESULT CUI_Text::Render()
{

	m_vColor = { m_fCurrentAlpha, m_fCurrentAlpha, m_fCurrentAlpha, m_fCurrentAlpha };

	if (m_isCenter)
	{
		m_pGameInstance->Draw_Font_Centered(m_strFontTag, m_strCaption.c_str(), { m_fX, m_fY }, XMLoadFloat4(&m_vColor), 0.f, m_fFontOffset, m_fFontScale);
	}
	else
	{
		m_pGameInstance->Draw_Font(m_strFontTag, m_strCaption.c_str(), { m_fX, m_fY }, XMLoadFloat4(&m_vColor), 0.f, m_fFontOffset, m_fFontScale);
	}

    return S_OK;
}

void CUI_Text::Update_UI_From_Tool(TEXT_UI_DESC& eDesc)
{
	// 나중에 채우죠?
}

HRESULT CUI_Text::Ready_Components(const wstring& strTextureTag)
{

	


    return S_OK;
}

CUI_Text* CUI_Text::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Text* pInstance = new CUI_Text(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Text");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Text::Clone(void* pArg)
{
	CUI_Text* pInstance = new CUI_Text(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Text");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Text::Free()
{
	__super::Free();

}
