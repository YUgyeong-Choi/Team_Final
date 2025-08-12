#include "UI_Component.h"
#include "GameInstance.h"

CUI_Component::CUI_Component(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CComponent{pDevice, pContext}
{
}

CUI_Component::CUI_Component(const CUI_Component& Prototype)
	:CComponent{Prototype}
{
}

json CUI_Component::Serialize()
{
	json j;

	j["ComponentTag"] = WStringToStringU8(m_strProtoTag);

	return j;
}

void CUI_Component::Deserialize(const json& j)
{
	m_strProtoTag = StringToWStringU8(j["ComponentTag"].get<string>());

}

HRESULT CUI_Component::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Component::Initialize(void* pArg)
{
	return S_OK;
}




void CUI_Component::Free()
{
	__super::Free();

}
