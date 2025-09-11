#include "UI_Manager.h"
#include "GameInstance.h"
#include "UIObject.h"
#include "UI_Popup.h"
#include "UI_Script_Text.h"
#include "UI_Script_Talk.h"
#include "UI_Pickup_Item.h"

IMPLEMENT_SINGLETON(CUI_Manager)

CUI_Manager::CUI_Manager()
	:m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}

void CUI_Manager::Initialize()
{	
	CComponent* pComponent = static_cast<CComponent*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_COMPONENT, m_pGameInstance->GetCurrentLevelIndex(), TEXT("Prototype_Component_Sound_UI")));
	if (nullptr == pComponent)
		return;

	CComponent** ppOut = reinterpret_cast<CComponent**>(&m_pSoundCom);

	*ppOut = pComponent;
}

void CUI_Manager::Emplace_UI(CUIObject* pUI, _wstring strTag)
{

	if (m_UImap.find(strTag) == m_UImap.end())
	{
		m_UImap.insert({ strTag, pUI });

		Safe_AddRef(pUI);
	}
	else 
	{
		Safe_Release(m_UImap[strTag]);
		m_UImap.erase(strTag);


		m_UImap.insert({ strTag, pUI });

		Safe_AddRef(pUI);
	}
	
}

_bool CUI_Manager::Find_Panel()
{
	

	auto& list = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player_Panel"));

	if (list.empty())
	{
		return false;
	}

	m_pPanel.clear();
	
	for (auto& pObj : list)
	{

		m_pPanel.push_back(static_cast<CUI_Container*>(pObj));

	}
	
	

	return true;
}

void CUI_Manager::On_Panel()
{
	
	if (!Find_Panel())
		return;

	for (auto& obj : m_pPanel)
	{
		(obj)->Set_isReverse(false);
		
	}
}

void CUI_Manager::Off_Panel()
{
	
	if (!Find_Panel())
		return;


	for (auto& obj : m_pPanel)
	{
		(obj)->Set_isReverse(true);
	}
}

void CUI_Manager::Activate_UI(_wstring strTag, _bool isActive)
{
	if (m_UImap.find(strTag) == m_UImap.end())
		return;

	CUI_Container* pUI = dynamic_cast<CUI_Container*>(m_UImap.find(strTag)->second);

	if (nullptr == pUI)
		return;

	pUI->Active_Update(isActive);

}

void CUI_Manager::Activate_Popup(_bool isActive)
{
	if (m_UImap.find(L"Popup") == m_UImap.end())
		return;

	CUIObject* pUI = m_UImap.find(L"Popup")->second;

	if (nullptr == pUI)
		return;

	static_cast<CUI_Popup*>(pUI)->Active_Update(isActive);

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

void CUI_Manager::Activate_TextScript(_bool isActive)
{
	if (m_UImap.find(L"TextScript") == m_UImap.end())
		return;

	if (nullptr == m_UImap.find(L"TextScript")->second)
		return;

	static_cast<CUI_Script_Text*>(m_UImap.find(L"TextScript")->second)->Active_Update(isActive);

}

void CUI_Manager::Activate_TalkScript(_bool isActive)
{
	if (m_UImap.find(L"TalkScript") == m_UImap.end())
		return;

	if (nullptr == m_UImap.find(L"TalkScript")->second)
		return;

	static_cast<CUI_Script_Talk*>(m_UImap.find(L"TalkScript")->second)->Active_Update(isActive);
}

void CUI_Manager::Update_TextScript(string& strText)
{
	if (m_UImap.find(L"TextScript") == m_UImap.end())
		return;

	if (nullptr == m_UImap.find(L"TextScript")->second)
		return;

	static_cast<CUI_Script_Text*>(m_UImap.find(L"TextScript")->second)->Update_Script(strText);
}

void CUI_Manager::Update_TalkScript(string& strName, string& strText, _bool isAuto)
{
	if (m_UImap.find(L"TalkScript") == m_UImap.end())
		return;

	if (nullptr == m_UImap.find(L"TalkScript")->second)
		return;

	static_cast<CUI_Script_Talk*>(m_UImap.find(L"TalkScript")->second)->Update_Script(strName, strText, isAuto);
}

_int CUI_Manager::Check_Script_Click_Button()
{
	if (m_UImap.find(L"TalkScript") == m_UImap.end())
		return -1;

	if (nullptr == m_UImap.find(L"TalkScript")->second)
		return -1;


	return static_cast<CUI_Script_Talk*>(m_UImap.find(L"TalkScript")->second)->Check_Click_Button();
}

void CUI_Manager::Sound_Play(string soundTag)
{
	m_pSoundCom->Play(soundTag);
}

void CUI_Manager::Update_PickUpItem(_int iItemType)
{
	if (m_UImap.find(L"Pickup_Item") == m_UImap.end())
		return;

	if (nullptr == m_UImap.find(L"Pickup_Item")->second)
		return;

	static_cast<CUI_Pickup_Item*>(m_UImap.find(L"Pickup_Item")->second)->Update_Description(iItemType);
}

void CUI_Manager::Free()
{
	__super::Free();

	for (auto& pUI : m_UImap)
		Safe_Release(pUI.second);
	

	m_UImap.clear();

	m_pPanel.clear();

	Safe_Release(m_pGameInstance);

	Safe_Release(m_pSoundCom);
}
