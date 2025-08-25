#include "Dynamic_UI.h"
#include "GameInstance.h"
#include "UI_Feature.h"
#include "UI_Featrue_Scale.h"
#include "UI_Feature_Fade.h"
#include "UI_Feature_Position.h"
#include "UI_Feature_UV.h"


vector<class CUI_Feature*>& CDynamic_UI::Get_Features()
{
	return m_pUIFeatures;
}

json CDynamic_UI::Serialize()
{
	json j = __super::Serialize();


	j["Texturetag"] = WStringToString(m_strTextureTag);
	j["iTextureLevel"] = m_iTextureLevel;
	j["iTextureIndex"] = m_iTextureIndex;
	j["iPassIndex"] = m_iPassIndex;
	j["fDuration"] = m_fDuration;

	for (const auto& pFeature : m_pUIFeatures)
		if(nullptr != pFeature)
			j["Features"].push_back(pFeature->Serialize());
	
		

	return j;
}

void CDynamic_UI::Deserialize(const json& j)
{
	__super::Deserialize(j);

	string textureTag = j["Texturetag"].get<string>();
	m_strTextureTag = StringToWStringU8(textureTag);

	m_iTextureLevel = j["iTextureLevel"];
	m_iTextureIndex = j["iTextureIndex"];
	m_iPassIndex = j["iPassIndex"];
	m_fDuration = j["fDuration"];

	if (j.contains("Features") && j["Features"].is_array())
	{
		for (const auto& featureJson : j["Features"])
		{
			string featureTag = featureJson["FeatureProtoTag"].get<string>();

			CUI_Feature* pFeature = dynamic_cast<CUI_Feature*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_COMPONENT, ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(featureTag), nullptr));
			if (nullptr == pFeature)
				continue;

			pFeature->Deserialize(featureJson);

			m_pUIFeatures.push_back(pFeature);
		}
	}

	CDynamic_UI::Ready_Components_File(m_strTextureTag);

	Update_Data();
}

CDynamic_UI::DYNAMIC_UI_DESC CDynamic_UI::Get_Desc()
{
	DYNAMIC_UI_DESC eDesc = {};

	eDesc.fSizeX = m_fSizeX;
	eDesc.fSizeY = m_fSizeY;
	eDesc.iPassIndex = m_iPassIndex;
	eDesc.iTextureIndex = m_iTextureIndex;
	eDesc.fX = m_fX;
	eDesc.fY = m_fY;
	eDesc.fOffset = m_fOffset;
	eDesc.strTextureTag = m_strTextureTag;
	eDesc.vColor = m_vColor;

	eDesc.strProtoTag = m_strProtoTag;
	eDesc.fDuration = m_fDuration;

	eDesc.fAlpha = m_fCurrentAlpha;
	eDesc.fRotation = m_fRotation;

	for (auto pFeature : m_pUIFeatures)
	{
		if(nullptr != pFeature)
			eDesc.FeatureDescs.push_back(&pFeature->Get_Desc());
	}

	return eDesc;
}


CDynamic_UI::CDynamic_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIObject{pDevice, pContext}
{
}

CDynamic_UI::CDynamic_UI(const CDynamic_UI& Prototype)
	:CUIObject{Prototype}
{
}

HRESULT CDynamic_UI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDynamic_UI::Initialize(void* pArg)
{
	
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_Dynamic_UI");

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

	for (auto& eDesc : eFeatures)
	{
		Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(eDesc->strProtoTag), eDesc);
	}

	m_strProtoTag = TEXT("Prototype_GameObject_Dynamic_UI");

	
	return S_OK;
}

void CDynamic_UI::Priority_Update(_float fTimeDelta)
{

}

void CDynamic_UI::Update(_float fTimeDelta)
{
	if (!m_isFromTool)
	{
		m_fElapsedTime += fTimeDelta;

		if (m_isUVMove)
			m_fUVTime += fTimeDelta;


		if (m_fElapsedTime > m_fDuration)
		{
			m_fElapsedTime = 0.f;
			++m_iCurrentFrame;
		}

		for (auto& pFeature : m_pUIFeatures)
		{
			if (nullptr == pFeature || Get_bDead())
				continue;

			pFeature->Update(m_iCurrentFrame, this, m_isReverse);
		}
			
	}

}

void CDynamic_UI::Late_Update(_float fTimeDelta)
{
	if (!m_isActive)
		return;

	
	if (!m_isDeferred)
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI, this);
	else
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI_DEFERRED, this);

	

}

HRESULT CDynamic_UI::Render()
{

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(m_iPassIndex)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CDynamic_UI::Update_UI_From_Frame(_int& iCurrentFrame)
{
	
	for (auto& pFeature : m_pUIFeatures)
	{
		if (nullptr == pFeature || Get_bDead())
			continue;

		pFeature->Update(iCurrentFrame, this, m_isReverse);
	}

}



void CDynamic_UI::Update_UI_From_Tool(void* pArg)
{
	DYNAMIC_UI_DESC* pDesc = static_cast<DYNAMIC_UI_DESC*>(pArg);

	m_fDuration = pDesc->fDuration;
	m_vColor = pDesc->vColor;
	m_fX = pDesc->fX;
	m_fY = pDesc->fY;
	m_fOffset = pDesc->fOffset;
	m_fSizeX = pDesc->fSizeX;
	m_fSizeY = pDesc->fSizeY;
	m_iPassIndex = pDesc->iPassIndex;
	m_iTextureIndex = pDesc->iTextureIndex;
	m_fRotation = pDesc->fRotation;

	D3D11_VIEWPORT			ViewportDesc{};
	_uint					iNumViewports = { 1 };

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);


	m_pTransformCom->Scaling(m_fSizeX, m_fSizeY);

	m_pTransformCom->Rotation(0.f, 0.f, XMConvertToRadians(m_fRotation));

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_fX - ViewportDesc.Width * 0.5f, -m_fY + ViewportDesc.Height * 0.5f, m_fOffset, 1.f));
}

void CDynamic_UI::Reset()
{
	m_iCurrentFrame = 0;

	for (auto& pFeature : m_pUIFeatures)
	{
		if (nullptr == pFeature || Get_bDead())
			continue;

		pFeature->Reset();
	}
}


HRESULT CDynamic_UI::Ready_Components(const wstring& strTextureTag)
{
	/* For.Com_Shader */
	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_DynamicUI"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	if (strTextureTag != L"")
	{
		/* For.Com_Texture */
		if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), strTextureTag,
			TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
   			return E_FAIL;
	}
	

	return S_OK;
}

HRESULT CDynamic_UI::Ready_Components_File(const wstring& strTextureTag)
{
	__super::Ready_Components_File(strTextureTag);

	m_strTextureTag = strTextureTag;

	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_DynamicUI"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	if (strTextureTag != L"")
	{
		/* For.Com_Texture */
		if (FAILED(__super::Replace_Component(ENUM_CLASS(LEVEL::STATIC), strTextureTag,
			TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
			return E_FAIL;
	}


	return S_OK;
}

HRESULT CDynamic_UI::Bind_ShaderResources()
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

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Color", &m_vColor, sizeof(_float4))))
		return E_FAIL;

	// 기본값을 던지고, 추가로 덮어쓰자

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Alpha", &m_fCurrentAlpha, sizeof(_float))))
		return E_FAIL;


	if (FAILED(m_pShaderCom->Bind_RawValue("g_UVTime", &m_fUVTime, sizeof(_float))))
		return E_FAIL;

	
	for (auto& pFeature : m_pUIFeatures)
	{
		if(nullptr != pFeature)
			pFeature->Bind_ShaderResources(m_pShaderCom);
	}
		


	return S_OK;
}

HRESULT CDynamic_UI::Add_Feature(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, void* pArg)
{
	CUI_Feature* pPartObject = dynamic_cast<CUI_Feature*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_COMPONENT, iPrototypeLevelIndex, strPrototypeTag, pArg));
	if (nullptr == pPartObject)
		return E_FAIL;

	m_pUIFeatures.push_back(pPartObject);

	return S_OK;
}

void CDynamic_UI::Update_Data()
{
	__super::Update_Data();



}

CDynamic_UI* CDynamic_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDynamic_UI* pInstance = new CDynamic_UI(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CDynamic_UI");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDynamic_UI::Clone(void* pArg)
{
	CDynamic_UI* pInstance = new CDynamic_UI(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDynamic_UI");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDynamic_UI::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);

	for (auto& pFeature : m_pUIFeatures)
		Safe_Release(pFeature);



}

