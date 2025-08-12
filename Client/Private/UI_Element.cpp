#include "UI_Element.h"

CUI_Element::CUI_Element(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIObject{pDevice, pContext}
{
}

CUI_Element::CUI_Element(const CUI_Element& Prototype)
	:CUIObject{Prototype}
{
}

HRESULT CUI_Element::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Element::Initialize(void* pArg)
{
	return S_OK;
}

void CUI_Element::Priority_Update(_float fTimeDelta)
{
}

void CUI_Element::Update(_float fTimeDelta)
{
}

void CUI_Element::Late_Update(_float fTimeDelta)
{
}

HRESULT CUI_Element::Render()
{
	return E_NOTIMPL;
}

CUI_Element* CUI_Element::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CUI_Element::Clone(void* pArg)
{
	return nullptr;
}

void CUI_Element::Free()
{
}
