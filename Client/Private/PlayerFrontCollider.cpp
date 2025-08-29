#include "PlayerFrontCollider.h"

#include "GameInstance.h"
#include "Player.h"

CPlayerFrontCollider::CPlayerFrontCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}
CPlayerFrontCollider::CPlayerFrontCollider(const CPlayerFrontCollider& Prototype)
	: CGameObject(Prototype)
{
}
HRESULT CPlayerFrontCollider::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayerFrontCollider::Initialize(void* pArg)
{
	FRONTCOLLIDER_DESC* pDesc = static_cast<FRONTCOLLIDER_DESC*>(pArg);
	
	m_InitPos = pDesc->InitPos;
	m_InitScale = pDesc->InitScale;
	m_szName = pDesc->szName;
	m_pOwner = dynamic_cast<CPlayer*>(pDesc->pOwner);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;
	if (FAILED(Ready_Actor()))
		return E_FAIL;

	return S_OK;
}

void CPlayerFrontCollider::Priority_Update(_float fTimeDelta)
{
	if (m_pOwner)
	{
		_vector vPlayerPos = m_pOwner->Get_TransfomCom()->Get_State(STATE::POSITION);
		_vector vPlayerLook = m_pOwner->Get_TransfomCom()->Get_State(STATE::LOOK);
		vPlayerPos = XMVectorSetY(vPlayerPos, XMVectorGetY(vPlayerPos) + 0.7f);
		vPlayerPos += vPlayerLook * 1.f;
		m_pTransformCom->Set_State(STATE::POSITION, vPlayerPos);
		Update_Collider_Actor();
	}
}

void CPlayerFrontCollider::Update(_float fTimeDelta)
{
}

void CPlayerFrontCollider::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CPlayerFrontCollider::Render()
{
#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider() && m_pPhysXActorCom->Get_ReadyForDebugDraw()) {
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif

	return S_OK;
}


HRESULT CPlayerFrontCollider::Ready_Components()
{
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"),
		TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CPlayerFrontCollider::Ready_Actor()
{
	// 3. Transform에서 S, R, T 분리
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 3-1. 스케일, 회전, 위치 변환
	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxVec3 halfExtents = PxVec3(0.2f, 1.f, 0.2f);
	PxSphereGeometry geom = m_pGameInstance->CookSphereGeometry(0.05f);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(true, false, false);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_INTERACT;
	filterData.word1 = WORLDFILTER::FILTER_MONSTERBODY;
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::TRIGGER);
	m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());


	return S_OK;
}

void CPlayerFrontCollider::Update_Collider_Actor()
{
	// 1. 월드 행렬 가져오기
	_matrix worldMatrix = m_pTransformCom->Get_WorldMatrix();

	// 2. 위치 추출
	_float4 vPos;
	XMStoreFloat4(&vPos, worldMatrix.r[3]);

	PxVec3 pos(vPos.x, vPos.y, vPos.z);
	pos.y += 0.5f;

	// 3. 회전 추출
	XMVECTOR boneQuat = XMQuaternionRotationMatrix(worldMatrix);
	XMFLOAT4 fQuat;
	XMStoreFloat4(&fQuat, boneQuat);
	PxQuat rot = PxQuat(fQuat.x, fQuat.y, fQuat.z, fQuat.w);

	// 4. PhysX Transform 적용
	m_pPhysXActorCom->Set_Transform(PxTransform(pos, rot));

}

void CPlayerFrontCollider::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}
void CPlayerFrontCollider::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{

	// 들어온 몬스터의 상태를 가져온다.
	CUnit* pUnit = dynamic_cast<CUnit*>(pOther);
	if (!pUnit)
		return;


	
	
	if ((m_pOwner)->Get_PlayerState() == EPlayerState::FATAL)
	{
		return;
	}

	const _float fBackDotThreshold = 0.9f;

	// 위치/룩 수집
	_vector vMonsterPos = pOther->Get_TransfomCom()->Get_State(STATE::POSITION);
	_vector vMonsterLook = pOther->Get_TransfomCom()->Get_State(STATE::LOOK);

	_vector vPlayerPos = m_pOwner->Get_TransfomCom()->Get_State(STATE::POSITION);
	_vector vPlayerLook = m_pOwner->Get_TransfomCom()->Get_State(STATE::LOOK);

	// 평면(XZ) 투영 + 정규화
	_vector vMonsterLookFlat = XMVector3Normalize(ProjectToXZ(vMonsterLook));
	_vector vPlayerLookFlat = XMVector3Normalize(ProjectToXZ(vPlayerLook));

	_vector vDirPlayerToMonster = XMVector3Normalize(ProjectToXZ(XMVectorSubtract(vMonsterPos, vPlayerPos)));
	_vector vDirMonsterToPlayer = XMVector3Normalize(ProjectToXZ(XMVectorSubtract(vPlayerPos, vMonsterPos)));

	_bool bPlayerBehindMonster = (XMVectorGetX(XMVector3Dot(vMonsterLookFlat, vDirPlayerToMonster)) >= fBackDotThreshold);

	
	if (bPlayerBehindMonster)
	{
		m_pOwner->SetbIsBackAttack(true);
		m_pOwner->SetFatalTarget(pUnit);

		/* [ 보스 콜라이더 타입 바뀌면 바꿔야함 ] */
		if (eColliderType == COLLIDERTYPE::MONSTER)
			m_pOwner->SetIsFatalBoss(false);
		else if (eColliderType == COLLIDERTYPE::MONSTER)
			m_pOwner->SetIsFatalBoss(false);
	}
	else
	{
		m_pOwner->SetbIsBackAttack(false);
		m_pOwner->SetFatalTargetNull();
	}
}
void CPlayerFrontCollider::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if ((m_pOwner)->Get_PlayerState() == EPlayerState::FATAL)
	{
		return;
	}

	m_pOwner->SetbIsBackAttack(false);
	
	
}

void CPlayerFrontCollider::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	
}
void CPlayerFrontCollider::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}



CPlayerFrontCollider* CPlayerFrontCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPlayerFrontCollider* pInstance = new CPlayerFrontCollider(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPlayerFrontCollider");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CPlayerFrontCollider::Clone(void* pArg)
{
	CPlayerFrontCollider* pInstance = new CPlayerFrontCollider(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPlayerFrontCollider");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPlayerFrontCollider::Free()
{
	__super::Free();

	Safe_Release(m_pPhysXActorCom);
}
