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



    return S_OK;
}

void CGrinder::Priority_Update(_float fTimeDelta)
{
}

void CGrinder::Update(_float fTimeDelta)
{
}

void CGrinder::Late_Update(_float fTimeDelta)
{
}

HRESULT CGrinder::Render()
{
    return S_OK;
}

void CGrinder::Activate()
{
    // 무기 게이지 연동? 하면 될듯
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

