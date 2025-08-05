#include "Panel_Player_LD.h"
#include "GameInstance.h"
#include "Icon_Item.h"
#include "Belt.h"
#include "Observer_Slot.h"
#include "Ramp.h"

CPanel_Player_LD::CPanel_Player_LD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CPanel_Player_LD::CPanel_Player_LD(const CPanel_Player_LD& Prototype)
	:CUI_Container{Prototype}
{
}

HRESULT CPanel_Player_LD::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPanel_Player_LD::Initialize(void* pArg)
{
	UI_CONTAINER_DESC eDesc = {};
	eDesc.strFilePath = TEXT("../Bin/Save/UI/Slot/Icon_Keys.json");

	if (FAILED(__super::Initialize(&eDesc)))
		return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_UI_Panel_Player_LD");

	// 위 아래 슬롯 
	eDesc.strFilePath = TEXT("../Bin/Save/UI/Slot/Slots_Up.json");

	m_pBelt_Up = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Slot/Slots_Down.json");

	m_pBelt_Down = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));


	// 콜백 등록
	if (nullptr == m_pGameInstance->Find_Observer(TEXT("Slot_Belts")))
	{

		m_pGameInstance->Add_Observer(TEXT("Slot_Belts"), new CObserver_Slot);

	}

	m_pGameInstance->Register_PushCallback(TEXT("Slot_Belts"), [this](_wstring eventType, void* data) {

		if (L"ChangeUpBelt" == eventType)
		{
			m_BeltUpDescs.clear();

			auto pObj = static_cast<CBelt*>(data);

			for (auto& pItem : pObj->Get_Items())
			{
				ITEM_DESC eDesc = {};

				if (nullptr != pItem)
				{
					eDesc.strPrototag = pItem->Get_ProtoTag();
					eDesc.isSelect = false;
					eDesc.isUsable = pItem->Get_isUsable();
					eDesc.isConsumable = pItem->Get_isConsumable();
					eDesc.iUseCount = pItem->Get_UseCount();
				}

				m_BeltUpDescs.push_back(eDesc);
			}

			if (!m_BeltUpDescs.empty())
				m_BeltUpDescs[0].isSelect = true;

			if (!m_BeltDownDescs.empty())
				m_BeltDownDescs[0].isSelect = false;

			// UI 갱신
			for (_int i = 0; i < m_pBelt_Up->Get_PartUI().size(); ++i)
			{
				static_cast<CIcon_Item*>(m_pBelt_Up->Get_PartUI()[i])->Update_ICon(&m_BeltUpDescs[i]);
			}

			if (!m_BeltDownDescs.empty())
			{
				for (_int i = 0; i < m_pBelt_Down->Get_PartUI().size(); ++i)
				{
					static_cast<CIcon_Item*>(m_pBelt_Down->Get_PartUI()[i])->Update_ICon(&m_BeltDownDescs[i]);
				}
			}
			
		}

		else if (L"ChangeDownBelt" == eventType)
		{
			m_BeltDownDescs.clear();

			auto pObj = static_cast<CBelt*>(data);

			for (auto& pItem : pObj->Get_Items())
			{
				ITEM_DESC eDesc = {};

				if (nullptr != pItem)
				{
					eDesc.strPrototag = pItem->Get_ProtoTag();
					eDesc.isSelect = false;
					eDesc.isUsable = pItem->Get_isUsable();
					eDesc.isConsumable = pItem->Get_isConsumable();
					eDesc.iUseCount = pItem->Get_UseCount();
				}

				m_BeltDownDescs.push_back(eDesc);
			}

			if (!m_BeltDownDescs.empty())
				m_BeltDownDescs[0].isSelect = true;

			if (!m_BeltUpDescs.empty())
				m_BeltUpDescs[0].isSelect = false;

			// UI 갱신
			if (!m_BeltUpDescs.empty())
			{
				for (_int i = 0; i < m_pBelt_Down->Get_PartUI().size(); ++i)
				{
					static_cast<CIcon_Item*>(m_pBelt_Down->Get_PartUI()[i])->Update_ICon(&m_BeltDownDescs[i]);
				}
			}
		

			for (_int i = 0; i < m_pBelt_Up->Get_PartUI().size(); ++i)
			{
				static_cast<CIcon_Item*>(m_pBelt_Up->Get_PartUI()[i])->Update_ICon(&m_BeltUpDescs[i]);
			}
		}
		

		});

	m_pGameInstance->Register_PushCallback(TEXT("Slot_Belts"), [this](_wstring eventType, void* data) {
		
		if (L"UseDownSelectItem" == eventType)
		{

			if (L"Prototype_GameObject_Ramp" == static_cast<CItem*>(data)->Get_ProtoTag())
			{
				ITEM_DESC eDesc = {};
				auto pObj = static_cast<CRamp*>(data);

				eDesc.strPrototag = L"Prototype_GameObject_Ramp";
				eDesc.isSelect = false;
				eDesc.isUsable = pObj->Get_isUsable();
				eDesc.isConsumable = pObj->Get_isConsumable();
				eDesc.iUseCount = pObj->Get_UseCount();


				if (pObj->Get_isLight())
				{

					eDesc.iItemIndex = 1;
				}
				else
				{
					eDesc.iItemIndex = 0;
				}

				m_BeltDownDescs[0] = eDesc;
			}
			static_cast<CIcon_Item*>(m_pBelt_Down->Get_PartUI()[0])->Update_ICon(&m_BeltDownDescs[0]);
		}

		if (L"UseUpSelectItem" == eventType)
		{
			// 음 일단 다른거부터 해보자

			if (L"Prototype_GameObject_Ramp" == static_cast<CItem*>(data)->Get_ProtoTag())
			{
				ITEM_DESC eDesc = {};
				auto pObj = static_cast<CRamp*>(data);

				eDesc.strPrototag = L"Prototype_GameObject_Ramp";
				eDesc.isSelect = false;
				eDesc.isUsable = pObj->Get_isUsable();
				eDesc.isConsumable = pObj->Get_isConsumable();
				eDesc.iUseCount = pObj->Get_UseCount();


				if (pObj->Get_isLight())
				{

					eDesc.iItemIndex = 1;
				}
				else
				{
					eDesc.iItemIndex = 0;
				}

				m_BeltUpDescs[0] = eDesc;
			}

			static_cast<CIcon_Item*>(m_pBelt_Up->Get_PartUI()[0])->Update_ICon(&m_BeltUpDescs[0]);
		}
		
		});

	return S_OK;
}

void CPanel_Player_LD::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CPanel_Player_LD::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	m_pBelt_Up->Update(fTimeDelta);
	m_pBelt_Down->Update(fTimeDelta);
	
	

	if (static_cast<CIcon_Item*>(m_pBelt_Up->Get_PartUI()[0])->Get_isSelect())
	{
		m_PartObjects[0]->Set_isActive(true);
		m_PartObjects[1]->Set_isActive(false);
	}
		
	if (static_cast<CIcon_Item*>(m_pBelt_Down->Get_PartUI()[0])->Get_isSelect())
	{
		m_PartObjects[1]->Set_isActive(true);
		m_PartObjects[0]->Set_isActive(false);
	}

}

void CPanel_Player_LD::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	m_pBelt_Up->Late_Update(fTimeDelta);
	m_pBelt_Down->Late_Update(fTimeDelta);

}

HRESULT CPanel_Player_LD::Render()
{
	return S_OK;
}

CPanel_Player_LD* CPanel_Player_LD::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPanel_Player_LD* pInstance = new CPanel_Player_LD(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPanel_Player_LD");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPanel_Player_LD::Clone(void* pArg)
{
	CPanel_Player_LD* pInstance = new CPanel_Player_LD(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPanel_Player_LD");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPanel_Player_LD::Free()
{
	__super::Free();

	Safe_Release(m_pBelt_Up);
	Safe_Release(m_pBelt_Down);

	m_BeltUpDescs.clear();
	m_BeltDownDescs.clear();

}
