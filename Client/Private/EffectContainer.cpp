#include "EffectContainer.h"

HRESULT CEffectContainer::Initialize_Prototype()
{
    return E_NOTIMPL;
}

HRESULT CEffectContainer::Initialize(void* pArg)
{
    return E_NOTIMPL;
}

void CEffectContainer::Priority_Update(_float fTimeDelta)
{
}

void CEffectContainer::Update(_float fTimeDelta)
{
}

void CEffectContainer::Late_Update(_float fTimeDelta)
{
}

HRESULT CEffectContainer::Render()
{
    return E_NOTIMPL;
}

HRESULT CEffectContainer::Ready_Components()
{
    return E_NOTIMPL;
}

HRESULT CEffectContainer::Bind_ShaderResources()
{
    return E_NOTIMPL;
}

CEffectContainer* CEffectContainer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CGameObject* CEffectContainer::Clone(void* pArg)
{
    return nullptr;
}

void CEffectContainer::Free()
{
}
