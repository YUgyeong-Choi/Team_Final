#include "UI_Canvus.h"

CUI_Canvus::CUI_Canvus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CDynamic_UI{pDevice, pContext}
{
}

CUI_Canvus::CUI_Canvus(const CUI_Canvus& Prototype)
	:CDynamic_UI{Prototype}
{
}

HRESULT CUI_Canvus::Initialize_Prototype()
{
	return E_NOTIMPL;
}

HRESULT CUI_Canvus::Initialize(void* pArg)
{
	return E_NOTIMPL;
}

void CUI_Canvus::Priority_Update(_float fTimeDelta)
{
}

void CUI_Canvus::Update(_float fTimeDelta)
{
}

void CUI_Canvus::Late_Update(_float fTimeDelta)
{
}

HRESULT CUI_Canvus::Render()
{
	return E_NOTIMPL;
}

HRESULT CUI_Canvus::Ready_Components(const wstring& strTextureTag)
{
	return E_NOTIMPL;
}

HRESULT CUI_Canvus::Bind_ShaderResources()
{
	return E_NOTIMPL;
}

void CUI_Canvus::Update_Elements()
{
}

CUI_Canvus* CUI_Canvus::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CUI_Canvus::Clone(void* pArg)
{
	return nullptr;
}

void CUI_Canvus::Free()
{
}
