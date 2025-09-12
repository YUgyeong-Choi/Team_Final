#include "UI_Guide.h"
#include "Static_UI.h"
#include "GameInstance.h"
#include "UI_Manager.h"
#include "Camera_Manager.h"

CUI_Guide::CUI_Guide(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUI_Container{pDevice, pContext}
{
}

CUI_Guide::CUI_Guide(const CUI_Guide& Prototype)
    :CUI_Container(Prototype)
{
}

json CUI_Guide::Serialize()
{
    json j = __super::Serialize();
    
    return j;
}

void CUI_Guide::Deserialize(const json& j)
{
    __super::Deserialize(j);
}

HRESULT CUI_Guide::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Guide::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    UI_GUIDE_DESC* pDesc = static_cast<UI_GUIDE_DESC*>(pArg);
    

    for (auto& partPath : pDesc->partPaths)
    {
        CUI_Container::UI_CONTAINER_DESC eDesc = {};
    
        eDesc.strFilePath = partPath; 

        m_Explainations.push_back(static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc)));

        m_Explainations.back()->Active_Update(false);
    }

    // 버튼

    CUI_Container::UI_CONTAINER_DESC eButtonDesc = {};

    eButtonDesc.strFilePath = TEXT("../Bin/Save/UI/Guide/Button.json");

    
    m_pButtonContainer = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eButtonDesc));

    for (auto& pPart : m_pButtonContainer->Get_PartUI())
    {
     
       m_Buttons.push_back(static_cast<CUI_Button*>(pPart));
     
    }

    


   // 백그라운드

    CUI_Container::UI_CONTAINER_DESC eBackDesc = {};

    eBackDesc.strFilePath = TEXT("../Bin/Save/UI/Guide/BackGround.json");
    
    m_pBackGround = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eBackDesc));
    
    
    m_iSize = static_cast<_int>(m_Explainations.size());

    m_iIndex = 0;

    for (auto& pExplain : m_Explainations)
        pExplain->Active_Update(false);


    if (ENUM_CLASS(LEVEL::LOGO) != m_pGameInstance->GetCurrentLevelIndex())
    {
        CUI_Manager::Get_Instance()->Off_Panel();

        list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster_Normal"));
        for (auto& obj : objList)
            obj->Set_TimeScale(0.f);

        CGameObject* pPlayer = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"));
        pPlayer->Set_TimeScale(0.f);

        if (m_Explainations.size() == 1)
            m_Explainations[0]->Active_Update(true);

    }


    m_fCurrentAlpha = 1.f;
   
    CUI_Manager::Get_Instance()->Sound_Play("SE_UI_OpenWindowTutorial_01");
    return S_OK;
}

void CUI_Guide::Priority_Update(_float fTimeDelta)
{
    if (m_isFade == false && m_fCurrentAlpha <= 0.f)
    {
        Set_bDead();
        m_pGameInstance->Set_GameTimeScale(1.f);
        CCamera_Manager::Get_Instance()->SetbMoveable(true);
        return;
    }

    // 키 입력
    Check_Button();
    
}


void CUI_Guide::Update(_float fTimeDelta)
{
    if (m_fDelay > 0)
    {
        m_fDelay -= fTimeDelta;
        return;
    }


    Fade(fTimeDelta);

    for (auto& pObj : m_pBackGround->Get_PartUI())
    {
        pObj->Update(fTimeDelta);
    }

    for (auto& pParts : m_Explainations)
    {
        for (auto& pObj : pParts->Get_PartUI())
            pObj->Update(fTimeDelta);
    }

    for (auto& pObj : m_Buttons)
    {
        pObj->Update(fTimeDelta);
    }

  
}

void CUI_Guide::Late_Update(_float fTimeDelta)
{

    if (m_fDelay > 0)
    {
        
        return;
    }

    if (m_isActive)
    {
        m_pBackGround->Late_Update(fTimeDelta);


        for (auto& pPart : m_Explainations)
        {
            pPart->Late_Update(fTimeDelta);
        }

        if (m_Explainations.size() >= 2)
        {
            for (auto& pButton : m_Buttons)
                pButton->Late_Update(fTimeDelta);
        }
        else
        {
            m_Buttons[0]->Late_Update(fTimeDelta);
        }

      
    }

   
}
 
HRESULT CUI_Guide::Render()
{
    return S_OK;
}

void CUI_Guide::Check_Button()
{

    if (m_pGameInstance->Key_Down(DIK_SPACE))
    {

        if (ENUM_CLASS(LEVEL::LOGO) != m_pGameInstance->GetCurrentLevelIndex())
        {
            list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster_Normal"));
            for (auto& obj : objList)
                obj->Set_TimeScale(1.f);

            CGameObject* pPlayer = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"));
            pPlayer->Set_TimeScale(1.f);

          
            CUI_Manager::Get_Instance()->On_Panel();
            CUI_Manager::Get_Instance()->Sound_Play("SE_UI_CloseWindowTutorial_01");

           

           
            FadeStart(1.f, 0.f, 0.25f);

            for (auto& pObj : m_pBackGround->Get_PartUI())
            {
                pObj->Set_isReverse(true);
            }

            for (auto& pParts : m_Explainations)
            {
                for (auto& pObj : pParts->Get_PartUI())
                    pObj->Set_isReverse(true);
            }

            for (auto& pObj : m_Buttons)
            {
                pObj->Set_isReverse(true);
            }

        

            return;
        }
        else
        {
            Active_Update(false);

        }

        m_iIndex = 0;
            
    }

    if (m_isActive)
    {
        if (m_Explainations.size() >= 2)
        {
            if (m_pGameInstance->Key_Down(DIK_A))
            {
                if (m_iIndex > 0)
                {
                    --m_iIndex;
                    CUI_Manager::Get_Instance()->Sound_Play("SE_UI_TutorialChangePage_01");
                }
            }
            else if (m_pGameInstance->Key_Down(DIK_D))
            {
                if (m_iIndex < m_iSize - 1)
                {
                    ++m_iIndex;
                    CUI_Manager::Get_Instance()->Sound_Play("SE_UI_TutorialChangePage_01");
                }
            }


            if (m_iIndex == 0)
            {
                m_Buttons[1]->Set_isActive(true);
                m_Buttons[2]->Set_isActive(false);
            }
            else if (m_iIndex == m_iSize - 1)
            {
                m_Buttons[1]->Set_isActive(false);
                m_Buttons[2]->Set_isActive(true);
            }
            else
            {

                m_Buttons[2]->Set_isActive(true);
            }

            for (_int i = 0; i < m_iSize; ++i)
            {
                if (i == m_iIndex)
                    m_Explainations[i]->Active_Update(true);
                else
                    m_Explainations[i]->Active_Update(false);
            }
        }

        
    }


    Click_Interaction();

}

void CUI_Guide::Click_Interaction()
{
    if (m_Buttons[0]->Check_MouseHover())
        m_Buttons[0]->Set_isMouseHover(true);

    if (m_pGameInstance->Mouse_Down(DIM::LBUTTON))
    {
        if (m_Buttons[0]->Get_isActive() && m_Buttons[0]->Check_Click())
        {


            if (ENUM_CLASS(LEVEL::LOGO) != m_pGameInstance->GetCurrentLevelIndex())
            {
                list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster_Normal"));
                for (auto& obj : objList)
                    obj->Set_TimeScale(1.f);

                CGameObject* pPlayer = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"));
                pPlayer->Set_TimeScale(1.f);


                CUI_Manager::Get_Instance()->On_Panel();
                return;
            }
            else
            {
                CUI_Manager::Get_Instance()->Sound_Play("SE_UI_CloseWindowTutorial_01");
                Active_Update(false);
            }

            m_iIndex = 0;
        }

        if (m_Explainations.size() >= 2)
        {
            if (m_Buttons[1]->Get_isActive() && m_Buttons[1]->Check_Click())
            {
                if (m_iIndex < m_iSize - 1)
                {
                    ++m_iIndex;
                    CUI_Manager::Get_Instance()->Sound_Play("SE_UI_TutorialChangePage_01");
                }

                m_Buttons[1]->Set_isHighlight(false);
            }

            if (m_Buttons[2]->Get_isActive() && m_Buttons[2]->Check_Click())
            {
                if (m_iIndex > 0)
                {
                    --m_iIndex;
                    CUI_Manager::Get_Instance()->Sound_Play("SE_UI_TutorialChangePage_01");
                }

                m_Buttons[2]->Set_isHighlight(false);
            }

        }

    }
}

void CUI_Guide::Active_Update(_bool isActive)
{
    m_isActive = isActive;

    for (auto& pExplain : m_Explainations)
        pExplain->Active_Update(isActive);

    for (auto& pButton : m_Buttons)
        pButton->Set_isActive(isActive);

    m_pBackGround->Active_Update(isActive);

    if (isActive)
        CUI_Manager::Get_Instance()->Sound_Play("SE_UI_OpenWindowTutorial_01");
    else
        CUI_Manager::Get_Instance()->Sound_Play("SE_UI_CloseWindowTutorial_01");
}



CUI_Guide* CUI_Guide::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Guide* pInstance = new CUI_Guide(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Guide");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_Guide::Clone(void* pArg)
{
    CUI_Guide* pInstance = new CUI_Guide(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_Guide");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_Guide::Free()
{
    __super::Free();

    Safe_Release(m_pBackGround);

    Safe_Release(m_pButtonContainer);

    for (auto& pContainer : m_Explainations)
        Safe_Release(pContainer);

}
