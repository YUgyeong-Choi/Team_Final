#pragma once

#include "Client_Defines.h"
#include "Base.h"
#include "UI_Container.h"

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


    // Ư�� ��Ȳ�� �г��� ���� Ų��
    _bool Find_Panel();
    void On_Panel();
    void Off_Panel();

    // �˾�
    void Activate_Popup(_bool isActive);
    void Set_Popup_Caption(_int iTriggerType);

    // ��ũ��Ʈ
     
    // ���븸 �ִ� ��ũ��Ʈ
    void Activate_TextScript(_bool isActive);

    // ���� ���ϴ��� ǥ���ϴ� ��ũ��Ʈ
    void Activate_TalkScript(_bool isActive);

    // ��ȭ ���� ������Ʈ �ϰ�, �� ��� ũ�⸦ �������ش�
    void Update_TextScript(string& strText);

    // ��ȭ ���� ������Ʈ �ϰ�, �� ��� ũ�⸦ �������ش�
    void Update_TalkScript(string& strName, string& strText, _bool isAuto);

    _int Check_Script_Click_Button();
  

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

