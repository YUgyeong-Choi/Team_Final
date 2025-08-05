#include "Grinder.h"

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
    return E_NOTIMPL;
}

HRESULT CGrinder::Initialize(void* pArg)
{
    return E_NOTIMPL;
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
    return E_NOTIMPL;
}

void CGrinder::Activate()
{
}

HRESULT CGrinder::Ready_Components()
{
    return E_NOTIMPL;
}

CGrinder* CGrinder::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CGameObject* CGrinder::Clone(void* pArg)
{
    return nullptr;
}

void CGrinder::Free()
{
}
