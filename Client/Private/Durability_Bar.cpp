#include "Durability_Bar.h"

CDurability_Bar::CDurability_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CDynamic_UI{pDevice, pContext}
{
}

CDurability_Bar::CDurability_Bar(const CDurability_Bar& Prototype)
    :CDynamic_UI{Prototype}
{
}

HRESULT CDurability_Bar::Initialize_Prototype()
{
    return E_NOTIMPL;
}

HRESULT CDurability_Bar::Initialize(void* pArg)
{
    return E_NOTIMPL;
}

void CDurability_Bar::Priority_Update(_float fTimeDelta)
{
}

void CDurability_Bar::Update(_float fTimeDelta)
{
}

void CDurability_Bar::Late_Update(_float fTimeDelta)
{
}

HRESULT CDurability_Bar::Render()
{
    return E_NOTIMPL;
}

HRESULT CDurability_Bar::Bind_ShaderResources()
{
    return S_OK;
}

HRESULT CDurability_Bar::Ready_Component(const wstring& strTextureTag)
{
    return E_NOTIMPL;
}

CDurability_Bar* CDurability_Bar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CGameObject* CDurability_Bar::Clone(void* pArg)
{
    return nullptr;
}

void CDurability_Bar::Free()
{
}
