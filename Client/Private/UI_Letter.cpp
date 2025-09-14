#include "UI_Letter.h"
#include "GameInstance.h"
#include "UI_Manager.h"
#include "Camera_Manager.h"

CUI_Letter::CUI_Letter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUI_Container{pDevice, pContext}
{
}

CUI_Letter::CUI_Letter(const CUI_Letter& Prototype)
    :CUI_Container{Prototype}
{
}


HRESULT CUI_Letter::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Letter::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    UI_LETTER_DESC* pDesc = static_cast<UI_LETTER_DESC*>(pArg);


    for (auto& partPath : pDesc->partPaths)
    {
        CUI_Container::UI_CONTAINER_DESC eDesc = {};

        eDesc.strFilePath = partPath;

        m_Explainations.push_back(static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc)));

        m_Explainations.back()->Active_Update(false);
    }

    // 백그라운드

    CUI_Container::UI_CONTAINER_DESC eBackDesc = {};

    eBackDesc.strFilePath = TEXT("../Bin/Save/UI/Letter/Letter_Background.json");

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
            obj->Set_TimeScale(0.0000001f);

        CGameObject* pPlayer = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"));
        pPlayer->Set_TimeScale(0.0000001f);

        if (m_Explainations.size() == 1)
            m_Explainations[0]->Active_Update(true);

    }


    m_fCurrentAlpha = 1.f;

    CUI_Manager::Get_Instance()->Sound_Play("SE_UI_OpenWindowTutorial_01");


    return S_OK;
}

void CUI_Letter::Priority_Update(_float fTimeDelta)
{
    if (m_isFade == false && m_fCurrentAlpha <= 0.f)
    {
        Set_bDead();
        m_pGameInstance->Set_GameTimeScale(1.f);
        CCamera_Manager::Get_Instance()->SetbMoveable(true);

        list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster_Normal"));
        for (auto& obj : objList)
            obj->Set_TimeScale(1.f);

        CGameObject* pPlayer = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"));
        pPlayer->Set_TimeScale(1.f);

        return;
    }

    // 키 입력
    Check_Button();
}

void CUI_Letter::Update(_float fTimeDelta)
{
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

  
}

void CUI_Letter::Late_Update(_float fTimeDelta)
{
    if (m_isActive)
    {
        m_pBackGround->Late_Update(fTimeDelta);


        for (auto& pPart : m_Explainations)
        {
            pPart->Late_Update(fTimeDelta);
        }


    }
}

HRESULT CUI_Letter::Render()
{
    return S_OK;
}

void CUI_Letter::Check_Button()
{
    if (m_pGameInstance->Key_Down(DIK_SPACE) || m_pGameInstance->Key_Down(DIK_ESCAPE))
    {

        if (ENUM_CLASS(LEVEL::LOGO) != m_pGameInstance->GetCurrentLevelIndex())
        {

            CUI_Manager::Get_Instance()->On_Panel();
            CUI_Manager::Get_Instance()->Sound_Play("SE_UI_CloseWindowTutorial_01");

            FadeStart(1.f, 0.f, 0.3f);

            for (auto& pObj : m_pBackGround->Get_PartUI())
            {
                pObj->Set_isReverse(true);
            }

            for (auto& pParts : m_Explainations)
            {
                for (auto& pObj : pParts->Get_PartUI())
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

            for (_int i = 0; i < m_iSize; ++i)
            {
                if (i == m_iIndex)
                    m_Explainations[i]->Active_Update(true);
                else
                    m_Explainations[i]->Active_Update(false);
            }
        }


    }

}

void CUI_Letter::Active_Update(_bool isActive)
{
    m_isActive = isActive;

    for (auto& pExplain : m_Explainations)
        pExplain->Active_Update(isActive);

    m_pBackGround->Active_Update(isActive);

    if (isActive)
        CUI_Manager::Get_Instance()->Sound_Play("SE_UI_OpenWindowTutorial_01");
    else
        CUI_Manager::Get_Instance()->Sound_Play("SE_UI_CloseWindowTutorial_01");
}

CUI_Letter* CUI_Letter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Letter* pInstance = new CUI_Letter(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Letter");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_Letter::Clone(void* pArg)
{
    CUI_Letter* pInstance = new CUI_Letter(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_Letter");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_Letter::Free()
{
    __super::Free();

    Safe_Release(m_pBackGround);

    for (auto& pContainer : m_Explainations)
        Safe_Release(pContainer);

}
