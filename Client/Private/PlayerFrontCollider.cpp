#include "PlayerFrontCollider.h"
/*
#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "Effect_Manager.h"
#include "SwordTrailEffect.h"
#include "PhysX_IgnoreSelfCallback.h"
*/
#include "Unit.h"

CPlayerFrontCollider::CPlayerFrontCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}
CPlayerFrontCollider::CPlayerFrontCollider(const CPlayerFrontCollider& Prototype)
	: CGameObject(Prototype)
{
}
HRESULT CPlayerFrontCollider::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayerFrontCollider::Initialize(void* pArg)
{
	WEAPON_DESC* pDesc = static_cast<WEAPON_DESC*>(pArg);
	
	m_InitPos = pDesc->InitPos;
	m_InitScale = pDesc->InitScale;
	m_szName = pDesc->szName;
	m_pOwner = dynamic_cast<CUnit*>(pDesc->pOwner);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CPlayerFrontCollider::Priority_Update(_float fTimeDelta)
{
}

void CPlayerFrontCollider::Update(_float fTimeDelta)
{
}

void CPlayerFrontCollider::Late_Update(_float fTimeDelta)
{
}

HRESULT CPlayerFrontCollider::Render()
{
	return S_OK;
}


HRESULT CPlayerFrontCollider::Ready_Components()
{
	return S_OK;
}

void CPlayerFrontCollider::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CPlayerFrontCollider::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CPlayerFrontCollider::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}


void CPlayerFrontCollider::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CPlayerFrontCollider::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}



CPlayerFrontCollider* CPlayerFrontCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPlayerFrontCollider* pInstance = new CPlayerFrontCollider(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPlayerFrontCollider");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CPlayerFrontCollider::Clone(void* pArg)
{
	CPlayerFrontCollider* pInstance = new CPlayerFrontCollider(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPlayerFrontCollider");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPlayerFrontCollider::Free()
{
	__super::Free();
}
