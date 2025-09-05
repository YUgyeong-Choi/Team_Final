#include "WatchDog.h"
#include "GameInstance.h"
#include "Player.h"
#include "LockOn_Manager.h"
#include "Weapon.h"

CWatchDog::CWatchDog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CMonster_Base{pDevice, pContext}
{
}

CWatchDog::CWatchDog(const CWatchDog& Prototype)
    :CMonster_Base{Prototype}
{
}

HRESULT CWatchDog::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CWatchDog::Initialize(void* pArg)
{
	UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);
	pDesc->fSpeedPerSec = 5.f;
	pDesc->fRotationPerSec = XMConvertToRadians(180.0f);

	m_fHeight = 1.f;
	m_vHalfExtents = { 0.5f, 0.5f, 0.5f };

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
	m_vRayOffset = { 0.f, 0.9f, 0.f, 0.f };



	return S_OK;
}

void CWatchDog::Priority_Update(_float fTimeDelta)
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
			
		}
	}

	if (m_fHp <= 0 && !m_bOffCollider)
	{
		
		m_bOffCollider = true;

		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			pPlayer->Get_Controller()->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
		}
		m_pPhysXActorCom->Init_SimulationFilterData();

		static_cast<CPlayer*>(m_pPlayer)->Set_HitTarget(this, true);
	}
}

void CWatchDog::Update(_float fTimeDelta)
{
	Calc_Pos(fTimeDelta);

	__super::Update(fTimeDelta);


	if (m_strStateName.find("Groggy_Loop") != m_strStateName.npos)
	{
		m_fDuration += fTimeDelta;

		m_pAnimator->SetFloat("GroggyTime", m_fDuration);
	}


}

void CWatchDog::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	Update_State();
}

HRESULT CWatchDog::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;


	return S_OK;
}

void CWatchDog::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (eColliderType == COLLIDERTYPE::MONSTER)
	{
		++m_iCollisionCount;
		m_vPushDir -= HitNormal;
	}
	else if (eColliderType == COLLIDERTYPE::PLAYER)
		m_isCollisionPlayer = true;



}

void CWatchDog::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (eColliderType == COLLIDERTYPE::MONSTER)
	{
		// 계속 충돌중이면 빠져나갈 수 있게 좀 보정을
		_vector vCorrection = HitNormal * 0.01f;
		m_vPushDir -= vCorrection;
	}


	//ReceiveDamage(pOther, eColliderType);
}

void CWatchDog::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
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

void CWatchDog::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CWatchDog::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	ReceiveDamage(pOther, eColliderType);
}

void CWatchDog::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}


void CWatchDog::Update_State()
{
	Check_Detect();

	m_strStateName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;

	if (m_isDetect)
	{
		int a = 10;
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
		m_fAwaySpeed = 3.f;

		m_isLookAt = true;
		m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_TurnDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));

	}


	if (m_strStateName.find("End") != m_strStateName.npos)
	{
		m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_TurnDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
	}
	

	if (m_iAttackCount == 4)
	{
		// 콤보 어택 bool  켜주기


		m_pAnimator->SetBool("IsComboAttack", true);

		m_iAttackCount = 0;

	}
}

void CWatchDog::Attack(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CWatchDog::AttackWithWeapon(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CWatchDog::ReceiveDamage(CGameObject* pOther, COLLIDERTYPE eColliderType)
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

		m_fHp -= pWeapon->Get_CurrentDamage();

		m_pHPBar->Add_Damage(pWeapon->Get_CurrentDamage());

		if (nullptr != m_pHPBar)
			m_pHPBar->Set_RenderTime(2.f);

		m_isDetect = true;

		if (m_fHp <= 0)
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
		

		if (static_cast<CPlayer*>(m_pPlayer)->GetAnimCategory() == CPlayer::eAnimCategory::ARM_ATTACKCHARGE)
		{
			m_isLookAt = false;
			m_pAnimator->SetTrigger("KnockBack");
			m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));


			m_vKnockBackDir = m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK);

			XMVector3Normalize(m_vKnockBackDir);

			return;
		}

		
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




		
	

	}
}

void CWatchDog::Calc_Pos(_float fTimeDelta)
{
	// 조건은 분기해 가면서

	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);

	if (m_strStateName.find("Hit") != m_strStateName.npos || m_strStateName.find("KnockBack") != m_strStateName.npos)
	{
		m_isLookAt = false;
		m_isCollisionPlayer = false;
	}

	if (m_strStateName.find("Run") != m_strStateName.npos )
	{
		_float fSpeed = { 1.f };

		_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

		m_vPushDir = XMVector3Normalize(m_vPushDir);

		m_vPushDir.m128_f32[3] = 0.f;

		_vector vDir = XMVector3Normalize(vLook) + m_vPushDir;


		m_pTransformCom->Go_Dir(vDir, fTimeDelta * fSpeed, nullptr, m_pNaviCom);

		
	}
	else if (m_strStateName.find("KnockBack") != m_strStateName.npos)
	{
		m_fAwaySpeed -= fTimeDelta * 3.f;

		if (m_fAwaySpeed <= 0.f)
			m_fAwaySpeed = 0.f;

		//RootMotionActive(fTimeDelta);

		m_pTransformCom->Go_Dir(m_vKnockBackDir, fTimeDelta * m_fAwaySpeed * 0.5f, nullptr, m_pNaviCom);
	}
	else if (m_strStateName.find("Jump") != m_strStateName.npos)
	{
		m_isCollisionPlayer = false;

		RootMotionActive(fTimeDelta);
	}
	else
	{
		RootMotionActive(fTimeDelta);
	}


	
}

void CWatchDog::Register_Events()
{
	
	m_pAnimator->RegisterEventListener("UpdateJumpCount", [this]() {

		m_iJumpConut = (++m_iAttackCount) % 4;

		m_pAnimator->SetInt("JumpCount", m_iJumpConut);

		});

	m_pAnimator->RegisterEventListener("SubJumpCount", [this]() {

		if (m_iJumpConut > 0)
		{
			--m_iJumpConut;

			m_pAnimator->SetInt("JumpCount", m_iJumpConut);
		}
			

		});

	m_pAnimator->RegisterEventListener("ComboEnd", [this]() {

		m_pAnimator->SetBool("IsComboAttack", false);

		});



	m_pAnimator->RegisterEventListener("NotLookAt", [this]() {

		m_isLookAt = false;

		});

	m_pAnimator->RegisterEventListener("LookAt", [this]() {

		m_isLookAt = true;

		});

	m_pAnimator->RegisterEventListener("AttackOn", [this]() {

		
		});

	m_pAnimator->RegisterEventListener("AttackOff", [this]() {

		
		});
}

void CWatchDog::Block_Reaction()
{
	m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
	m_pAnimator->SetTrigger("Hit");
}

void CWatchDog::Reset()
{
	m_fHp = 300;

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

	// 공격용 콜라이더 리셋

	__super::Reset();

	if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
	{
		pPlayer->Get_Controller()->Remove_IgnoreActors(m_pPhysXActorCom->Get_Actor());
	}
	m_pPhysXActorCom->Set_SimulationFilterData(m_pPhysXActorCom->Get_FilterData());

	m_isFatal = false;


}

HRESULT CWatchDog::Ready_Weapon()
{
	// 일단 냅둬

    return S_OK;
}

CWatchDog* CWatchDog::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWatchDog* pInstance = new CWatchDog(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CButtler_Train");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CWatchDog::Clone(void* pArg)
{
	CWatchDog* pInstance = new CWatchDog(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CWatchDog");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CWatchDog::Free()
{
	__super::Free();

	Safe_Release(m_pAttackCollider);
}
