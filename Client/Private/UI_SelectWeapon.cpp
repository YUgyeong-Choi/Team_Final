#include "UI_SelectWeapon.h"
#include "GameInstance.h"
#include "UI_Button.h"
#include "Camera_Manager.h"
#include "UI_Manager.h"
#include "ActionType_Icon.h"
#include "Player.h"
#include "TriggerBox.h"

CUI_SelectWeapon::CUI_SelectWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUI_Container{pDevice, pContext}
{
}

CUI_SelectWeapon::CUI_SelectWeapon(const CUI_SelectWeapon& Prototype)
    :CUI_Container{Prototype}
{
}

HRESULT CUI_SelectWeapon::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_SelectWeapon::Initialize(void* pArg)
{
    // 백그라운드
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    // 버튼

    SELECT_WEAPON_UI_DESC* pDesc = static_cast<SELECT_WEAPON_UI_DESC*>(pArg);

    m_pTarget = pDesc->pTarget;

    SELECT_WEAPON_UI_DESC eDesc{};

    eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectWeapon/SelectWeapon_Button.json");
    m_pButtons = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

    eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectWeapon/SelectWeapon_Icon.json");
    m_pIcons = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

    // 무기 선택하면 나오는 화면
    eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectWeapon/SelectWeapon_Contiune.json");
    m_pBackgrounds = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

    // 결정 버튼
    eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectWeapon/SelectWeapon_Button_Continue.json");
    m_pButton_Select = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));

    eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectWeapon/SelectWeapon_Text.json");
    m_pText = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));
    
   


    // 선택된 이미지 강조하기 위한 ui 이미지 추가
    eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectWeapon/SelectWeapon_Effect.json");
    m_pSelectEffect = static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc));


    static_cast<CActionType_Icon*>(m_pIcons->Get_PartUI()[0])->Set_isSelect(true);

    _vector vPos = m_pIcons->Get_PartUI()[0]->Get_TransfomCom()->Get_State(STATE::POSITION);

    m_pSelectEffect->Get_PartUI()[0]->Get_TransfomCom()->Set_State(STATE::POSITION, vPos);

    _float3 vScale = m_pIcons->Get_PartUI()[0]->Get_TransfomCom()->Compute_Scaled();

    vPos -= {vScale.x * 0.25f, vScale.y * 0.45f, 0.f, 0.f};

    m_pSelectEffect->Get_PartUI()[1]->Get_TransfomCom()->Set_State(STATE::POSITION, vPos);


    CUI_Manager::Get_Instance()->Sound_Play("SE_UI_OpenWindow_01");

    return S_OK;
}

void CUI_SelectWeapon::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);

    m_pButtons->Priority_Update(fTimeDelta);
    m_pIcons->Priority_Update(fTimeDelta);
    m_pBackgrounds->Priority_Update(fTimeDelta);
    m_pButton_Select->Priority_Update(fTimeDelta);
    m_pText->Priority_Update(fTimeDelta);
    m_pSelectEffect->Priority_Update(fTimeDelta);

}

void CUI_SelectWeapon::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    m_pButtons->Update(fTimeDelta);
    m_pIcons->Update(fTimeDelta);
    m_pSelectEffect->Update(fTimeDelta);
    
    if (m_isSelectWeapon)
    {
        m_pBackgrounds->Update(fTimeDelta);
        m_pButton_Select->Update(fTimeDelta);
        m_pText->Update(fTimeDelta);
    }

    if (m_pGameInstance->Key_Down(DIK_ESCAPE) && m_isSelectWeapon)
    {
        m_isSelectWeapon = false;
        CUI_Manager::Get_Instance()->Sound_Play("SE_UI_CloseWindow_01");
    }

    if (m_pGameInstance->Key_Down(DIK_ESCAPE) && !m_isSelectWeapon)
    {
        CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true, 0.f);
        CUI_Manager::Get_Instance()->On_Panel();

        static_cast<CTriggerBox*>(m_pTarget)->Reset_DoOnce();

        Set_bDead();

        CUI_Manager::Get_Instance()->Sound_Play("SE_UI_CloseWindow_01");
    }


    if (m_pGameInstance->Key_Down(DIK_SPACE) && m_isSelectWeapon)
    {
        CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true, 0.f);
        CUI_Manager::Get_Instance()->On_Panel();

        m_pTarget->Set_bDead();

        Set_bDead();

        CPlayer* pPlayer = GET_PLAYER(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION));
        pPlayer->GetWeapon();
        CUI_Manager::Get_Instance()->Sound_Play("SE_UI_RewardItem_02");
    }
   
    // 첫 화면에서 두번째 화면
    if (m_pGameInstance->Key_Down(DIK_SPACE) && !m_isSelectWeapon)
    {
        m_isSelectWeapon = true;
        CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Selected_Default_03");
    }

   


    if (m_pGameInstance->Mouse_Down(DIM::LBUTTON))
    {

    }

    if (m_pGameInstance->Key_Down(DIK_A))
    {

        if (m_iSelectIndex <= 0)
        {
            return;
        }
        
        static_cast<CActionType_Icon*>(m_pIcons->Get_PartUI()[m_iSelectIndex])->Set_isSelect(false);
        --m_iSelectIndex;
        static_cast<CActionType_Icon*>(m_pIcons->Get_PartUI()[m_iSelectIndex])->Set_isSelect(true);
        CUI_Manager::Get_Instance()->Sound_Play("SE_UI_QuickSlotChange_03");
    }
    else if (m_pGameInstance->Key_Down(DIK_D))
    {
        if (m_iSelectIndex >= m_pIcons->Get_PartUI().size() - 1)
        {
            return;
        }

        static_cast<CActionType_Icon*>(m_pIcons->Get_PartUI()[m_iSelectIndex])->Set_isSelect(false);
        ++m_iSelectIndex;
        static_cast<CActionType_Icon*>(m_pIcons->Get_PartUI()[m_iSelectIndex])->Set_isSelect(true);
        CUI_Manager::Get_Instance()->Sound_Play("SE_UI_QuickSlotChange_03");
    }

    _vector vPos = m_pIcons->Get_PartUI()[m_iSelectIndex]->Get_TransfomCom()->Get_State(STATE::POSITION);

    m_pSelectEffect->Get_PartUI()[0]->Get_TransfomCom()->Set_State(STATE::POSITION, vPos);

    _float3 vScale = m_pIcons->Get_PartUI()[m_iSelectIndex]->Get_TransfomCom()->Compute_Scaled();

    vPos -= {vScale.x * 0.25f, vScale.y * 0.45f, 0.f, 0.f};

    m_pSelectEffect->Get_PartUI()[1]->Get_TransfomCom()->Set_State(STATE::POSITION, vPos);
   
}

void CUI_SelectWeapon::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    m_pButtons->Late_Update(fTimeDelta);
    m_pIcons->Late_Update(fTimeDelta);
    m_pSelectEffect->Late_Update(fTimeDelta);

    if (m_isSelectWeapon)
    {
        m_pBackgrounds->Late_Update(fTimeDelta);
        m_pButton_Select->Late_Update(fTimeDelta);
        m_pText->Late_Update(fTimeDelta);
    }
}

HRESULT CUI_SelectWeapon::Render()
{
    return S_OK;
}

void CUI_SelectWeapon::Update_Script(const string strName, const string strText, _bool isAuto)
{
}

void CUI_SelectWeapon::Active_Update(_bool isActive)
{
    __super::Active_Update(isActive);
    m_pButtons->Active_Update(isActive);
    m_pIcons->Active_Update(isActive);
    m_pSelectEffect->Active_Update(isActive);

}

CUI_SelectWeapon* CUI_SelectWeapon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_SelectWeapon* pInstance = new CUI_SelectWeapon(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_SelectWeapon");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_SelectWeapon::Clone(void* pArg)
{
    CUI_SelectWeapon* pInstance = new CUI_SelectWeapon(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_SelectWeapon");
        Safe_Release(pInstance);
    }

    return pInstance;
}


void CUI_SelectWeapon::Free()
{
    __super::Free();

    Safe_Release(m_pButtons);
    Safe_Release(m_pIcons);
    Safe_Release(m_pBackgrounds);
    Safe_Release(m_pButton_Select);
    Safe_Release(m_pText);
    Safe_Release(m_pSelectEffect);

    
}
