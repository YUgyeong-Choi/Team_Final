#include "UI_Feature.h"


CUI_Feature::CUI_Feature(const CUI_Feature& Prototype)
	:CComponent{Prototype}
{
}

HRESULT CUI_Feature::Initialize_Prototype()
{
	return E_NOTIMPL;
}

HRESULT CUI_Feature::Initialize(void* pArg)
{
	return E_NOTIMPL;
}


void CUI_Feature::Free()
{
}
