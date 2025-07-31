#include "UI_Feature.h"


CUI_Feature::CUI_Feature(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CComponent{ pDevice, pContext }
{
}

CUI_Feature::CUI_Feature(const CUI_Feature& Prototype)
	:CComponent{Prototype}
{
}

json CUI_Feature::Serialize()
{
	json j;

	j["iStartFrame"] = m_iStartFrame;
	j["iEndFrame"] = m_iEndFrame;
	j["isLoop"] = m_isLoop;

	return j;
}

void CUI_Feature::Deserialize(const json& j)
{
	m_iStartFrame = j["iStartFrame"];
	m_iEndFrame = j["iEndFrame"];
	m_isLoop = j["isLoop"].get<_bool>();

	m_iRange = m_iEndFrame - m_iStartFrame;
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
