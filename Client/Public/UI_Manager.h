#pragma once

#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
class CUIObject;
NS_END

NS_BEGIN(Client)



class CUI_Manager final : public CBase
{
    DECLARE_SINGLETON(CUI_Manager)
private:
    CUI_Manager();
    virtual ~CUI_Manager() = default;


public:

    void Emplace_UI(CUIObject* pUI, _wstring strTag);


    // 특정 상황에 패널을 껏다 킨다
    _bool Find_Panel();
    void On_Panel();
    void Off_Panel();

  

    // 팝업
    void Activate_Popup(_bool isActive);
    void Set_Popup_Caption(_int iTriggerType);
    // 
    
    


public:

private:
    CGameInstance* m_pGameInstance = { nullptr };
private:
    // 
    list<CUIObject*> m_pPanel = {};

    map<_wstring, CUIObject*> m_UImap = {};



public:
    virtual void Free() override;
};

NS_END

