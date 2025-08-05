#include "Observer_Slot.h"
#include "Belt.h"
#include "Ramp.h"

CObserver_Slot::CObserver_Slot()
{
	Register_PullCallback([this](_wstring eventType, void* data) {
		
		
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
			m_BeltUpDescs[0].isSelect = true;
		}

		if (L"ChangeDownBelt" == eventType)
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
			m_BeltDownDescs[0].isSelect = true;
		}

		
		});

	Register_PullCallback([this](_wstring eventType, void* data) {
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
		}

		});
}

void CObserver_Slot::Free()
{
	__super::Free();
}
