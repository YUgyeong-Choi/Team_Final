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
	:CMonster_Base (Prototype)
{
}

HRESULT CButtler_Train::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CButtler_Train::Initialize(void* pArg)
{
	/* [ 데미지 설정 ] */
	m_fDamage = 12.f;

	if(FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Weapon()))
		return E_FAIL;

	m_fDetectDist = 10.f;
	m_fGroggyThreshold = 100;
	
	m_fHp = 300;

	if(nullptr != m_pHPBar)
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
		//m_pWeapon->Clear_Owner();
		//m_pWeapon->Gravity_On();

		// 충돌만 False
		m_pWeapon->Collider_ShapeOff();
		m_pWeapon->Collider_FilterOff();
		m_pPhysXActorCom->Set_ShapeFlag(false, false, true);
		m_pPhysXActorCom->Init_SimulationFilterData();
		if (m_pAnimator->IsFinished())
		{
			// 씬에 지우는 건 딱 한번만
			m_pPhysXActorCom->RemovePhysX();
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

void CButtler_Train::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	
	// 무기가 상태마다 한번씩 데미지 주고
	// 이제 초기화하면 다시 데미지 줄 수 있게
	ReceiveDamage(pOther, eColliderType);

}

void CButtler_Train::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	ReceiveDamage(pOther, eColliderType);
}

void CButtler_Train::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{

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


		if (pWeapon->Find_CollisonObj(this))
		{
			return;
		}

		m_pWeapon->SetisAttack(false);

		pWeapon->Add_CollisonObj(this);
		pWeapon->Calc_Durability(3.f);

		m_fHp -= pWeapon->Get_CurrentDamage() / 2.f;

		m_pHPBar->Add_Damage(pWeapon->Get_CurrentDamage() / 2.f);

		m_fGroggyThreshold -= pWeapon->Get_CurrentDamage() / 10.f;

		if (nullptr != m_pHPBar)
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

			if (m_strStateName.find("Hit") != m_strStateName.npos)
			{
				
				m_pAnimator->Get_CurrentAnimController()->SetState(m_strStateName);

			}
			else
			{
				m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
				m_pAnimator->SetTrigger("Hit");
			}

			

			
			if(m_fGroggyThreshold <= 0)
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
	if (m_strStateName.find("Run") != m_strStateName.npos || m_strStateName.find("Walk_F") != m_strStateName.npos)
	{
		_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
		m_pTransformCom->Go_Dir(vLook, fTimeDelta * 0.5f, nullptr, m_pNaviCom);
	}
	else 
	{
		RootMotionActive(fTimeDelta);
	}

}

void CButtler_Train::Register_Events()
{
	m_pAnimator->RegisterEventListener("AddAttackCount", [this]() {

		++m_iAttackCount;

		});

	m_pAnimator->RegisterEventListener("BackMoveEnd", [this]() {

		m_pAnimator->SetBool("IsBack", false);

		});

	m_pAnimator->RegisterEventListener("NotLookAt", [this]() {

		m_isLookAt = false;

		});

	m_pAnimator->RegisterEventListener("LookAt", [this]() {

		m_isLookAt = true;

		});

	m_pAnimator->RegisterEventListener("AttackOn", [this]() {

		m_pWeapon->SetisAttack(true);
		m_pWeapon->Clear_CollisionObj();
		});

	m_pAnimator->RegisterEventListener("AttackOff", [this]() {

		m_pWeapon->SetisAttack(false);
		m_pWeapon->Clear_CollisionObj();
		});

}

void CButtler_Train::Guard_Reaction()
{
	m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION))));
	m_pAnimator->SetTrigger("Hit");
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
	Desc.vLocalOffset = { -0.5f,0.f,0.f,1.f };
	Desc.vPhsyxExtent = { 0.4f, 0.2f, 0.2f };
	
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