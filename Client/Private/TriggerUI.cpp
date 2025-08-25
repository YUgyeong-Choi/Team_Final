#include "TriggerUI.h"
#include "GameInstance.h"
#include "UI_Manager.h"

CTriggerUI::CTriggerUI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CTriggerBox{ pDevice, pContext }
{

}

CTriggerUI::CTriggerUI(const CTriggerUI& Prototype)
	: CTriggerBox(Prototype)
{

}

HRESULT CTriggerUI::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CTriggerUI::Initialize(void* pArg)
{
   	CTriggerUI::TRIGGERUI_DESC* TriggerUIDESC = static_cast<TRIGGERUI_DESC*>(pArg);

	m_eTriggerUIType = TriggerUIDESC->eTriggerUIType;

 	if (FAILED(__super::Initialize(TriggerUIDESC)))
		return E_FAIL;


	return S_OK;
}

void CTriggerUI::Priority_Update(_float fTimeDelta)
{
	if (m_bDead)
		m_pPhysXTriggerCom->RemovePhysX();
}

void CTriggerUI::Update(_float fTimeDelta)
{
}

void CTriggerUI::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTriggerUI::Render()
{
	__super::Render();

	return S_OK;
}

void CTriggerUI::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bDoOnce)
	{
		m_bDoOnce = true;

		switch (m_eTriggerUIType)
		{
		case Client::TRIGGERUI_TYPE::STATIONTITLE:
			break;
		case Client::TRIGGERUI_TYPE::GUARD:
			break;
		default:
			break;
		}

		Set_bDead();
	}
		
}

void CTriggerUI::On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{

}

void CTriggerUI::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

CTriggerUI* CTriggerUI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTriggerUI* pGameInstance = new CTriggerUI(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTriggerUI");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CTriggerUI::Clone(void* pArg)
{
	CTriggerUI* pGameInstance = new CTriggerUI(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTriggerUI");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CTriggerUI::Free()
{
	__super::Free();
}
