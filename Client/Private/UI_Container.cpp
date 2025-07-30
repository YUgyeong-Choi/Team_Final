#include "UI_Container.h"
#include "Static_UI.h"
#include "Dynamic_UI.h"
#include "UI_Text.h"
#include "GameInstance.h"

CUI_Container::CUI_Container(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIObject{pDevice, pContext}
{
}

CUI_Container::CUI_Container(const CUI_Container& Prototype)
	:CUIObject{Prototype}
{
}



json CUI_Container::Serialize()
{
	json j = __super::Serialize();

	j["FilePath"] = WStringToStringU8(m_strFilePath);


	return j;
}

void CUI_Container::Deserialize(const json& j)
{
	__super::Deserialize(j);
	
	
	string strFilePath = j["FilePath"];

	m_strFilePath = StringToWStringU8(strFilePath);

}

HRESULT CUI_Container::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Container::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	UI_CONTAINER_DESC* pDesc = static_cast<UI_CONTAINER_DESC*>(pArg);

	m_strFilePath = pDesc->strFilePath;

	json j;

	ifstream file(WStringToStringU8(m_strFilePath));

	file >> j;

	for (const auto& eUIJson : j)
	{
		string protoTag = eUIJson["ProtoTag"];

		if ("Prototype_GameObject_Static_UI" == protoTag)
		{
			CStatic_UI::STATIC_UI_DESC eDesc = {};

			string textureTag = eUIJson["Texturetag"];
			eDesc.strTextureTag = StringToWStringU8(textureTag);
			eDesc.iTextureLevel = eUIJson["iTextureLevel"];
		
			Add_PartObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(protoTag), &eDesc);
			

		}
		else if ("Prototype_GameObject_Dynamic_UI" == protoTag)
		{
			CDynamic_UI::DYNAMIC_UI_DESC eDesc = {};

			string textureTag = eUIJson["Texturetag"];
			eDesc.strTextureTag = StringToWStringU8(textureTag);
			eDesc.iTextureLevel = eUIJson["iTextureLevel"];
			
			Add_PartObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(protoTag), &eDesc);
		

		}
		else if ("Prototype_GameObject_UI_Text" == protoTag)
		{
			CUI_Text::TEXT_UI_DESC eDesc = {};

			Add_PartObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(protoTag), &eDesc);

		}

		m_PartObjects.back()->Deserialize(eUIJson);

		m_PartObjects.back()->Update_Data();
	}
	file.close();

	return S_OK;
}

void CUI_Container::Priority_Update(_float fTimeDelta)
{
	for (auto& pObj : m_PartObjects)
	{
		if(nullptr != pObj)
			pObj->Priority_Update(fTimeDelta);
	}
	
}

void CUI_Container::Update(_float fTimeDelta)
{
	for (auto& pObj : m_PartObjects)
	{
		if (nullptr != pObj)
			pObj->Update(fTimeDelta);
	}

}

void CUI_Container::Late_Update(_float fTimeDelta)
{
	for (auto& pObj : m_PartObjects)
	{
		if (nullptr != pObj)
			pObj->Late_Update(fTimeDelta);
	}

}

HRESULT CUI_Container::Render()
{
	return S_OK;
}

void CUI_Container::Active_Update(_bool isActive)
{

	m_isActive = isActive;

	for (auto& pObj : m_PartObjects)
		pObj->Set_isActive(isActive);

}

HRESULT CUI_Container::Add_PartObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, void* pArg)
{
	CUIObject* pPartObject = dynamic_cast<CUIObject*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, iPrototypeLevelIndex, strPrototypeTag, pArg));
	if (nullptr == pPartObject)
		return E_FAIL;

	m_PartObjects.push_back(pPartObject);

	return S_OK;

}


CUI_Container* CUI_Container::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Container* pInstance = new CUI_Container(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Container");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Container::Clone(void* pArg)
{
	CUI_Container* pInstance = new CUI_Container(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Container");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Container::Free()
{
	__super::Free();

	for (auto& pObj : m_PartObjects)
		Safe_Release(pObj);

	m_PartObjects.clear();
}
