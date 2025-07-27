#include "UI_Feature.h"


CUI_Feature::CUI_Feature(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CComponent{ pDevice, pContext }
{
}

CUI_Feature::CUI_Feature(const CUI_Feature& Prototype)
	:CComponent{Prototype}
{
}

HRESULT CUI_Feature::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Feature::Initialize(void* pArg)
{
	return S_OK;
}


void CUI_Feature::Free()
{
	__super::Free();
}
