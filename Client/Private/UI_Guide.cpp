#include "UI_Guide.h"
#include "Static_UI.h"

CUI_Guide::CUI_Guide(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUI_Container{pDevice, pContext}
{
}

CUI_Guide::CUI_Guide(const CUI_Guide& Prototype)
    :CUI_Container{Prototype}
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
    return E_NOTIMPL;
}

HRESULT CUI_Guide::Initialize(void* pArg)
{
    return E_NOTIMPL;
}

void CUI_Guide::Priority_Update(_float fTimeDelta)
{
}

void CUI_Guide::Update(_float fTimeDelta)
{
}

void CUI_Guide::Late_Update(_float fTimeDelta)
{
}

HRESULT CUI_Guide::Render()
{
    return E_NOTIMPL;
}

CUI_Guide* CUI_Guide::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CGameObject* CUI_Guide::Clone(void* pArg)
{
    return nullptr;
}

void CUI_Guide::Free()
{
}
