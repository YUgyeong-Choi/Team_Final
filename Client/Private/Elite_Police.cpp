#include "Bone.h"
#include <Player.h>
#include "Elite_Police.h"
#include "GameInstance.h"
#include "Weapon_Monster.h"
#include "LockOn_Manager.h"
#include "Client_Calculation.h"
#include <PhysX_IgnoreSelfCallback.h>
#include "UI_Guide.h"
#include "UI_Manager.h"

CElite_Police::CElite_Police(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CEliteUnit{ pDevice, pContext }
{
}

CElite_Police::CElite_Police(const CElite_Police& Prototype)
	:CEliteUnit(Prototype)
{
	m_pAnimator = nullptr;
	m_eUnitType = EUnitType::ELITE_MONSTER;
}

HRESULT CElite_Police::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CElite_Police::Initialize(void* pArg)
{
	m_fDamage = 50.f;
	m_iPatternLimit = 1;
	m_fMaxRootMotionSpeed = 30.f;
	m_fRootMotionAddtiveScale = 1.f;
	m_fAttckDleay = 2.5f;
	m_fTooCloseDistance = 2.0f;
	m_fChasingDistance = 3.f;
	m_fMinimumTurnAngle = 85.f;
	m_bIsFirstAttack = false;
	m_fGroggyScale_Weak = 0.1f;
	m_fGroggyScale_Strong = 0.2f;
	m_fGroggyScale_Charge = 0.25f;
	if (pArg == nullptr)
	{
		UNIT_DESC UnitDesc{};
		UnitDesc.eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
		UnitDesc.fRotationPerSec = XMConvertToRadians(300.f);
		UnitDesc.fSpeedPerSec = m_fWalkSpeed;
		lstrcpy(UnitDesc.szName, TEXT("Elite_Police"));
		UnitDesc.szMeshID = TEXT("Elite_Police");
		UnitDesc.InitPos = _float3(55.f, 0.f, -7.5f);
		//UnitDesc.InitPos = _float3(55.5f, 0.f, -7.5f);
		UnitDesc.InitScale = _float3(0.9f, 0.9f, 0.9f);

		if (FAILED(__super::Initialize(&UnitDesc)))
			return E_FAIL;
	}
	else
	{
		UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);

		lstrcpy(pDesc->szName, TEXT("Elite_Police"));
		pDesc->szMeshID = TEXT("Elite_Police");
		pDesc->fRotationPerSec = XMConvertToRadians(300.f);
		pDesc->fSpeedPerSec = m_fWalkSpeed;

		//UnitDesc.InitPos = _float3(55.f, 0.f, -7.5f);
		//UnitDesc.InitPos = _float3(55.5f, 0.f, -7.5f);
		//UnitDesc.InitScale = _float3(0.9f, 0.9f, 0.9f);

		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;
	}

	if (FAILED(Ready_Weapon()))
		return E_FAIL;


	m_fMaxHp = 1000.f;
	m_fHp = m_fMaxHp;
	CUI_MonsterHP_Bar::HPBAR_DESC eDesc{};

	eDesc.fSizeX = 1.f;
	eDesc.fSizeY = 1.f;
	eDesc.fHeight = 3.5f;
	eDesc.pHP = &m_fHp;
	eDesc.pIsGroggy = &m_bGroggyActive;
	eDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();

	m_pHPBar = static_cast<CUI_MonsterHP_Bar*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT,
		ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_HPBar"), &eDesc));


	if (m_pHPBar)
		m_pHPBar->Set_MaxHp(m_fMaxHp);

	m_iLockonBoneIndex = m_pModelCom->Find_BoneIndex("Bip001-Spine2");
	m_vRayOffset = { 0.f, 1.8f, 0.f, 0.f };

	auto pShape = m_pPhysXActorCom->Get_Shape();
	PxGeometryHolder geomHolder = pShape->getGeometry();
	PxBoxGeometry box = geomHolder.box();
	box.halfExtents = PxVec3(0.6f, 1.2f, 1.1f);
	pShape->setGeometry(box);

	PxTransform localPose = pShape->getLocalPose();
	localPose.p += PxVec3(0.f, -0.5f, 0.f);
	pShape->setLocalPose(localPose);


	Ready_AttackPatternWeight();


	// 플레이어 카메라 레이충돌 무시하기 위한
	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXElbow->Get_Actor());
	m_pPhysXActorCom->Add_IngoreActors(static_cast<CWeapon_Monster*>(m_pWeapon)->Get_PhysXActor()->Get_Actor());
	return S_OK;
}

void CElite_Police::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
#ifdef _DEBUG
	if (KEY_DOWN(DIK_TAB))
	{
		cout << "현재 폴리스 상태 : ";

		switch (m_eCurrentState)
		{
		case Client::CEliteUnit::EEliteState::IDLE:
			cout << "IDLE" << endl;
			break;
		case Client::CEliteUnit::EEliteState::WALK:
			cout << "WALK" << endl;
			break;
		case Client::CEliteUnit::EEliteState::RUN:
			cout << "RUN" << endl;
			break;
		case Client::CEliteUnit::EEliteState::TURN:
			cout << "TURN" << endl;
			break;
		case Client::CEliteUnit::EEliteState::ATTACK:
			cout << "ATTACK" << endl;
			break;
		case Client::CEliteUnit::EEliteState::GROGGY:
			cout << "GROGGY" << endl;
			break;
		case Client::CEliteUnit::EEliteState::PARALYZATION:
			break;
		case Client::CEliteUnit::EEliteState::FATAL:
			break;
		case Client::CEliteUnit::EEliteState::DEAD:
			break;
		case Client::CEliteUnit::EEliteState::CUTSCENE:
			break;
		case Client::CEliteUnit::EEliteState::NONE:
			break;
		default:
			break;
		}
		m_pAnimator->SetInt("AttackType", COMBO5);
		m_pAnimator->SetTrigger("Attack");
	}
#endif // _DEBUG

	if (KEY_DOWN(DIK_Y))
		m_pAnimator->SetTrigger("Fatal");
	auto pCurState = m_pAnimator->Get_CurrentAnimController()->GetCurrentState();
	if (pCurState && m_fHp <= 0.f)
	{
		if (pCurState->stateName.find("Death") != pCurState->stateName.npos)
		{

			m_fEmissive = 0.f;
			if (!m_pAnimator->IsBlending() && m_pAnimator->IsFinished())
			{
				m_pGameInstance->Push_WillRemove(L"Layer_Monster_Normal", this);
				m_pWeapon->SetbIsActive(false);
				Safe_Release(m_pHPBar);
			}
		}
		else if (pCurState->stateName.find("Fatal_Hit_End") != pCurState->stateName.npos)
		{
			if (!m_pAnimator->IsBlending() && m_pAnimator->IsFinished())
			{
				m_eCurrentState = EEliteState::DEAD;
				m_pGameInstance->Push_WillRemove(L"Layer_Monster_Normal", this);
				m_pWeapon->SetbIsActive(false);
				Safe_Release(m_pHPBar);
			}
		}
	}



	if (m_fHp <= 0 && m_ePrevState != EEliteState::DEAD)
	{
		m_pWeapon->Collider_FilterOff();
		EnableColliders(false);
		CLockOn_Manager::Get_Instance()->Set_Off(this);
		m_bUseLockon = false;
	}

	if (m_bDead)
		m_pHPBar->Set_bDead();
	
	

	if (nullptr != m_pHPBar)
		m_pHPBar->Priority_Update(fTimeDelta);


	if (!m_isFirstGroggy)
	{
		if (m_eCurrentState == EEliteState::GROGGY)
		{
			// 두개 생성 시작
			CUI_Guide::UI_GUIDE_DESC eDesc{};

			eDesc.partPaths = { TEXT("../Bin/Save/UI/Guide/Guide_Groggy.json") ,  TEXT("../Bin/Save/UI/Guide/Guide_Fatal.json") };

			m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Guide"),
				m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player_UI_Guide"), &eDesc);

			
			

			m_isFirstGroggy = true;
		}
	}

	if (m_eCurrentState == EEliteState::DEAD && m_fHp <= 0.f)
	{
		if (m_isDropItem)
		{
			return;
		}
		// 
		static_cast<CPlayer*>(m_pPlayer)->Set_GetKey();
		CUI_Manager::Get_Instance()->Activate_UI(TEXT("Pickup_Item"), false);
		CUI_Manager::Get_Instance()->Update_PickUpItem(ENUM_CLASS(ITEM_TAG::KEY));
		CUI_Manager::Get_Instance()->Activate_UI(TEXT("Pickup_Item"), true);

		m_isDropItem = true;
	}

	if (m_bGroggyActive)
	{
		if (m_pHPBar)
			m_pHPBar->Set_RenderTime(1.f);
	}

}

void CElite_Police::Update(_float fTimeDelta)
{
	if (CalculateCurrentHpRatio() <= 0.f)
	{
		if (m_eCurrentState != EEliteState::FATAL)
			m_pAnimator->SetTrigger("Die");
		m_bUseLockon = false;
	}
	__super::Update(fTimeDelta);

	if (nullptr != m_pHPBar)
		m_pHPBar->Update(fTimeDelta);

}

void CElite_Police::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
	if (nullptr != m_pHPBar)
		m_pHPBar->Late_Update(fTimeDelta);

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider() && m_pPhysXElbow->Get_ReadyForDebugDraw())
	{
		m_pGameInstance->Add_DebugComponent(m_pPhysXElbow);
	}
#endif
}

HRESULT CElite_Police::Ready_Components(void* pArg)
{
	if (FAILED(__super::Ready_Components(pArg)))
		return E_FAIL;

	// 엘리트 경찰은 팔꿈치에 무기 콜라이더
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX2"), reinterpret_cast<CComponent**>(&m_pPhysXElbow))))
		return E_FAIL;

	return S_OK;
}

HRESULT CElite_Police::Ready_Actor()
{
	if (FAILED(__super::Ready_Actor()))
		return E_FAIL;
	_vector S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());
	if (m_pRightElbowBone)
	{
		auto elbowLocalMatrix = m_pRightElbowBone->Get_CombinedTransformationMatrix();
		auto elbowWorldMatrix = XMLoadFloat4x4(elbowLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
		XMMatrixDecompose(&S, &R, &T, elbowWorldMatrix);

		PxQuat elbowRotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
		PxVec3 elbowPositionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
		PxTransform elbowPose(elbowPositionVec, elbowRotationQuat);
		PxSphereGeometry elbowGeom = m_pGameInstance->CookSphereGeometry(0.85f);
		m_pPhysXElbow->Create_Collision(m_pGameInstance->GetPhysics(), elbowGeom, elbowPose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXElbow->Set_ShapeFlag(false, true, true);
		PxFilterData elbowFilterData{};
		elbowFilterData.word0 = WORLDFILTER::FILTER_MONSTERWEAPON;
		elbowFilterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
		m_pPhysXElbow->Set_SimulationFilterData(elbowFilterData);
		m_pPhysXElbow->Set_QueryFilterData(elbowFilterData);
		m_pPhysXElbow->Set_Owner(this);
		m_pPhysXElbow->Set_ColliderType(COLLIDERTYPE::MONSTER_WEAPON_BODY);
		m_pPhysXElbow->Set_Kinematic(true);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXElbow->Get_Actor());
		m_pPhysXElbow->Init_SimulationFilterData();
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			if (auto pController = pPlayer->Get_Controller())
			{
				pController->Add_IngoreActors(m_pPhysXElbow->Get_Actor());
			}
		}
	}
	return S_OK;
}


void CElite_Police::Ready_BoneInformation()
{
	auto it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Bip001-R-Forearm"); });
	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pRightElbowBone = *it;
	}
}


HRESULT CElite_Police::Ready_Weapon()
{
	CWeapon_Monster::MONSTER_WEAPON_DESC Desc{};
	Desc.eMeshLevelID = LEVEL::STATIC;
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.InitPos = { 0.125f, 0.f, 0.f };
	Desc.InitScale = { 1.1f,0.5f,1.1f };
	Desc.iRender = 0;

	Desc.szMeshID = TEXT("Elite_Police_Weapon");
	lstrcpy(Desc.szName, TEXT("Elite_Police_Weapon"));
	Desc.vAxis = { 1.f,0.f,0.f,0.f };
	Desc.fRotationDegree = { -90.f };
	Desc.vLocalOffset = { -0.f,0.f,0.f,1.f };
	Desc.vPhsyxExtent = { 1.45f, 0.6f, 0.6f };
	Desc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-R-Hand"));
	Desc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	Desc.pOwner = this;

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_Weapon"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Monster_Elite_Weapon"), &pGameObject, &Desc)))
		return E_FAIL;

	m_pWeapon = dynamic_cast<CWeapon_Monster*>(pGameObject);
	m_pWeapon->SetDamage(m_fDamage);
	Safe_AddRef(m_pWeapon);

	m_pWeapon->SetisAttack(false);
	m_pWeapon->Set_WeaponTrail_Active(true);
	return S_OK;
}

_bool CElite_Police::CanProcessTurn()
{
	return m_bReturnToSpawn == false;
}


void CElite_Police::HandleMovementDecision(_float fDistance, _float fTimeDelta)
{
	if (m_bSpawned == false)
		return;

	if (m_bReturnToSpawn)
	{
	/*	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
		_vector vTarget = XMLoadFloat3(&m_InitPos);
		_float fDistance = XMVectorGetX(XMVector3Length(vTarget - vPos));
		_vector vDir = XMVector3Normalize(vTarget - vPos);

		m_pAnimator->SetFloat("Distance ", abs(fDistance));
		m_pTransformCom->Set_SpeedPerSec(m_fWalkSpeed);
		m_pAnimator->SetBool("Move", true);
		m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::FRONT));
		m_eCurrentState = EEliteState::WALK;

		m_pTransformCom->LookAtWithOutY(vTarget);
		m_pTransformCom->Go_Dir(vDir, fTimeDelta,nullptr,m_pNaviCom);


		if (XMVectorGetX(XMVector3Length(vTarget - vPos)) < 0.5f)
		{
			m_bReturnToSpawn = false;
			m_eCurrentState = EEliteState::IDLE;
			m_pAnimator->SetBool("Move", false);
		}
		return;*/
	}

	if (fDistance < m_fTooCloseDistance)
	{
		if (m_fChangeMoveDirCooldown <= 0.f)
		{
			m_fWalkSpeed = 2.5f;
			m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::BACK));
			m_fChangeMoveDirCooldown = 2.f;
		}
		else
		{
			m_fChangeMoveDirCooldown -= fTimeDelta;
		}
	}
	else if (fDistance <= m_fChasingDistance) // 공격 범위 근처
	{
		_vector vLook = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
		_vector vToPlayer = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) - m_pTransformCom->Get_State(STATE::POSITION);
		vToPlayer = XMVector3Normalize(XMVectorSetY(vToPlayer, 0.f));

		// 서로 락온이라서 외적으로 
		_vector vCross = XMVector3Cross(vLook, vToPlayer);
		_float fCrossY = XMVectorGetY(vCross);

		if (m_fChangeMoveDirCooldown <= 0.f)
		{
			m_fWalkSpeed = 2.5f;
			if (fCrossY > 0.f) // 플레이어가 오른쪽에 있음
				m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::LEFT));
			else               // 플레이어가 왼쪽에 있음
				m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::RIGHT));

			m_fChangeMoveDirCooldown = 1.5f;
		}
		else
		{
			m_fChangeMoveDirCooldown -= fTimeDelta;
		}
	}
	else
	{
		if (m_fChangeMoveDirCooldown <= 0.f)
		{
			m_fWalkSpeed = 3.f;
			m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::FRONT));
			m_fChangeMoveDirCooldown = 0.5f;
		}
		else
		{
			m_fChangeMoveDirCooldown -= fTimeDelta;
		}
	}

	m_eCurrentState = EEliteState::WALK;
	m_pAnimator->SetFloat("Distance", abs(fDistance));
}

void CElite_Police::UpdateAttackPattern(_float fDistance, _float fTimeDelta)
{
	if (m_bReturnToSpawn)
		return;

	if (m_bPlayedDetect == false)
		return;

	if (fDistance < ATTACK_DISTANCE_CLOSE || fDistance > ATTACK_DISTANCE_MIDDLE)
		return; // 너무 가깝거나 너무 멀면 공격 안 함

	if (false == UpdateTurnDuringAttack(fTimeDelta))
	{
		return;
	}

	if (m_eCurrentState == EEliteState::ATTACK)
	{
		return;
	}

	if (m_fAttackCooldown > 0.f)
	{
		m_fAttackCooldown -= fTimeDelta;
		m_fAttackCooldown = max(m_fAttackCooldown, 0.f);
		if (m_fAttackCooldown > 0.f)
			return; // 공격 쿨타임이 남아있으면 업데이트 중지
	}


	EPoliceAttackPattern eAttackType = static_cast<EPoliceAttackPattern>(GetRandomAttackPattern(fDistance));

	SetupAttackByType(eAttackType);

	m_pAnimator->SetBool("Move", false);
	m_pAnimator->SetInt("AttackType", eAttackType);
	m_pAnimator->SetTrigger("Attack");
	m_ePrevState = m_eCurrentState;
	m_eCurrentState = EEliteState::ATTACK;
	m_fAttackCooldown = m_fAttckDleay;
	m_fChangeMoveDirCooldown = 0.f;
}

void CElite_Police::UpdateStateByNodeID(_uint iNodeID)
{
	m_ePrevState = m_eCurrentState;

	m_iPrevNodeID = m_iCurNodeID;
	switch (iNodeID)
	{
	case ENUM_CLASS(EliteMonsterStateID::Idle):
		m_eCurrentState = EEliteState::IDLE;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Walk_B):
	case ENUM_CLASS(EliteMonsterStateID::Walk_F):
	case ENUM_CLASS(EliteMonsterStateID::Walk_R):
	case ENUM_CLASS(EliteMonsterStateID::Walk_L):
	{
		m_pTransformCom->Set_SpeedPerSec(m_fWalkSpeed);
		m_eCurrentState = EEliteState::WALK;
		break;
	}
	case ENUM_CLASS(EliteMonsterStateID::Run_F):
		m_pTransformCom->Set_SpeedPerSec(m_fRunSpeed);
		m_eCurrentState = EEliteState::RUN;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Atk_Combo1):
	case ENUM_CLASS(EliteMonsterStateID::Atk_Combo2):
	case ENUM_CLASS(EliteMonsterStateID::Atk_Combo3):
		m_eCurrentState = EEliteState::ATTACK;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Atk_Combo4):
		m_eCurrentState = EEliteState::ATTACK;
		break;

		break;
	case ENUM_CLASS(EliteMonsterStateID::Death_B):
		m_eCurrentState = EEliteState::DEAD;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Turn_L):
	case ENUM_CLASS(EliteMonsterStateID::Turn_R):
		m_eCurrentState = EEliteState::TURN;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Groggy_Start):
	case ENUM_CLASS(EliteMonsterStateID::Groggy_Loop):
	case ENUM_CLASS(EliteMonsterStateID::Groggy_End):
		m_eCurrentState = EEliteState::GROGGY;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Fatal_Hit_Start):
	case ENUM_CLASS(EliteMonsterStateID::Fatal_Hit_Ing):
	case ENUM_CLASS(EliteMonsterStateID::Fatal_Hit_End):
		m_eCurrentState = EEliteState::FATAL;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Paralyzation_Start):
	case ENUM_CLASS(EliteMonsterStateID::Paralyzation_Loop):
	case ENUM_CLASS(EliteMonsterStateID::Paralyzation_End):
		m_eCurrentState = EEliteState::PARALYZATION;
		break;
	default:
		break;
	}
	if (iNodeID == ENUM_CLASS(EliteMonsterStateID::Atk_Combo5) ||
		iNodeID == ENUM_CLASS(EliteMonsterStateID::Atk_Combo2_2))
	{
		m_bRootMotionClamped = true;
	}
	else
	{
		m_bRootMotionClamped = false;
	}
	m_iCurNodeID = iNodeID;
}

void CElite_Police::UpdateSpecificBehavior(_float fTimeDelta)
{
	//if (m_bReturnToSpawn)
	//	return;
	if (m_pPlayer)
	{
	/*	if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			if (pPlayer->Get_PlayerState() == EPlayerState::DEAD)
			{
				m_bReturnToSpawn = true;
				return;
			}
			else
			{
				m_bReturnToSpawn = false;
			}
		}*/
		if (m_bPlayedDetect == false && Get_DistanceToPlayer() <= m_fDetectRange)
		{
			_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
			_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
			_vector vDirVec = vPlayerPos - vMyPos;
			_float fDiffY = abs(XMVectorGetY(vDirVec));
			if (m_fDetectDiffY >= fDiffY)
			{
				m_bPlayedDetect = true;
				m_pAnimator->SetTrigger("Detect");
			}
		}

		if ((m_eCurrentState == EEliteState::RUN || m_eCurrentState == EEliteState::WALK)
			&& m_eCurrentState != EEliteState::ATTACK
			&& m_eCurrentState != EEliteState::TURN)  // Turn 상태 제외
		{
			m_pTransformCom->LookAtWithOutY(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION));
		}
	}

	//if (IsTargetInFront(180.f) == false)
	//{

	//	m_pAnimator->SetBool("Move", false);
	//	m_pAnimator->SetInt("AttackType", COMBO4);
	//	m_pAnimator->SetTrigger("Attack");
	//	m_ePrevState = m_eCurrentState;
	//	m_eCurrentState = EEliteState::ATTACK;
	//	m_fAttackCooldown = m_fAttckDleay;
	//}
}

void CElite_Police::EnableColliders(_bool bEnable)
{
	__super::EnableColliders(bEnable);
	if (bEnable)
	{
		m_pPhysXElbow->Set_SimulationFilterData(m_pPhysXElbow->Get_FilterData());
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			if (auto pController = pPlayer->Get_Controller())
			{
				pController->Remove_IgnoreActors(m_pPhysXElbow->Get_Actor());
			}
		}
	}
	else
	{
		m_pPhysXElbow->Init_SimulationFilterData();
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			if (auto pController = pPlayer->Get_Controller())
			{
				pController->Add_IngoreActors(m_pPhysXElbow->Get_Actor());
			}
		}
	}
}

void CElite_Police::Ready_EffectNames()
{
}

void CElite_Police::ProcessingEffects(const _wstring& stEffectTag)
{
}

HRESULT CElite_Police::EffectSpawn_Active(_int iEffectId, _bool bActive, _bool bIsOnce)
{
	return S_OK;
}

HRESULT CElite_Police::Spawn_Effect()
{
	return S_OK;
}

HRESULT CElite_Police::Ready_Effect()
{
	return S_OK;
}

void CElite_Police::Register_Events()
{
	__super::Register_Events();

	m_pAnimator->RegisterEventListener("SetRootStep", [this]()
		{
			m_fMaxRootMotionSpeed = 1.f;
		});

	m_pAnimator->RegisterEventListener("ResetRootStep", [this]()
		{
			m_fMaxRootMotionSpeed = 30.f;
			m_fRootMotionAddtiveScale = 1.f;
		});

	m_pAnimator->RegisterEventListener("SetRootLargeStep", [this]()
		{
			m_fMaxRootMotionSpeed = 100.f;
			m_fRootMotionAddtiveScale = 1.75f;
		});

	m_pAnimator->RegisterEventListener("Spawned", [this]()
		{
			m_bSpawned = true;
		});
	m_pAnimator->RegisterEventListener("Turnnig", [this]()
		{
			SetTurnTimeDuringAttack(1.7f, 1.4f);
		});

	m_pAnimator->RegisterEventListener("LargeTurnnig", [this]()
		{
			_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
			vLook = XMVectorSetY(vLook, 0.f);
			vLook = XMVector3Normalize(vLook);

			// 반대 방향
			_vector vDir = -vLook;
			m_pTransformCom->RotateToDirectionImmediately(vDir);
		});

	m_pAnimator->RegisterEventListener("WeaponAttackOn", [this]() {

		if (m_pWeapon == nullptr)
			return;
		m_pWeapon->SetisAttack(true);
		m_pWeapon->Clear_CollisionObj();
		});

	m_pAnimator->RegisterEventListener("WeaponAttackOff", [this]() {
		if (m_pWeapon == nullptr)
			return;
		m_pWeapon->SetisAttack(false);
		m_pWeapon->Clear_CollisionObj();
		});


	m_pAnimator->RegisterEventListener("ElbowOn", [this]() {
		Set_ElbowHit(false);
		m_pPhysXElbow->Set_SimulationFilterData(m_pPhysXElbow->Get_FilterData());
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			if (auto pController = pPlayer->Get_Controller())
			{
				pController->Remove_IgnoreActors(m_pPhysXElbow->Get_Actor());
			}
		}
		});

	m_pAnimator->RegisterEventListener("ElbowOff", [this]() {
		m_pPhysXElbow->Init_SimulationFilterData();
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			if (auto pController = pPlayer->Get_Controller())
			{
				pController->Add_IngoreActors(m_pPhysXElbow->Get_Actor());
			}
		}
		Set_ElbowHit(false);
		});

	m_pAnimator->RegisterEventListener("ChageLook", [this]() {

		if (m_pTransformCom)
		{
			m_pTransformCom->RotateToDirectionImmediately(-m_pTransformCom->Get_State(STATE::LOOK));
			//_vector vNewLook = m_pTransformCom->Get_State(STATE::LOOK) * -1.f;
			//vNewLook = XMVector3Normalize(vNewLook);

			//_vector vUp = XMVectorSet(0, 1, 0, 0); // World Up
			//_vector vRight = XMVector3Cross(vUp, vNewLook);
			//vRight = XMVector3Normalize(vRight);


			//vUp = XMVector3Cross(vNewLook, vRight);
			//vUp = XMVector3Normalize(vUp);

			//// 현재 위치는 유지
			//_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

			//m_pTransformCom->Set_State(STATE::RIGHT, vRight);
			//m_pTransformCom->Set_State(STATE::UP, vUp);
			//m_pTransformCom->Set_State(STATE::LOOK, vNewLook);
			//m_pTransformCom->Set_State(STATE::POSITION, vPos);
		}
		});


}

void CElite_Police::Reset()
{
	__super::Reset();
	m_pPlayer = GET_PLAYER(m_pGameInstance->GetCurrentLevelIndex());
	m_bReturnToSpawn = false;
	m_bPlayedDetect = false;
	m_bSpawned = false;
	m_fGroggyScale_Weak = 0.1f;
	m_fGroggyScale_Strong = 0.15f;
	m_fGroggyScale_Charge = 0.2f;
}

_int CElite_Police::GetRandomAttackPattern(_float fDistance)
{
	EPoliceAttackPattern ePattern = AP_NONE;
	m_PatternWeighForDisttMap = m_PatternWeightMap;
	ChosePatternWeightByDistance(fDistance);


	_float fTotalWeight = accumulate(m_PatternWeighForDisttMap.begin(), m_PatternWeighForDisttMap.end(), 0.f,
		[](_float fAcc, const pair<_int, _float>& Pair) { return fAcc + Pair.second; });

	_float fRandomVal = GetRandomFloat(0.f, fTotalWeight);
	_float fCurWeight = 0.f;
	for (const auto& [pattern, weight] : m_PatternWeighForDisttMap)
	{
		if (weight <= 0.f)
			continue; // 가중치가 0 이하인 패턴은 무시
		fCurWeight += weight;
		if (fRandomVal <= fCurWeight)
		{
			ePattern = static_cast<EPoliceAttackPattern>(pattern);
			m_ePrevAttackPattern = m_eCurAttackPattern;
			m_eCurAttackPattern = ePattern;
			UpdatePatternWeight(ePattern);
			break;
		}
	}
	return ePattern;
}

void CElite_Police::UpdatePatternWeight(_int iPattern)
{
	m_PatternCountMap[iPattern]++;
	if (m_PatternCountMap[iPattern] >= m_iPatternLimit)
	{
		m_PatternWeightMap[iPattern] *= (1.f - m_fWeightDecreaseRate); // 가중치 감소
		m_PatternWeightMap[iPattern] = max(m_PatternWeightMap[iPattern], m_fMinWeight); // 최소 가중치로 설정
		m_PatternCountMap[iPattern] = 0;

		for (auto& [pattern, weight] : m_PatternWeightMap)
		{
			if (pattern != iPattern)
			{
				weight += (m_fMaxWeight - weight) * m_fWeightIncreaseRate; // 가중치 증가
				weight = min(weight, m_fMaxWeight); // 최대 가중치로 제한
			}
		}
	}
}

_bool CElite_Police::CanMove() const
{
	return (m_bReturnToSpawn ||
		(m_eCurrentState == EEliteState::IDLE ||
			m_eCurrentState == EEliteState::WALK ||
			m_eCurrentState == EEliteState::RUN) &&
		m_eCurrentState != EEliteState::GROGGY &&
		m_eCurrentState != EEliteState::DEAD &&
		m_eCurrentState != EEliteState::PARALYZATION &&
		m_eCurrentState != EEliteState::ATTACK &&
		m_eCurrentState != EEliteState::FATAL &&
		m_bPlayedDetect);
}



void CElite_Police::ChosePatternWeightByDistance(_float fDistance)
{
	if (fDistance >= ATTACK_DISTANCE_CLOSE && fDistance < ATTACK_DISTANCE_MIDDLE)
	{

		for (auto& [pattern, weight] : m_PatternWeighForDisttMap)
		{
			auto it = find(m_vecCloseAttackPatterns.begin(), m_vecCloseAttackPatterns.end(), pattern);
			if (it == m_vecCloseAttackPatterns.end())
			{
				m_PatternWeighForDisttMap[pattern] *= 0.f;
			}
		}

	}
	else if (fDistance >= ATTACK_DISTANCE_MIDDLE && fDistance)
	{
		for (auto& [pattern, weight] : m_PatternWeighForDisttMap)
		{
			auto it = find(m_vecMiddleAttackPatterns.begin(), m_vecMiddleAttackPatterns.end(), pattern);
			if (it == m_vecMiddleAttackPatterns.end())
			{
				m_PatternWeighForDisttMap[pattern] *= 0.f;
			}
		}
	}
}

void CElite_Police::SetupAttackByType(_int iPattern)
{
	switch (iPattern)
	{
	case COMBO1:
	case COMBO2:
	case COMBO3:
	case COMBO4:
		m_bRootMotionClamped = false;
		static_cast<CPlayer*>(m_pPlayer)->SetHitMotion(HITMOTION::NORMAL);
		break;
	case COMBO5:
		m_bRootMotionClamped = true;
		static_cast<CPlayer*>(m_pPlayer)->SetHitMotion(HITMOTION::KNOCKBACK);
		break;
	default:
		m_bRootMotionClamped = false;
		break;
	}
}

void CElite_Police::Ready_AttackPatternWeight()
{
	m_PatternWeightMap.clear();
	m_PatternCountMap.clear();
	vector<EPoliceAttackPattern> m_vecBossPatterns = {
			COMBO1, COMBO2, COMBO3,COMBO5
	};

	for (const auto& pattern : m_vecBossPatterns)
	{
		m_PatternWeightMap[pattern] = m_fBasePatternWeight;
		m_PatternCountMap[pattern] = 0;
	}
}

void CElite_Police::Update_Collider()
{
	__super::Update_Collider();

	auto elbowLocalMatrix = m_pRightElbowBone->Get_CombinedTransformationMatrix();
	auto elbowWorldMatrix = XMLoadFloat4x4(elbowLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
	_float4 elbowPos;
	XMStoreFloat4(&elbowPos, elbowWorldMatrix.r[3]);
	PxVec3 armPos(elbowPos.x, elbowPos.y, elbowPos.z);
	_vector boneQuatForElbow = XMQuaternionRotationMatrix(elbowWorldMatrix);
	_float4 fQuatForElbow;
	XMStoreFloat4(&fQuatForElbow, boneQuatForElbow);
	PxQuat elbowRot = PxQuat(fQuatForElbow.x, fQuatForElbow.y, fQuatForElbow.z, fQuatForElbow.w);
	m_pPhysXElbow->Set_Transform(PxTransform(armPos, elbowRot));
}

void CElite_Police::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{

}

void CElite_Police::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CElite_Police::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CElite_Police::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	ReceiveDamage(pOther, eColliderType);

	if (eColliderType == COLLIDERTYPE::PLAYER)
	{
		if (m_bElbowHit)
		{
			static_cast<CPlayer*>(pOther)->SetfReceiveDamage(25.f);
			static_cast<CPlayer*>(pOther)->SetHitMotion(HITMOTION::KNOCKBACK);
		}
	}
}

void CElite_Police::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}


CElite_Police* CElite_Police::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CElite_Police* pInstance = new CElite_Police(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CElite_Police");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CElite_Police::Clone(void* pArg)
{
	CElite_Police* pInstance = new CElite_Police(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CElite_Police");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CElite_Police::Free()
{
	__super::Free();

	Safe_Release(m_pWeapon);
	Safe_Release(m_pPhysXElbow);
}
