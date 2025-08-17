#include "Buttler_Train.h"
#include "GameInstance.h"
#include "Weapon_Monster.h"
#include "LockOn_Manager.h"
#include "PhysX_IgnoreSelfCallback.h"
#include "Client_Calculation.h"

CButtler_Train::CButtler_Train(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMonster_Base{pDevice, pContext}
{
}

CButtler_Train::CButtler_Train(const CButtler_Train& Prototype)
	:CMonster_Base{Prototype}
{
}

HRESULT CButtler_Train::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CButtler_Train::Initialize(void* pArg)
{
	if(FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Weapon()))
		return E_FAIL;


	m_fDetectDist = 10.f;
	
	m_fHp = 300;

	m_pHPBar->Set_MaxHp(m_fHp);

	// 락온 용
	m_iLockonBoneIndex = m_pModelCom->Find_BoneIndex("Bip001-Spine2");
	m_vRayOffset = { 0.f, 1.8f, 0.f, 0.f };
	
	
	return S_OK;
}

void CButtler_Train::Priority_Update(_float fTimeDelta)
{

	__super::Priority_Update(fTimeDelta);

	if (m_strStateName.find("Dead") != m_strStateName.npos)
	{
		if (m_pAnimator->IsFinished())
		{
			m_pWeapon->Set_bDead();
		}
	}

	
}

void CButtler_Train::Update(_float fTimeDelta)
{

	Calc_Pos(fTimeDelta);




	if (m_strStateName.find("Groggy_Loop") != m_strStateName.npos)
	{
		m_fDuration += fTimeDelta;

		m_pAnimator->SetFloat("GroggyTime", m_fDuration);
	}

	__super::Update(fTimeDelta);

}

void CButtler_Train::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	Update_State();


	
}

HRESULT CButtler_Train::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;
	

	return S_OK;
}

void CButtler_Train::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	

	// 이걸르 무기에 옮겨야됨
	// 무기가 상태마다 한번씩 데미지 주고
	// 이제 초기화하면 다시 데미지 줄 수 있게
	// 수정 해야됨
	// 플레이어 상태 좀 잡히면 다시 
	ReceiveDamage(pOther, eColliderType);

}

void CButtler_Train::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	ReceiveDamage(pOther, eColliderType);
}

void CButtler_Train::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	ReceiveDamage(pOther, eColliderType);
}

void CButtler_Train::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Train::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Train::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Train::Update_State()
{
	 Check_Detect();

	 m_strStateName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;

	 if (!m_isDetect || m_fHp <= 0)
	 {
		 m_strStateName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
		 return;
	 }
		



	_vector vDist = {};
	vDist = m_pTransformCom->Get_State(STATE::POSITION) - m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);

	
	m_pAnimator->SetFloat("Distance", XMVectorGetX(XMVector3Length(vDist)));

    m_strStateName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;


	if (m_strStateName.find("Idle") != m_strStateName.npos)
	{
		//m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_TurnDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));

	}

	if (m_strStateName.find("Run") != m_strStateName.npos || m_strStateName.find("Walk") != m_strStateName.npos)
		m_isLookAt = true;
	else
		m_isLookAt = false;

	if (m_strStateName.find("Attack") != m_strStateName.npos)
	{
		if (m_strStateName.find("Light") != m_strStateName.npos)
		{
			m_pAnimator->SetBool("UseLightAttack", false);
		}
		else
		{
			m_pAnimator->SetBool("UseLightAttack", true);
		}

		
	}

	if (m_iAttackCount == 10)
	{
		// 뒤로 가게 하기
		m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_TurnDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
		m_pAnimator->SetBool("IsBack", true);
		//m_pAnimator->SetInt("Dir", _int(Calc_Ray()));
		
		m_iAttackCount = 0;

		
	}


	if (m_strStateName.find("Groggy_Out") != m_strStateName.npos)
	{
		m_fDuration = 0.f;
		m_fGroggyThreshold = 100;
	}
	

	if (m_strStateName.find("Attack") != m_strStateName.npos)
	{
		if (m_pAnimator->IsFinished())
		{
			++m_iAttackCount;
		}
	}

	if (m_strStateName.find("Walk") != m_strStateName.npos)
	{
		if (m_strStateName.find("F") == m_strStateName.npos)
		{
			if (m_pAnimator->IsFinished())
				m_pAnimator->SetBool("IsBack", false);
		}
	}

}

void CButtler_Train::Attack(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Train::AttackWithWeapon(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Train::ReceiveDamage(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (m_strStateName == "Dead")
		return;

	if (eColliderType == COLLIDERTYPE::PLAYER_WEAPON)
	{
		auto pWeapon = static_cast<CWeapon*>(pOther);

		if (false == pWeapon->GetisAttack())
			return;

		if (pWeapon->Find_CollisonObj(this))
		{
			return;
		}

		pWeapon->Add_CollisonObj(this);
		pWeapon->Calc_Durability(3);

		m_fHp -= pWeapon->Get_CurrentDamage() / 10.f;

		m_fGroggyThreshold -= pWeapon->Get_CurrentDamage();
		m_pHPBar->Set_RenderTime(2.f);

		if (m_fHp <= 0)
		{

			m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
			m_pAnimator->SetTrigger("Dead");
			m_strStateName = "Dead";

			CLockOn_Manager::Get_Instance()->Set_Off(this);
			m_bUseLockon = false;
			return;
		}

		if (!m_isCanGroggy)
		{
			if (m_strStateName.find("KnockBack") != m_strStateName.npos || m_strStateName.find("Groggy") != m_strStateName.npos)
				return;

			m_pAnimator->SetTrigger("Hit");
			m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
			if(m_fGroggyThreshold < 0)
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

void CButtler_Train::Calc_Pos(_float fTimeDelta)
{
	if (m_strStateName.find("Run") != m_strStateName.npos)
	{
		_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
		m_pTransformCom->Go_Dir(vLook, fTimeDelta * 0.5f, nullptr, m_pNaviCom);
	}
	else if (m_strStateName.find("Walk") != m_strStateName.npos)
	{
		_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);

		if (m_strStateName.find("Walk_F") != m_strStateName.npos)
		{
			m_pTransformCom->Go_Dir(vLook, fTimeDelta * 0.25f, nullptr, m_pNaviCom);
		}
		else
		{
			m_isLookAt = false;
			RootMotionActive(fTimeDelta);
		}
	}
	else if (m_strStateName.find("Attack") != m_strStateName.npos || m_strStateName.find("KnockBack") != m_strStateName.npos)
	{
		m_isLookAt = true;
		RootMotionActive(fTimeDelta);
	}

}

CMonster_Base::MONSTER_DIR CButtler_Train::Calc_Ray()
{
	PxVec3 origin = m_pPhysXActorCom->Get_Actor()->getGlobalPose().p;
	origin.y += 2.f;
	XMFLOAT3 fLook;
	XMStoreFloat3(&fLook, m_pTransformCom->Get_State(STATE::LOOK) * -1);
	PxVec3 direction = PxVec3(fLook.x, fLook.y, fLook.z);
	direction.normalize();
	_float fRayLength = 1.f;

	PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
	PxRaycastBuffer hit;
	PxQueryFilterData filterData;
	filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	unordered_set<PxActor*> ignoreActors = m_pPhysXActorCom->Get_IngoreActors();
	CIgnoreSelfCallback callback(ignoreActors);

	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());

	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, fRayLength, hit, hitFlags, filterData, &callback))
	{
		if (hit.hasBlock)
		{
			PxRigidActor* hitActor = hit.block.actor;

			if (hitActor != m_pPhysXActorCom->Get_Actor())
			{
				PxVec3 hitPos = hit.block.position;
				PxVec3 hitNormal = hit.block.normal;

				CPhysXActor* pHitActor = static_cast<CPhysXActor*>(hitActor->userData);

				if (pHitActor)
				{
					if (nullptr == pHitActor->Get_Owner())
						return MONSTER_DIR::B;
					pHitActor->Get_Owner()->On_Hit(this, m_pPhysXActorCom->Get_ColliderType());
				}

				//printf("RayHitPos X: %f, Y: %f, Z: %f\n", hitPos.x, hitPos.y, hitPos.z);
				//printf("RayHitNormal X: %f, Y: %f, Z: %f\n", hitNormal.x, hitNormal.y, hitNormal.z);
				m_bRayHit = true;
				m_vRayHitPos = hitPos;

				return MONSTER_DIR(GetRandomInt(2, 3));
			}

			
			
		}
	}


#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		DEBUGRAY_DATA _data{};
		_data.vStartPos = m_pPhysXActorCom->Get_Actor()->getGlobalPose().p;
		_data.vStartPos.y += 2.f;
		XMFLOAT3 fLook;
		XMStoreFloat3(&fLook, m_pTransformCom->Get_State(STATE::LOOK) * -1.f);
		_data.vDirection = PxVec3(fLook.x, fLook.y, fLook.z);
		_data.fRayLength = 1.f;
		_data.bIsHit = m_bRayHit;
		_data.vHitPos = m_vRayHitPos;
		m_pPhysXActorCom->Add_RenderRay(_data);

		m_bRayHit = false;
		m_vRayHitPos = {};
	}
#endif


	return MONSTER_DIR::B;
}

HRESULT CButtler_Train::Ready_Weapon()
{
	CWeapon_Monster::MONSTER_WEAPON_DESC Desc{};
	Desc.eMeshLevelID = LEVEL::STATIC;
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.InitPos = { 0.125f, 0.f, 0.f };
	Desc.InitScale = { 1.f, 0.6f, 1.f };
	Desc.iRender = 0;

	Desc.szMeshID = TEXT("Buttler_Train_Weapon");
	lstrcpy(Desc.szName, TEXT("Buttler_Train_Weapon"));
	Desc.vAxis = { 0.f,1.f,0.f,0.f };
	Desc.fRotationDegree = {90.f};

	Desc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-R-Hand"));
	Desc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_Weapon"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Monster_Weapon"), &pGameObject, &Desc)))
		return E_FAIL;

	m_pWeapon = dynamic_cast<CWeapon_Monster*>(pGameObject);

	Safe_AddRef(m_pWeapon);


	return S_OK;
}

CButtler_Train* CButtler_Train::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CButtler_Train* pInstance = new CButtler_Train(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CButtler_Train");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CButtler_Train::Clone(void* pArg)
{
	CButtler_Train* pInstance = new CButtler_Train(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CButtler_Train");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CButtler_Train::Free()
{
	__super::Free();

	Safe_Release(m_pWeapon);

}