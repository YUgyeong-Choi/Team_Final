#include "Fuoco.h"
#include "Bone.h"
#include "GameInstance.h"
#include "LockOn_Manager.h"
#include "Client_Calculation.h"

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
	UnitDesc.fRotationPerSec = XMConvertToRadians(120.f);
	UnitDesc.fSpeedPerSec = 3.f;
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
	
	m_pPlayer = m_pGameInstance->Get_Object(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"),0);

	if (m_pNaviCom)
	{
	m_pNaviCom->Select_Cell(m_pTransformCom->Get_State(STATE::POSITION));
	_float fY = m_pNaviCom->Compute_NavigationY(m_pTransformCom->Get_State(STATE::POSITION));
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION), fY));
	}
	return S_OK;
}

void CFuoco::Priority_Update(_float fTimeDelta)
{
#ifdef _DEBUG
	if (KEY_DOWN(DIK_TAB))
	{
		cout << "현재 플레이어와의 거리 : " << Get_DistanceToPlayer() << endl;
		cout << "현재 애니메이션 상태 : " << m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName << endl;
		//if (m_bStartPhase2 == false)
		//	m_bStartPhase2 = true;
		m_pAnimator->SetInt("SkillType", FootAtk);
		m_pAnimator->SetTrigger("Attack");	
	}
#endif
}

void CFuoco::Update(_float fTimeDelta)
{
	UpdateBossState(fTimeDelta); // 상태 업데이트
	__super::Update(fTimeDelta); // 애니메이션 재생
	Update_Collider(); // 콜라이더 업데이트

	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom)) 
	{
		CLockOn_Manager::Get_Instance()->Add_LockOnTarget(this);
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

void CFuoco::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (pOther)
	{
		if (eColliderType == COLLIDERTYPE::PALYER)
		{
			if (m_pAnimator->GetInt("SkillType") == FootAtk)
			{
				m_pAnimator->SetBool("IsHit", true);
				SetTurnTimeDuringAttack(2.f); // 퓨리 어택 
			}
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
	_wstring wsPrototypeTag = TEXT("Prototype_Component_Navigation_");

	switch (iLevelIndex)
	{
	case ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION):
		wsPrototypeTag += TEXT("STATION");
		break;
	case ENUM_CLASS(LEVEL::KRAT_HOTEL):
		wsPrototypeTag += TEXT("HOTEL");
		break;
	default:
		return E_FAIL;
	}


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

	PxVec3 halfExtents = PxVec3(scaleVec.x * 1.2f, scaleVec.y*1.7f, scaleVec.z * 0.8f);
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY | FILTER_PLAYERWEAPON; // 일단 보류
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
		armFilterData.word1 = WORLDFILTER::FILTER_PLAYERBODY | FILTER_PLAYERWEAPON; // 일단 보류
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
		footFilterData.word1 = WORLDFILTER::FILTER_PLAYERBODY | FILTER_PLAYERWEAPON; // 일단 보류
		m_pPhysXActorComForFoot->Set_SimulationFilterData(footFilterData);
		m_pPhysXActorComForFoot->Set_QueryFilterData(footFilterData);
		m_pPhysXActorComForFoot->Set_Owner(this);
		m_pPhysXActorComForFoot->Set_ColliderType(COLLIDERTYPE::MONSTER);
		m_pPhysXActorComForFoot->Set_Kinematic(true);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorComForFoot->Get_Actor());
	}


	return S_OK;
}

void CFuoco::Update_Collider()
{
	_matrix worldMatrix = m_pTransformCom->Get_WorldMatrix();

	_float4 vPos;
	XMStoreFloat4(&vPos, worldMatrix.r[3]);

	PxVec3 pos(vPos.x, vPos.y+2.f, vPos.z);
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

	_float fDistance = Get_DistanceToPlayer();
	_bool bCanMove = fDistance >= CHASING_DISTANCE && m_eCurrentState != EFuocoState::ATTACK &&
		m_eCurrentState != EFuocoState::GROGGY && m_eCurrentState != EFuocoState::DEAD && !m_bIsFirstAttack;
	m_pAnimator->SetBool("Move", bCanMove);

	if (bCanMove)
	{
		m_pAnimator->SetInt("MoveDir", 0);
	}
	else
	{
		_int iMoveDir = GetRandomInt(1, 3);
		m_pAnimator->SetInt("MoveDir", iMoveDir);
	}


	UpdateAttackPattern(fDistance, fTimeDelta); // 공격 패턴 업데이트

	switch (iCurrentAnimStateNodeID)
	{
	  case ENUM_CLASS(BossStateID::IDLE):
		  m_eCurrentState = EFuocoState::IDLE;
		break;
	  case ENUM_CLASS(BossStateID::WALK_B):
	  case ENUM_CLASS(BossStateID::WALK_F):
	  case ENUM_CLASS(BossStateID::WALK_R):
	  case ENUM_CLASS(BossStateID::WALK_L):
		  m_eCurrentState = EFuocoState::WALK;
		  break;
	  case ENUM_CLASS(BossStateID::GROGGY_END):
	  case ENUM_CLASS(BossStateID::GROGGY_START):
	  case ENUM_CLASS(BossStateID::GROGGY_LOOP):
		  m_eCurrentState = EFuocoState::GROGGY;
		  break;
	  case ENUM_CLASS(BossStateID::DEAD_B):
	  case ENUM_CLASS(BossStateID::DEAD_F):
	  case ENUM_CLASS(BossStateID::SPECIAL_DIE):
		  m_eCurrentState = EFuocoState::DEAD;
	  case ENUM_CLASS(BossStateID::TURN_L):
	  case ENUM_CLASS(BossStateID::TURN_R):
		  m_eCurrentState = EFuocoState::TURN;
		  break;
	  default:
		  m_eCurrentState = EFuocoState::ATTACK;
			  break;
	}
	if (m_eCurrentState != EFuocoState::ATTACK)
	{
		m_pAnimator->SetBool("IsHit", false);
		m_pAnimator->SetBool("IsFront", IsTargetInFront());
	}
	UpdateMove(fTimeDelta);
}

void CFuoco::UpdateMove(_float fTimeDelta)
{
	_bool bIsRootMotion = m_pAnimator->GetCurrentAnim()->IsRootMotionEnabled();
	_float fDistance = Get_DistanceToPlayer();

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


	_bool bIsTurn = abs(XMConvertToDegrees(fYaw)) > MINIMUM_TURN_ANGLE &&( m_eCurrentState == EFuocoState::IDLE||
		m_eCurrentState == EFuocoState::WALK);
	
	if (bIsTurn)
	{
		m_pAnimator->SetTrigger("Turn");
		m_pAnimator->SetInt("TurnDir", (fYaw >= 0.f) ? 0 : 1); // 0: 오른쪽, 1: 왼쪽
#ifdef _DEBUG
		cout << "회전 각도: " << XMConvertToDegrees(fYaw) << "도" << endl;
		cout << "회전 방향 : " << ((fYaw >= 0.f) ? "오른쪽" : "왼쪽") << endl;
#endif
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
		m_fAttackCooldown = 3.f;
		SetTurnTimeDuringAttack(1.f);
		return;
	}

	if (m_bStartPhase2)
	{
		m_pAnimator->SetTrigger("Attack");
		m_pAnimator->SetInt("SkillType", StrikeFury);
		m_pAnimator->SetTrigger("Phase2Start");
		m_pAnimator->SetBool("Move", false);
		m_bStartPhase2 = false;
		m_bIsPhase2 = true;
		m_fAttackCooldown = 10.f;
	}

	if (false == UpdateTurnDuringAttack(fTimeDelta))
	{
		return;
	}

	if (m_eCurrentState == EFuocoState::ATTACK)
	{
		return;
	}

	if (m_fAttackCooldown >= 0.f)
	{
		m_fAttackCooldown -= fTimeDelta;
		return;
	}

	EBossAttackPattern eSkillType = BAP_NONE; 
	while (!IsValidAttackType(eSkillType))
	{
		eSkillType = static_cast<EBossAttackPattern>(GetRandomInt(1, m_bIsPhase2 ? 14 : 9));
	}

	SetupAttackByType(eSkillType);

	m_pAnimator->SetInt("SkillType", eSkillType);
	m_pAnimator->SetTrigger("Attack");


	m_fAttackCooldown = 3.f;
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

_bool CFuoco::IsTargetInFront() const
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
	if (m_fTurnTimeDuringAttack >= 0.000001f)
	{
		_vector vDir = GetTargetDirection();
		vDir = XMVectorSetY(vDir, 0.f);
		vDir = XMVector3Normalize(vDir);
		m_pTransformCom->RotateToDirectionSmoothly(vDir, fTimeDelta);
		m_fTurnTimeDuringAttack -= fTimeDelta;
		cout << "회전 시간 남음: " << m_fTurnTimeDuringAttack << endl;
		return false;
	}

	m_fTurnTimeDuringAttack = 0.f; // 초기화
	return true;
}

void CFuoco::SetupAttackByType(EBossAttackPattern ePattern)
{

	switch (ePattern)
	{
	case Client::CFuoco::SwingAtk:
	{
		_bool bIsCombo = GetRandomInt(0, 1) == 1;
		m_pAnimator->SetBool("IsCombo", bIsCombo);
		if (bIsCombo)
		{
			_int iComboType = GetRandomInt(1, 2);
			m_pAnimator->SetInt("SwingCombo", iComboType);
		}
	}
		break;
	case Client::CFuoco::FootAtk:
		break;
	case Client::CFuoco::SlamAtk:
		break;
	case Client::CFuoco::Uppercut:
		m_pAnimator->SetBool("IsFront", IsTargetInFront());
		break;
	case Client::CFuoco::StrikeFury:
		break;
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
