#include "UI_Script.h"

CUI_Script::CUI_Script(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CUI_Script::CUI_Script(const CUI_Script& Prototype)
	:CUI_Container{Prototype}
{
}

json CUI_Script::Serialize()
{
	return json();
}

void CUI_Script::Deserialize(const json& j)
{
}



void CUI_Script::Set_isReverse(_bool isReverse)
{
}

HRESULT CUI_Script::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Script::Initialize(void* pArg)
{
	return E_NOTIMPL;
}

void CUI_Script::Priority_Update(_float fTimeDelta)
{
}

void CUI_Script::Update(_float fTimeDelta)
{
}

void CUI_Script::Late_Update(_float fTimeDelta)
{
}

HRESULT CUI_Script::Render()
{
	return E_NOTIMPL;
}

void CUI_Script::Active_Update(_bool isActive)
{
}

void CUI_Script::Add_UI_From_Tool(CUIObject* pObj)
{
}

CUI_Script* CUI_Script::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CUI_Script::Clone(void* pArg)
{
	return nullptr;
}

void CUI_Script::Free()
{
}
