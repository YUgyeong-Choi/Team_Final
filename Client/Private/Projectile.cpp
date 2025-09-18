#include "Projectile.h"
#include "GameInstance.h"
#include "PhysXDynamicActor.h"

CProjectile::CProjectile(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}
CProjectile::CProjectile(const CProjectile& Prototype)
	: CGameObject(Prototype),
	m_pModelCom{ Prototype.m_pModelCom },
	m_pShaderCom{ Prototype.m_pShaderCom }
{

}


HRESULT CProjectile::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CProjectile::Initialize(void* pArg)
{
	PROJECTILE_DESC* pDesc = static_cast<PROJECTILE_DESC*>(pArg);

	if (nullptr == pDesc)
		return E_FAIL;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_fElapsedTime = 0.f;
	m_fSpeed = pDesc->fSpeed; // 초기 스피드
	m_fLifeTime = pDesc->fLifeTime; // 생명 주기
	m_vDirection = XMLoadFloat3(&pDesc->vDir); // 방향
	_vector vStartPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPos), 1.f); // 시작 위치
	m_vStartPos = vStartPos;
	m_fRadius = pDesc->fRadius; // 구체 반지름
	m_fStartTime = pDesc->fStartTime; // 중력 시작 시간 
	m_fGravityOnDist = pDesc->fGravityOnDist; // 중력 시작 거리
	m_bUseDistTrigger = pDesc->bUseDistTrigger; // 중력 시작 시간으로 판단해서 사용
	m_bUseTimeTrigger = pDesc->bUseTimeTrigger; // 중력 시작 거리로 판단해서 사용
	if (FAILED(Ready_Actor()))
		return E_FAIL;

	if (FAILED(Ready_Effect()))
		return E_FAIL;

	return S_OK;
}

void CProjectile::Priority_Update(_float fTimeDelta)
{
	if (m_bDead)
	{
		m_pPhysXActorCom->RemovePhysX();
	}
}

void CProjectile::Update(_float fTimeDelta)
{
	m_fElapsedTime += fTimeDelta;
	if (m_fElapsedTime >= m_fLifeTime)
	{
		m_fElapsedTime = 0.f;
		Set_bDead();
		return;
	}

	if (m_pPhysXActorCom == nullptr)
		return;

	if (m_bUseDistTrigger || m_bUseTimeTrigger)
	{

		if (m_bGravity == false)
		{
			if (m_bUseTimeTrigger)
			{
				if (m_fElapsedTime >= m_fStartTime)
				{
					m_bGravity = true;
					if (auto pActor = m_pPhysXActorCom->Get_Actor())
					{
						pActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
					}
				}
			}
			else // 거리 기반
			{
				_float3 vCurPos = Get_WorldPosFromActor();
				_float3 vStartPos = { XMVectorGetX(m_vStartPos), XMVectorGetY(m_vStartPos), XMVectorGetZ(m_vStartPos) };
				_float fDist = XMVectorGetX(XMVector3Length(XMVectorSubtract(XMLoadFloat3(&vCurPos), XMLoadFloat3(&vStartPos))));
				if (fDist >= m_fGravityOnDist)
				{
					m_bGravity = true;
					if (auto pActor = m_pPhysXActorCom->Get_Actor())
					{
						pActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
					}
				}
			}
		}
	}
	if (auto pActor = m_pPhysXActorCom->Get_Actor())
	{
		PxTransform pose = pActor->getGlobalPose();
		_vector vPos = XMVectorSet(pose.p.x, pose.p.y, pose.p.z, 1.f);
		m_pTransformCom->Set_State(STATE::POSITION, vPos);

		if (m_pSoundCom)
		{
			_float3 pos{};
			XMStoreFloat3(&pos, vPos);
			m_pSoundCom->Update3DPosition(pos);
		}
	}
}

void CProjectile::Late_Update(_float fTimeDelta)
{
#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider())
	{
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif
}

HRESULT CProjectile::Render()
{
	if (FAILED(Bind_Shader()))
		return E_FAIL;
	return S_OK;
}

_float3 CProjectile::Get_WorldPosFromActor() const
{
	_float3 vPos = { 0.f, 0.f, 0.f };
	if (auto pActor = m_pPhysXActorCom->Get_Actor())
	{
		PxTransform transform = pActor->getGlobalPose();
		vPos.x = transform.p.x;
		vPos.y = transform.p.y;
		vPos.z = transform.p.z;
	}
	return vPos;
}

HRESULT CProjectile::Ready_Components()
{	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;
	return S_OK;
}

HRESULT CProjectile::Ready_Actor()
{
	PxQuat RotationQuat = PxQuat(PxIdentity);
	PxVec3 PositionVec = PxVec3(XMVectorGetX(m_vStartPos), XMVectorGetY(m_vStartPos), XMVectorGetZ(m_vStartPos));
	PxTransform armPose(PositionVec, RotationQuat);
	PxSphereGeometry Geom = m_pGameInstance->CookSphereGeometry(m_fRadius);
	if (FAILED(m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), Geom, armPose, m_pGameInstance->GetMaterial(L"Default"))))
		return E_FAIL;
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);
	PxFilterData FilterData{};
	FilterData.word0 = WORLDFILTER::FILTER_MONSTERWEAPON;
	FilterData.word1 = WORLDFILTER::FILTER_PLAYERBODY | WORLDFILTER::FILTER_MAP;

	m_pPhysXActorCom->Set_SimulationFilterData(FilterData);
	m_pPhysXActorCom->Set_QueryFilterData(FilterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::MONSTER_WEAPON);
	m_pPhysXActorCom->Set_Kinematic(false);

	if (auto pActor = m_pPhysXActorCom->Get_Actor())
	{
		// 초기에 중력 비활성화
		pActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	}
	PxVec3 velocity = PxVec3(XMVectorGetX(m_vDirection), XMVectorGetY(m_vDirection), XMVectorGetZ(m_vDirection)) * m_fSpeed;

	if (auto pRigid = m_pPhysXActorCom->Get_Actor()->is<PxRigidDynamic>())
	{
		pRigid->setLinearVelocity(velocity);
		//		pRigid->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true); // 연속 충돌 감지 활성화 (빠른 거면)
	}
	// 씬에 액터 추가

	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());
	return S_OK;
}

HRESULT CProjectile::Bind_Shader()
{
	return S_OK;
}

void CProjectile::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CProjectile::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CProjectile::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CProjectile::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CProjectile::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CProjectile::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

CProjectile* CProjectile::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CProjectile* pGameInstance = new CProjectile(pDevice, pContext);
	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CProjectile");
		Safe_Release(pGameInstance);
	}
	return pGameInstance;
}

CGameObject* CProjectile::Clone(void* pArg)
{
	CProjectile* pGameInstance = new CProjectile(*this);
	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CProjectile");
		Safe_Release(pGameInstance);
	}
	return pGameInstance;
}

void CProjectile::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pSoundCom);
	Safe_Release(m_pPhysXActorCom);
	if (m_pEffect)
		m_pEffect->End_Effect();

}
