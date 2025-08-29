#include "Grinder.h"
#include "GameInstance.h"

CGrinder::CGrinder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CItem{pDevice, pContext}
{
}

CGrinder::CGrinder(const CGrinder& Prototype)
    :CItem{Prototype}
{
}

HRESULT CGrinder::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CGrinder::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_strProtoTag = TEXT("Prototype_GameObject_Grinder");


    m_isRender = true;

    Ready_Components();

    m_isActive = false;

    return S_OK;
}

void CGrinder::Priority_Update(_float fTimeDelta)
{
}

void CGrinder::Update(_float fTimeDelta)
{


    if (m_isActive)
    {
        m_fElapsedTime += fTimeDelta;
        // 무기 내구도 증가 
        _float fTemp = 0.25f;

        // 시간 측정해서 일정 시간 이상이면 내구도 차는 속도를 늘린다
        if (m_fElapsedTime > 1.f)
        {
            fTemp = 0.5f;
            m_pGameInstance->Notify(L"Weapon_Status", L"AddDurablity", &fTemp);
        }
        else
        {
            m_pGameInstance->Notify(L"Weapon_Status", L"AddDurablity", &fTemp);
        }
    }
    else
    {
        m_fElapsedTime = 0;
    }
}

void CGrinder::Late_Update(_float fTimeDelta)
{
}

HRESULT CGrinder::Render()
{
    return S_OK;
}

void CGrinder::Activate(_bool isActive)
{
    // 무기 게이지 연동? 하면 될듯

    m_isActive = isActive;
}

void CGrinder::Use()
{
    Activate(!m_isActive);
}

ITEM_DESC CGrinder::Get_ItemDesc()
{
    ITEM_DESC eDesc = {};

    eDesc.iItemIndex = 0;
    eDesc.strPrototag = m_strProtoTag;
    eDesc.isUsable = true;
    eDesc.isConsumable = false;


    return eDesc;
}

HRESULT CGrinder::Ready_Components()
{
    return S_OK;
}

CGrinder* CGrinder::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGrinder* pInstance = new CGrinder(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CPlayer");
        Safe_Release(pInstance);
    }
    return pInstance;
}
CGameObject* CGrinder::Clone(void* pArg)
{
    CGrinder* pInstance = new CGrinder(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CPlayer");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CGrinder::Free()
{
    __super::Free();

    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);

}

