#include "Fuoco.h"

#include "Oil.h"
#include "Bone.h"
#include "Player.h"
#include "Projectile.h"
#include "FlameField.h"
#include "GameInstance.h"
#include "SpringBoneSys.h"
#include "Effect_Manager.h"
#include "LockOn_Manager.h"
#include "Camera_Manager.h"
#include "UI_MonsterHP_Bar.h"
#include "Client_Calculation.h"
#include "PhysX_IgnoreSelfCallback.h"
#include "SwordTrailEffect.h"

CFuoco::CFuoco(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBossUnit(pDevice, pContext)
{
}

CFuoco::CFuoco(const CFuoco& Prototype)
	: CBossUnit(Prototype)
{
}
HRESULT CFuoco::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFuoco::Initialize(void* pArg)
{
	
	if (pArg == nullptr)
	{
		UNIT_DESC UnitDesc{};
		UnitDesc.eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
		UnitDesc.fRotationPerSec = XMConvertToRadians(180.f);
		UnitDesc.fSpeedPerSec = m_fWalkSpeed;
		lstrcpy(UnitDesc.szName, TEXT("FireEater"));
		UnitDesc.szMeshID = TEXT("FireEater");
		UnitDesc.InitPos = _float3(55.f, 0.f, -7.5f);
		UnitDesc.InitScale = _float3(0.9f, 0.9f, 0.9f);

		if (FAILED(__super::Initialize(&UnitDesc)))
			return E_FAIL;
	}
	else
	{
		UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);

		lstrcpy(pDesc->szName, TEXT("FireEater"));
		pDesc->szMeshID = TEXT("FireEater");
		pDesc->fRotationPerSec = XMConvertToRadians(180.f);
		pDesc->fSpeedPerSec = m_fWalkSpeed;
		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;
	}

	m_fMaxRootMotionSpeed = 18.f;

	m_fDamage = 15.f;
	m_fAttckDleay = 1.5f;
	m_fChasingDistance = 4.f;
	m_iPatternLimit = 1;
	return S_OK;
}

void CFuoco::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_bDead)
		m_pHPBar->Set_bDead();

#ifdef _DEBUG
	static _int i = -1;
	static array<_int, 13> testArray{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,13 };

	if (KEY_DOWN(DIK_X))
	{
		i++;
		if (i >= 13)
			i = 0;
		m_eCurAttackPattern = static_cast<EBossAttackPattern>(i + 1);
		switch (m_eCurAttackPattern)
		{
		case CFuoco::SlamCombo:
			cout << "SlamCombo" << endl;
			break;
		case CFuoco::Uppercut:
			cout << "Uppercut" << endl;
			break;
		case CFuoco::SwingAtkSeq:
			cout << "SwingAtkSeq" << endl;
			break;
		case CFuoco::SwingAtk:
			cout << "SwingAtk" << endl;
			break;
		case CFuoco::SlamFury:
			cout << "SlamFury" << endl;
			break;
		case CFuoco::FootAtk:
			cout << "FootAtk" << endl;
			break;
		case CFuoco::P2_FlameField:
			cout << "P2_FlameField" << endl;
			break;
		case CFuoco::SlamAtk:
			cout << "SlamAtk" << endl;
			break;
		case CFuoco::StrikeFury:
			cout << "StrikeFury" << endl;
			break;
		case CFuoco::P2_FireOil:
			cout << "P2_FireOil" << endl;
			break;
		case CFuoco::P2_FireBall:
			cout << "P2_FireBall" << endl;
			break;
		case CFuoco::P2_FireFlame:
			cout << "P2_FireFlame" << endl;
			break;
		case CFuoco::P2_FireBall_B:
			cout << "P2_FireBall_B" << endl;
			break;
		default:
			cout << "Unknown" << endl;;
			break;
		}

	}

	if (KEY_DOWN(DIK_C))
	{
		if (i == -1)
			return;
		m_pAnimator->SetInt("SkillType", testArray[i]);
		m_pAnimator->SetTrigger("Attack");
	}


	if (KEY_DOWN(DIK_V))
	{
		m_bDebugMode = !m_bDebugMode;
		if (m_bDebugMode)
			cout << "디버그 모드 ON" << endl;
		else
			cout << "디버그 모드 OFF" << endl;
	}

#endif

	if (nullptr != m_pHPBar)
		m_pHPBar->Priority_Update(fTimeDelta);
}

void CFuoco::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (m_bDeathProcessed)
	{
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			pPlayer->SetbEnding(true);
		}
	}

	if (m_fFireFlameDuration > 0.f)
	{
		m_fFireFlameDuration -= fTimeDelta;
		FlamethrowerAttack(15.f);
		{
			if (m_fFireFlameDuration <= 0.f)
				m_fFireFlameDuration = 0.f;
		}
	}

	if (nullptr != m_pHPBar)
		m_pHPBar->Update(fTimeDelta);

	if (m_pPlayer&&static_cast<CUnit*>(m_pPlayer)->GetHP() <= 0 && m_pHPBar)
		m_pHPBar->Set_RenderTime(0.f);

}

void CFuoco::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider())
	{
		if (m_pPhysXActorComForArm->Get_ReadyForDebugDraw())
			m_pGameInstance->Add_DebugComponent(m_pPhysXActorComForArm);
		if (m_pPhysXActorComForFoot->Get_ReadyForDebugDraw())
			m_pGameInstance->Add_DebugComponent(m_pPhysXActorComForFoot);
	}
#endif

	if (nullptr != m_pHPBar)
		m_pHPBar->Late_Update(fTimeDelta);
}

void CFuoco::Reset()
{
	__super::Reset();
	m_bWaitPhase2Rotate = false;
	m_vPhase2TurnDir = {};
	m_bPhase2TurnProcessed = false;
	m_bPhase2TurnFinished = false;
	m_bPlayerCollided = false;
	m_bUsedFlameFiledOnLowHp = false;
	m_iLastComboType = -1;
	m_fFireFlameDuration = 0.f;
	m_eCurAttackPattern = EBossAttackPattern::BAP_NONE;
	m_ePrevAttackPattern = EBossAttackPattern::BAP_NONE;
}

HRESULT CFuoco::Ready_Components(void* pArg)
{
	if (FAILED(__super::Ready_Components(pArg)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX2"), reinterpret_cast<CComponent**>(&m_pPhysXActorComForArm))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX3"), reinterpret_cast<CComponent**>(&m_pPhysXActorComForFoot))))
		return E_FAIL;

	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_FireEater"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CFuoco::Ready_Actor()
{
	if (FAILED(__super::Ready_Actor()))
		return E_FAIL;
	_vector S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 부모꺼 부르면서 밑에꺼 부르기
	if (m_pFistBone)
	{
		const PxTransform armPose = GetBonePose(m_pFistBone);

		PxSphereGeometry armGeom = m_pGameInstance->CookSphereGeometry(1.7f);
		m_pPhysXActorComForArm->Create_Collision(
			m_pGameInstance->GetPhysics(), 
			armGeom, 
			armPose,
			m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorComForArm->Set_ShapeFlag(false, true, true);
		PxFilterData armFilterData{};
		armFilterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		armFilterData.word1 = WORLDFILTER::FILTER_PLAYERBODY | WORLDFILTER::FILTER_MONSTERWEAPON;
		m_pPhysXActorComForArm->Set_SimulationFilterData(armFilterData);
		m_pPhysXActorComForArm->Set_QueryFilterData(armFilterData);
		m_pPhysXActorComForArm->Set_Owner(this);
		m_pPhysXActorComForArm->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
		m_pPhysXActorComForArm->Set_Kinematic(true);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorComForArm->Get_Actor());
	}

	if (m_pFootBone)
	{
		const PxTransform footPose = GetBonePose(m_pFootBone);

		PxSphereGeometry footGeom = m_pGameInstance->CookSphereGeometry(1.f);
		m_pPhysXActorComForFoot->Create_Collision(
			m_pGameInstance->GetPhysics(),
			footGeom, 
			footPose,
			m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorComForFoot->Set_ShapeFlag(false, true, true);
		PxFilterData footFilterData{};
		footFilterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		footFilterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
		m_pPhysXActorComForFoot->Set_SimulationFilterData(footFilterData);
		m_pPhysXActorComForFoot->Set_QueryFilterData(footFilterData);
		m_pPhysXActorComForFoot->Set_Owner(this);
		m_pPhysXActorComForFoot->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
		m_pPhysXActorComForFoot->Set_Kinematic(true);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorComForFoot->Get_Actor());
	}

	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorComForArm->Get_Actor());
	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorComForFoot->Get_Actor());

	return S_OK;
}

void CFuoco::Ready_BoneInformation()
{
	m_iLockonBoneIndex = m_pModelCom->Find_BoneIndex("Bip001-Spine");

	auto it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Bip001-L-Hand"); });
	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pLeftBone = *it;
	}

	it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Bone001-Canon01"); });

	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pCannonBone = *it;
	}


	it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Bone001-Middle-Finger01"); });

	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pMiddleFingierBone = *it;
	}


	it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Bip001-R-Forearm"); });

	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pRForearmBone = *it;
	}

	it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Bone001-Fist01"); });

	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pFistBone = *it;
	}



	it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Bip001-L-Foot"); });

	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pFootBone = *it;
	}
}

void CFuoco::Update_Collider()
{
	__super::Update_Collider();

	if (m_pPhysXActorComForArm && m_pFistBone)
		m_pPhysXActorComForArm->Set_Transform(GetBonePose(m_pFistBone));

	if (m_pPhysXActorComForFoot && m_pFootBone)
		m_pPhysXActorComForFoot->Set_Transform(GetBonePose(m_pFootBone));
}

void CFuoco::UpdateAttackPattern(_float fDistance, _float fTimeDelta)
{
	if (m_eCurrentState == EEliteState::DEAD || m_bDead
		|| m_eCurrentState == EEliteState::CUTSCENE
		|| m_eCurrentState == EEliteState::GROGGY ||
		m_eCurrentState == EEliteState::PARALYZATION ||
		m_eCurrentState == EEliteState::FATAL)
		return;
	if (m_fFirstChaseBeforeAttack >= 0.f)
	{
		if (m_eCurrentState != EEliteState::IDLE) // Idle이면 감소 안 함
			m_fFirstChaseBeforeAttack -= fTimeDelta;
		return;
	}
	// 퓨리 돌진 9번
	if (m_bIsFirstAttack)
	{
		m_pAnimator->SetInt("SkillType", StrikeFury);
		m_pAnimator->SetTrigger("Attack");
		m_bIsFirstAttack = false;
		m_pAnimator->SetBool("Move", false);
		m_fAttackCooldown = m_fAttckDleay;
		m_ePrevState = m_eCurrentState;
		m_eCurrentState = EEliteState::ATTACK;
		m_eAttackType = EAttackType::FURY_AIRBORNE;
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
			pPlayer->SetHitedAttackType(EAttackType::FURY_AIRBORNE);
		m_PatternCountMap[StrikeFury]++;
		m_PatternWeightMap[StrikeFury] *= m_fWeightDecreaseRate;
		return;
	}

	if (m_bWaitPhase2Rotate)
	{
		return;
	}

	if (CheckConditionFlameField())
	{
		m_fAttackCooldown = 5.f;
		return;
	}


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

#ifdef _DEBUG
	if (m_bDebugMode)
		return;
#endif // _DEBUG

	EBossAttackPattern eSkillType = static_cast<EBossAttackPattern>(GetRandomAttackPattern(fDistance));

	SetupAttackByType(eSkillType);

	m_pAnimator->SetBool("Move", false);
	m_pAnimator->SetInt("SkillType", eSkillType);
	m_pAnimator->SetTrigger("Attack");
	m_eCurrentState = EEliteState::ATTACK;
	m_fAttackCooldown = m_fAttckDleay;
	m_pSoundCom->Play_Random("VO_NPC_NHM_Boss_Fire_Eater_Attack_", 9);
}

void CFuoco::UpdateStateByNodeID(_uint iNodeID)
{
	m_iPrevNodeID = m_iCurNodeID;
	m_iCurNodeID = iNodeID;
	m_ePrevState = m_eCurrentState;
	_bool bIsFury = false;
	switch (iNodeID)
	{
	case ENUM_CLASS(BossStateID::IDLE):
		m_eCurrentState = EEliteState::IDLE;
		break;
	case ENUM_CLASS(BossStateID::WALK_B):
	case ENUM_CLASS(BossStateID::WALK_F):
	case ENUM_CLASS(BossStateID::WALK_R):
	case ENUM_CLASS(BossStateID::WALK_L):
	{
		m_pTransformCom->Set_SpeedPerSec(m_fWalkSpeed);
		m_eCurrentState = EEliteState::WALK;
	}
	break;
	case ENUM_CLASS(BossStateID::RUN_F):
		m_pTransformCom->Set_SpeedPerSec(m_fRunSpeed);
		m_eCurrentState = EEliteState::RUN;
		break;
	case ENUM_CLASS(BossStateID::GROGGY_START):
	case ENUM_CLASS(BossStateID::GROGGY_LOOP):
	case ENUM_CLASS(BossStateID::GROGGY_END):
		m_eCurrentState = EEliteState::GROGGY;
		break;
	case ENUM_CLASS(BossStateID::DEAD_B):
	case ENUM_CLASS(BossStateID::DEAD_F):
	case ENUM_CLASS(BossStateID::SPECIAL_DIE):
		m_eCurrentState = EEliteState::DEAD;
		EFFECT_MANAGER->Set_Dead_EffectContainer(TEXT("Fuoco_BellyFire"));
		EFFECT_MANAGER->Set_Dead_EffectContainer(TEXT("Fuoco_HeadSmoke1"));
		EFFECT_MANAGER->Set_Dead_EffectContainer(TEXT("Fuoco_HeadSmoke2"));
		break;
	case ENUM_CLASS(BossStateID::TURN_L):
	case ENUM_CLASS(BossStateID::TURN_R):
		m_eCurrentState = EEliteState::TURN;
		break;
	case ENUM_CLASS(BossStateID::PARALYZATION_START):
	case ENUM_CLASS(BossStateID::PARALYZATION_LOOP):
	case ENUM_CLASS(BossStateID::PARALYZATION_END):
		m_eCurrentState = EEliteState::PARALYZATION;
		break;
	case ENUM_CLASS(BossStateID::FATAL_START):
	case ENUM_CLASS(BossStateID::FATAL_LOOP):
	case ENUM_CLASS(BossStateID::FATAL_END):
		m_eCurrentState = EEliteState::FATAL;
		break;
	case ENUM_CLASS(BossStateID::ATK_SWING_SEQ):
		m_eCurrentState = EEliteState::ATTACK;
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(45.f);

		if (m_iPrevNodeID != ENUM_CLASS(BossStateID::ATK_SWING_SEQ))
		{
			//m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_SK_WS_Long_0");
		}
		break;
	case ENUM_CLASS(BossStateID::ATK_SWING_SEQ2):
		m_eCurrentState = EEliteState::ATTACK;
		if (m_iPrevNodeID != ENUM_CLASS(BossStateID::ATK_SWING_SEQ2))
		{
			//m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_SK_WS_Long_1");
		}
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(60.f);
		break;
	case ENUM_CLASS(BossStateID::ATK_SWING_SEQ3):
	{
		m_eCurrentState = EEliteState::ATTACK;
		if (m_iPrevNodeID != ENUM_CLASS(BossStateID::ATK_SWING_SEQ3))
		{
			EffectSpawn_Active(EF_LASTSPIN, true);
		}
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(70.f);
	}
	break;
	case ENUM_CLASS(BossStateID::ATK_SWING_SEQ_RESET):
	case ENUM_CLASS(BossStateID::ATK_SWING_SEQ_RESET2):
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(100.f);
		m_eCurrentState = EEliteState::ATTACK;
		break;
	case ENUM_CLASS(BossStateID::ATK_SWING_END):
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(55.f);
		m_eCurrentState = EEliteState::ATTACK;
		break;
	case ENUM_CLASS(BossStateID::ATK_SWING_R):
	case ENUM_CLASS(BossStateID::ATK_SWING_R_COM1):
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(55.f);
		m_eCurrentState = EEliteState::ATTACK;
		break;
	case ENUM_CLASS(BossStateID::ATK_SWING_R_COM2):
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(55.f);
		m_eCurrentState = EEliteState::ATTACK;
		break;

	case ENUM_CLASS(BossStateID::ATK_SWING_L_COM2):
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(55.f);
		m_eCurrentState = EEliteState::ATTACK;
		break;

	case ENUM_CLASS(BossStateID::ATK_SWING_L_COM1):
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(55.f);
		m_eCurrentState = EEliteState::ATTACK;
		break;

	case ENUM_CLASS(BossStateID::CUTSCENE):
		m_eCurrentState = EEliteState::CUTSCENE;
		break;
	case ENUM_CLASS(BossStateID::ATK_SLAM_COMBO_LEFT_END):
	case ENUM_CLASS(BossStateID::ATK_SLAM_COMBO_RIGHT_END):
		SetTurnTimeDuringAttack(0.7f,1.2f);
		break;
	case ENUM_CLASS(BossStateID::ATK_SLAM_FURY_START):
		m_eAttackType = EAttackType::FURY_STAMP;
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
			pPlayer->SetHitedAttackType(EAttackType::FURY_STAMP);
		bIsFury = true;
		m_eCurrentState = EEliteState::ATTACK;
		break;
	case ENUM_CLASS(BossStateID::ATK_SLAM_FURY):
	case ENUM_CLASS(BossStateID::ATK_SLAM_FURY_END):
		bIsFury = true;
		m_eCurrentState = EEliteState::ATTACK;
		break;
	case ENUM_CLASS(BossStateID::ATK_STRIKE_FURY):
		bIsFury = true;
		m_eCurrentState = EEliteState::ATTACK;
		break;
	default:
		m_eCurrentState = EEliteState::ATTACK;
		break;
	}
	if (m_ePrevState == EEliteState::FATAL && m_eCurrentState != EEliteState::FATAL)
	{
		m_fMaxRootMotionSpeed = 18.f;
		m_fRootMotionAddtiveScale = 1.2f;
	}
	if (bIsFury)
	{
		m_eFuryState = EFuryState::Fury;
	}
	else
	{
		m_eFuryState = EFuryState::None;
	}

}

void CFuoco::UpdateSpecificBehavior(_float fTimeDelta)
{
	if (m_eCurrentState == EEliteState::DEAD)
		return;

	if (m_eCurrentState != EEliteState::ATTACK)
	{
		m_pAnimator->SetBool("IsHit", false);
	}
	else if (m_eCurrentState == EEliteState::ATTACK)
	{
		m_pAnimator->SetBool("Move", false);
	}

	if ((m_eCurrentState == EEliteState::RUN || m_eCurrentState == EEliteState::WALK)
		&& m_eCurrentState != EEliteState::ATTACK
		&& m_eCurrentState != EEliteState::TURN && m_bWaitPhase2Rotate == false)  // Turn 상태 제외
	{
		m_pTransformCom->LookAtWithOutY(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION));
	}

	if (m_bIsPhase2&& m_eCurrentState != EEliteState::PARALYZATION&& m_bWaitPhase2Rotate)
	{
		// 현재 중앙으로 턴을 안했으면
		if (m_eCurrentState != EEliteState::TURN&& m_bPhase2TurnProcessed == false)
		{	// 방향을 정하고
			
			_vector vDir = XMLoadFloat4(&m_vCenterPos) - m_pTransformCom->Get_State(STATE::POSITION);
			vDir = XMVectorSetY(vDir, 0.f);
			vDir = XMVector3Normalize(vDir);

			_vector vCurrentLook = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
			_float fDot = XMVectorGetX(XMVector3Dot(vCurrentLook, vDir));
			fDot = clamp(fDot, -1.f, 1.f);
			_vector vCross = XMVector3Cross(vCurrentLook, vDir);
			_float fSign = (XMVectorGetY(vCross) < 0.f) ? -1.f : 1.f;
			_float fYaw = acosf(fDot) * fSign; // 회전 각도 (라디안 단위) -180~180
			m_pAnimator->SetInt("TurnDir", (fYaw >= 0.f) ? 0 : 1); // 0: 오른쪽, 1: 왼쪽
			m_pAnimator->SetTrigger("Turn");
			m_pAnimator->SetBool("Move", false); // 회전 중에는 이동하지 않음
			m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::FRONT));
			m_ePrevState = m_eCurrentState;
			m_eCurrentState = EEliteState::TURN;
			m_bPhase2TurnProcessed = true;
			XMStoreFloat3(&m_vPhase2TurnDir, vDir);
		}

		if(m_bPhase2TurnFinished)
		{
			m_fAttackCooldown = m_fAttckDleay;
			m_ePrevState = m_eCurrentState;
			m_eCurrentState = EEliteState::ATTACK;
			m_eAttackType = EAttackType::FURY_AIRBORNE;
			m_pAnimator->SetInt("SkillType", StrikeFury);
			m_pAnimator->SetTrigger("Attack");
			m_pAnimator->SetTrigger("Phase2Start");
			m_pAnimator->SetBool("Move", false);
			m_bWaitPhase2Rotate = false;
			if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
				pPlayer->SetHitedAttackType(EAttackType::FURY_AIRBORNE);
		}
		else if(m_bPhase2TurnProcessed)
		{
			m_pTransformCom->RotateToDirectionSmoothly(XMLoadFloat3(&m_vPhase2TurnDir), fTimeDelta);
		}
	}
}

void CFuoco::EnableColliders(_bool bEnable)
{
	__super::EnableColliders(bEnable);
	if (bEnable)
	{
		m_pPhysXActorComForArm->Set_SimulationFilterData(m_pPhysXActorComForArm->Get_FilterData());
		m_pPhysXActorComForFoot->Set_SimulationFilterData(m_pPhysXActorComForFoot->Get_FilterData());
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			if (auto pController = pPlayer->Get_Controller())
			{
				pController->Remove_IgnoreActors(m_pPhysXActorComForArm->Get_Actor());
				pController->Remove_IgnoreActors(m_pPhysXActorComForFoot->Get_Actor());
			}
		}
	}
	else
	{
		m_pPhysXActorComForArm->Init_SimulationFilterData();
		m_pPhysXActorComForFoot->Init_SimulationFilterData();
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			if (auto pController = pPlayer->Get_Controller())
			{
				pController->Add_IngoreActors(m_pPhysXActorComForArm->Get_Actor());
				pController->Add_IngoreActors(m_pPhysXActorComForFoot->Get_Actor());
			}
		}
	}
}

_bool CFuoco::CanProcessTurn()
{
	// 2페이즈이고 중앙으로 턴을 기다리는 중이면 false
	return !(m_bIsPhase2 && m_bWaitPhase2Rotate);
}


void CFuoco::SetupAttackByType(_int iPattern)
{

	switch (iPattern)
	{
	case Client::CFuoco::SlamCombo:
	{
		_bool bIsCombo = GetRandomInt(0, 1) == 1;
		m_pAnimator->SetBool("IsCombo", bIsCombo);
		if (bIsCombo)
		{
			_int iDir = GetYawSignFromDiretion();
			m_pAnimator->SetInt("Direction", iDir);
		}
	}
	break;
	case Client::CFuoco::SwingAtk:
	{
		_bool bIsCombo = GetRandomInt(0, 1) == 1;
		m_pAnimator->SetBool("IsCombo", bIsCombo);
		if (bIsCombo)
		{
				_int iComboType;

				if (m_iLastComboType == -1) // 첫 시작은 랜덤
					iComboType = GetRandomInt(0, 1);
				else
					iComboType = 1 - m_iLastComboType; // 이전 값과 반대로

				m_pAnimator->SetInt("SwingCombo", iComboType);
				m_iLastComboType = iComboType;
		}
		m_eAttackType = EAttackType::KNOCKBACK;
	}
	break;
	case Client::CFuoco::SlamFury:
		SetTurnTimeDuringAttack(2.f, 1.5f);
		m_eAttackType = EAttackType::FURY_STAMP;
	case Client::CFuoco::FootAtk:
		m_eAttackType = EAttackType::AIRBORNE;
		break;
	case Client::CFuoco::SlamAtk:
		SetTurnTimeDuringAttack(1.f);
		m_eAttackType = EAttackType::STAMP;
		break;
	case Client::CFuoco::Uppercut:
		SetTurnTimeDuringAttack(1.f);
		m_eAttackType = EAttackType::NORMAL;
		break;
	case Client::CFuoco::StrikeFury:
		m_eAttackType = EAttackType::FURY_AIRBORNE;
		break;
	case Client::CFuoco::P2_FireFlame:
	{
		_int iDir = GetYawSignFromDiretion();
		m_pAnimator->SetInt("Direction", iDir);
		m_eAttackType = EAttackType::NONE;
	}
	break;
	case Client::CFuoco::P2_FireOil:
		m_eAttackType = EAttackType::NONE;
		break;
	case Client::CFuoco::P2_FireBall:
	{
		_int iDir = GetRandomInt(0, 2);
		m_pAnimator->SetInt("Direction", iDir);
		m_eAttackType = EAttackType::KNOCKBACK;
	}
	break;
	case Client::CFuoco::P2_FireBall_B:
		m_eAttackType = EAttackType::KNOCKBACK;
		break;
	default:
		break;
	}
	if (iPattern == Client::CFuoco::SlamFury)
	{
		m_bRootMotionClamped = true;
	}
	else
	{
		m_bRootMotionClamped = false;
	}
	ApplyAttackTypeToPlayer(m_eAttackType);
}

void CFuoco::Register_Events()
{
	if (nullptr == m_pAnimator)
		return;

	CEliteUnit::Register_Events();

	m_pAnimator->RegisterEventListener("IsFront",
		[this]()
		{
			if (IsTargetInFront(5.f))
			{
				m_pAnimator->SetBool("IsFront", true);
				SetTurnTimeDuringAttack(1.f);
			}
			else
			{
				m_pAnimator->SetBool("IsFront", false);
			}
		});
	m_pAnimator->RegisterEventListener("Turnning", [this]()
		{
			_bool bIsFront = IsTargetInFront(180.f,20.f);

			if (bIsFront == false)
			{
				SetTurnTimeDuringAttack(1.5f, 1.4f);
			}
			else
			{
				SetTurnTimeDuringAttack(1.3f);
			}

		});

	m_pAnimator->RegisterEventListener("ActiveHpBar", [this]()
		{
			if (m_pHPBar)
			{
				m_pHPBar->Set_RenderTime(0.0016f);
				return;
			}

			CUI_MonsterHP_Bar::HPBAR_DESC eDesc{};
			eDesc.strName = TEXT("왕의 불꽃 푸오코");
			eDesc.isBoss = true;
			eDesc.pHP = &m_fHp;
			eDesc.pIsGroggy = &m_bGroggyActive;

			m_pHPBar = static_cast<CUI_MonsterHP_Bar*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT,
				ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_HPBar"), &eDesc));

		});


	m_pAnimator->RegisterEventListener("CollidersOff", [this]() {
		EnableColliders(false);
		});
	m_pAnimator->RegisterEventListener("CollidersOn", [this]() {
		EnableColliders(true);
		});

	m_pAnimator->RegisterEventListener("ColliderArmOn", [this]()
		{
			m_pPhysXActorComForArm->Set_SimulationFilterData(m_pPhysXActorComForArm->Get_FilterData());
		});
	m_pAnimator->RegisterEventListener("ColliderArmOff", [this]()
		{
			m_pPhysXActorComForArm->Init_SimulationFilterData();
		});
	m_pAnimator->RegisterEventListener("ColliderFootOn", [this]()
		{
			m_pPhysXActorComForFoot->Set_SimulationFilterData(m_pPhysXActorComForFoot->Get_FilterData());
			if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
			{
				if (auto pController = pPlayer->Get_Controller())
				{
					pController->Remove_IgnoreActors(m_pPhysXActorComForFoot->Get_Actor());
				}
			}
		});
	m_pAnimator->RegisterEventListener("ColliderFootOff", [this]()
		{
			m_pPhysXActorComForFoot->Init_SimulationFilterData();
			if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
			{
				if (auto pController = pPlayer->Get_Controller())
				{
					pController->Add_IngoreActors(m_pPhysXActorComForFoot->Get_Actor());
				}
			}
		});

	m_pAnimator->RegisterEventListener("FireBall", [this]()
		{
			if (m_pPlayer == nullptr)
				return;
			EffectSpawn_Active(EF_FIRE_BALL, true);
			FireProjectile(ProjectileType::FireBall, 24.5f);
		});

	m_pAnimator->RegisterEventListener("FireOilFirst", [this]()
		{
			FireProjectile(ProjectileType::Oil);
		});
	m_pAnimator->RegisterEventListener("FireOilSecond", [this]()
		{
			FireProjectile(ProjectileType::Oil, 5.f);
		});

	m_pAnimator->RegisterEventListener("FireBallCombo", [this]()
		{
			_bool bIsCombo = GetRandomInt(0, 99) < 85;
			if (m_iFireBallComboCount == LIMIT_FIREBALL_COMBO_COUNT)
			{
				m_iFireBallComboCount = 0;
				return;
			}
			if (bIsCombo)
			{
				_int iDir = GetRandomInt(0, 2);
				switch (iDir)
				{
				case 0: // 왼쪽
					m_pAnimator->SetInt("Direction", iDir);
					m_pAnimator->Get_CurrentAnimController()->SetState(ENUM_CLASS(BossStateID::P2_ATK_FIRE_BALL_L));
					break;
					break;
				case 1: // 오른쪽
					m_pAnimator->SetInt("Direction", iDir);
					m_pAnimator->Get_CurrentAnimController()->SetState(ENUM_CLASS(BossStateID::P2_ATK_FIRE_BALL_R));
					break;
				case 2: // 중앙
					m_pAnimator->SetInt("Direction", iDir);
					m_pAnimator->Get_CurrentAnimController()->SetState(ENUM_CLASS(BossStateID::P2_ATK_FIRE_BALL_F));
				}
				m_iFireBallComboCount++;

			}
			else
			{
				m_iFireBallComboCount = 0;
			}
		});

	m_pAnimator->RegisterEventListener("Flamethrower", [this]()
		{
			m_fFireFlameDuration = 1.7f;
			FlamethrowerAttack(15.f);
		});

	m_pAnimator->RegisterEventListener("IgnorePlayerCollision", [this]()
		{
			if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
			{
				if (auto pController = pPlayer->Get_Controller())
				{
					pController->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
				}
			}
		});

	m_pAnimator->RegisterEventListener("BeginStrikeFuryCollisionFlag", [this]()
		{
			if (m_pPhysXActorCom)
			{
				m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
			}
		});

	m_pAnimator->RegisterEventListener("EndStrikeFuryCollisionFlag", [this]()
		{
			if (m_pPhysXActorCom)
			{
				m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::MONSTER);
			}
		});

	m_pAnimator->RegisterEventListener("SpawnFlameField", [this]()
		{
			SpawnFlameField();
			EFFECT_MANAGER->Set_Dead_EffectContainer(TEXT("Fuoco_FieldBellyFire"));
			CEffectContainer::DESC Desc = {};

			_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);


			XMStoreFloat4x4(&Desc.PresetMatrix, XMMatrixTranslation(vPos.m128_f32[0], vPos.m128_f32[1] + 0.5f, vPos.m128_f32[2]));
			CGameObject* pEC = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Fuoco_FlameField_Erupt_P1"), &Desc);
			if (pEC == nullptr)
				MSG_BOX("이펙트 생성 실패함");
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_SK_FlameThrow_Short_", 3);
		});

	m_pAnimator->RegisterEventListener("OnGroundScratchEffect", [this]()
		{

			EffectSpawn_Active(EF_SWING_ATK, true, false);
		});

	m_pAnimator->RegisterEventListener("DecalScratchEffect", [this]()
		{
			Spawn_Decal(m_pFistBone, 
				TEXT("Prototype_Component_Texture_FireEater_Scratch_Normal"), 
				TEXT("Prototype_Component_Texture_FireEater_Scratch_Mask"),
			XMVectorSet(10.f, 0.5f, 5.f, 0));

		});

	m_pAnimator->RegisterEventListener("OffGroundScratchEffect", [this]()
		{
			EffectSpawn_Active(EF_SWING_ATK, false);
		});

	m_pAnimator->RegisterEventListener("OnRollingSparkEffect", [this]()
		{
			EffectSpawn_Active(EF_SWING_ATK_SEQ, true);
		});

	m_pAnimator->RegisterEventListener("OnSlamEffect", [this]()
		{
			EffectSpawn_Active(EF_SLAM, true);

		});

	m_pAnimator->RegisterEventListener("OnFlamethrowerEffect", [this]()
		{
			EffectSpawn_Active(EF_FIRE_FLAME, true);
		});


	m_pAnimator->RegisterEventListener("OnCutSceneEffect", [this]()
		{
			EffectSpawn_Active(EF_CUTSCENE, true);
		});

	m_pAnimator->RegisterEventListener("SetRootStep", [this]()
		{
			m_fRootMotionAddtiveScale = 7.f;
			m_fMaxRootMotionSpeed = 40.f;
		});

	m_pAnimator->RegisterEventListener("ResetRootStep", [this]()
		{
			m_fMaxRootMotionSpeed = 18.f;
			m_fRootMotionAddtiveScale = 1.2f;
		});


	m_pAnimator->RegisterEventListener("BellyFireSpawn", [this]()
		{
			CEffectContainer::DESC BellyFireDesc = {};

			_matrix combmat = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bone001-Ball01"))) * m_pTransformCom->Get_WorldMatrix();
			_vector vPos = combmat.r[3];
			_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
			vPos += vLook * 0.6f;
			XMStoreFloat4x4(&BellyFireDesc.PresetMatrix, XMMatrixTranslation(vPos.m128_f32[0], vPos.m128_f32[1] + 0.5f, vPos.m128_f32[2]));

			CGameObject* pEC = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Fuoco_FlameField_BellyFire_P2"), &BellyFireDesc);
			if (pEC == nullptr)
				MSG_BOX("이펙트 생성 실패함");
			EFFECT_MANAGER->Store_EffectContainer(TEXT("Fuoco_FieldBellyFire"), static_cast<CEffectContainer*>(pEC));
		});

	m_pAnimator->RegisterEventListener("Phase2TurnFinished", [this]()
		{
			if (m_bIsPhase2)
			{
				m_bPhase2TurnFinished = true;
			}
		});

	m_pAnimator->RegisterEventListener("ActivateFuryState", [this]()
		{
			m_eFuryState = EFuryState::Fury;
		});

	m_pAnimator->RegisterEventListener("DeactivateFuryState", [this]()
		{
			m_eFuryState = EFuryState::None;
		});

	m_pAnimator->RegisterEventListener("DeadSoundEffect", [this]()
		{
			if(m_pSoundCom)
				m_pSoundCom->Play_Random("VO_NPC_NHM_Boss_Fire_Eater_Dead_", 3);
		});


}

void CFuoco::Ready_AttackPatternWeightForPhase1()
{
	m_PatternWeightMap.clear();
	m_PatternCountMap.clear();
	vector<EBossAttackPattern> m_vecBossPatterns = {
		SlamCombo,Uppercut,SwingAtk,SwingAtkSeq,SlamFury,FootAtk,
		SlamAtk,StrikeFury
	};

	for (const auto& pattern : m_vecBossPatterns)
	{
		m_PatternWeightMap[pattern] = m_fBasePatternWeight;
		m_PatternCountMap[pattern] = 0;
	}
}

void CFuoco::Ready_AttackPatternWeightForPhase2()
{
	if (m_eCurrentState == EEliteState::FATAL
		 || m_eCurrentState == EEliteState::PARALYZATION)
		return;
	EffectSpawn_Active(EF_SWING_ATK, false);
	m_pAnimator->SetTrigger("Paralyzation");
	m_pAnimator->SetPlayRate(1.f);
	m_bGroggyActive = false;
	m_fGroggyGauge = 0.f;
	m_bStartPhase2 = true;
	vector<EBossAttackPattern> m_vecBossPatterns = {
		SlamCombo,SwingAtk,SwingAtkSeq,SlamFury,
		StrikeFury,P2_FireOil,P2_FireBall,P2_FireFlame,
		P2_FireBall_B
	};
	m_PatternWeightMap.clear();
	m_PatternCountMap.clear();
	for (const auto& pattern : m_vecBossPatterns)
	{
		if (pattern == P2_FireOil)
		{
			m_PatternWeightMap[pattern] = m_fBasePatternWeight * 4.f;;
		}
		else if (pattern == P2_FireBall || pattern == P2_FireBall_B)
		{
			m_PatternWeightMap[pattern] = m_fBasePatternWeight * 2.2f;
		}
		else if (pattern == P2_FireFlame)
		{

			m_PatternWeightMap[pattern] = m_fBasePatternWeight * 0.8f;
		}
		else
		{
			m_PatternWeightMap[pattern] = m_fBasePatternWeight * 0.3f;
		}

		m_PatternCountMap[pattern] = 0;
	}
	SwitchFury(false, 1.f); 
	m_pSoundCom->Play("Dialog_CH03_PhaseChange_01_text_3");
}

_int CFuoco::GetRandomAttackPattern(_float fDistance)
{
	EBossAttackPattern ePattern = BAP_NONE;
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
			ePattern = static_cast<EBossAttackPattern>(pattern);
			m_ePrevAttackPattern = m_eCurAttackPattern;
			m_eCurAttackPattern = ePattern;
			UpdatePatternWeight(ePattern);
			break;
		}
	}
	return ePattern;
}

void CFuoco::ChosePatternWeightByDistance(_float fDistance)
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
	else if (fDistance >= ATTACK_DISTANCE_MIDDLE && fDistance < ATTACK_DISTANCE_FAR)
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
	else if (fDistance >= ATTACK_DISTANCE_FAR)
	{
		for (auto& [pattern, weight] : m_PatternWeighForDisttMap)
		{
			auto it = find(m_vecFarAttackPatterns.begin(), m_vecFarAttackPatterns.end(), pattern);
			if (it == m_vecFarAttackPatterns.end())
			{
				m_PatternWeighForDisttMap[pattern] *= 0.f;
			}
		}
	}
}

void CFuoco::FireProjectile(ProjectileType type, _float fSpeed)
{
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	CProjectile::PROJECTILE_DESC desc{};
	_int iLevelIndex = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);
	switch (type)
	{
	case Client::CFuoco::ProjectileType::FireBall:
	{
		if (m_pPlayer == nullptr)
			return;
		if (m_pLeftBone)
		{
			auto handLocalMatrix = m_pLeftBone->Get_CombinedTransformationMatrix();
			auto handWorldMatrix = XMLoadFloat4x4(handLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
			_float3 localOffset = { 0.0f, 0.f, 100.f };
			_vector spawnPos = XMVector3TransformCoord(XMLoadFloat3(&localOffset), handWorldMatrix);
			vPos = XMVectorSetW(spawnPos, 1.f);
		}
		else
		{
			// 손이 없으면 기본 위치 사용
			vPos = m_pTransformCom->Get_State(STATE::POSITION);
		}


		// 발사 각도 구하기 (속도, 위치, 중력 가속도를 알고 있을 때 각도를 구하는 공식)
		const _float G = 9.81f; // 중력 가속도
		_vector vTargetPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
		vTargetPos = XMVectorSetY(vTargetPos, XMVectorGetY(vTargetPos) + 0.7f);
		_vector vHorizontalStartPos = XMVectorSetY(vPos, 0.0f);
		_vector vHorizontalTargetPos = XMVectorSetY(vTargetPos, 0.0f);

		// 수평 거리 계산
		_float fHorizontalDist = XMVectorGetX(XMVector3Length(vHorizontalTargetPos - vHorizontalStartPos));
		// 높이 차이
		_float fYDiff = XMVectorGetY(vTargetPos) - XMVectorGetY(vPos);

		_float fLaunchAngle = 0.0f;

		_float fVelocitySquared = fSpeed * fSpeed; // v^2
		// 스피드가 항상 양수라서 도달 안하는 경우는 없을듯 (혹시나 나중에 넘지 못하는 각도라면 음수 체크 해야할듯)
		// 델타 = v^4 - g*(g*x^2 + 2y*v^2) x는 dist 
		_float fDeltaValue = (fVelocitySquared * fVelocitySquared) - G * ((G * fHorizontalDist * fHorizontalDist) + (2 * fYDiff * fVelocitySquared));

		_float fTanThetaLow = (fVelocitySquared - sqrtf(fDeltaValue)) / (G * fHorizontalDist); // 낮은 발사각의 해로 처리 
		fLaunchAngle = atanf(fTanThetaLow); // 라디안 각도 계산


		_float fCostheta = cosf(fLaunchAngle);
		_float fSintheta = sinf(fLaunchAngle);

		_vector vHorizontalDir = XMVector3Normalize(vHorizontalTargetPos - vHorizontalStartPos);
		_vector vInitialVelocity = vHorizontalDir * (fSpeed * fCostheta) + XMVectorSet(0.0f, fSpeed * fSintheta, 0.0f, 0.0f);
		_float fTotalTime = fHorizontalDist / (fSpeed * fCostheta) + 7.f; // 0.5초 추가 라이프 타임


		XMStoreFloat3(&desc.vPos, vPos);
		XMStoreFloat3(&desc.vDir, XMVector3Normalize(vInitialVelocity));

		desc.fLifeTime = fTotalTime;
		desc.fGravityOnDist = 0.f;
		desc.fSpeed = fSpeed;
		desc.fGravityOnDist = 0.f;
		desc.fStartTime = 0.f;
		desc.bUseTimeTrigger = true;
		desc.bUseDistTrigger = false;
		desc.fRadius = 0.33f;
		lstrcpy(desc.szName, TEXT("FireBall"));


		if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_FireBall"), iLevelIndex, TEXT("Layer_Projectile"), &desc)))
		{
			return;
		}
	}
	break;
	case Client::CFuoco::ProjectileType::Oil:
	{

		if (m_pCannonBone)
		{
			auto cannonLocalMatrix = m_pCannonBone->Get_CombinedTransformationMatrix();
			auto cannonWorldMatrix = XMLoadFloat4x4(cannonLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
			_float3 localOffset = { 0.0f, 0.0f, 0.f };
			_vector spawnPos = XMVector3TransformCoord(XMLoadFloat3(&localOffset), cannonWorldMatrix);
			vPos = XMVectorSetW(spawnPos, 1.f);
		}
		else
		{
			vPos = m_pTransformCom->Get_State(STATE::POSITION);
		}
		XMStoreFloat3(&desc.vPos, vPos);
		desc.fStartTime = 0.6f;
		desc.bUseTimeTrigger = true;
		desc.bUseDistTrigger = false;
		desc.fRadius = 0.2f;
		desc.fLifeTime = 3.f;
		lstrcpy(desc.szName, TEXT("Oil"));
		_vector vBaseDir = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
		const _float fAngle = 7.f;
		_vector vLeftDirOfBase = XMVector3Rotate(vBaseDir, XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-fAngle)));
		_vector vRightDirOfBase = XMVector3Rotate(vBaseDir, XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(fAngle)));
		_vector vUpDirOfBase = XMVector3Rotate(vBaseDir, XMQuaternionRotationAxis(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(-fAngle)));
		_vector vDownDirOfBase = XMVector3Rotate(vBaseDir, XMQuaternionRotationAxis(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(fAngle)));
		array<_float3, 4> vDirArray;
		XMStoreFloat3(&vDirArray[0], vDownDirOfBase);
		XMStoreFloat3(&vDirArray[1], vLeftDirOfBase);
		XMStoreFloat3(&vDirArray[2], vRightDirOfBase);
		XMStoreFloat3(&vDirArray[3], vUpDirOfBase);

		for (_int i = 0; i < 4; i++)
		{
			desc.vDir = vDirArray[i];
			// 스피드 랜덤 패턴
			_float fRandomSpeed = fSpeed * GetRandomFloat(0.9f, 1.5f);
			desc.fSpeed = fRandomSpeed;
			// 나중에 Oil로 바꾸기
			if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_Oil"), iLevelIndex, TEXT("Layer_Projectile"), &desc)))
			{
				return;
			}
		}
	}
	break;
	default:
		break;
	}
}

void CFuoco::FlamethrowerAttack(_float fConeAngle, _int iRayCount, _float fDistance)
{
	if (!m_pLeftBone)
		return;

	auto handLocalMatrix = m_pLeftBone->Get_CombinedTransformationMatrix();
	auto handWorldMatrix = XMLoadFloat4x4(handLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
	_vector vOrigin = XMVector3TransformCoord(XMVectorSet(0.f, 0.f, 100.f, 1.f), handWorldMatrix); // 손 위치에서 시작
	_float4x4 worldMatrix{};
	XMStoreFloat4x4(&worldMatrix, handWorldMatrix);

	PxVec3 origin(XMVectorGetX(vOrigin), XMVectorGetY(vOrigin), XMVectorGetZ(vOrigin));
	PxVec3 vDir = PxVec3(worldMatrix._31, worldMatrix._32, worldMatrix._33); // 손의 Look 방향
	PxVec3 vRight = PxVec3(worldMatrix._11, worldMatrix._12, worldMatrix._13); // 손의 Right 방향
	vDir.normalize();
	vRight.normalize();
	const _float pitchBiasDeg = 1.f;                
	PxQuat qBias(XMConvertToRadians(pitchBiasDeg), vRight);
	PxVec3 vDirBiased = qBias.rotate(vDir);

	PxHitFlags hitFlags(PxHitFlag::eDEFAULT);
	PxQueryFilterData filterData;
	filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
	unordered_set<PxActor*> ignoreActors;
	ignoreActors.insert(m_pPhysXActorCom->Get_Actor());
	ignoreActors.insert(m_pPhysXActorComForArm->Get_Actor());
	ignoreActors.insert(m_pPhysXActorComForFoot->Get_Actor());

	CIgnoreSelfCallback callback(ignoreActors);

	for (_int i = 0; i < iRayCount; i++)
	{
		_float fCurAngle = -fConeAngle * 0.5f + (fConeAngle / (iRayCount - 1)) * i; // -세타 ~ 세타
		PxQuat vRot = PxQuat(XMConvertToRadians(fCurAngle), vRight); // 회전 쿼터니언 생성
		PxVec3 vRayDir = vRot.rotate(vDirBiased); // Look 방향을 회전

		PxRaycastBuffer hit;
		if (m_pGameInstance->Get_Scene()->raycast(origin, vRayDir, fDistance, hit, hitFlags, filterData, &callback))
		{
			if (hit.hasBlock)
			{
				PxRigidActor* hitActor = hit.block.actor;
				PxVec3 hitPos = hit.block.position;
				PxVec3 hitNormal = hit.block.normal;
				CPhysXActor* pHitActor = static_cast<CPhysXActor*>(hitActor->userData);

				if (pHitActor && pHitActor->Get_Owner())
				{
					if (nullptr == pHitActor->Get_Owner())
						return;

					if (pHitActor->Get_Owner()->Get_Name() == TEXT("Player"))
					{
						if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
						{
							pPlayer->SetElementTypeWeight(EELEMENT::FIRE, 0.4f);
							pPlayer->SetHitMotion(HITMOTION::NONE_MOTION);
						}
					}
					else if (pHitActor->Get_Owner()->Get_Name() == TEXT("Oil"))
					{
						if (auto pOil = dynamic_cast<COil*>(pHitActor->Get_Owner()))
						{
							pOil->Explode_Oil();
						}
					}
				}

				m_bRayHit = true;
				m_vRayHitPos = hitPos;
		
			}

#ifdef _DEBUG
			if (m_pGameInstance->Get_RenderCollider())
			{
				DEBUGRAY_DATA _data{};
				_data.vStartPos = origin;
				_data.vDirection = vRayDir;
				_data.fRayLength = (hit.hasBlock ? hit.block.distance : fDistance); // 맞으면 hit 거리, 아니면 지정 거리
				_data.bIsHit = hit.hasBlock;
				_data.vHitPos = hit.hasBlock ? hit.block.position : (origin + vRayDir * fDistance);
				m_pPhysXActorCom->Add_RenderRay(_data);
			}
#endif

		}
	}

}

void CFuoco::SpawnFlameField()
{
	_int iLevelIndex = m_pGameInstance->GetCurrentLevelIndex();
	CFlameField::FLAMEFIELD_DESC Desc{};
	XMStoreFloat3(&Desc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	Desc.fExpandRadius = 13.f; // 확장 반경
	Desc.fExpandTime = 1.5f; // 확장까지 끝나야 하는 시간
	if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_FlameField"), iLevelIndex, TEXT("Layer_FlameField"), &Desc)))
	{
		return;
	}

}

void CFuoco::Ready_EffectNames()
{
	m_EffectMap[EF_CUTSCENE].emplace_back(TEXT("EC_Fuoco_Cutscene_Slam"));
	// Phase 1

	m_EffectMap[EF_SWING_ATK].emplace_back(TEXT("EC_Fuoco_Spin3_FloorFountain_P5"));
	m_EffectMap[EF_SWING_ATK].emplace_back(TEXT("EC_Fuoco_Spin3_HandSpark_P1"));
	m_EffectMap[EF_SLAM].emplace_back(TEXT("EC_Fuoco_Slam_Imsi_P2"));
	m_EffectMap[EF_SWING_ATK_SEQ].emplace_back(TEXT("EC_Fuoco_SpinReady_HandSpark_P2"));
	m_EffectMap[EF_LASTSPIN].emplace_back(TEXT("EC_Fuoco_Spin3_LastSpinFlame_S1P1_wls"));
	// Phase 2
	m_EffectMap[EF_FIRE_FLAME].emplace_back(TEXT("EC_Fuoco_FlameThrow_P1"));
	m_EffectMap[EF_FIRE_BALL].emplace_back(TEXT("EC_Fuoco_Spawn_Fireball"));

}


void CFuoco::ProcessingEffects(const _wstring& stEffectTag)
{
	if (m_pFistBone == nullptr)
		return;

	CEffectContainer::DESC desc = {};
	if (stEffectTag == TEXT("EC_Fuoco_Spin3_FloorFountain_P5"))
	{
		auto worldmat = XMLoadFloat4x4(m_pFistBone->Get_CombinedTransformationMatrix()) * m_pTransformCom->Get_WorldMatrix();
		_vector rot, trans, scale;
		XMMatrixDecompose(&scale, &rot, &trans, worldmat);

		_vector finalRot = XMQuaternionMultiply(XMQuaternionInverse(rot), XMQuaternionRotationAxis(XMVectorSet(0.f, 0.f, 1.f, 0.f), XMConvertToRadians(-60.f)));

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixRotationQuaternion(finalRot) *
			XMMatrixTranslation(worldmat.r[3].m128_f32[0],
				m_pTransformCom->Get_State(STATE::POSITION).m128_f32[1],
				worldmat.r[3].m128_f32[2]));
	}
	else if (stEffectTag == TEXT("EC_Fuoco_Spin3_HandSpark_P1"))
	{
		auto worldmat = XMLoadFloat4x4(m_pFistBone->Get_CombinedTransformationMatrix()) * m_pTransformCom->Get_WorldMatrix();

		desc.pSocketMatrix = m_pFistBone->Get_CombinedTransformationMatrix();
		desc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixIdentity());

	}
	else if (stEffectTag == TEXT("EC_Fuoco_SpinReady_HandSpark_P2"))
	{
		desc.pSocketMatrix = m_pFistBone->Get_CombinedTransformationMatrix();
		desc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixIdentity());
	}
	else if (stEffectTag == TEXT("EC_Fuoco_Slam_Imsi_P2"))
	{
		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;
		const _float4x4* socketPtr = m_pFistBone->Get_CombinedTransformationMatrix();
		const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
		_matrix socket = XMLoadFloat4x4(socketPtr);
		_matrix parent = XMLoadFloat4x4(parentPtr);

		_matrix comb = socket * parent;

		_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);
		
		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(position));

		Spawn_Decal(m_pFistBone,
			TEXT("Prototype_Component_Texture_FireEater_Slam_Normal"),
			TEXT("Prototype_Component_Texture_FireEater_Slam_Mask"),
			XMVectorSet(10.f, 0.5f, 10.f, 0));

	}
	else if (stEffectTag == TEXT("EC_Fuoco_Spin3_LastSpinFlame_S1P1_wls"))
	{
		desc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-L-Finger0"));
		desc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixRotationAxis(_vector{ 1.f, 0.f, 0.f, 0.f }, XMConvertToRadians(90.f)) *
			XMMatrixTranslation(0.f, 0.f, 1.5f));
	}
	else if (stEffectTag == TEXT("EC_Fuoco_FlameThrow_P1"))
	{
		desc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-L-Finger0"));
		desc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixRotationAxis(XMVector3Normalize(_vector{ 1.f, -0.3f, 0.f, 0.f }), XMConvertToRadians(90.f)) *
			XMMatrixTranslation(0.f, 0.f, 1.5f));
	}
	else if (stEffectTag == TEXT("EC_Fuoco_Spawn_Fireball"))
	{
		auto worldmat = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-L-Finger0")))
			* m_pTransformCom->Get_WorldMatrix();
		_vector vHandPos = worldmat.r[3];
		_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
		_vector vDir = XMVector3Normalize(vPlayerPos - vHandPos);
		_vector vOffsetPos = vHandPos + vDir * 1.f;
		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixScaling(1.5f, 1.5f, 1.5f) *
			XMMatrixTranslation(vOffsetPos.m128_f32[0], vOffsetPos.m128_f32[1], vOffsetPos.m128_f32[2]));

	}
	else if(stEffectTag == TEXT("EC_Fuoco_Cutscene_Slam"))
	{
		const _float4x4* socketPtr = m_pFistBone->Get_CombinedTransformationMatrix();
		const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
		_matrix socket = XMLoadFloat4x4(socketPtr);
		_matrix parent = XMLoadFloat4x4(parentPtr);

		_matrix comb = socket * parent;

		_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(position));
	}
	else
	{
		return;
	}

	if (MAKE_EFFECT(ENUM_CLASS(m_iLevelID), stEffectTag, &desc) == nullptr)
		MSG_BOX("이펙트 생성 실패함");
}

HRESULT CFuoco::EffectSpawn_Active(_int iEffectId, _bool bActive, _bool bIsOnce) // 어떤 이펙트를 스폰할지 결정
{
	auto it = m_EffectMap.find(iEffectId);
	if (it == m_EffectMap.end())
		return E_FAIL; // 해당 패턴 이펙트 없음

	const vector<_wstring>& effectTags = it->second;

	if (bActive)
	{
		// 여러 개 이펙트를 동시에 등록
		for (const auto& effectTag : effectTags)
		{
			m_ActiveEffect.push_back({ effectTag, bIsOnce });
		}
	}
	else
	{
		for (auto itEff = m_ActiveEffect.begin(); itEff != m_ActiveEffect.end(); )
		{
			// 여러 개니까 전부 비교해서 삭제
			if (find(effectTags.begin(), effectTags.end(), itEff->first) != effectTags.end())
				itEff = m_ActiveEffect.erase(itEff);
			else
				++itEff;
		}
	}


	return S_OK;
}

HRESULT CFuoco::Spawn_Effect() // 이펙트를 스폰 (대신 각각의 로직에 따라서 함수 호출)
{
	if (m_ActiveEffect.empty())
		return S_OK;

	for (auto it = m_ActiveEffect.begin(); it != m_ActiveEffect.end(); )
	{
		const _wstring& EffectTag = it->first;
		ProcessingEffects(EffectTag);
		if (it->second) // 한번만 실행이면
		{
			it = m_ActiveEffect.erase(it);
		}
		else
		{
			++it;
		}
	}
	return S_OK;
}

HRESULT CFuoco::Ready_Effect()
{

	// Static 이펙트

	CEffectContainer::DESC BellyFireDesc = {};
	BellyFireDesc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bone001-Ball01"));

	BellyFireDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	XMStoreFloat4x4(&BellyFireDesc.PresetMatrix, XMMatrixIdentity());
	CGameObject* pEC = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Fuoco_BellyFire_P1S2"), &BellyFireDesc);
	if (pEC == nullptr)
		MSG_BOX("이펙트 생성 실패함");

	EFFECT_MANAGER->Store_EffectContainer(TEXT("Fuoco_BellyFire"), static_cast<CEffectContainer*>(pEC));

	/**************************************************/
	pEC = nullptr;
	CEffectContainer::DESC HeadSmokeDesc = {};
	HeadSmokeDesc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bone001-Head01"));
	HeadSmokeDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	XMStoreFloat4x4(&HeadSmokeDesc.PresetMatrix, XMMatrixRotationAxis(_vector{ 0.f, 0.f, 1.f, 0.f }, XMConvertToRadians(-90.f)) * XMMatrixTranslation(1.5f, -0.45f, -0.4f));
	pEC = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Fuoco_HeadSmoke_P2"), &HeadSmokeDesc);
	if (pEC == nullptr)
		MSG_BOX("이펙트 생성 실패함");

	EFFECT_MANAGER->Store_EffectContainer(TEXT("Fuoco_HeadSmoke1"), static_cast<CEffectContainer*>(pEC));

	pEC = nullptr;
	XMStoreFloat4x4(&HeadSmokeDesc.PresetMatrix, XMMatrixRotationAxis(_vector{ 0.f, 0.f, 1.f, 0.f }, XMConvertToRadians(-90.f)) * XMMatrixTranslation(1.5f, -0.45f, 0.45f));
	pEC = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Fuoco_HeadSmoke_Small_P1"), &HeadSmokeDesc);
	if (pEC == nullptr)
		MSG_BOX("이펙트 생성 실패함");

	EFFECT_MANAGER->Store_EffectContainer(TEXT("Fuoco_HeadSmoke2"), static_cast<CEffectContainer*>(pEC));



	/************************ 소드 트레일 이펙트 **************************/
	CSwordTrailEffect::DESC desc = {};
	desc.pParentCombinedMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	desc.iLevelID = m_iLevelID;

	desc.pInnerSocketMatrix = m_pRForearmBone->Get_CombinedTransformationMatrix();
	desc.pOuterSocketMatrix = m_pMiddleFingierBone->Get_CombinedTransformationMatrix();
	m_pTrailEffect = dynamic_cast<CSwordTrailEffect*>(MAKE_SINGLEEFFECT(ENUM_CLASS(m_iLevelID), TEXT("TE_FireEater"), TEXT("Layer_Effect"), 0.f, 0.f, 0.f, &desc));
	if (!m_pTrailEffect)
		return E_FAIL;
	m_pTrailEffect->Set_TrailActive(false);
	return S_OK;
}

void CFuoco::Ready_SoundEvents()
{
	if (m_pSoundCom)
	{
		m_pSoundCom->Set3DState(0.f, 60.f);
		m_pSoundCom->SetVolume(1.f);
	}
	m_pAnimator->RegisterEventListener("MoveSound", [this]()
	{
	   m_pSoundCom->Play_Random("SE_NPC_FS_Boss_Fire_Eater_Stone_", 4); 
	   m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_Movement_", 6);
	   m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_Rustle_0", 6,1);
	}
	);

	m_pAnimator->RegisterEventListener("LArm_RustleSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_LArm_Rustle_0", 6,1);
		}
	);

	m_pAnimator->RegisterEventListener("RattleSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_Rattle_0", 9, 1);
		}
	);

	m_pAnimator->RegisterEventListener("BreakSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_Break_0", 3, 1);
		}
	);

	m_pAnimator->RegisterEventListener("BodyFallSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_Bodyfall_0", 6, 1);
		}
	);

	m_pAnimator->RegisterEventListener("RecoilSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_Recoil_0", 3,1);

		}
	);

	m_pAnimator->RegisterEventListener("FireBallSound", [this]()
	{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_SK_PJ_Fire_Ball_Shot_", 3); 
			m_pSoundCom->Play_Random("SE_NPC_SK_PJ_Fireball_Firing_0", 3,1); 
		
	}
	);

	m_pAnimator->RegisterEventListener("FlamethrowerStartSound", [this]() // 소리가 안나옴
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_SK_FlameThrow_Start_", 3); }
	);

	m_pAnimator->RegisterEventListener("FlamethrowerLoopSound", [this]()
		{
			m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_SK_FlameThrow_Loop_0"); }
	);

	m_pAnimator->RegisterEventListener("FlamethrowerEndSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_SK_FlameThrow_End_", 3); }
	);

	m_pAnimator->RegisterEventListener("StrikeSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_SK_RArm_Roll_", 3); }
	);

	m_pAnimator->RegisterEventListener("RArmFallSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_RArm_Fall_0", 3,1); }
	);

	m_pAnimator->RegisterEventListener("RunSound", [this]()
		{
			m_pSoundCom->Play_Random("VO_NPC_NHM_Boss_Fire_Eater_Run_", 3); }
	);

	m_pAnimator->RegisterEventListener("SlamSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_SK_RArm_Impact_", 6); }
	);

	m_pAnimator->RegisterEventListener("SwingAfterSalmSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_SK_WS_Short_", 3); }
	);
	m_pAnimator->RegisterEventListener("SwingAtkStartSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_SK_RArm_Roll_", 3); }
	);

	m_pAnimator->RegisterEventListener("FireOilSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_RArm_Put_", 3); }
	);

	m_pAnimator->RegisterEventListener("StandUpSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_Standup_", 6); }
	);

	m_pAnimator->RegisterEventListener("StartPhase1Sound", [this]()
		{
			m_pSoundCom->Play("Dialog_CH03_Attack_P1_01_text_3"); }
	);

	m_pAnimator->RegisterEventListener("FurnaceOpenSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_Furnace_Open_",3); }
	);

	m_pAnimator->RegisterEventListener("FurnaceCloseSound", [this]()
		{
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_Furnace_Close_", 3); }
	);

	m_pAnimator->RegisterEventListener("GroggySound", [this]()
		{
			m_pSoundCom->Play_Random("VO_NPC_NHM_Boss_Fire_Eater_Groggy_", 9); }
	);

	m_pAnimator->RegisterEventListener("SwingImpactSound", [this]()
		{
			switch (m_iCurNodeID)
			{
			case ENUM_CLASS(BossStateID::ATK_SWING_SEQ):
				m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_SK_WS_Long_0");
				break;
			case ENUM_CLASS(BossStateID::ATK_SWING_SEQ2):
				m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_SK_WS_Long_1");
				break;
			case ENUM_CLASS(BossStateID::ATK_SWING_SEQ3):
				m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_SK_WS_Long_2");
				m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_SK_FlameThrow_Short_",3);
				break;
			case ENUM_CLASS(BossStateID::ATK_SWING_L_COM1):
				m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_SK_WS_Long_0");
				break;
			case ENUM_CLASS(BossStateID::ATK_SWING_L_COM2):
				m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_SK_WS_Long_1");
				break;
			case ENUM_CLASS(BossStateID::ATK_SWING_R_COM1):
				m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_SK_WS_Long_1");
				break;
			case ENUM_CLASS(BossStateID::ATK_SWING_R_COM2):
				m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_SK_WS_Long_0");
				break;
			default:
				m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_SK_WS_Long_0");
				break;
			}
			m_pSoundCom->Play_Random("VO_NPC_NHM_Boss_Fire_Eater_Growl_", 3);
		});

	m_pAnimator->RegisterEventListener("FuryStartSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_MT_Mechanic_M_Land_01");
			}
		});

	m_pAnimator->RegisterEventListener("AttackSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play_Random("VO_NPC_NHM_Boss_Fire_Eater_Attack_",8);
			}
		});

	m_pAnimator->RegisterEventListener("RockDebrisSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_SK_FX_Rock_Debris_L_01");
				m_pSoundCom->Play("SE_NPC_MT_Dust_M_01");
			}
		});

}

void CFuoco::UpdatePatternWeight(_int iPattern)
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

_bool CFuoco::CheckConditionFlameField()
{
	if (m_bStartPhase2 || (!m_bUsedFlameFiledOnLowHp && CalculateCurrentHpRatio() <= 0.3f))
	{
		m_pAnimator->ResetTrigger("Attack");
		if (CalculateCurrentHpRatio() <= 0.3f && !m_bUsedFlameFiledOnLowHp)
		{
			m_pAnimator->SetInt("SkillType", P2_FlameField);
			m_bUsedFlameFiledOnLowHp = true;
			m_pAnimator->SetTrigger("Attack");
			m_pAnimator->SetPlayRate(1.f);

		}
		else
		{
			m_pAnimator->SetPlayRate(1.f);
			m_bWaitPhase2Rotate = true;
			m_bGroggyActive = false;
			m_fGroggyGauge = 0.f;
			m_bStartPhase2 = false;
			m_bIsPhase2 = true;
			m_pSoundCom->Play("Dialog_CH03_PhaseChange_01_text_3");
		}
		m_pAnimator->SetBool("Move", false);
		m_eCurrentState = EEliteState::ATTACK;
		EffectSpawn_Active(EF_SWING_ATK, false);
		return true;
	}
	return false;
}


void CFuoco::ReChallenge()
{
	__super::ReChallenge();

	if (m_pHPBar)
	{
		m_pHPBar->Set_RenderTime(0.0016f);
		return;
	}

	CUI_MonsterHP_Bar::HPBAR_DESC eDesc{};
	eDesc.strName = TEXT("왕의 불꽃 푸오코");
	eDesc.isBoss = true;
	eDesc.pHP = &m_fHp;
	eDesc.pIsGroggy = &m_bGroggyActive;

	m_pHPBar = static_cast<CUI_MonsterHP_Bar*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT,
		ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_HPBar"), &eDesc));
}

void CFuoco::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{

	__super::On_CollisionStay(pOther, eColliderType, HitPos, HitNormal);

}

void CFuoco::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	ReceiveDamage(pOther, eColliderType);

	if (eColliderType == COLLIDERTYPE::PLAYER_WEAPON)
	{
		m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_MT_Dmg_", 3);
	}

	if (auto pPlayer = dynamic_cast<CPlayer*>(pOther))
	{
		_uint curNodeID = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->iNodeId;
		switch (curNodeID)
		{
		case ENUM_CLASS(BossStateID::ATK_SLAM_COMBO_START):
		case ENUM_CLASS(BossStateID::ATK_SLAM_COMBO_LOOP):
		case ENUM_CLASS(BossStateID::ATK_SLAM_COMBO_END):
		case ENUM_CLASS(BossStateID::ATK_SLAM):
		case ENUM_CLASS(BossStateID::ATK_UPPERCUT_FRONT):
			pPlayer->SetHitMotion(HITMOTION::STAMP);
			pPlayer->SetfReceiveDamage(DAMAGE_HEAVY);
			pPlayer->SetHitedAttackType(EAttackType::STAMP);
			break;
		case ENUM_CLASS(BossStateID::ATK_SLAM_FURY):
			pPlayer->SetHitMotion(HITMOTION::STAMP);
			pPlayer->SetHitedAttackType(EAttackType::FURY_STAMP);
			pPlayer->SetfReceiveDamage(DAMAGE_FURY);
			break;
		case ENUM_CLASS(BossStateID::ATK_SWING_R):
		case ENUM_CLASS(BossStateID::ATK_SWING_L_COM1):
		case ENUM_CLASS(BossStateID::ATK_SWING_R_COM1):
		case ENUM_CLASS(BossStateID::ATK_SWING_L_COM2):
		case ENUM_CLASS(BossStateID::ATK_SWING_R_COM2):
		case ENUM_CLASS(BossStateID::ATK_SWING_SEQ_START):
		case ENUM_CLASS(BossStateID::ATK_SWING_SEQ_RESET):
		case ENUM_CLASS(BossStateID::ATK_SWING_SEQ_RESET2):
		case ENUM_CLASS(BossStateID::ATK_SWING_SEQ):
		case ENUM_CLASS(BossStateID::ATK_SWING_SEQ2):
		case ENUM_CLASS(BossStateID::ATK_SWING_SEQ3):
		case ENUM_CLASS(BossStateID::ATK_SLAM_COMBO_LEFT_END):
		case ENUM_CLASS(BossStateID::ATK_SLAM_COMBO_RIGHT_END):
			pPlayer->SetHitMotion(HITMOTION::KNOCKBACK);
			pPlayer->SetfReceiveDamage(DAMAGE_MEDIUM);
			pPlayer->SetHitedAttackType(EAttackType::KNOCKBACK);
			break;
		case ENUM_CLASS(BossStateID::ATK_UPPERCUT_START):
			pPlayer->SetHitMotion(HITMOTION::NORMAL);
			pPlayer->SetHitedAttackType(EAttackType::NORMAL);
			break;
		case ENUM_CLASS(BossStateID::ATK_FOOT):
			pPlayer->SetfReceiveDamage(DAMAGE_LIGHT);
			pPlayer->SetHitMotion(HITMOTION::UP);
			pPlayer->SetHitedAttackType(EAttackType::AIRBORNE);
			if (m_pAnimator->GetInt("SkillType") == FootAtk)
			{
				SetTurnTimeDuringAttack(2.5f, 1.3f); // 퓨리 어택 
				m_pAnimator->SetBool("IsHit", true);
			}
			break;
		case ENUM_CLASS(BossStateID::ATK_STRIKE_FURY):
			pPlayer->SetfReceiveDamage(DAMAGE_FURY);
			pPlayer->SetHitMotion(HITMOTION::UP);
			pPlayer->SetHitedAttackType(EAttackType::FURY_AIRBORNE);
			break;
		default:
			break;


		}
		if (curNodeID == ENUM_CLASS(BossStateID::ATK_SWING_SEQ3))
		{
			pPlayer->SetElementTypeWeight(EELEMENT::FIRE, 0.3f);
		}
	}

	if (eColliderType == COLLIDERTYPE::BOSS_WEAPON)
	{
		if (auto pOil = dynamic_cast<COil*>(pOther))
		{
			pOil->Explode_Oil();
		}
	}
}


CFuoco* CFuoco::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFuoco* pInstance = new CFuoco(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CFuoco");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CFuoco::Clone(void* pArg)
{
	CFuoco* pInstance = new CFuoco(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CFuoco");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CFuoco::Free()
{
	__super::Free();
	Safe_Release(m_pPhysXActorComForArm);
	Safe_Release(m_pPhysXActorComForFoot);
}
