#include "Panel_Player_Arm.h"
#include "GameInstance.h"

CPanel_Player_Arm::CPanel_Player_Arm(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUI_Container{pDevice, pContext}
{
}

CPanel_Player_Arm::CPanel_Player_Arm(const CPanel_Player_Arm& Prototype)
    :CUI_Container{Prototype}
{
}

HRESULT CPanel_Player_Arm::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CPanel_Player_Arm::Initialize(void* pArg)
{
    UI_CONTAINER_DESC eDesc = {};
    eDesc.strFilePath = TEXT("../Bin/Save/UI/LegionArm/LegionArm.json");

    if (FAILED(__super::Initialize(&eDesc)))
        return E_FAIL;

    // 콜백 등록

    return S_OK;
}

void CPanel_Player_Arm::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CPanel_Player_Arm::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CPanel_Player_Arm::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CPanel_Player_Arm::Render()
{
	return S_OK;
}

CPanel_Player_Arm* CPanel_Player_Arm::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPanel_Player_Arm* pInstance = new CPanel_Player_Arm(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPanel_Player_Arm");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPanel_Player_Arm::Clone(void* pArg)
{
	CPanel_Player_Arm* pInstance = new CPanel_Player_Arm(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPanel_Player_Arm");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPanel_Player_Arm::Free()
{
	__super::Free();

}

