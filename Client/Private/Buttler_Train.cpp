#include "Buttler_Train.h"
#include "GameInstance.h"
#include "Weapon_Monster.h"
#include "LockOn_Manager.h"
#include "PhysX_IgnoreSelfCallback.h"
#include "Client_Calculation.h"
#include <Player.h>

CButtler_Train::CButtler_Train(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMonster_Base{ pDevice, pContext }
{
}

CButtler_Train::CButtler_Train(const CButtler_Train& Prototype)
	:CMonster_Base(Prototype)
{
}

HRESULT CButtler_Train::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CButtler_Train::Initialize(void* pArg)
{
	/* [ 데미지 설정 ] */
	

	UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);
	pDesc->fSpeedPerSec = 5.f;
	pDesc->fRotationPerSec = XMConvertToRadians(180.0f);

	m_fHeight = 1.f;
	m_vHalfExtents = { 0.5f, 1.f, 0.5f };

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;



	if (FAILED(Ready_Weapon()))
		return E_FAIL;

	m_fDetectDist = 10.f;
	m_fGroggyThreshold = 100;

	m_fHp = 500;

	if (nullptr != m_pHPBar)
		m_pHPBar->Set_MaxHp(m_fHp);

	// 락온 용
	m_iLockonBoneIndex = m_pModelCom->Find_BoneIndex("Bip001-Spine2");
	m_vRayOffset = { 0.f, 1.8f, 0.f, 0.f };

	m_pWeapon->Collider_FilterOff();

	if (m_eSpawnType == SPAWN_TYPE::PATROL)
	{
		m_pAnimator->SetTrigger("SpawnPatrol");
		m_isPatrol = true;

	}

	m_pWeapon->Set_WeaponTrail_Active(false);

	m_pSoundCom->StopAll();
	
	return S_OK; 
}

void CButtler_Train::Priority_Update(_float fTimeDelta)
{

	__super::Priority_Update(fTimeDelta);

	auto pCurState = m_pAnimator->Get_CurrentAnimController()->GetCurrentState();
	if (pCurState && pCurState->stateName.find("Dead") != pCurState->stateName.npos)
	{
		m_fEmissive = 0.f;
		m_pWeapon->SetbIsActive(false);

		if (!m_pAnimator->IsBlending() && m_pAnimator->IsFinished())
		{
			cout << pCurState->stateName << endl;
			//(m_pWeapon)->Set_bDead();
			//Set_bDead();
			m_pGameInstance->Push_WillRemove(L"Layer_Monster_Normal", this, false);
			

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
		m_pWeapon->Set_WeaponTrail_Active(false);
	}
}

void CButtler_Train::Update(_float fTimeDelta)
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
		m_pWeapon->Set_WeaponTrail_Active(false);
	}

	if (m_strStateName.find("Getup") != m_strStateName.npos)
	{
		m_isFatal = false;
	}

	if (m_isPatrol)
	{
		m_fPatrolTime += fTimeDelta;

		if (m_fPatrolTime > 7.5f)
		{
			m_pAnimator->SetBool("Turn", true);
			m_fPatrolTime = 0.f;
		}
	
	}

	
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

void CButtler_Train::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (eColliderType == COLLIDERTYPE::MONSTER)
	{
		++m_iCollisionCount;

		XMStoreFloat4(&m_vPushDir, XMVectorSubtract(XMLoadFloat4(&m_vPushDir) , HitNormal));
		m_vPushDir.w = 0.f;
		
	}
	else if (eColliderType == COLLIDERTYPE::PLAYER)
		m_isCollisionPlayer = true;



}

void CButtler_Train::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (eColliderType == COLLIDERTYPE::MONSTER)
	{
		// 계속 충돌중이면 빠져나갈 수 있게 좀 보정을
		_vector vCorrection = HitNormal * 0.01f;

		XMStoreFloat4(&m_vPushDir, XMVectorSubtract(XMLoadFloat4(&m_vPushDir), vCorrection));
		m_vPushDir.w = 0.f;

	}
	

	//ReceiveDamage(pOther, eColliderType);
}

void CButtler_Train::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
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

void CButtler_Train::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Train::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
 	ReceiveDamage(pOther, eColliderType);
}

void CButtler_Train::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Train::Update_State()
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


	m_pAnimator->SetFloat("Distance", XMVectorGetX(XMVector3Length(vDist)));

	m_strStateName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;




	if (m_strStateName.find("Idle") != m_strStateName.npos)
	{
		//m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_TurnDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));

	}


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

	if (m_iAttackCount == 3)
	{
		// 뒤로 가게 하기

		if (XMVectorGetX(XMVector3Length(vDist)) < 1.f)
			m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_TurnDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
		else
			m_pAnimator->SetInt("Dir", ENUM_CLASS(MONSTER_DIR::B));
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


		if (pWeapon->Find_CollisonObj(this, eColliderType))
		{
			return;
		}

		//m_pWeapon->SetisAttack(false);

		pWeapon->Add_CollisonObj(this);
		pWeapon->Calc_Durability(3.f);

		m_fHp -= pWeapon->Get_CurrentDamage() ;

		m_pHPBar->Add_Damage(pWeapon->Get_CurrentDamage() );

		m_fGroggyThreshold -= pWeapon->Get_CurrentDamage() / 10.f;

		if (nullptr != m_pHPBar)
			m_pHPBar->Set_RenderTime(2.f);

		static_cast<CPlayer*>(m_pPlayer)->Add_Mana(10.f);

		m_isDetect = true;


		if (m_fHp <= 0 && !m_isFatal)
		{
			m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
			m_pAnimator->SetTrigger("Dead");
			m_strStateName = "Dead";

			CLockOn_Manager::Get_Instance()->Set_Off(this);

			m_pWeapon->Set_WeaponTrail_Active(false);
			m_bUseLockon = false;

			if (nullptr != m_pHPBar)
				m_pHPBar->Set_RenderTime(0.f);
			return;
		}
		else if (m_fHp <= 0 && m_isFatal)
		{
			CLockOn_Manager::Get_Instance()->Set_Off(nullptr);
			m_bUseLockon = false;
			m_pWeapon->Set_WeaponTrail_Active(false);
			
			return;
		}

		if (m_fHp > 0 && m_isFatal)
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->SetVolume("SE_NPC_Servant02_MT_Dmg_00", 1.f);
				m_pSoundCom->Play("SE_NPC_Servant02_MT_Dmg_00");

			}
		}

		if (static_cast<CPlayer*>(m_pPlayer)->GetAnimCategory() == CPlayer::eAnimCategory::ARM_ATTACKCHARGE)
		{
			m_isLookAt = false;
			m_pAnimator->SetTrigger("KnockBack");
			m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));



			XMStoreFloat4(&m_vKnockBackDir, XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)));

			

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

void CButtler_Train::Calc_Pos(_float fTimeDelta)
{
	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);

	if (m_strStateName.find("Run") != m_strStateName.npos || m_strStateName.find("Walk_F") != m_strStateName.npos)
	{
		_float fSpeed = { 1.f };
		if (m_strStateName.find("Walk_F") != m_strStateName.npos)
		{
			fSpeed = 0.5f;
		}

		if(m_strStateName.find("Patrol") != m_strStateName.npos)
			fSpeed = 0.25f;

		_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
		

		XMStoreFloat4(&m_vPushDir, XMVector3Normalize(XMLoadFloat4(&m_vPushDir)));

		m_vPushDir.z = 0.f;
		
		_vector vDir = XMVector3Normalize(vLook)  + XMLoadFloat4(&m_vPushDir) ;


		m_pTransformCom->Go_Dir(vDir, fTimeDelta * fSpeed, nullptr, m_pNaviCom);
		

	}
	else
	{
		if (m_strStateName.find("Fatal") != m_strStateName.npos ||
			m_strStateName.find("Down") != m_strStateName.npos ||
			m_strStateName.find("KnockBack") != m_strStateName.npos ||
			m_strStateName.find("Hit") != m_strStateName.npos)
		{
			m_isLookAt = false;
			m_isCollisionPlayer = false;
		}

		if (m_strStateName.find("Turn") != m_strStateName.npos)
		{
			_vector vAxis = {0.f,1.f,0.f,0.f};
			m_isLookAt = false;
			m_pTransformCom->Rotate_Special(fTimeDelta, 1.f, vAxis, 180.f);
			m_pAnimator->SetBool("Turn", false);
			return;
		}


		if (m_strStateName.find("Away") == m_strStateName.npos && m_strStateName.find("KnockBack") == m_strStateName.npos)
		{
			m_fAwaySpeed = 1.f;
			m_fKnockBackSpeed = 5.f;
			RootMotionActive(fTimeDelta);

			return;
		}




		if (m_strStateName.find("Away") != m_strStateName.npos)
		{
			m_fAwaySpeed -= fTimeDelta * 0.5f;
			
			if (m_fAwaySpeed <= 0.f)
				m_fAwaySpeed = 0.f;


			if (m_strStateName.find("B") == m_strStateName.npos)
			{
				vLook *= -1.f;

			}




			m_pTransformCom->Go_Dir(vLook, fTimeDelta * m_fAwaySpeed, nullptr, m_pNaviCom);
		}
		else if (m_strStateName.find("KnockBack") != m_strStateName.npos)
		{
			m_fKnockBackSpeed -= fTimeDelta * 10.f;

			if (m_fKnockBackSpeed <= 0.f)
				m_fKnockBackSpeed = 0.f;

			RootMotionActive(fTimeDelta);

			m_pTransformCom->Go_Dir(XMLoadFloat4(&m_vKnockBackDir), fTimeDelta * m_fKnockBackSpeed * 0.5f, nullptr, m_pNaviCom);
		}

	}
}



	



void CButtler_Train::Register_Events()
{
	__super::Register_Events();

	m_pAnimator->RegisterEventListener("AddAttackCount", [this]() {

		++m_iAttackCount;

		});

	m_pAnimator->RegisterEventListener("BackMoveEnd", [this]() {

		m_pAnimator->SetBool("IsBack", false);

		});

	m_pAnimator->RegisterEventListener("AttackOn", [this]() {

		m_pWeapon->SetisAttack(true);
		m_pWeapon->Clear_CollisionObj();
		m_pWeapon->Set_WeaponTrail_Active(true);
		});

	m_pAnimator->RegisterEventListener("AttackOff", [this]() {

		m_pWeapon->SetisAttack(false);
		m_pWeapon->Clear_CollisionObj();
		m_pWeapon->Set_WeaponTrail_Active(false);
		});


}

void CButtler_Train::Register_SoundEvent()
{
	m_pAnimator->RegisterEventListener("WalkSound", [this]()
		{
			if (m_pSoundCom && m_bSoundCheck)
			{
				m_pSoundCom->Stop("SE_NPC_Servant02_MT_Movement_04");
				m_pSoundCom->Play("SE_NPC_Servant02_MT_Movement_04");
			}
		});

	m_pAnimator->RegisterEventListener("HitSound", [this]()
		{
			if (m_pSoundCom && m_bSoundCheck)
			{
				m_pSoundCom->SetVolume("SE_NPC_Servant02_MT_Dmg_00", 1.f);
				m_pSoundCom->Stop("SE_NPC_Servant02_MT_Dmg_00");
				m_pSoundCom->Play("SE_NPC_Servant02_MT_Dmg_00");
			}
		});

	m_pAnimator->RegisterEventListener("KnockBackSound", [this]()
		{
			if (m_pSoundCom && m_bSoundCheck)
			{
				m_pSoundCom->Play("SE_NPC_SK_GetHit_ToughSpecialHit_Heartbeat_01");
			}
		});

	m_pAnimator->RegisterEventListener("IdleSound", [this]()
		{
			if (m_pSoundCom && m_bSoundCheck)
			{
				m_pSoundCom->Stop("SE_NPC_Servant02_MT_Dmg_00");
				m_pSoundCom->Play("SE_NPC_Servant02_MT_Dmg_00");
			}
		});

	m_pAnimator->RegisterEventListener("GetupSound", [this]()
		{
			if (m_pSoundCom && m_bSoundCheck)
			{
				_int iNum = _int(floorf(m_pGameInstance->Compute_Random(0.f, 3.9f)));

				m_pSoundCom->Play("SE_NPC_Servant02_MT_Getup_0" + to_string(iNum));
			}
		});

	m_pAnimator->RegisterEventListener("DeadSound", [this]()
		{
			if (m_pSoundCom && m_bSoundCheck)
			{
				_int iNum = _int(floorf(m_pGameInstance->Compute_Random(0.f, 8.9f)));

				string strTag = "VO_NPC_NHM_Servant02_Dead_0" + to_string(iNum);
			
				m_pSoundCom->Play(strTag);
			}
		});

	m_pAnimator->RegisterEventListener("AttackSound", [this]()
		{
			if (m_pSoundCom && m_bSoundCheck)
			{
				_int iNum = _int(floorf(m_pGameInstance->Compute_Random(0.f, 17.9f)));

				string strTag = "VO_NPC_NHM_Servant02_Attack_" + to_string(iNum);

				m_pSoundCom->Play(strTag);
			}
		});


	m_pAnimator->RegisterEventListener("FallSound", [this]()
		{
			if (m_pSoundCom && m_bSoundCheck)
			{
				_int iNum = _int(floorf(m_pGameInstance->Compute_Random(0.f, 2.9f)));

				string strTag = "SE_NPC_Servant02_MT_Bodyfall_0" + to_string(iNum);

				m_pSoundCom->Play(strTag);
			}
		});

}

void CButtler_Train::Block_Reaction()
{
	m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
	m_pAnimator->SetTrigger("Hit");
	m_pWeapon->Set_WeaponTrail_Active(false);
}

void CButtler_Train::Start_Fatal_Reaction()
{
	m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
	m_pAnimator->SetTrigger("Fatal");

	m_isFatal = true;
	m_pWeapon->SetisAttack(false);
	m_pWeapon->Clear_CollisionObj();
	m_pWeapon->Set_WeaponTrail_Active(false);
}

void CButtler_Train::Reset()
{
	m_fHp = 450;

	if (nullptr != m_pHPBar)
	{
		m_pHPBar->Set_MaxHp(m_fHp);
		m_pHPBar->Reset();
	}
	
	/*if (m_eSpawnType == SPAWN_TYPE::STAND)
	{
		m_pAnimator->SetTrigger("SpawnStand");
	}*/

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

	if (m_eSpawnType == SPAWN_TYPE::PATROL)
	{
		m_pAnimator->SetTrigger("SpawnPatrol");
		m_isPatrol = true;

	}

	m_pWeapon->SetDamage(40.f);
	m_pWeapon->Set_WeaponTrail_Active(false);
}

void CButtler_Train::PlayDetectSound()
{
	_int iNum = _int(floorf(m_pGameInstance->Compute_Random(0.f, 2.9f)));
	
	m_pSoundCom->Play("VO_NPC_NHM_Servant02_Spawn_0" + to_string(iNum));
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
	Desc.fRotationDegree = { 90.f };
	Desc.vLocalOffset = { -0.0f,0.f,0.f,1.f };
	Desc.vPhsyxExtent = { 0.75f, 0.2f, 0.2f };

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

HRESULT CButtler_Train::Ready_Sound()
{
	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_Buttler"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
		return E_FAIL;


	

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