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


	for (const auto& pObj : m_PartObjects)
	{
		if (nullptr != pObj)
		{
			j["Parts"].push_back(pObj->Serialize());
		}
	}
		
		

	return j;
}

void CUI_Container::Deserialize(const json& j)
{
	__super::Deserialize(j);
	
	
	if (true == j.contains("FilePath"))
	{
		string strFilePath = j["FilePath"];

		m_strFilePath = StringToWStringU8(strFilePath);
	}
	
	if (j.contains("Parts") && j["Parts"].is_array())
	{
		for (const auto& objJson : j["Parts"])
		{
			string protoTag = objJson["ProtoTag"].get<string>();

			Add_PartObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(protoTag), nullptr);

			m_PartObjects.back()->Deserialize(objJson);

			m_PartObjects.back()->Update_Data();
		}
	}
}

vector<CUIObject*>& CUI_Container::Get_PartUI()
{
	return m_PartObjects;
}

void CUI_Container::Set_isReverse(_bool isReverse)
{
	m_isReverse = isReverse;

	for (auto& pObj : m_PartObjects)
	{
		if (nullptr != pObj)
			pObj->Set_isReverse(isReverse);
	}
}

HRESULT CUI_Container::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Container::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_UI_Container");

	if (nullptr == pArg)
		return S_OK;

	UI_CONTAINER_DESC* pDesc = static_cast<UI_CONTAINER_DESC*>(pArg);

	m_strFilePath = pDesc->strFilePath;

	if (m_strFilePath.empty())
		return S_OK;

	json j;

	ifstream file(WStringToStringU8(m_strFilePath));

	file >> j;

	for (const auto& eUIJson : j)
	{
		string protoTag = eUIJson["ProtoTag"];

		if (protoTag == "Prototype_GameObject_UI_Container")
		{
			// 여기는 파츠 바로 생성하도록

			if (eUIJson.contains("Parts") && eUIJson["Parts"].is_array())
			{
				for (const auto& partJson : eUIJson["Parts"])
				{
					string childProtoTag = partJson["ProtoTag"];

					if (FAILED(Add_PartObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(childProtoTag), nullptr)))
						return E_FAIL;

					m_PartObjects.back()->Deserialize(partJson);
					m_PartObjects.back()->Update_Data();
				}
			}
		}
		else
		{
			// 여기는 컨테이너 생성하도록
			
			if (FAILED(Add_PartObject(ENUM_CLASS(LEVEL::STATIC), StringToWStringU8(protoTag), nullptr)))
				return E_FAIL;

			m_PartObjects.back()->Deserialize(eUIJson);

			m_PartObjects.back()->Update_Data();
		}
			

		
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

void CUI_Container::Add_UI_From_Tool(CUIObject* pObj)
{
	if (nullptr == pObj)
		return;

	Safe_AddRef(pObj);

	m_PartObjects.push_back(pObj);
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
