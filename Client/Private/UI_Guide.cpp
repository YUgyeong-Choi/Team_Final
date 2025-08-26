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
      
    }


    return S_OK;
}

void CUI_Guide::Priority_Update(_float fTimeDelta)
{
    
   

    // 키 입력
    Check_Button();
    
}


void CUI_Guide::Update(_float fTimeDelta)
{
    

  
}

void CUI_Guide::Late_Update(_float fTimeDelta)
{

    if (m_isActive)
    {
        m_pBackGround->Late_Update(fTimeDelta);


        for (auto& pPart : m_Explainations)
        {
            pPart->Late_Update(fTimeDelta);
        }

        for (auto& pButton : m_Buttons)
            pButton->Late_Update(fTimeDelta);
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
       
       
        m_iIndex = 0;

        if (ENUM_CLASS(LEVEL::LOGO) != m_pGameInstance->GetCurrentLevelIndex())
        {
          
            CUI_Manager::Get_Instance()->On_Panel();
            Set_bDead();
            return;
        }
        else
        {
            Active_Update(false);

        }
            
    }

    if (m_isActive)
    {
        if (m_pGameInstance->Key_Down(DIK_A))
        {
            --m_iIndex;

            if (m_iIndex < 0)
                m_iIndex = 0;
        }
        else if (m_pGameInstance->Key_Down(DIK_D))
        {
            ++m_iIndex;

            if (m_iIndex >= m_iSize)
                m_iIndex = m_iSize - 1;
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
          

            m_iIndex = 0;

            if (ENUM_CLASS(LEVEL::LOGO) != m_pGameInstance->GetCurrentLevelIndex())
            {
                Set_bDead();
                CUI_Manager::Get_Instance()->On_Panel();
               
                return;
            }
            else
            {
                Active_Update(false);
            }


        }


        if (m_Buttons[1]->Get_isActive()&& m_Buttons[1]->Check_Click())
        {
            ++m_iIndex;

            if (m_iIndex >= m_iSize)
                m_iIndex = m_iSize - 1;

            m_Buttons[1]->Set_isHighlight(false);
        }

        if (m_Buttons[2]->Get_isActive() && m_Buttons[2]->Check_Click())
        {
            --m_iIndex;

            if (m_iIndex < 0)
                m_iIndex = 0;

            m_Buttons[2]->Set_isHighlight(false);
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

}
