#include "Dynamic_UI.h"

CDynamic_UI::CDynamic_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)	
	:CUIObject{pDevice, pContext}
{
}

CDynamic_UI::CDynamic_UI(const CDynamic_UI& Prototype)
	:CUIObject{Prototype}
{
}

HRESULT CDynamic_UI::Initialize(void* pArg)
{
	return S_OK;
}

void CDynamic_UI::Priority_Update(_float fTimeDelta)
{
}

void CDynamic_UI::Update(_float fTimeDelta)
{
}

void CDynamic_UI::Late_Update(_float fTimeDelta)
{
}

HRESULT CDynamic_UI::Render()
{
	return S_OK;
}



void CDynamic_UI::Free()
{
	__super::Free();
}
