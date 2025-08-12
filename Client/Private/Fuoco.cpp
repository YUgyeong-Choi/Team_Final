#include "Fuoco.h"
#include "Bone.h"
#include "GameInstance.h"
#include <Client_Calculation.h>

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
	UnitDesc.fRotationPerSec = XMConvertToRadians(90.f);
	UnitDesc.fSpeedPerSec = 20.f;
	lstrcpy(UnitDesc.szName, TEXT("FireEater"));
	UnitDesc.szMeshID = TEXT("FireEater");
	//UnitDesc.InitPos = _float3(5.f, 0.f, 0.f);
	UnitDesc.InitPos = _float3(55.5f, 0.f, -7.5f);
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

	return S_OK;
}

void CFuoco::Priority_Update(_float fTimeDelta)
{
	if (KEY_DOWN(DIK_TAB))
	{
		cout << "현재 플레이어와의 거리 : " << Get_DistanceToPlayer() << endl;
		cout << "현재 애니메이션 상태 : " << m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName << endl;
	}
}

void CFuoco::Update(_float fTimeDelta)
{
	UpdateBossState(fTimeDelta); // 상태 업데이트
	__super::Update(fTimeDelta); // 애니메이션 재생
	Update_Collider(); // 콜라이더 업데이트
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
}

void CFuoco::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFuoco::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
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
	PxMeshScale meshScale(scaleVec);

	PxVec3 halfExtents = PxVec3(scaleVec.x * 1.2f, scaleVec.y, scaleVec.z * 0.8f);
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
		if (!strcmp(pBone->Get_Name(), "Bone001-Ring-Finger01"))
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
		PxVec3 armScaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S)) * 0.5f;
		PxQuat armRotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
		PxVec3 armPositionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
		PxTransform armPose(armPositionVec, armRotationQuat);
		PxMeshScale armMeshScale(armScaleVec);
		PxVec3 armHalfExtents = PxVec3(armScaleVec.x, armScaleVec.y, armScaleVec.z);
		PxBoxGeometry armGeom = m_pGameInstance->CookBoxGeometry(armHalfExtents);
		m_pPhysXActorComForArm->Create_Collision(m_pGameInstance->GetPhysics(), armGeom, armPose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorComForArm->Set_ShapeFlag(true, false, true);
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
		PxVec3 footScaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S)) * 0.5f;
		PxQuat footRotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
		PxVec3 footPositionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
		PxTransform footPose(footPositionVec, footRotationQuat);
		PxMeshScale footMeshScale(footScaleVec);
		PxVec3 footHalfExtents = PxVec3(footScaleVec.x, footScaleVec.y, footScaleVec.z);
		PxBoxGeometry footGeom = m_pGameInstance->CookBoxGeometry(footHalfExtents);
		m_pPhysXActorComForFoot->Create_Collision(m_pGameInstance->GetPhysics(), footGeom, footPose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorComForFoot->Set_ShapeFlag(true, false, true);
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

	PxVec3 pos(vPos.x, vPos.y, vPos.z);
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
		return; // 컷신 중에는 상태 업데이트를 하지 않음

	_float fDistance = Get_DistanceToPlayer();
	_bool bCanMove = fDistance >= CHASING_DISTANCE&& m_eCurrentState!= EFuocoState::ATTACK;
	m_pAnimator->SetBool("Move", bCanMove);

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
	  default:
		  m_eCurrentState = EFuocoState::ATTACK;
			  break;
	}

	UpdateMove(fTimeDelta);
}

void CFuoco::UpdateMove(_float fTimeDelta)
{
	_bool bIsRootMotion = m_pAnimator->GetCurrentAnim()->IsRootMotionEnabled();
	_float fDistance = Get_DistanceToPlayer();
	if (ATTACK_DISTANCE >= fDistance)
	{
		return;
	}

	if (bIsRootMotion)
	{
		_float3 vDelta = m_pAnimator->GetRootMotionDelta();
		_vector vCurrentPos = m_pTransformCom->Get_State(STATE::POSITION);
		_vector vNewPos = vCurrentPos + XMLoadFloat3(&vDelta);
		m_pTransformCom->Set_State(STATE::POSITION, vNewPos);
	}
	else
	{
		_vector vTargetDirection = GetTargetDirection();
		if (XMVector3Equal(vTargetDirection, XMVectorZero()))
			return; // 방향이 0이면 이동하지 않음
		// 이동 속도 적용
		vTargetDirection = XMVector3Normalize(vTargetDirection);
		m_pTransformCom->Go_Dir(vTargetDirection, fTimeDelta);

	}
	_vector vDir = GetTargetDirection();
	vDir = XMVector3Normalize(vDir);
	vDir = XMVectorSetZ(vDir, 0.f); // Z축은 0으로 설정하여 평면 이동
	vDir = XMVectorSetX(vDir, 0.f);
	vDir = XMVectorNegate(vDir); // 플레이어 방향을 바라보도록 반전
	// 현재 상태에서 많이 회전했으면 애니메이터 Turn true
	_vector vCurrentLook = m_pTransformCom->Get_State(STATE::LOOK);
	_float fDot = XMVectorGetX(XMVector3Dot(vCurrentLook, vDir));
	//if (fDot < 0.5f) // 0.5f는 회전 임계값, 필요에 따라 조정
	//{
	//	m_pAnimator->SetBool("Turn", true);
	//}
	//else
	//{
	//	m_pAnimator->SetBool("Turn", false);
	//}
	m_pTransformCom->Turn(vDir, fTimeDelta);
}

void CFuoco::UpdateAttackPattern(_float fDistance, _float fTimeDelta)
{
	if (m_eCurrentState == EFuocoState::ATTACK)
		return;
	if (m_fAttackCooldown >= 0.f)
	{
		m_fAttackCooldown -= fTimeDelta;
		return;
	}

	_int iSkillType = GetRandomInt(0, 14);
	m_pAnimator->SetInt("SkillType", iSkillType);
	m_pAnimator->SetTrigger("Attack");


	m_fAttackCooldown = 5.f;
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
	_vector vForward = m_pTransformCom->Get_State(STATE::LOOK);
	_vector vToPlayer = vPlayerPos - vThisPos;
	_float fDot = XMVectorGetX(XMVector3Dot(vForward, vToPlayer));
	return fDot > 0.f; // 앞쪽에 있으면 true
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
	Safe_Release(m_pPhysXActorCom);
	Safe_Release(m_pPhysXActorComForArm);
	Safe_Release(m_pPhysXActorComForFoot);

	__super::Free();
}
