#include "TextButton_UI.h"

CTextButton_UI::CTextButton_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CButton_UI{pDevice, pContext}
{
}

CTextButton_UI::CTextButton_UI(const CTextButton_UI& Prototype)
	:CButton_UI{Prototype}
{
}

HRESULT CTextButton_UI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTextButton_UI::Initialize(void* pArg)
{ 

	__super::Initialize(pArg);  



	return S_OK;
}

void CTextButton_UI::Priority_Update(_float fTimeDelta)
{
}

void CTextButton_UI::Update(_float fTimeDelta)
{
}

void CTextButton_UI::Late_Update(_float fTimeDelta)
{
}

HRESULT CTextButton_UI::Render()
{
	return E_NOTIMPL;
}

HRESULT CTextButton_UI::Ready_Components()
{
	return E_NOTIMPL;
}

HRESULT CTextButton_UI::Bind_ShaderResources()
{
	return E_NOTIMPL;
}

CTextButton_UI* CTextButton_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CTextButton_UI::Clone(void* pArg)
{
	return nullptr;
}

void CTextButton_UI::Free()
{
}
