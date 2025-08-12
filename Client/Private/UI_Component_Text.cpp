#include "UI_Component_Text.h"
#include "GameInstance.h"



CUI_Component_Text::CUI_Component_Text(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Component{pDevice, pContext}
{
}

CUI_Component_Text::CUI_Component_Text(const CUI_Component_Text& Prototype)
	:CUI_Component{Prototype}
{
}

json CUI_Component_Text::Serialize()
{
	json j = __super::Serialize();
	
	

	return json();
}

void CUI_Component_Text::Deserialize(const json& j)
{
	__super::Deserialize(j);

	
}

HRESULT CUI_Component_Text::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Component_Text::Initialize(void* pArg)
{
	m_strProtoTag = TEXT("UI_Com_Text");


	return S_OK;
}

void CUI_Component_Text::Draw(CUIObject* pUI)
{

	if (pUI->Get_isFade())
	{
		float fAlpha = pUI->Get_Alpha();
		m_vColor = { fAlpha, fAlpha, fAlpha, fAlpha };
	}
		

	switch (m_eAlignType)
	{
	case Client::TEXTALIGN::LEFT:
		m_pGameInstance->Draw_Font(m_strFontTag, m_strCaption.c_str(), m_vPos, XMLoadFloat4(&m_vColor), m_fRotation, m_fOffset, m_fScale);
		break;
	case Client::TEXTALIGN::CENTER:
		m_pGameInstance->Draw_Font_Centered(m_strFontTag, m_strCaption.c_str(), m_vPos, XMLoadFloat4(&m_vColor), m_fRotation, m_fOffset, m_fScale);
		break;
	case Client::TEXTALIGN::RIGHT:
		m_pGameInstance->Draw_Font_Righted(m_strFontTag, m_strCaption.c_str(), m_vPos, XMLoadFloat4(&m_vColor), m_fRotation, m_fOffset, m_fScale);
		break;
	case Client::TEXTALIGN::END:
		break;
	default:
		break;
	}

}

void CUI_Component_Text::Change_Caption(_wstring& strCaption)
{
	m_strCaption = strCaption;
}



CUI_Component_Text* CUI_Component_Text::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CComponent* CUI_Component_Text::Clone(void* pArg)
{
	return nullptr;
}

void CUI_Component_Text::Free()
{
}
