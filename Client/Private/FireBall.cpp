#include "Oil.h"
#include "Player.h"
#include "FireBall.h"
#include "SoundController.h"
#include "PhysXDynamicActor.h"

CFireBall::CFireBall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile(pDevice, pContext)
{
}

CFireBall::CFireBall(const CFireBall& Prototype)
	: CProjectile(Prototype)
	, m_fDamge(Prototype.m_fDamge)
{
}

HRESULT CFireBall::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CFireBall::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	PxFilterData FilterData{};
	FilterData.word0 = WORLDFILTER::FILTER_MONSTERWEAPON;
	FilterData.word1 =  WORLDFILTER::FILTER_MONSTERWEAPON | WORLDFILTER::FILTER_PLAYERBODY | WORLDFILTER::FILTER_MAP;

	m_pPhysXActorCom->Set_SimulationFilterData(FilterData);
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
	m_pSoundCom->Play("SE_NPC_SK_PJ_Fireball_Loop_01");
	return S_OK;
}

void CFireBall::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CFireBall::Update(_float fTimeDelta)
{
	if (ProcessCollisionPriority())
		return;
	__super::Update(fTimeDelta);
}

void CFireBall::Late_Update(_float fTimeDelta)
{

	__super::Late_Update(fTimeDelta);
}

HRESULT CFireBall::Render()
{
    return __super::Render();
}

void CFireBall::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (eColliderType == COLLIDERTYPE::BOSS_WEAPON)
	{
		if (auto pOil = dynamic_cast<COil*>(pOther))
		{
			pOil->Explode_Oil();
		}
	}

	if (eColliderType == COLLIDERTYPE::PLAYER)
	{
		m_CollisionPriority[Player] = pOther;
	}
	else if (eColliderType == COLLIDERTYPE::ENVIRONMENT_CONVEX || eColliderType == COLLIDERTYPE::ENVIRONMENT_TRI
		|| eColliderType == COLLIDERTYPE::BREAKABLE_OBJECT)
	{
		m_CollisionPriority[Environment] = pOther;
	}
	if(m_pPhysXActorCom)
		m_pPhysXActorCom->Set_ShapeFlag(false, false, true);
	//if(m_pEffect)
	//	m_pEffect->End_Effect();
}

void CFireBall::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CFireBall::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CFireBall::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFireBall::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{

}

void CFireBall::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT CFireBall::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;

	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_FireEater"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
		return E_FAIL;
    return S_OK;
}

HRESULT CFireBall::Ready_Effect()
{
	//CEffectContainer::DESC desc = {};
	//desc.pSocketMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	//XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixIdentity());
	//m_pEffect = dynamic_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_FireballTESTTESTTESTM1P1"), &desc));
	//if (nullptr == m_pEffect)
	//	MSG_BOX("이펙트 생성 실패함");

	return S_OK;
}

_bool CFireBall::ProcessCollisionPriority()
{
	for (_int i = 0; i < ECollisionPriority::End; i++)
	{
		auto pObj = m_CollisionPriority[i];
		if (pObj)
		{
			switch (i)
			{
			case ECollisionPriority::Oil:
				if (auto pOil = dynamic_cast<COil*>(pObj))
				{
					pOil->Explode_Oil();
					Set_bDead();
				}
				return true;;

			case ECollisionPriority::Player:
				if (auto pPlayer = dynamic_cast<CPlayer*>(pObj))
				{
					pPlayer->SetHitMotion(HITMOTION::KNOCKBACK);
					pPlayer->SetfReceiveDamage(15.f);
					pPlayer->SetElementTypeWeight(EELEMENT::FIRE, 0.5f);
					Set_bDead();
				}
				return true;;
			case ECollisionPriority::Environment:
				Set_bDead();
				return true;;
			}
			m_pSoundCom->StopAll();
		}
	}

	return false;
}

CFireBall* CFireBall::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFireBall* pInstance = new CFireBall(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CFireBall");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CFireBall::Clone(void* pArg)
{
	CFireBall* pInstance = new CFireBall(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CFireBall");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CFireBall::Free()
{
	__super::Free();
}
