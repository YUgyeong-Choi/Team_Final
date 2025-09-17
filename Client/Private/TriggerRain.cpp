#include "TriggerRain.h"
#include "GameInstance.h"
#include "Effect_Manager.h"

CTriggerRain::CTriggerRain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CTriggerBox{ pDevice, pContext }
{

}

CTriggerRain::CTriggerRain(const CTriggerRain& Prototype)
	: CTriggerBox(Prototype)
{

}

HRESULT CTriggerRain::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CTriggerRain::Initialize(void* pArg)
{
	CTriggerRain::TRIGGERNOMESH_DESC* TriggerNoMeshDESC = static_cast<TRIGGERNOMESH_DESC*>(pArg);

	if (FAILED(__super::Initialize(TriggerNoMeshDESC)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_bBossDoor = TriggerNoMeshDESC->bBossDoor;

	return S_OK;
}

void CTriggerRain::Priority_Update(_float fTimeDelta)
{
	
}

void CTriggerRain::Update(_float fTimeDelta)
{
}

void CTriggerRain::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTriggerRain::Render()
{
	__super::Render();
	return S_OK;
}

void CTriggerRain::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (m_bBossDoor)
		EFFECT_MANAGER->Set_Active_Effect(TEXT("PlayerRainVolume"), false);
	else
		EFFECT_MANAGER->Set_Active_Effect(TEXT("PlayerRainVolume"), m_bRainActive);
		
}

void CTriggerRain::On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{

}

void CTriggerRain::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bBossDoor)
		m_bRainActive = !m_bRainActive;
}

HRESULT CTriggerRain::Ready_Components()
{	

	return S_OK;
}


CTriggerRain* CTriggerRain::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTriggerRain* pGameInstance = new CTriggerRain(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTriggerRain");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CTriggerRain::Clone(void* pArg)
{
	CTriggerRain* pGameInstance = new CTriggerRain(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTriggerRain");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CTriggerRain::Free()
{
	__super::Free();
}
