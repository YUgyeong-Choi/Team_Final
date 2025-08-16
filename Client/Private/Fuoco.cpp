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
	: CUnit(pDevice, pContext)
{
}

CFuoco::CFuoco(const CFuoco& Prototype)
	: CUnit(Prototype)
{
}
HRESULT CFuoco::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFuoco::Initialize(void* pArg)
{

	UNIT_DESC UnitDesc{};
	UnitDesc.eLevelID = LEVEL::KRAT_CENTERAL_STATION;
	UnitDesc.fRotationPerSec = XMConvertToRadians(140.f);
	UnitDesc.fSpeedPerSec = m_fWalkSpeed;
	lstrcpy(UnitDesc.szName, TEXT("FireEater"));
	UnitDesc.szMeshID = TEXT("FireEater");
	UnitDesc.InitPos = _float3(55.f, 0.f, -7.5f);
	//UnitDesc.InitPos = _float3(55.5f, 0.f, -7.5f);
	UnitDesc.InitScale = _float3(0.9f, 0.9f, 0.9f);

	if (FAILED(__super::Initialize(&UnitDesc)))
		return E_FAIL;


	if (FAILED(LoadFromJson()))
		return E_FAIL;

	if (FAILED(this->Ready_Components()))
		return E_FAIL;

	if (FAILED(this->Ready_Actor()))
		return E_FAIL;

	m_pPlayer = m_pGameInstance->Get_Object(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"), 0);

	if (m_pNaviCom)
	{
		m_pNaviCom->Select_Cell(m_pTransformCom->Get_State(STATE::POSITION));
		_float fY = m_pNaviCom->Compute_NavigationY(m_pTransformCom->Get_State(STATE::POSITION));
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION), fY));
	}
	Ready_AttackPatternWeightForPhase1();
	Ready_BoneInformation();

	return S_OK;
}

void CFuoco::Priority_Update(_float fTimeDelta)
{
#ifdef _DEBUG
	if (KEY_DOWN(DIK_TAB))
	{
		cout << "현재 플레이어와의 거리 : " << Get_DistanceToPlayer() << endl;
		cout << "현재 애니메이션 상태 : " << m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName << endl;
		cout << "현재 이동 방향 " << m_pAnimator->GetInt("MoveDir") << endl;

		//m_pAnimator->SetTrigger("Attack");
		//m_pAnimator->SetInt("SkillType", P2_FireFlame);
		//m_pAnimator->SetTrigger("Paralyzation");
		m_pAnimator->SetTrigger("Fatal");
		//m_pAnimator->SetTrigger("Groggy");
		//if (m_bStartPhase2 == false)
		//	m_bStartPhase2 = true;
	//	m_fHP -= 10.f;
		//FireProjectile(ProjectileType::Oil);
	}
#endif

	if (CalculateCurrentHpRatio() <= 0.5f && m_bIsPhase2 == false)
	{
		Ready_AttackPatternWeightForPhase2();
	}
}

void CFuoco::Update(_float fTimeDelta)
{
	if (CalculateCurrentHpRatio() <= 0.f)
	{
		m_pAnimator->SetTrigger("SpecialDie");
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


	UpdateBossState(fTimeDelta); // 상태 업데이트
	__super::Update(fTimeDelta); // 애니메이션 재생
	Update_Collider(); // 콜라이더 업데이트

	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom))
	{
		CLockOn_Manager::Get_Instance()->Add_LockOnTarget(this);
		_matrix LockonMat = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(m_iLockonBoneIndex));

		_vector vLockonPos = XMVector3TransformCoord(LockonMat.r[3], m_pTransformCom->Get_WorldMatrix());

		XMStoreFloat4(&m_vLockonPos, vLockonPos);
	}
}

void CFuoco::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_SHADOW, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider())
	{
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorComForArm);
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorComForFoot);
	}
#endif
}

HRESULT CFuoco::LoadFromJson()
{
	string path = "../Bin/Save/AnimationEvents/" + m_pModelCom->Get_ModelName() + "_events.json";
	ifstream ifs(path);
	if (ifs.is_open())
	{
		json root;
		ifs >> root;
		if (root.contains("animations"))
		{
			auto& animationsJson = root["animations"];
			auto& clonedAnims = m_pModelCom->GetAnimations();

			for (const auto& animData : animationsJson)
			{
				const string& clipName = animData["ClipName"];

				for (auto& pAnim : clonedAnims)
				{
					if (pAnim->Get_Name() == clipName)
					{
						pAnim->Deserialize(animData);
						break;
					}
				}
			}
		}
	}
	else
	{
		MSG_BOX("Failed to open animation events file.");
		return E_FAIL;
	}

	path = "../Bin/Save/AnimationStates/" + m_pModelCom->Get_ModelName() + "_States.json";
	ifstream ifsStates(path);
	if (ifsStates.is_open())
	{
		json rootStates;
		ifsStates >> rootStates;
		m_pAnimator->Deserialize(rootStates);
	}
	else
	{
		MSG_BOX("Failed to open animation states file.");
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CFuoco::Ready_Components()
{
	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX2"), reinterpret_cast<CComponent**>(&m_pPhysXActorComForArm))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX3"), reinterpret_cast<CComponent**>(&m_pPhysXActorComForFoot))))
		return E_FAIL;


	_int iLevelIndex = m_pGameInstance->GetCurrentLevelIndex();
	if (iLevelIndex == ENUM_CLASS(LEVEL::JW))
	{
		return S_OK;
	}

	_wstring wsPrototypeTag = TEXT("Prototype_Component_Navigation");
	if (FAILED(__super::Add_Component(iLevelIndex, wsPrototypeTag.c_str(),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNaviCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CFuoco::Ready_Actor()
{
	_vector S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);

	PxVec3 halfExtents = PxVec3(scaleVec.x * 1.2f, scaleVec.y * 1.7f, scaleVec.z * 0.8f);
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERWEAPON; // 일단 보류
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::MONSTER);
	m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	auto& vecBones = m_pModelCom->Get_Bones();

	for (auto& pBone : vecBones)
	{
		if (!strcmp(pBone->Get_Name(), "Bone001-Fist01"))
		{
			m_pFistBone = pBone;
		}
		if (!strcmp(pBone->Get_Name(), "Bip001-L-Foot"))
		{
			m_pFootBone = pBone;
		}
	}
	if (m_pFistBone)
	{

		auto fistLocalMatrix = m_pFistBone->Get_CombinedTransformationMatrix();
		auto fistWorldMatrix = XMLoadFloat4x4(fistLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
		XMMatrixDecompose(&S, &R, &T, fistWorldMatrix);

		PxQuat armRotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
		PxVec3 armPositionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
		PxTransform armPose(armPositionVec, armRotationQuat);
		PxSphereGeometry armGeom = m_pGameInstance->CookSphereGeometry(0.8f);
		m_pPhysXActorComForArm->Create_Collision(m_pGameInstance->GetPhysics(), armGeom, armPose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorComForArm->Set_ShapeFlag(true, true, true);
		PxFilterData armFilterData{};
		armFilterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		armFilterData.word1 = WORLDFILTER::FILTER_PLAYERBODY; // 일단 보류
		m_pPhysXActorComForArm->Set_SimulationFilterData(armFilterData);
		m_pPhysXActorComForArm->Set_QueryFilterData(armFilterData);
		m_pPhysXActorComForArm->Set_Owner(this);
		m_pPhysXActorComForArm->Set_ColliderType(COLLIDERTYPE::MONSTER);
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
		m_pPhysXActorComForFoot->Set_ShapeFlag(true, true, true);
		PxFilterData footFilterData{};
		footFilterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		footFilterData.word1 = WORLDFILTER::FILTER_PLAYERBODY; // 일단 보류
		m_pPhysXActorComForFoot->Set_SimulationFilterData(footFilterData);
		m_pPhysXActorComForFoot->Set_QueryFilterData(footFilterData);
		m_pPhysXActorComForFoot->Set_Owner(this);
		m_pPhysXActorComForFoot->Set_ColliderType(COLLIDERTYPE::MONSTER);
		m_pPhysXActorComForFoot->Set_Kinematic(true);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorComForFoot->Get_Actor());
	}


	return S_OK;
}

void CFuoco::Ready_BoneInformation()
{
	m_iLockonBoneIndex = m_pModelCom->Find_BoneIndex("Bip001-Spine1");

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
}

void CFuoco::Update_Collider()
{
	_matrix worldMatrix = m_pTransformCom->Get_WorldMatrix();

	_float4 vPos;
	XMStoreFloat4(&vPos, worldMatrix.r[3]);

	PxVec3 pos(vPos.x, vPos.y + 2.f, vPos.z);
	XMVECTOR boneQuat = XMQuaternionRotationMatrix(worldMatrix);
	XMFLOAT4 fQuat;
	XMStoreFloat4(&fQuat, boneQuat);
	PxQuat rot = PxQuat(fQuat.x, fQuat.y, fQuat.z, fQuat.w);

	m_pPhysXActorCom->Set_Transform(PxTransform(pos, rot));


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

void CFuoco::UpdateBossState(_float fTimeDelta)
{
	if (!m_pPlayer)
		return;

	_uint iCurrentAnimStateNodeID = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->iNodeId;
	if (iCurrentAnimStateNodeID == ENUM_CLASS(BossStateID::CUTSCENE))
	{
		return; // 컷신 중에는 상태 업데이트를 하지 않음
	}

	UpdateStateByNodeID(iCurrentAnimStateNodeID); // 애니메이션 상태에 따라 현재 상태 업데이트

	if (m_eCurrentState == EFuocoState::DEAD)
		return;

	if (m_eCurrentState != EFuocoState::ATTACK)
	{
		m_pAnimator->SetBool("IsHit", false);
	}
	else if (m_eCurrentState == EFuocoState::ATTACK)
	{
		m_pAnimator->SetBool("Move", false);
	}
	_float fDistance = Get_DistanceToPlayer();

	UpdateAttackPattern(fDistance, fTimeDelta);// 공격 패턴 업데이트
	UpdateMovement(fDistance, fTimeDelta);
}

void CFuoco::UpdateMovement(_float fDistance, _float fTimeDelta)
{
	_bool bCanMove = (m_eCurrentState == EFuocoState::IDLE ||
		m_eCurrentState == EFuocoState::WALK ||
		m_eCurrentState == EFuocoState::RUN) &&
		m_eCurrentState != EFuocoState::GROGGY &&
		m_eCurrentState != EFuocoState::DEAD &&
		m_eCurrentState != EFuocoState::PARALYZATION &&
		m_eCurrentState != EFuocoState::ATTACK &&
		m_eCurrentState != EFuocoState::FATAL &&
		!m_bIsFirstAttack;

	m_pAnimator->SetBool("Move", bCanMove);
	if (bCanMove)
	{
		// 가까우면 
		if (fDistance < CHASING_DISTANCE)
		{
			if (m_fChangeMoveDirCooldown > 0.f)
			{
				m_fChangeMoveDirCooldown -= fTimeDelta;
				m_fChangeMoveDirCooldown = max(m_fChangeMoveDirCooldown, 0.f);
			}
			else
			{
				_int iMoveDir = GetRandomInt(1, 3);
				m_pAnimator->SetInt("MoveDir", iMoveDir);
				m_fChangeMoveDirCooldown = 5.f;
			}
		}
		else if (fDistance >= CHASING_DISTANCE)
		{
			m_pAnimator->SetInt("MoveDir", 0);
		}
		m_eCurrentState = EFuocoState::WALK;
		m_pAnimator->SetFloat("Distance", abs(fDistance));
	}
	_bool bIsRootMotion = m_pAnimator->GetCurrentAnim()->IsRootMotionEnabled();

	if (bIsRootMotion)
	{
		ApplyRootMotionDelta(fTimeDelta); // 루트 모션 적용
	}
	else
	{
		UpdateNormalMove(fTimeDelta);
	}

	_vector vDir = GetTargetDirection();
	if (XMVector3Equal(vDir, XMVectorZero()))
		return; // 방향이 0이면 회전하지 않음
	vDir = XMVectorSetY(vDir, 0.f);
	vDir = XMVector3Normalize(vDir);


	// 현재 상태에서 많이 회전했으면 애니메이터 Turn true
	_vector vCurrentLook = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
	_float fDot = XMVectorGetX(XMVector3Dot(vCurrentLook, vDir));
	fDot = clamp(fDot, -1.f, 1.f);
	_vector vCross = XMVector3Cross(vCurrentLook, vDir);
	_float fSign = (XMVectorGetY(vCross) < 0.f) ? -1.f : 1.f;
	_float fYaw = acosf(fDot) * fSign; // 회전 각도 (라디안 단위) -180~180


	_bool bIsTurn = abs(XMConvertToDegrees(fYaw)) > MINIMUM_TURN_ANGLE && (m_eCurrentState == EFuocoState::IDLE ||
		m_eCurrentState == EFuocoState::WALK || m_eCurrentState == EFuocoState::RUN);

	if (bIsTurn && m_eCurrentState != EFuocoState::TURN)
	{
		m_pAnimator->SetTrigger("Turn");
		m_pAnimator->SetInt("TurnDir", (fYaw >= 0.f) ? 0 : 1); // 0: 오른쪽, 1: 왼쪽
		m_pAnimator->SetBool("Move", false); // 회전 중에는 이동하지 않음
	}

	if (m_eCurrentState == EFuocoState::TURN)
	{
		m_pTransformCom->RotateToDirectionSmoothly(vDir, fTimeDelta);
	}
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
		m_fAttackCooldown = m_fAttckDleay;
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


	if (m_eCurrentState == EFuocoState::ATTACK)
	{
		return;
	}


	EBossAttackPattern eSkillType = GetRandomAttackPattern(fDistance);


	SetupAttackByType(eSkillType);

	m_pAnimator->SetBool("Move", false);
	m_pAnimator->SetInt("SkillType", eSkillType);
	m_pAnimator->SetTrigger("Attack");
	m_eCurrentState = EFuocoState::ATTACK;
	m_fAttackCooldown = m_fAttckDleay;
}

void CFuoco::UpdateStateByNodeID(_uint iNodeID)
{
	switch (iNodeID)
	{
	case ENUM_CLASS(BossStateID::IDLE):
		m_eCurrentState = EFuocoState::IDLE;
		break;
	case ENUM_CLASS(BossStateID::WALK_B):
	case ENUM_CLASS(BossStateID::WALK_F):
	case ENUM_CLASS(BossStateID::WALK_R):
	case ENUM_CLASS(BossStateID::WALK_L):
	{
		m_pTransformCom->SetfSpeedPerSec(m_fWalkSpeed);
		m_eCurrentState = EFuocoState::WALK;
	}
	break;
	case ENUM_CLASS(BossStateID::RUN_F):
		m_pTransformCom->SetfSpeedPerSec(m_fRunSpeed);
		m_eCurrentState = EFuocoState::RUN;
		break;
	case ENUM_CLASS(BossStateID::GROGGY_START):
	case ENUM_CLASS(BossStateID::GROGGY_LOOP):
	case ENUM_CLASS(BossStateID::GROGGY_END):
		m_eCurrentState = EFuocoState::GROGGY;
		break;
	case ENUM_CLASS(BossStateID::DEAD_B):
	case ENUM_CLASS(BossStateID::DEAD_F):
	case ENUM_CLASS(BossStateID::SPECIAL_DIE):
		m_eCurrentState = EFuocoState::DEAD;
		break;
	case ENUM_CLASS(BossStateID::TURN_L):
	case ENUM_CLASS(BossStateID::TURN_R):
		m_eCurrentState = EFuocoState::TURN;
		break;
	case ENUM_CLASS(BossStateID::PARALYZATION_START):
	case ENUM_CLASS(BossStateID::PARALYZATION_LOOP):
	case ENUM_CLASS(BossStateID::PARALYZATION_END):
		m_eCurrentState = EFuocoState::PARALYZATION;
		break;
	case ENUM_CLASS(BossStateID::FATAL_START):
	case ENUM_CLASS(BossStateID::FATAL_LOOP):
	case ENUM_CLASS(BossStateID::FATAL_END):
		m_eCurrentState = EFuocoState::FATAL;
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
	default:
		m_eCurrentState = EFuocoState::ATTACK;
		break;
	}
}



_float CFuoco::Get_DistanceToPlayer() const
{
	if (!m_pPlayer)
		return FLT_MAX;

	_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
	_vector vThisPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vDiff = vPlayerPos - vThisPos;
	_float fDistance = XMVectorGetX(XMVector3Length(vDiff));

	return fDistance;
}

_bool CFuoco::IsTargetInFront(_float fDectedAngle) const
{
	if (!m_pPlayer)
		return false;

	_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
	_vector vThisPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vForward = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
	_vector vToPlayerXZ = XMVectorSetY(vPlayerPos - vThisPos, 0.f);
	_float fDot = XMVectorGetX(XMVector3Dot(vForward, vToPlayerXZ));
	fDot = clamp(fDot, -1.f, 1.f); // -1 ~ 1 사이로 제한
	_float fAngle = cosf(XMConvertToRadians(30.f)); // 시야각 60도 기준

	return fDot > fAngle; // 앞쪽에 있으면 true
}

_vector CFuoco::GetTargetDirection() const
{
	if (!m_pPlayer)
		return XMVectorZero();
	_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
	_vector vThisPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vToPlayer = vPlayerPos - vThisPos;
	return XMVector3Normalize(vToPlayer); // 플레이어 방향 벡터 반환
}

void CFuoco::ApplyRootMotionDelta(_float fTimeDelta)
{
	_float3	 rootMotionDelta = m_pAnimator->GetRootMotionDelta();
	_float4  rootMotionQuat = m_pAnimator->GetRootRotationDelta();
	_vector vLocal = XMLoadFloat3(&rootMotionDelta);
	vLocal = XMVectorScale(vLocal, m_fRootMotionAddtiveScale);
	_vector vRotQuat = XMQuaternionNormalize(XMLoadFloat4(&rootMotionQuat));

	_vector vScale, vCurRotQuat, vTrans;
	XMMatrixDecompose(&vScale, &vCurRotQuat, &vTrans, m_pTransformCom->Get_WorldMatrix());
	_vector vNewRotQut = XMQuaternionNormalize(XMQuaternionMultiply(vRotQuat, vCurRotQuat));

	_vector vWorldDelta = XMVector3Transform(vLocal, XMMatrixRotationQuaternion(vNewRotQut));
	vWorldDelta = XMVectorSetY(vWorldDelta, 0.f);
	_float fDeltaMag = XMVectorGetX(XMVector3Length(vWorldDelta));
	_vector finalDelta = vWorldDelta;
	if (fDeltaMag > m_fSmoothThreshold)
	{
		_float alpha = clamp(fTimeDelta * m_fSmoothSpeed, 0.f, 1.f);
		finalDelta = XMVectorLerp(m_PrevWorldDelta, vWorldDelta, alpha);
	}

	m_PrevWorldDelta = finalDelta;
	_vector vNext = XMVectorAdd(vTrans, finalDelta);
	if (m_pNaviCom)
	{
		if (m_pNaviCom->isMove(vNext))
		{
			_float fY = m_pNaviCom->Compute_NavigationY(vNext);
			vTrans = XMVectorSetY(vNext, fY);
		}
	}
	_matrix newWorld = XMMatrixScalingFromVector(vScale) * XMMatrixRotationQuaternion(vNewRotQut) * XMMatrixTranslationFromVector(vTrans);
	m_pTransformCom->Set_WorldMatrix(newWorld);
}

void CFuoco::UpdateNormalMove(_float fTimeDelta)
{
	if (m_eCurrentState == EFuocoState::WALK || m_eCurrentState == EFuocoState::RUN)
	{
		_vector vTargetPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);

		_int iMoveDir = m_pAnimator->GetInt("MoveDir");

		switch (iMoveDir)
		{
		case ENUM_CLASS(EMoveDirection::FRONT):
			m_pTransformCom->ChaseWithOutY(vTargetPos, fTimeDelta, CHASING_DISTANCE, nullptr, m_pNaviCom);
			break;
		case ENUM_CLASS(EMoveDirection::BACK):
			m_pTransformCom->Go_Backward(fTimeDelta, nullptr, m_pNaviCom);
			break;
		case ENUM_CLASS(EMoveDirection::LEFT):
			m_pTransformCom->Go_Left(fTimeDelta, nullptr, m_pNaviCom);
			break;
		case ENUM_CLASS(EMoveDirection::RIGHT):
			m_pTransformCom->Go_Right(fTimeDelta, nullptr, m_pNaviCom);
			break;
		}

		_float fY = m_pNaviCom->Compute_NavigationY(m_pTransformCom->Get_State(STATE::POSITION));
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION), fY));
	}
}

_bool CFuoco::UpdateTurnDuringAttack(_float fTimeDelta)
{
	if (m_fTurnTimeDuringAttack >= 0.001f)
	{
		_vector vDir = GetTargetDirection();
		vDir = XMVectorSetY(vDir, 0.f);
		vDir = XMVector3Normalize(vDir);
		m_pTransformCom->RotateToDirectionSmoothly(vDir, fTimeDelta * m_fAddtiveRotSpeed);
		m_fTurnTimeDuringAttack -= fTimeDelta;
		return false;
	}

	m_fAddtiveRotSpeed = 1.f;
	m_fTurnTimeDuringAttack = 0.f; // 초기화
	return true;
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
	}
	break;
	case Client::CFuoco::SlamFury:
		SetTurnTimeDuringAttack(1.5f, 1.3f);
	case Client::CFuoco::FootAtk:
		SetTurnTimeDuringAttack(1.f);
		break;
	case Client::CFuoco::SlamAtk:
		SetTurnTimeDuringAttack(1.f);
		break;
	case Client::CFuoco::Uppercut:
		SetTurnTimeDuringAttack(1.f);
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
		break;
	case Client::CFuoco::P2_FireBall:
	{
		_int iDir = GetRandomInt(0, 2);
		m_pAnimator->SetInt("Direction", iDir);

	}
	break;
	case Client::CFuoco::P2_FireBall_B:
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
		[this](const string& eventName)
		{
			CCamera_Manager::Get_Instance()->Shake_Camera(0.15f, 0.2f);
		});
	m_pAnimator->RegisterEventListener("IsFront",
		[this](const string& eventName)
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
	m_pAnimator->RegisterEventListener("Turnning", [this](const string&)
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

	m_pAnimator->RegisterEventListener("ResetAnim", [this](const string& eventName)
		{
			m_pAnimator->GetCurrentAnim()->ResetTrack();
		});

	m_pAnimator->RegisterEventListener("SlowAnimSpeed", [this](const string&) {
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(
			m_pAnimator->GetCurrentAnim()->GetTickPerSecond() * 0.5f);
		});
	m_pAnimator->RegisterEventListener("ResetAnimSpeed", [this](const string&) {
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(
			m_pAnimator->GetCurrentAnim()->GetTickPerSecond() * 1.5f);
		});

	m_pAnimator->RegisterEventListener("CollidersOff", [this](const string&) {
		m_pPhysXActorCom->Set_ShapeFlag(false, false, false);
		m_pPhysXActorComForArm->Set_ShapeFlag(false, false, false);
		m_pPhysXActorComForFoot->Set_ShapeFlag(false, false, false);
		});

	m_pAnimator->RegisterEventListener("FireBall", [this](const string& eventName)
		{
			FireProjectile(ProjectileType::FireBall, 25.f);
		});

	m_pAnimator->RegisterEventListener("FireOilFirst", [this](const string& eventName)
		{
			FireProjectile(ProjectileType::Oil);
		});
	m_pAnimator->RegisterEventListener("FireOilSecond", [this](const string& eventName)
		{
			FireProjectile(ProjectileType::Oil, 5.f);
		});

	m_pAnimator->RegisterEventListener("FireBallCombo", [this](const string& eventName)
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

	m_pAnimator->RegisterEventListener("Flamethrower", [this](const string& eventName)
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
		desc.fGravityOnDist = Get_DistanceToPlayer() * 0.2f;
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
		lstrcpy(desc.szName, TEXT("FireBall"));
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
			if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_FireBall"), iLevelIndex, TEXT("Layer_Projectile"), &desc)))
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
		if (CalculateCurrentHpRatio() <= 0.2f && !m_bUsedFlameFiledOnLowHp)
		{
			m_pAnimator->SetInt("SkillType", P2_FlameField);
			m_bUsedFlameFiledOnLowHp = true;
		}
		else
		{
			m_pAnimator->SetInt("SkillType", StrikeFury);
			m_pAnimator->SetTrigger("Phase2Start");
			m_bStartPhase2 = false;
			m_bIsPhase2 = true;
		}
		m_pAnimator->SetBool("Move", false);
		m_pAnimator->SetTrigger("Attack");

		return true;
	}
	return false;
}


void CFuoco::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (pOther)
	{
		if (eColliderType == COLLIDERTYPE::PLAYER)
		{
			if (m_pAnimator->GetInt("SkillType") == FootAtk)
			{
				m_pAnimator->SetBool("IsHit", true);
				SetTurnTimeDuringAttack(2.f, 1.2f); // 퓨리 어택 
			}
		}
		if (eColliderType == COLLIDERTYPE::PLAYER_WEAPON)
		{
			m_fHP -= 5.f;
			cout << "푸오코 현재 체력 : " << m_fHP << endl;
		}
	}
}

void CFuoco::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFuoco::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
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
	Safe_Release(m_pNaviCom);
	Safe_Release(m_pPhysXActorCom);
	Safe_Release(m_pPhysXActorComForArm);
	Safe_Release(m_pPhysXActorComForFoot);

}
