#include "UI_Text.h"
#include "GameInstance.h"

json CUI_Text::Serialize()
{
	json j;
	j = __super::Serialize();


	j["FontTag"] = WStringToStringU8(m_strFontTag);
	j["Caption"] = WStringToStringU8(m_strCaption);
	j["FontOffset"]["X"] = m_fFontOffset.x;
	j["FontOffset"]["Y"] = m_fFontOffset.y;
	j["FontScale"] = m_fFontScale;
	j["IsCenter"] = m_isCenter;


	return j;
}

void CUI_Text::Deserialize(const json& j)
{
	__super::Deserialize(j);

	string fontTag = j["FontTag"].get<string>();
	m_strFontTag = StringToWStringU8(fontTag);

	string caption = j["Caption"].get<string>();
	m_strCaption = StringToWStringU8(caption);

	m_fFontOffset = { j["FontOffset"]["X"].get<_float>(), j["FontOffset"]["Y"].get<_float>() };
	m_fFontScale = j["FontScale"].get<_float>();
	m_isCenter = j["IsCenter"].get<_bool>();
}

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

	

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	TEXT_UI_DESC* pDesc = static_cast<TEXT_UI_DESC*>(pArg);


	m_strFontTag = pDesc->strFontTag;
	m_strCaption = pDesc->strCaption;

	m_fFontOffset = pDesc->fFontOffset;
	m_fFontScale = pDesc->fFontScale;

	m_isCenter = pDesc->isCenter;

	m_isDeferred = false;

	m_strProtoTag = TEXT("Prototype_GameObject_UI_Text");

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
	if(m_isFade)
		m_vColor = { m_fCurrentAlpha, m_fCurrentAlpha, m_fCurrentAlpha, m_fCurrentAlpha };

	if (m_isCenter)
	{
		m_pGameInstance->Draw_Font_Centered(m_strFontTag, m_strCaption.c_str(), { m_fX, m_fY }, XMLoadFloat4(&m_vColor), m_fRotation, m_fFontOffset, m_fFontScale);
	}
	else
	{
		m_pGameInstance->Draw_Font(m_strFontTag, m_strCaption.c_str(), { m_fX, m_fY }, XMLoadFloat4(&m_vColor), m_fRotation, m_fFontOffset, m_fFontScale);
	}

    return S_OK;
}

void CUI_Text::Update_UI_From_Tool(TEXT_UI_DESC& eDesc)
{
	m_strCaption = eDesc.strCaption;
	m_fFontOffset = eDesc.fFontOffset;
	m_fFontScale = eDesc.fFontScale;
	m_isCenter = eDesc.isCenter;

	m_vColor = eDesc.vColor;
	m_fX = eDesc.fX;
	m_fY = eDesc.fY;
	m_fOffset = eDesc.fOffset;
	m_fSizeX = eDesc.fSizeX;
	m_fSizeY = eDesc.fSizeY;
	m_fRotation = eDesc.fRotation;

	m_strCaption = eDesc.strCaption;

	D3D11_VIEWPORT			ViewportDesc{};
	_uint					iNumViewports = { 1 };

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);


	m_pTransformCom->Scaling(m_fSizeX, m_fSizeY);

	m_pTransformCom->Rotation(0.f, 0.f, XMConvertToRadians(m_fRotation));

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_fX - ViewportDesc.Width * 0.5f, -m_fY + ViewportDesc.Height * 0.5f, m_fOffset, 1.f));
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
