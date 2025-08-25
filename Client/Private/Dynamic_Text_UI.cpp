#include "Dynamic_Text_UI.h"
#include "GameInstance.h"
#include "UI_Feature.h"

#include <sstream>


CDynamic_Text_UI::CDynamic_Text_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CDynamic_UI{pDevice, pContext}
{
}

CDynamic_Text_UI::CDynamic_Text_UI(const CDynamic_Text_UI& Prototype)
	:CDynamic_UI{Prototype}
{
}

json CDynamic_Text_UI::Serialize()
{
	json j = __super::Serialize();

	j["FontTag"] = WStringToStringU8(m_strFontTag);
	j["Caption"] = WStringToStringU8(m_strCaption);
	j["IsCenter"] = ENUM_CLASS(m_eAlignType);

	return j;
}

void CDynamic_Text_UI::Deserialize(const json& j)
{
	__super::Deserialize(j);

	string fontTag = j["FontTag"].get<string>();
	m_strFontTag = StringToWStringU8(fontTag);

	string caption = j["Caption"].get<string>();
	m_strCaption = StringToWStringU8(caption);

	m_eAlignType = TEXTALIGN(j["IsCenter"].get<int>());
}

HRESULT CDynamic_Text_UI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDynamic_Text_UI::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_Dynamic_Text_UI");

	m_strFontTag = L"Font_Medium";

	_float2 fSize = m_pGameInstance->Calc_Draw_Range(L"Font_Medium", L"aaaa");

	m_fPaddingY = fSize.y;

	if (nullptr == pArg)
		return S_OK;


	DYNAMIC_UI_DESC* pDesc = static_cast<DYNAMIC_UI_DESC*>(pArg);



	m_strTextureTag = pDesc->strTextureTag;

	if (FAILED(Ready_Components(m_strTextureTag)))
		return E_FAIL;

	m_iPassIndex = pDesc->iPassIndex;
	m_iTextureIndex = pDesc->iTextureIndex;

	m_fDuration = pDesc->fDuration;


	m_vColor = pDesc->vColor;

	m_isFromTool = pDesc->isFromTool;

	// 파일 읽어서 분기를 나눠야될듯?

	auto& eFeatures = pDesc->FeatureDescs;

	for (auto& pDesc : eFeatures)
	{
		Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(pDesc->strProtoTag), pDesc);
	}

	m_strProtoTag = TEXT("Prototype_GameObject_Dynamic_UI");

	

	return S_OK;
}

void CDynamic_Text_UI::Priority_Update(_float fTimeDelta)
{
}

void CDynamic_Text_UI::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CDynamic_Text_UI::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CDynamic_Text_UI::Render()
{
	if (nullptr != m_pTextureCom)
	{
		if (FAILED(__super::Render()))
			return E_FAIL;
	}

	for (auto& pFeature : m_pUIFeatures)
	{
		if (nullptr != pFeature)
			pFeature->Bind_ShaderResources(m_pShaderCom);
	}


	if (m_isFade)
		m_vColor = { m_fCurrentAlpha, m_fCurrentAlpha, m_fCurrentAlpha, m_fCurrentAlpha };

	if(m_strCaption.find(L"\n") != _wstring::npos)
		SplitLinedraw();
	else
	{
		switch (m_eAlignType)
		{
		case Client::TEXTALIGN::LEFT:
			m_pGameInstance->Draw_Font(m_strFontTag, m_strCaption.c_str(), { m_fX, m_fY }, XMLoadFloat4(&m_vColor) * m_fCurrentAlpha, m_fRotation, { 0.f,0.f }, (m_fSizeX / g_iWinSizeX + m_fSizeY / g_iWinSizeY), m_fOffset);
			break;
		case Client::TEXTALIGN::CENTER:
			m_pGameInstance->Draw_Font_Centered(m_strFontTag, m_strCaption.c_str(), { m_fX, m_fY }, XMLoadFloat4(&m_vColor) * m_fCurrentAlpha, m_fRotation, { 0.f,0.f }, (m_fSizeX / g_iWinSizeX + m_fSizeY / g_iWinSizeY), m_fOffset);
			break;
		case Client::TEXTALIGN::RIGHT:
			m_pGameInstance->Draw_Font_Righted(m_strFontTag, m_strCaption.c_str(), { m_fX, m_fY }, XMLoadFloat4(&m_vColor) * m_fCurrentAlpha, m_fRotation, { 0.f,0.f }, (m_fSizeX / g_iWinSizeX + m_fSizeY / g_iWinSizeY), m_fOffset);
			break;
		case Client::TEXTALIGN::END:
			break;
		default:
			break;
		}
	}

	

	return S_OK;
}

void CDynamic_Text_UI::Update_UI_From_Tool(void* pArg)
{
	__super::Update_UI_From_Tool(pArg);

	DYNAMIC_UI_DESC* pDesc = static_cast<DYNAMIC_UI_DESC*>(pArg);

	m_strCaption = StringToWStringU8(pDesc->strText);

	m_eAlignType = TEXTALIGN(pDesc->iAlignType);
}

void CDynamic_Text_UI::Set_Caption(_wstring strCaption)
{
	m_strCaption = strCaption;
}

void CDynamic_Text_UI::SplitLinedraw()
{
	wstringstream wss(m_strCaption);
	wstring line;
	vector<std::wstring> lines;

	_float fY = m_fY - m_fPaddingY * 0.35f;

	while (std::getline(wss, line, L'\n')) {
		

		switch (m_eAlignType)
		{
		case Client::TEXTALIGN::LEFT:
			m_pGameInstance->Draw_Font(m_strFontTag, line.c_str(), {m_fX, fY }, XMLoadFloat4(&m_vColor) * m_fCurrentAlpha, m_fRotation, {0.f,0.f}, (m_fSizeX / g_iWinSizeX + m_fSizeY / g_iWinSizeY), m_fOffset);
			break;
		case Client::TEXTALIGN::CENTER:
			m_pGameInstance->Draw_Font_Centered(m_strFontTag, line.c_str(), { m_fX, fY }, XMLoadFloat4(&m_vColor) * m_fCurrentAlpha, m_fRotation, { 0.f,0.f }, (m_fSizeX / g_iWinSizeX + m_fSizeY / g_iWinSizeY), m_fOffset);
			break;
		case Client::TEXTALIGN::RIGHT:
			m_pGameInstance->Draw_Font_Righted(m_strFontTag, line.c_str(), { m_fX, fY }, XMLoadFloat4(&m_vColor) * m_fCurrentAlpha, m_fRotation, { 0.f,0.f }, (m_fSizeX / g_iWinSizeX + m_fSizeY / g_iWinSizeY), m_fOffset);
			break;
		case Client::TEXTALIGN::END:
			break;
		default:
			break;
		}

		fY += m_fPaddingY * 0.7f;
	}

}

CDynamic_Text_UI* CDynamic_Text_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDynamic_Text_UI* pInstance = new CDynamic_Text_UI(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CDynamic_Text_UI");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDynamic_Text_UI::Clone(void* pArg)
{
	CDynamic_Text_UI* pInstance = new CDynamic_Text_UI(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDynamic_Text_UI");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDynamic_Text_UI::Free()
{
	__super::Free();

}