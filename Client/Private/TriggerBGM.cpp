#include "TriggerBGM.h"
#include "GameInstance.h"
#include "Client_Calculation.h"

CTriggerBGM::CTriggerBGM(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CTriggerBox{ pDevice, pContext }
{

}

CTriggerBGM::CTriggerBGM(const CTriggerBGM& Prototype)
	: CTriggerBox(Prototype)
{

}

HRESULT CTriggerBGM::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CTriggerBGM::Initialize(void* pArg)
{
	CTriggerBGM::TRIGGERBGM_DESC* TriggerBGMDESC = static_cast<TRIGGERBGM_DESC*>(pArg);
	m_strInBGM = TriggerBGMDESC->strInBGM;
	m_strOutBGM = TriggerBGMDESC->strOutBGM;


	if (FAILED(__super::Initialize(TriggerBGMDESC)))
		return E_FAIL;


	return S_OK;
}

void CTriggerBGM::Priority_Update(_float fTimeDelta)
{
}

void CTriggerBGM::Update(_float fTimeDelta)
{
}

void CTriggerBGM::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTriggerBGM::Render()
{
	__super::Render();
#ifdef _DEBUG

	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXTriggerCom))
	{
		if (m_pGameInstance->Get_RenderCollider())
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXTriggerCom)))
				return E_FAIL;
		}
	}

#endif
	return S_OK;
}

void CTriggerBGM::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{

}

void CTriggerBGM::On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CTriggerBGM::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	m_bInSound = !m_bInSound;
	if (m_bInSound)
	{
		m_pGameInstance->Change_BGM(m_strOutBGM);
	}
	else
	{
		m_pGameInstance->Change_BGM(m_strInBGM);
	}
}


HRESULT CTriggerBGM::Ready_Components()
{	

	return S_OK;
}


CTriggerBGM* CTriggerBGM::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTriggerBGM* pGameInstance = new CTriggerBGM(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTriggerBGM");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CTriggerBGM::Clone(void* pArg)
{
	CTriggerBGM* pGameInstance = new CTriggerBGM(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTriggerBGM");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CTriggerBGM::Free()
{
	__super::Free();
}
