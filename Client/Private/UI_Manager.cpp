#include "UI_Manager.h"
#include "GameInstance.h"
#include "UIObject.h"
#include "UI_Popup.h"

IMPLEMENT_SINGLETON(CUI_Manager)

CUI_Manager::CUI_Manager()
	:m_pGameInstance{ CGameInstance::Get_Instance() }
{
}

void CUI_Manager::Add_UI(CUIObject* pUI, _wstring strTag)
{
	
	m_UImap.insert({ strTag, pUI });

	Safe_AddRef(pUI);
}

void CUI_Manager::Remove_UI(_wstring strTag)
{
	if (m_UImap.find(strTag) == m_UImap.end())
		return;


	CUIObject* pUI = m_UImap.find(strTag)->second;

	Safe_Release(pUI);

	m_UImap.erase(strTag);


}

_bool CUI_Manager::Find_Panel()
{
	auto& list = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player_Panel"));

	if (list.empty())
	{
		return false;
	}


	for (auto& pObj : list)
	{
		
		m_pPanel.push_back(static_cast<CUIObject*>(pObj));
	}

	return true;
}

void CUI_Manager::On_Panel()
{
	
	if (m_pPanel.empty())
		 Find_Panel();



	for (auto& obj : m_pPanel)
	{
		(obj)->Set_isReverse(false);
	}
}

void CUI_Manager::Off_Panel()
{
	
	if (m_pPanel.empty())
		Find_Panel();



	for (auto& obj : m_pPanel)
	{
		(obj)->Set_isReverse(true);
	}
}

void CUI_Manager::Activate_Popup(_bool isActive)
{
	if (m_UImap.find(L"Popup") == m_UImap.end())
		return;

	CUIObject* pUI = m_UImap.find(L"Popup")->second;

	if (nullptr == pUI)
		return;

	pUI->Set_isActive(isActive);

}

void CUI_Manager::Set_Popup_Caption(_int iTriggerType)
{
	if (m_UImap.find(L"Popup") == m_UImap.end())
		return;

	CUIObject* pUI = m_UImap.find(L"Popup")->second;

	if (nullptr == pUI)
		return;

	static_cast<CUI_Popup*>(pUI)->Set_String(iTriggerType);
}

void CUI_Manager::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);

	for (auto& pUI : m_UImap)
	{
		Safe_Release(pUI.second);
	}

	m_UImap.clear();
	m_pPanel.clear();
}
