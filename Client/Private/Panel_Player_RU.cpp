#include "Panel_Player_RU.h"

CPanel_Player_RU::CPanel_Player_RU(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Container{pDevice, pContext}
{
}

CPanel_Player_RU::CPanel_Player_RU(const CPanel_Player_RU& Prototype)
	:CUI_Container{Prototype}
{
}

HRESULT CPanel_Player_RU::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPanel_Player_RU::Initialize(void* pArg)
{
	UI_CONTAINER_DESC eDesc = {};
	eDesc.strFilePath = TEXT("../Bin/Save/UI/Panel_Player_RU.json");

	if (FAILED(__super::Initialize(&eDesc)))
		return E_FAIL;

	// 콜백 등록

	return S_OK;
}

void CPanel_Player_RU::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CPanel_Player_RU::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CPanel_Player_RU::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CPanel_Player_RU::Render()
{
	return S_OK;
}

CPanel_Player_RU* CPanel_Player_RU::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPanel_Player_RU* pInstance = new CPanel_Player_RU(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPanel_Player_RD");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPanel_Player_RU::Clone(void* pArg)
{
	CPanel_Player_RU* pInstance = new CPanel_Player_RU(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPanel_Player_RU");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPanel_Player_RU::Free()
{
	__super::Free();

}

