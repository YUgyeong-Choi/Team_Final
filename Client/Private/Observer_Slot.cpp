#include "Observer_Slot.h"
#include "Belt.h"
#include "Lamp.h"

CObserver_Slot::CObserver_Slot()
{
	/*Register_PullCallback([this](_wstring eventType, void* data) {
		
		
		if (L"ChangeUpBelt" == eventType)
		{
			m_BeltUpDescs.clear();

			auto pUpBelt = static_cast<CBelt*>(data);

			auto& ViewItems = pUpBelt->Get_ViewItems();

			for (auto& pView : ViewItems)
			{
				if(nullptr != pView)
					m_BeltUpDescs.push_back(pView->Get_ItemDesc());
			}
			if (m_BeltUpDescs.size() > 0)
				m_BeltUpDescs[0].isSelect = true;
		}

		if (L"ChangeDownBelt" == eventType)
		{
			m_BeltDownDescs.clear();

			auto pUpBelt = static_cast<CBelt*>(data);

			auto& ViewItems = pUpBelt->Get_ViewItems();

			for (auto& pView : ViewItems)
			{
				if (nullptr != pView)
					m_BeltDownDescs.push_back(pView->Get_ItemDesc());
			}

			if (m_BeltDownDescs.size() > 0)
				m_BeltDownDescs[0].isSelect = true;
			
				
		}

		
		});

	Register_PullCallback([this](_wstring eventType, void* data) {
		if (L"UseDownSelectItem" == eventType)
		{
			
			auto pObj = static_cast<CItem*>(data);

			m_BeltDownDescs[0] = pObj->Get_ItemDesc();

			m_BeltDownDescs[0].isSelect = true;
			
		}

		if (L"UseUpSelectItem" == eventType)
		{

			auto pObj = static_cast<CItem*>(data);

			m_BeltUpDescs[0] = pObj->Get_ItemDesc();

			m_BeltUpDescs[0].isSelect = true;

		}

		});*/
}

void CObserver_Slot::Free()
{
	__super::Free();
}
