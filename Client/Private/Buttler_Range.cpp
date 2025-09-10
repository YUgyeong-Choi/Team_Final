#include "Buttler_Range.h"
#include "GameInstance.h"
#include "Weapon_Monster.h"
#include "Player.h"
#include "LockOn_Manager.h" 
#include "PhysX_IgnoreSelfCallback.h"
#include "FireBall.h"
#include "Effect_Manager.h"
#include "EffectContainer.h"

CButtler_Range::CButtler_Range(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CMonster_Base{pDevice, pContext}
{
}

CButtler_Range::CButtler_Range(const CButtler_Range& Prototype)
    :CMonster_Base{Prototype}
{
}

HRESULT CButtler_Range::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CButtler_Range::Initialize(void* pArg)
{
	UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);
	pDesc->fSpeedPerSec = 5.f;
	pDesc->fRotationPerSec = XMConvertToRadians(60.0f);

	m_fHeight = 1.f;
	m_vHalfExtents = { 0.5f,1.f,0.5f };

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Weapon()))
		return E_FAIL;

	m_fDetectDist = 10.f;
	m_fGroggyThreshold = 100;

	m_fHp = 300;

	if (nullptr != m_pHPBar)
		m_pHPBar->Set_MaxHp(m_fHp);

	// 락온 용
	m_iLockonBoneIndex = m_pModelCom->Find_BoneIndex("Bip001-Spine2");
	m_vRayOffset = { 0.f, 1.8f, 0.f, 0.f };

	m_pWeapon->Collider_FilterOff();


	// 서로는 충돌 무시하게

	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
	m_pPhysXActorCom->Add_IngoreActors((m_pWeapon)->Get_PhysXActor()->Get_Actor());
	

	return S_OK;
}

void CButtler_Range::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	auto pCurState = m_pAnimator->Get_CurrentAnimController()->GetCurrentState();
	if (pCurState && pCurState->stateName.find("Dead") != pCurState->stateName.npos)
	{
		m_fEmissive = 0.f;

		if (!m_pAnimator->IsBlending() && m_pAnimator->IsFinished())
		{
			cout << pCurState->stateName << endl;
			//(m_pWeapon)->Set_bDead();
			//Set_bDead();
			m_pGameInstance->Push_WillRemove(L"Layer_Monster_Normal", this);
			m_pWeapon->SetbIsActive(false);
		}
	}

	if (m_fHp <= 0 && !m_bOffCollider)
	{
		m_pWeapon->Collider_FilterOff();
		m_bOffCollider = true;

		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			pPlayer->Get_Controller()->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
		}
		m_pPhysXActorCom->Init_SimulationFilterData();

		static_cast<CPlayer*>(m_pPlayer)->Set_HitTarget(this, true);
	}
}

void CButtler_Range::Update(_float fTimeDelta)
{
	Calc_Pos(fTimeDelta);

	__super::Update(fTimeDelta);


	if (m_strStateName.find("Groggy_Loop") != m_strStateName.npos)
	{
		m_fDuration += fTimeDelta;

		m_pAnimator->SetFloat("GroggyTime", m_fDuration);
	}


	if (m_strStateName.find("Hit") != m_strStateName.npos)
	{
		m_pWeapon->SetisAttack(false);
	}

	if (m_strStateName.find("Getup") != m_strStateName.npos)
	{
		m_isFatal = false;
	}
}

void CButtler_Range::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	Update_State();
}

HRESULT CButtler_Range::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;


	return S_OK;
}

void CButtler_Range::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (eColliderType == COLLIDERTYPE::MONSTER)
	{
		++m_iCollisionCount;
		m_vPushDir -= HitNormal;
	}
	else if (eColliderType == COLLIDERTYPE::PLAYER)
		m_isCollisionPlayer = true;
}

void CButtler_Range::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (eColliderType == COLLIDERTYPE::MONSTER)
	{
		// 계속 충돌중이면 빠져나갈 수 있게 좀 보정을
		_vector vCorrection = HitNormal * 0.01f;
		m_vPushDir -= vCorrection;
	}
}

void CButtler_Range::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (eColliderType == COLLIDERTYPE::MONSTER)
	{
		--m_iCollisionCount;

		if (m_iCollisionCount <= 0)
		{
			m_iCollisionCount = 0;
			m_vPushDir = { 0.f, 0.f, 0.f, 0.f };
		}
	}
	else if (eColliderType == COLLIDERTYPE::PLAYER)
		m_isCollisionPlayer = false;
}

void CButtler_Range::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Range::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	ReceiveDamage(pOther, eColliderType);
}

void CButtler_Range::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Range::Update_State()
{
	Check_Detect();

	m_strStateName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;

	if (m_isFatal)
	{
		m_pAnimator->SetFloat("Hp", m_fHp);
	}

	if (!m_isDetect || m_fHp <= 0)
	{
		m_strStateName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
		return;
	}




	_vector vDist = {};
	vDist = m_pTransformCom->Get_State(STATE::POSITION) - m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);


	//m_pAnimator->SetFloat("Distance", XMVectorGetX(XMVector3Length(vDist)));

	m_strStateName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;




	if (m_strStateName.find("Idle") != m_strStateName.npos || m_strStateName.find("Attack") != m_strStateName.npos)
	{
	
		m_isLookAt = true;
		
		RayCast(m_pPhysXActorCom);

		if (m_bRayHit)
		{
			m_pAnimator->SetBool("IsAttack", true);
		}
		else
		{
			m_pAnimator->SetBool("IsAttack", false);
		}
	}
	else
	{
		m_isLookAt = false;
	}

	if (m_iAttackCount == 3)
	{
		// 뒤로 가게 하기
		//m_pAnimator->SetInt("Dir", ENUM_CLASS(MONSTER_DIR::B));
		m_pAnimator->SetBool("IsBack", true);

		m_iAttackCount = 0;


	}


	if (m_strStateName.find("Groggy_Out") != m_strStateName.npos)
	{
		m_fDuration = 0.f;
		m_fGroggyThreshold = 100;
	}

	if (m_strStateName.find("Groggy_Loop") != m_strStateName.npos)
	{
		m_isGroogyLoop = true;
	}
}

void CButtler_Range::Attack(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Range::AttackWithWeapon(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Range::ReceiveDamage(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (m_strStateName == "Dead")
		return;

	if (eColliderType == COLLIDERTYPE::PLAYER_WEAPON)
	{
		auto pWeapon = static_cast<CWeapon*>(pOther);


		if (pWeapon->Find_CollisonObj(this, eColliderType))
		{
			return;
		}

		//m_pWeapon->SetisAttack(false);

		pWeapon->Add_CollisonObj(this);
		pWeapon->Calc_Durability(3.f);

		m_fHp -= pWeapon->Get_CurrentDamage();

		m_pHPBar->Add_Damage(pWeapon->Get_CurrentDamage());

		m_fGroggyThreshold -= pWeapon->Get_CurrentDamage() / 10.f;

		static_cast<CPlayer*>(m_pPlayer)->Add_Mana(10.f);

		if (nullptr != m_pHPBar)
			m_pHPBar->Set_RenderTime(2.f);

		m_isDetect = true;

		if (m_fHp <= 0 && !m_isFatal)
		{

			m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
			m_pAnimator->SetTrigger("Dead");
			m_strStateName = "Dead";

			CLockOn_Manager::Get_Instance()->Set_Off(this);
			m_bUseLockon = false;

			if (nullptr != m_pHPBar)
				m_pHPBar->Set_RenderTime(0.f);
			return;
		}
		else if (m_fHp <= 0 && m_isFatal)
		{
			CLockOn_Manager::Get_Instance()->Set_Off(nullptr);
			m_bUseLockon = false;


			return;
		}

		if (static_cast<CPlayer*>(m_pPlayer)->GetAnimCategory() == CPlayer::eAnimCategory::ARM_ATTACKCHARGE)
		{
			m_isLookAt = false;
			m_pAnimator->SetTrigger("KnockBack");
			m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));


			m_vKnockBackDir = m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK);

			XMVector3Normalize(m_vKnockBackDir);

			return;
		}



		if (!m_isCanGroggy)
		{
			if (m_strStateName.find("KnockBack") != m_strStateName.npos || m_strStateName.find("Groggy") != m_strStateName.npos ||
				m_strStateName.find("Fatal") != m_strStateName.npos || m_strStateName.find("Down") != m_strStateName.npos)
				return;

			if (m_strStateName.find("Hit") != m_strStateName.npos)
			{

				m_pAnimator->Get_CurrentAnimController()->SetState(m_strStateName);

			}
			else
			{
				m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
				m_pAnimator->SetTrigger("Hit");
			}




			if (m_fGroggyThreshold <= 0)
				m_isCanGroggy = true;
		}
		else
		{
			if (m_strStateName.find("KnockBack") == m_strStateName.npos && m_strStateName.find("Groggy") == m_strStateName.npos)
			{
				m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
				m_pAnimator->SetTrigger("Groggy");
				m_isCanGroggy = false;
			}

		}
	}
}

void CButtler_Range::Calc_Pos(_float fTimeDelta)
{
	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);

	if (m_strStateName.find("Fatal") != m_strStateName.npos || 
		m_strStateName.find("Down") != m_strStateName.npos || 
		m_strStateName.find("KnockBack") != m_strStateName.npos ||
		m_strStateName.find("Hit") != m_strStateName.npos)
	{
		m_isLookAt = false;
		m_isCollisionPlayer = false;
	}





	if (m_strStateName.find("Down") == m_strStateName.npos && m_strStateName.find("KnockBack") == m_strStateName.npos && m_strStateName.find("Hit") == m_strStateName.npos)
	{
		m_fHitSpeed = 1.f;
		m_fAwaySpeed = 1.f;
		RootMotionActive(fTimeDelta);

		return;
	}




	if (m_strStateName.find("Down") != m_strStateName.npos)
	{
		m_fAwaySpeed -= fTimeDelta * 0.5f;

		if (m_fAwaySpeed <= 0.f)
			m_fAwaySpeed = 0.f;



		m_pTransformCom->Go_Dir(vLook, fTimeDelta * m_fAwaySpeed, nullptr, m_pNaviCom);
	}
	else if (m_strStateName.find("KnockBack") != m_strStateName.npos)
	{
		m_fAwaySpeed -= fTimeDelta * 0.5f;

		if (m_fAwaySpeed <= 0.f)
			m_fAwaySpeed = 0.f;

		RootMotionActive(fTimeDelta);

		m_pTransformCom->Go_Dir(m_vKnockBackDir, fTimeDelta * m_fAwaySpeed * 0.5f, nullptr, m_pNaviCom);
	}
	else if (m_strStateName.find("Hit") != m_strStateName.npos)
	{
		m_fHitSpeed -= fTimeDelta ;

		if (m_fHitSpeed <= 0.f)
			m_fHitSpeed = 0.f;

		if (m_strStateName.find("B") == m_strStateName.npos)
			vLook *= -1.f;



		m_pTransformCom->Go_Dir(vLook, fTimeDelta * m_fHitSpeed * 0.7f , nullptr, m_pNaviCom);
	}
	

}

void CButtler_Range::Register_Events()
{
	__super::Register_Events();

	m_pAnimator->RegisterEventListener("AddAttackCount", [this]() {

		++m_iAttackCount;

		// 투사체 만들어서 쏘는거 추가하기

		const _float* vWeaponPos = m_pWeapon->Get_CombinedWorldMatrix()->m[3] ;

		_vector vPos = { vWeaponPos[0], vWeaponPos[1], vWeaponPos[2], vWeaponPos[3] };
		vPos -= m_vRayOffset * 0.5f;
		CProjectile::PROJECTILE_DESC desc{};
		_int iLevelIndex = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);

		_vector vDir = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) + static_cast<CUnit*>(m_pPlayer)->Get_RayOffset() * 0.3f - vPos;

		// XZ 방향 추출 
		_vector dirXZ = XMVectorSet(vDir.m128_f32[0], 0.f, vDir.m128_f32[2], 0.f);
		dirXZ = XMVector3Normalize(dirXZ);

		// Look의 XZ
		_vector lookXZ = m_pTransformCom->Get_State(STATE::LOOK);
		lookXZ = XMVectorSet(lookXZ.m128_f32[0], 0.f, lookXZ.m128_f32[2], 0.f);
		lookXZ = XMVector3Normalize(lookXZ);

		// 각도 구하기
		float dot = XMVectorGetX(XMVector3Dot(dirXZ, lookXZ));
		dot = max(-1.f, min(1.f, dot)); // 안전하게 clamp
		float angle = acosf(dot);

		// 회전 방향 결정 
		float crossY = XMVectorGetY(XMVector3Cross(dirXZ, lookXZ));
		if (crossY < 0.f) angle = -angle;

		// Y축 회전 행렬
		XMMATRIX rot = XMMatrixRotationY(angle);

		// vDir 전체를 회전
		_vector vRotated = XMVector3TransformNormal(vDir, rot);


		vRotated = XMVector3Normalize(vRotated);

		desc.bUseDistTrigger = false;
		desc.bUseTimeTrigger = false;
		desc.fGravityOnDist = 0.f;
		desc.fLifeTime = 5.f;
		desc.fRadius = 0.1f;
		desc.fRotationPerSec = 0.f;
		desc.fSpeed = 2.3f;
		desc.fSpeedPerSec = 5.f;
		desc.iLevelID = iLevelIndex;
		lstrcpy(desc.szName, TEXT("Bullet"));
		desc.vDir = { vRotated.m128_f32[0], vRotated.m128_f32[1], vRotated.m128_f32[2] };
		desc.vPos = { vPos.m128_f32[0], vPos.m128_f32[1], vPos.m128_f32[2] };

		if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_Bullet"), iLevelIndex, TEXT("Layer_Projectile_Normal"), &desc)))
		{
			return;
		}

		CEffectContainer::DESC Effdesc = {};
		XMStoreFloat4x4(&Effdesc.PresetMatrix, XMMatrixTranslation(vWeaponPos[0], vWeaponPos[1], vWeaponPos[2]));
		if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_ButlerRange_Muzzle_P3"), &Effdesc))
			MSG_BOX("이펙트 생성 실패함");

		});

	m_pAnimator->RegisterEventListener("BackMoveEnd", [this]() {

		m_pAnimator->SetBool("IsBack", false);

		});


}

void CButtler_Range::Start_Fatal_Reaction()
{
	m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
	m_pAnimator->SetTrigger("Fatal");

	m_isFatal = true;
	m_pWeapon->SetisAttack(false);
	m_pWeapon->Clear_CollisionObj();
}

void CButtler_Range::Reset()
{
	m_fHp = 300;

	if (nullptr != m_pHPBar)
		m_pHPBar->Set_MaxHp(m_fHp);

	m_iAttackCount = {};
	m_fDuration = 0.f;
	m_fAwaySpeed = 1.f;
	m_fGroggyThreshold = 100;

	auto stEntry = m_pAnimator->Get_CurrentAnimController()->GetEntryState();
	m_pAnimator->Get_CurrentAnimController()->SetState(stEntry->stateName);

	m_pWeapon->SetbIsActive(true);

	__super::Reset();

	if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
	{
		pPlayer->Get_Controller()->Remove_IgnoreActors(m_pPhysXActorCom->Get_Actor());
	}
	m_pPhysXActorCom->Set_SimulationFilterData(m_pPhysXActorCom->Get_FilterData());

	m_isFatal = false;
}

void CButtler_Range::RayCast(CPhysXActor* actor)
{

	_vector vDir = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) - static_cast<CUnit*>(m_pPlayer)->Get_RayOffset() * 0.5f - m_pTransformCom->Get_State(STATE::POSITION);

	// XZ 방향 추출 
	_vector dirXZ = XMVectorSet(vDir.m128_f32[0], 0.f, vDir.m128_f32[2], 0.f);
	dirXZ = XMVector3Normalize(dirXZ);

	// Look의 XZ
	_vector lookXZ = m_pTransformCom->Get_State(STATE::LOOK);
	lookXZ = XMVectorSet(lookXZ.m128_f32[0], 0.f, lookXZ.m128_f32[2], 0.f);
	lookXZ = XMVector3Normalize(lookXZ);

	// 각도 구하기
	float dot = XMVectorGetX(XMVector3Dot(dirXZ, lookXZ));
	dot = max(-1.f, min(1.f, dot)); // 안전하게 clamp
	float angle = acosf(dot);

	// 회전 방향 결정 
	float crossY = XMVectorGetY(XMVector3Cross(dirXZ, lookXZ));
	if (crossY < 0.f) angle = -angle;

	// Y축 회전 행렬
	XMMATRIX rot = XMMatrixRotationY(angle);

	// vDir 전체를 회전
	_vector vRotated = XMVector3TransformNormal(vDir, rot);

	_vector vOffset = m_vRayOffset * 0.5f;

	PxVec3 origin = actor->Get_Actor()->getGlobalPose().p +VectorToPxVec3(vOffset);
	XMFLOAT3 fLook;
	XMStoreFloat3(&fLook, vDir);
	PxVec3 direction = PxVec3(fLook.x, fLook.y, fLook.z);
	direction.normalize();
	_float fRayLength = 10.f;

	PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
	PxRaycastBuffer hit;
	PxQueryFilterData filterData;
	filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	unordered_set<PxActor*> ignoreActors = actor->Get_IngoreActors();
	CIgnoreSelfCallback callback(ignoreActors);

	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, fRayLength, hit, hitFlags, filterData, &callback))
	{
		if (hit.hasBlock)
		{
			PxRigidActor* hitActor = hit.block.actor;

			if (hitActor == actor->Get_Actor())
			{
				printf(" Ray hit myself  skipping\n");
				return;
			}
			PxVec3 hitPos = hit.block.position;
			PxVec3 hitNormal = hit.block.normal;

			CPhysXActor* pHitActor = static_cast<CPhysXActor*>(hitActor->userData);

			if (pHitActor)
			{
				if (nullptr == pHitActor->Get_Owner())
					return;
				pHitActor->Get_Owner()->On_Hit(this, actor->Get_ColliderType());

				if (COLLIDERTYPE::PLAYER == pHitActor->Get_ColliderType() || COLLIDERTYPE::PLAYER_WEAPON == pHitActor->Get_ColliderType())
				{
					m_bRayHit = true;
					m_vRayHitPos = hitPos;
				}
				else
				{
					m_bRayHit = false;
					m_vRayHitPos = {};
				}
			}

		
			
			

			
		}
	}

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		DEBUGRAY_DATA _data{};
		_data.vStartPos = actor->Get_Actor()->getGlobalPose().p + VectorToPxVec3(vOffset);
		XMFLOAT3 fLook;
		XMStoreFloat3(&fLook, vDir);
		_data.vDirection = PxVec3(fLook.x, fLook.y, fLook.z);
		_data.fRayLength = 10.f;
		_data.bIsHit = m_bRayHit;
		_data.vHitPos = m_vRayHitPos;
		actor->Add_RenderRay(_data);

		//m_bRayHit = false;
		//m_vRayHitPos = {};
	}
#endif
}

HRESULT CButtler_Range::Ready_Weapon()
{
	CWeapon_Monster::MONSTER_WEAPON_DESC Desc{};
	Desc.eMeshLevelID = LEVEL::STATIC;
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.InitPos = { 0.125f, 0.f, 0.f };
	Desc.InitScale = { 1.05f, 0.84f, 1.05f };
	Desc.iRender = 0;

	Desc.szMeshID = TEXT("Buttler_Range_Weapon");
	lstrcpy(Desc.szName, TEXT("Buttler_Range_Weapon"));
	Desc.vAxis = { 1.f,0.f,0.2f,0.f };
	Desc.fRotationDegree = { 180.f };
	//Desc.vLocalOffset = { -0.5f,0.f,0.f,1.f };
	Desc.vPhsyxExtent = { 0.8f, 0.2f, 0.2f };

	Desc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-R-Hand"));
	Desc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	Desc.pOwner = this;

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_Weapon"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Monster_Weapon"), &pGameObject, &Desc)))
		return E_FAIL;

	m_pWeapon = dynamic_cast<CWeapon_Monster*>(pGameObject);

	Safe_AddRef(m_pWeapon);


	return S_OK;
}

CButtler_Range* CButtler_Range::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CButtler_Range* pInstance = new CButtler_Range(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CButtler_Train");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CButtler_Range::Clone(void* pArg)
{
	CButtler_Range* pInstance = new CButtler_Range(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CButtler_Basic");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CButtler_Range::Free()
{
	__super::Free();

	Safe_Release(m_pWeapon);

}