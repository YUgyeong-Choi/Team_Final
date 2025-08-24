#include "Fuoco.h"
#include "Bone.h"
#include "Projectile.h"
#include "GameInstance.h"
#include "Effect_Manager.h"
#include "LockOn_Manager.h"
#include "Camera_Manager.h"
#include "Client_Calculation.h"
#include <Player.h>
#include <PhysX_IgnoreSelfCallback.h>

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
	/* [ 데미지 설정 ] */
	m_fDamage = 15.f;

	if (pArg == nullptr)
	{
		UNIT_DESC UnitDesc{};
		UnitDesc.eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
		UnitDesc.fRotationPerSec = XMConvertToRadians(140.f);
		UnitDesc.fSpeedPerSec = m_fWalkSpeed;
		lstrcpy(UnitDesc.szName, TEXT("FireEater"));
		UnitDesc.szMeshID = TEXT("FireEater");
		UnitDesc.InitPos = _float3(55.f, 0.f, -7.5f);
		//UnitDesc.InitPos = _float3(55.5f, 0.f, -7.5f);
		UnitDesc.InitScale = _float3(0.9f, 0.9f, 0.9f);

		if (FAILED(__super::Initialize(&UnitDesc)))
			return E_FAIL;
	}
	else
	{
		UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);

		lstrcpy(pDesc->szName, TEXT("FireEater"));
		pDesc->szMeshID = TEXT("FireEater");
		pDesc->fRotationPerSec = XMConvertToRadians(140.f);
		pDesc->fSpeedPerSec = m_fWalkSpeed;

		//UnitDesc.InitPos = _float3(55.f, 0.f, -7.5f);
		//UnitDesc.InitPos = _float3(55.5f, 0.f, -7.5f);
		//UnitDesc.InitScale = _float3(0.9f, 0.9f, 0.9f);

		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;
	}

	return S_OK;
}

void CFuoco::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);	
#ifdef _DEBUG
	if (KEY_DOWN(DIK_TAB))
	{
		m_pAnimator->SetTrigger("Attack");
		m_pAnimator->SetInt("SkillType", P2_FireOil);
		//m_pAnimator->SetTrigger("Paralyzation");
	//	m_pAnimator->SetTrigger("Fatal");
		//m_pAnimator->SetTrigger("Groggy");
		//if (m_bStartPhase2 == false)
		//	m_bStartPhase2 = true;
	//	m_fHP -= 10.f;
		//FireProjectile(ProjectileType::Oil);
	}
#endif
}

void CFuoco::Update(_float fTimeDelta)
{
	if (CalculateCurrentHpRatio() <= 0.f)
	{
		m_pAnimator->SetTrigger("SpecialDie");
		m_bUseLockon = false;
	}

	if (m_fFireFlameDuration > 0.f)
	{
		m_fFireFlameDuration -= fTimeDelta;
		FlamethrowerAttack();
		if (m_fFireFlameDuration <= 0.f)
		{
			m_fFireFlameDuration = 0.f;
		}
	}

	__super::Update(fTimeDelta); 
}

void CFuoco::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider())
	{
		//m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
		if(m_pPhysXActorComForArm->Get_ReadyForDebugDraw())
			m_pGameInstance->Add_DebugComponent(m_pPhysXActorComForArm);
		if (m_pPhysXActorComForFoot->Get_ReadyForDebugDraw())
			m_pGameInstance->Add_DebugComponent(m_pPhysXActorComForFoot);
	}
#endif
}


HRESULT CFuoco::Ready_Components(void* pArg)
{
	if (FAILED(__super::Ready_Components(pArg)))
		return E_FAIL;	

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX2"), reinterpret_cast<CComponent**>(&m_pPhysXActorComForArm))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX3"), reinterpret_cast<CComponent**>(&m_pPhysXActorComForFoot))))
		return E_FAIL;

	return S_OK;
}

HRESULT CFuoco::Ready_Actor()
{
	if(FAILED(__super::Ready_Actor()))
		return E_FAIL;
	_vector S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 부모꺼 부르면서 밑에꺼 부르기
	if (m_pFistBone)
	{

		auto fistLocalMatrix = m_pFistBone->Get_CombinedTransformationMatrix();
		auto fistWorldMatrix = XMLoadFloat4x4(fistLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
		XMMatrixDecompose(&S, &R, &T, fistWorldMatrix);

		PxQuat armRotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
		PxVec3 armPositionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
		PxTransform armPose(armPositionVec, armRotationQuat);
		PxSphereGeometry armGeom = m_pGameInstance->CookSphereGeometry(1.45f);
		m_pPhysXActorComForArm->Create_Collision(m_pGameInstance->GetPhysics(), armGeom, armPose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorComForArm->Set_ShapeFlag(false, true, true);
		PxFilterData armFilterData{};
		armFilterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		armFilterData.word1 = WORLDFILTER::FILTER_PLAYERBODY; 
		m_pPhysXActorComForArm->Set_SimulationFilterData(armFilterData);
		m_pPhysXActorComForArm->Set_QueryFilterData(armFilterData);
		m_pPhysXActorComForArm->Set_Owner(this);
		m_pPhysXActorComForArm->Set_ColliderType(COLLIDERTYPE::MONSTER_WEAPON);
		m_pPhysXActorComForArm->Set_Kinematic(true);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorComForArm->Get_Actor());
	}

	if (m_pFootBone)
	{
		auto footLocalMatrix = m_pFootBone->Get_CombinedTransformationMatrix();
		auto footWorldMatrix = XMLoadFloat4x4(footLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
		XMMatrixDecompose(&S, &R, &T, footWorldMatrix);

		PxQuat footRotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
		PxVec3 footPositionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
		PxTransform footPose(footPositionVec, footRotationQuat);
		PxSphereGeometry footGeom = m_pGameInstance->CookSphereGeometry(0.8f);
		m_pPhysXActorComForFoot->Create_Collision(m_pGameInstance->GetPhysics(), footGeom, footPose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorComForFoot->Set_ShapeFlag(true, false, true);
		PxFilterData footFilterData{};
		footFilterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		footFilterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
		m_pPhysXActorComForFoot->Set_SimulationFilterData(footFilterData);
		m_pPhysXActorComForFoot->Set_QueryFilterData(footFilterData);
		m_pPhysXActorComForFoot->Set_Owner(this);
		m_pPhysXActorComForFoot->Set_ColliderType(COLLIDERTYPE::MONSTER_WEAPON);
		m_pPhysXActorComForFoot->Set_Kinematic(true);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorComForFoot->Get_Actor());
	}


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
	// 이것도 부모꺼 부르면서 밑에꺼 부르기
	//_matrix worldMatrix = m_pTransformCom->Get_WorldMatrix();

	//_float4 vPos;
	//XMStoreFloat4(&vPos, worldMatrix.r[3]);

	//PxVec3 pos(vPos.x, vPos.y + 2.f, vPos.z);
	//XMVECTOR boneQuat = XMQuaternionRotationMatrix(worldMatrix);
	//XMFLOAT4 fQuat;
	//XMStoreFloat4(&fQuat, boneQuat);
	//PxQuat rot = PxQuat(fQuat.x, fQuat.y, fQuat.z, fQuat.w);

	//m_pPhysXActorCom->Set_Transform(PxTransform(pos, rot));

	__super::Update_Collider();

	auto fistLocalMatrix = m_pFistBone->Get_CombinedTransformationMatrix();
	auto fistWorldMatrix = XMLoadFloat4x4(fistLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
	_float4 fistPos;
	XMStoreFloat4(&fistPos, fistWorldMatrix.r[3]);
	PxVec3 armPos(fistPos.x, fistPos.y, fistPos.z);
	_vector boneQuatForArm = XMQuaternionRotationMatrix(fistWorldMatrix);
	_float4 fQuatForArm;
	XMStoreFloat4(&fQuatForArm, boneQuatForArm);
	PxQuat armRot = PxQuat(fQuatForArm.x, fQuatForArm.y, fQuatForArm.z, fQuatForArm.w);
	m_pPhysXActorComForArm->Set_Transform(PxTransform(armPos, armRot));

	auto footLocalMatrix = m_pFootBone->Get_CombinedTransformationMatrix();
	auto footWorldMatrix = XMLoadFloat4x4(footLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
	_float4 footPos;
	XMStoreFloat4(&footPos, footWorldMatrix.r[3]);
	PxVec3 footPosVec(footPos.x, footPos.y, footPos.z);
	_vector boneQuatForFoot = XMQuaternionRotationMatrix(footWorldMatrix);
	_float4 fQuatForFoot;
	XMStoreFloat4(&fQuatForFoot, boneQuatForFoot);
	PxQuat footRot = PxQuat(fQuatForFoot.x, fQuatForFoot.y, fQuatForFoot.z, fQuatForFoot.w);
	m_pPhysXActorComForFoot->Set_Transform(PxTransform(footPosVec, footRot));

}

void CFuoco::UpdateAttackPattern(_float fDistance, _float fTimeDelta)
{

	// 퓨리 돌진 9번
	if (m_bIsFirstAttack)
	{
		m_pAnimator->SetTrigger("Attack");
		m_pAnimator->SetInt("SkillType", StrikeFury);
		m_bIsFirstAttack = false;
		m_pAnimator->SetBool("Move", false);
		m_fAttackCooldown = m_fAttckDleay;
		SetTurnTimeDuringAttack(1.5f, 1.4f);
		return;
	}

	if (CheckConditionFlameField())
	{
		m_fAttackCooldown = 10.f;
		return;
	}
	if (false == UpdateTurnDuringAttack(fTimeDelta))
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


	if (fDistance >= 25.f)
		return;


	if (m_eCurrentState == EBossState::ATTACK)
	{
		return;
	}


	EBossAttackPattern eSkillType = GetRandomAttackPattern(fDistance);


	SetupAttackByType(eSkillType);

	m_pAnimator->SetBool("Move", false);
	m_pAnimator->SetInt("SkillType", eSkillType);
	m_pAnimator->SetTrigger("Attack");
	m_eCurrentState = EBossState::ATTACK;
	m_fAttackCooldown = m_fAttckDleay;
}

void CFuoco::UpdateStateByNodeID(_uint iNodeID)
{
	switch (iNodeID)
	{
	case ENUM_CLASS(BossStateID::IDLE):
		m_eCurrentState = EBossState::IDLE;
		break;
	case ENUM_CLASS(BossStateID::WALK_B):
	case ENUM_CLASS(BossStateID::WALK_F):
	case ENUM_CLASS(BossStateID::WALK_R):
	case ENUM_CLASS(BossStateID::WALK_L):
	{
		m_pTransformCom->SetfSpeedPerSec(m_fWalkSpeed);
		m_eCurrentState = EBossState::WALK;
	}
	break;
	case ENUM_CLASS(BossStateID::RUN_F):
		m_pTransformCom->SetfSpeedPerSec(m_fRunSpeed);
		m_eCurrentState = EBossState::RUN;
		break;
	case ENUM_CLASS(BossStateID::GROGGY_START):
	case ENUM_CLASS(BossStateID::GROGGY_LOOP):
	case ENUM_CLASS(BossStateID::GROGGY_END):
		m_eCurrentState = EBossState::GROGGY;
		break;
	case ENUM_CLASS(BossStateID::DEAD_B):
	case ENUM_CLASS(BossStateID::DEAD_F):
	case ENUM_CLASS(BossStateID::SPECIAL_DIE):
		m_eCurrentState = EBossState::DEAD;
		break;
	case ENUM_CLASS(BossStateID::TURN_L):
	case ENUM_CLASS(BossStateID::TURN_R):
		m_eCurrentState = EBossState::TURN;
		break;
	case ENUM_CLASS(BossStateID::PARALYZATION_START):
	case ENUM_CLASS(BossStateID::PARALYZATION_LOOP):
	case ENUM_CLASS(BossStateID::PARALYZATION_END):
		m_eCurrentState = EBossState::PARALYZATION;
		break;
	case ENUM_CLASS(BossStateID::FATAL_START):
	case ENUM_CLASS(BossStateID::FATAL_LOOP):
	case ENUM_CLASS(BossStateID::FATAL_END):
		m_eCurrentState = EBossState::FATAL;
		break;
	case ENUM_CLASS(BossStateID::ATK_SWING_SEQ):
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(45.f);
		break;
	case ENUM_CLASS(BossStateID::ATK_SWING_SEQ2):
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(60.f);
		break;
	case ENUM_CLASS(BossStateID::ATK_SWING_SEQ3):
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(70.f);
		break;
	case ENUM_CLASS(BossStateID::ATK_SWING_SEQ_RESET):
	case ENUM_CLASS(BossStateID::ATK_SWING_SEQ_RESET2):
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(100.f);
		break;
	case ENUM_CLASS(BossStateID::ATK_SWING_END):
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(55.f);
		break;
	case ENUM_CLASS(BossStateID::ATK_SWING_R):
	case ENUM_CLASS(BossStateID::ATK_SWING_R_COM1):
	case ENUM_CLASS(BossStateID::ATK_SWING_R_COM2):
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(55.f);
		break;
	case ENUM_CLASS(BossStateID::CUTSCENE):
		m_eCurrentState = EBossState::CUTSCENE;
		break;
	default:
		m_eCurrentState = EBossState::ATTACK;
		break;
	}
}

void CFuoco::UpdateSpecificBehavior()
{
	if (m_eCurrentState == EBossState::DEAD)
		return;

	if (m_eCurrentState != EBossState::ATTACK)
	{
		m_pAnimator->SetBool("IsHit", false);
	}
	else if (m_eCurrentState == EBossState::ATTACK)
	{
		m_pAnimator->SetBool("Move", false);
	}
}

void CFuoco::SetupAttackByType(EBossAttackPattern ePattern)
{

	switch (ePattern)
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
			_int iComboType = GetRandomInt(0, 1);
			m_pAnimator->SetInt("SwingCombo", iComboType);
		}
		m_eBossAttackType = EBossAttackType::KNOCKBACK;
	}
	break;
	case Client::CFuoco::SlamFury:
		SetTurnTimeDuringAttack(1.5f, 1.3f);
		m_eBossAttackType = EBossAttackType::FURY_STAMP;
	case Client::CFuoco::FootAtk:
		SetTurnTimeDuringAttack(1.f);
		break;
	case Client::CFuoco::SlamAtk:
		SetTurnTimeDuringAttack(1.f);
		m_eBossAttackType = EBossAttackType::STAMP;
		break;
	case Client::CFuoco::Uppercut:
		SetTurnTimeDuringAttack(1.f);
		m_eBossAttackType = EBossAttackType::NORMAL;
		break;
	case Client::CFuoco::StrikeFury:
		SetTurnTimeDuringAttack(1.2f);
		break;
	case Client::CFuoco::P2_FireFlame:
	{
		_int iDir = GetYawSignFromDiretion();
		m_pAnimator->SetInt("Direction", iDir);
	}
	case Client::CFuoco::P2_FireOil:
		m_eBossAttackType = EBossAttackType::NONE;
		break;
	case Client::CFuoco::P2_FireBall:
	{
		_int iDir = GetRandomInt(0, 2);
		m_pAnimator->SetInt("Direction", iDir);
		m_eBossAttackType = EBossAttackType::KNOCKBACK;
	}
	break;
	case Client::CFuoco::P2_FireBall_B:
		m_eBossAttackType = EBossAttackType::KNOCKBACK;
		break;
	default:
		break;
	}
}

void CFuoco::Register_Events()
{
	if (nullptr == m_pAnimator)
		return;

	m_pAnimator->RegisterEventListener("CameraShake",
		[this]()
		{
			CCamera_Manager::Get_Instance()->Shake_Camera(0.15f, 0.2f);
		});
	m_pAnimator->RegisterEventListener("IsFront",
		[this]()
		{
			if (IsTargetInFront(40.f))
			{
				m_pAnimator->SetBool("IsFront", true);
			}
			else
			{
				m_pAnimator->SetBool("IsFront", false);
			}
		});
	m_pAnimator->RegisterEventListener("Turnning", [this]()
		{
			_bool bIsFront = IsTargetInFront(180.f);

			if (bIsFront == false)
			{
				SetTurnTimeDuringAttack(2.5f, 1.4f);
			}
			else
			{
				SetTurnTimeDuringAttack(1.3f);
			}

		});

	m_pAnimator->RegisterEventListener("ResetAnim", [this]()
		{
			m_pAnimator->GetCurrentAnim()->ResetTrack();
		});

	m_pAnimator->RegisterEventListener("SlowAnimSpeed", [this]() {
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(
			m_pAnimator->GetCurrentAnim()->GetTickPerSecond() * 0.5f);
		});
	m_pAnimator->RegisterEventListener("ResetAnimSpeed", [this]() {
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(
			m_pAnimator->GetCurrentAnim()->GetTickPerSecond() * 1.5f);
		});

	m_pAnimator->RegisterEventListener("CollidersOff", [this]() {
		m_pPhysXActorCom->Init_SimulationFilterData();
		m_pPhysXActorComForArm->Init_SimulationFilterData();
		m_pPhysXActorComForFoot->Init_SimulationFilterData();
		});
	m_pAnimator->RegisterEventListener("CollidersOn", [this]() {
		m_pPhysXActorCom->Set_SimulationFilterData(m_pPhysXActorCom->Get_FilterData());
		m_pPhysXActorComForArm->Set_SimulationFilterData(m_pPhysXActorComForArm->Get_FilterData());
		m_pPhysXActorComForFoot->Set_SimulationFilterData(m_pPhysXActorComForFoot->Get_FilterData());
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
		});
	m_pAnimator->RegisterEventListener("ColliderFootOff", [this]()
		{
			m_pPhysXActorComForFoot->Init_SimulationFilterData();
		});

	m_pAnimator->RegisterEventListener("FireBall", [this]()
		{
			FireProjectile(ProjectileType::FireBall, 25.f);
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
			_bool bIsCombo = GetRandomInt(0, 1) == 1;
			if (m_iFireBallComboCount == LIMIT_FIREBALL_COMBO_COUNT)
			{
				m_iFireBallComboCount = 0;
				return;
			}
			if (bIsCombo)
			{
				_int iDir = GetRandomInt(0, 1);
				switch (iDir)
				{
				case 0: // 왼쪽
					m_pAnimator->SetInt("Direction", iDir);
					m_pAnimator->Get_CurrentAnimController()->SetState(ENUM_CLASS(BossStateID::P2_ATK_FIRE_BALL_L));
					break;
				//case 1: // 중앙
				//	m_pAnimator->SetInt("Direction", 2);
				//	m_pAnimator->Get_CurrentAnimController()->SetState(ENUM_CLASS(BossStateID::P2_ATK_FIRE_BALL_F));
				//	break;
				case 1: // 오른쪽
					m_pAnimator->SetInt("Direction", iDir);
					m_pAnimator->Get_CurrentAnimController()->SetState(ENUM_CLASS(BossStateID::P2_ATK_FIRE_BALL_R));
					break;
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
			m_fFireFlameDuration = 1.5f;
			FlamethrowerAttack();
		});
}

void CFuoco::Ready_AttackPatternWeightForPhase1()
{
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
//	m_pAnimator->SetTrigger("Paralyzation");
	m_pAnimator->SetTrigger("Groggy");
	m_bStartPhase2 = true;
	vector<EBossAttackPattern> m_vecBossPatterns = {
		SlamCombo,Uppercut,SwingAtk,SwingAtkSeq,SlamFury,FootAtk,
		SlamAtk,StrikeFury,P2_FireOil,P2_FireBall,P2_FireFlame,
		P2_FireBall_B
	};
	m_PatternWeightMap.clear();
	m_PatternCountMap.clear();
	for (const auto& pattern : m_vecBossPatterns)
	{
		m_PatternWeightMap[pattern] = m_fBasePatternWeight;
		m_PatternCountMap[pattern] = 0;
	}
}

CFuoco::EBossAttackPattern CFuoco::GetRandomAttackPattern(_float fDistance)
{
	EBossAttackPattern ePattern = BAP_NONE;
	m_PatternWeighForDisttMap = m_PatternWeightMap;
	ChosePatternWeightByDistance(fDistance);


	_float fTotalWeight = accumulate(m_PatternWeighForDisttMap.begin(), m_PatternWeighForDisttMap.end(), 0.f,
		[](_float fAcc, const pair<EBossAttackPattern, _float>& Pair) { return fAcc + Pair.second; });

	_float fRandomVal = GetRandomFloat(0.f, fTotalWeight);
	_float fCurWeight = 0.f;
	for (const auto& [pattern, weight] : m_PatternWeighForDisttMap)
	{
		if (weight <= 0.f)
			continue; // 가중치가 0 이하인 패턴은 무시
		fCurWeight += weight;
		if (fRandomVal <= fCurWeight)
		{
			ePattern = pattern;
			m_ePrevAttackPattern = m_eCurAttackPattern;
			m_eCurAttackPattern = ePattern;
#ifdef _DEBUG
			PatterDebugFunc();
#endif
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
	desc.fSpeed = fSpeed;
	_int iLevelIndex = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);
	switch (type)
	{
	case Client::CFuoco::ProjectileType::FireBall:
	{
		if (m_pLeftBone)
		{
			auto handLocalMatrix = m_pLeftBone->Get_CombinedTransformationMatrix();
			auto handWorldMatrix = XMLoadFloat4x4(handLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
			_float3 localOffset = { 0.0f, 0.0f, 100.f };
			_vector spawnPos = XMVector3TransformCoord(XMLoadFloat3(&localOffset), handWorldMatrix);
			vPos = XMVectorSetW(spawnPos, 1.f);
		}
		else
		{
			// 손이 없으면 기본 위치 사용
			vPos = m_pTransformCom->Get_State(STATE::POSITION);
		}

		XMStoreFloat3(&desc.vPos, vPos);
		XMStoreFloat3(&desc.vDir, GetTargetDirection());

		desc.fLifeTime = 3.f;
		desc.fGravityOnDist = Get_DistanceToPlayer() * 0.15f;
		desc.fStartTime = 1.f;
		desc.bUseTimeTrigger = false;
		desc.bUseDistTrigger = true;
		desc.fRadius = 0.35f;
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
		desc.fStartTime = 0.5f;
		desc.bUseTimeTrigger = true;
		desc.bUseDistTrigger = false;
		desc.fRadius = 0.2f;
		desc.fLifeTime = 3.f;
		lstrcpy(desc.szName, TEXT("Oil"));
		_vector vBaseDir = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
		// base에서 30도씩 회전 시키기
		_vector vLeftDirOfBase = XMVector3Rotate(vBaseDir, XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-5.f)));
		_vector vRightDirOfBase = XMVector3Rotate(vBaseDir, XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(5.f)));
		array<_float3, 3> vDirArray;
		XMStoreFloat3(&vDirArray[0], vBaseDir);
		XMStoreFloat3(&vDirArray[1], vLeftDirOfBase);
		XMStoreFloat3(&vDirArray[2], vRightDirOfBase);

		for (_int i = 0; i < 3; i++)
		{
			desc.vDir = vDirArray[i];
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
	PxVec3 vRight = PxVec3(worldMatrix._11, worldMatrix._12, worldMatrix._13); // 손의 Up 방향
	vDir.normalize();
	vRight.normalize();

	PxHitFlags hitFlags(PxHitFlag::eDEFAULT);
	PxQueryFilterData filterData;
	filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC| PxQueryFlag::ePREFILTER; // 
	unordered_set<PxActor*> ignoreActors;
	ignoreActors.insert(m_pPhysXActorCom->Get_Actor());
	ignoreActors.insert(m_pPhysXActorComForArm->Get_Actor());
	ignoreActors.insert(m_pPhysXActorComForFoot->Get_Actor());
	
	CIgnoreSelfCallback callback(ignoreActors);

	for (_int i = 0; i < iRayCount; i++)
	{
		_float fCurAngle = -fConeAngle * 0.5f + (fConeAngle / (iRayCount - 1)) * i; // -세타 ~ 세타
		PxQuat vRot = PxQuat(XMConvertToRadians(fCurAngle), vRight); // 회전 쿼터니언 생성
		PxVec3 vRayDir = vRot.rotate(vDir); // Look 방향을 회전

		PxRaycastBuffer hit;
		if (m_pGameInstance->Get_Scene()->raycast(origin, vRayDir, fDistance, hit, hitFlags, filterData, &callback))
		{
			if (hit.hasBlock)
			{
				PxRigidActor* hitActor = hit.block.actor;
				PxVec3 hitPos = hit.block.position;
				PxVec3 hitNormal = hit.block.normal;
				CPhysXActor* pHitActor = static_cast<CPhysXActor*>(hitActor->userData);

				if (pHitActor&& pHitActor->Get_Owner())
				{
					if (nullptr == pHitActor->Get_Owner())
						return;
					// 데미지 계산 나중에 생각해보기
					pHitActor->Get_Owner()->On_Hit(this, COLLIDERTYPE::MONSTER_WEAPON);
				}

				//printf("RayHitPos X: %f, Y: %f, Z: %f\n", hitPos.x, hitPos.y, hitPos.z);
				//printf("RayHitNormal X: %f, Y: %f, Z: %f\n", hitNormal.x, hitNormal.y, hitNormal.z);
				m_bRayHit = true;
				m_vRayHitPos = hitPos;
			}
#ifdef _DEBUG
			if (m_pGameInstance->Get_RenderCollider()) {
				DEBUGRAY_DATA _data{};
				_data.vStartPos = origin;
				XMFLOAT3 fLook;
				XMStoreFloat3(&fLook, m_pTransformCom->Get_State(STATE::LOOK));
				_data.vDirection = vRayDir;
				_data.fRayLength = 10.f;
				_data.bIsHit = fDistance;
				_data.vHitPos = m_vRayHitPos;
				m_pPhysXActorCom->Add_RenderRay(_data);
			}
#endif
		}
#ifdef _DEBUG
		else if(m_pGameInstance->Get_RenderCollider()) {
			DEBUGRAY_DATA _data{};
			_data.vStartPos = origin;
			_data.vDirection = vRayDir; // 회전된 방향 사용
			_data.fRayLength = fDistance;
			_data.bIsHit = false;
			_data.vHitPos = PxVec3(0, 0, 0);
			m_pPhysXActorCom->Add_RenderRay(_data);
		}
#endif // _DEBUG

	}

}

void CFuoco::UpdatePatternWeight(EBossAttackPattern ePattern)
{
	m_PatternCountMap[ePattern]++;
	if (m_PatternCountMap[ePattern] >= m_iPatternLimit)
	{
		m_PatternWeightMap[ePattern] *= (1.f - m_fWeightDecreaseRate); // 가중치 감소
		m_PatternWeightMap[ePattern] = max(m_PatternWeightMap[ePattern], m_fMinWeight); // 최소 가중치로 설정
		m_PatternCountMap[ePattern] = 0;

		for (auto& [pattern, weight] : m_PatternWeightMap)
		{
			if (pattern != ePattern)
			{
				weight += (m_fMaxWeight - weight) * m_fWeightIncreaseRate; // 가중치 증가
				weight = min(weight, m_fMaxWeight); // 최대 가중치로 제한
			}
		}
	}
}

_bool CFuoco::CheckConditionFlameField()
{
	if (m_bStartPhase2 || (!m_bUsedFlameFiledOnLowHp && CalculateCurrentHpRatio() <= 0.2f))
	{
		m_pAnimator->ResetTrigger("Attack");
		if (CalculateCurrentHpRatio() <= 0.2f && !m_bUsedFlameFiledOnLowHp)
		{
			m_pAnimator->SetInt("SkillType", P2_FlameField);
			m_bUsedFlameFiledOnLowHp = true;
			m_pAnimator->SetTrigger("Attack");
		}
		else
		{
			m_pAnimator->SetInt("SkillType", StrikeFury);
			m_pAnimator->SetTrigger("Attack");
			m_pAnimator->SetTrigger("Phase2Start");
			m_bStartPhase2 = false;
			m_bIsPhase2 = true;
		}
		m_pAnimator->SetBool("Move", false);
		m_eCurrentState = EBossState::ATTACK;
		return true;
	}
	return false;
}


void CFuoco::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{

	if (pOther)
	{
		if (eColliderType == COLLIDERTYPE::PLAYER)
		{
			if (m_pAnimator->GetInt("SkillType") == FootAtk)
			{
				m_pAnimator->SetBool("IsHit", true);
				SetTurnTimeDuringAttack(2.f, 1.2f); // 퓨리 어택 
				if (auto pPlayer = dynamic_cast<CPlayer*>(pOther))
				{
					auto pAnimator = pPlayer->Get_Animator();
					pAnimator->SetBool("IsUp", true);
					pAnimator->SetTrigger("Hited");
				}
			}
		}

	}
}

void CFuoco::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CFuoco::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (pOther)
	{
		//if (eColliderType == COLLIDERTYPE::PALYER)
		//{
		//	if (m_pAnimator->CheckBool("IsHit"))
		//		m_pAnimator->SetBool("IsHit", false);
		//}
	}
}

void CFuoco::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFuoco::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	ReceiveDamage(pOther, eColliderType);

	if (auto pPlayer = dynamic_cast<CPlayer*>(pOther))
	{
		auto pAnimator = pPlayer->Get_Animator();
		_vector vDir = GetTargetDirection();
		_float fDot = XMVectorGetX(XMVector3Dot(vDir, pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)));
		if (fDot > 0.f)
		{
			pAnimator->SetInt("HitDir", 2); // 뒤에서 맞음
		}
		else if (fDot < 0.f)
		{
			pAnimator->SetInt("HitDir",0); // 앞에서 맞음
		}
		else
		{
			pAnimator->SetInt("HitDir", 1); // 옆에서 맞음
		}
		_uint curNodeID = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->iNodeId;
		switch (curNodeID)
		{
		case ENUM_CLASS(BossStateID::ATK_SLAM_COMBO_START):
		case ENUM_CLASS(BossStateID::ATK_SLAM_COMBO_LOOP):
		case ENUM_CLASS(BossStateID::ATK_SLAM_COMBO_END):
		case ENUM_CLASS(BossStateID::ATK_SLAM):
		case ENUM_CLASS(BossStateID::ATK_SLAM_FURY):
		case ENUM_CLASS(BossStateID::ATK_UPPERCUT_FRONT):
			pAnimator->SetTrigger("Stamp");
			break;
		case ENUM_CLASS(BossStateID::ATK_SWING_R):
		case ENUM_CLASS(BossStateID::ATK_SWING_L_COM1):
		case ENUM_CLASS(BossStateID::ATK_SWING_R_COM1):
		case ENUM_CLASS(BossStateID::ATK_SWING_L_COM2):
		case ENUM_CLASS(BossStateID::ATK_SWING_R_COM2):
		case ENUM_CLASS(BossStateID::ATK_SWING_SEQ):
		case ENUM_CLASS(BossStateID::ATK_SWING_SEQ2):
		case ENUM_CLASS(BossStateID::ATK_SWING_SEQ3):
		case ENUM_CLASS(BossStateID::ATK_SLAM_COMBO_LEFT_END):
		case ENUM_CLASS(BossStateID::ATK_SLAM_COMBO_RIGHT_END):
			pAnimator->SetTrigger("Knockback");
			break;
		case ENUM_CLASS(BossStateID::ATK_UPPERCUT_START):
			pAnimator->SetTrigger("Hited");
			break;
		default:
			pAnimator->SetBool("IsUp", false);
			break;
		}

	}
}

void CFuoco::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
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
	//Safe_Release(m_pNaviCom);
	//Safe_Release(m_pPhysXActorCom);
	Safe_Release(m_pPhysXActorComForArm);
	Safe_Release(m_pPhysXActorComForFoot);

}
