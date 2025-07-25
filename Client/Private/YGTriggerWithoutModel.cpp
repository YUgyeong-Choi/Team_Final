#include "YGTrrigerWithoutModel.h"

#include "GameInstance.h"
#include "PhysX_IgnoreSelfCallback.h"

CYGTrrigerWithoutModel::CYGTrrigerWithoutModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CYGTrrigerWithoutModel::CYGTrrigerWithoutModel(const CYGTrrigerWithoutModel& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CYGTrrigerWithoutModel::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CYGTrrigerWithoutModel::Initialize(void* pArg)
{
	CGameObject::GAMEOBJECT_DESC _desc{};
	lstrcpy(_desc.szName, TEXT("YGTrigger"));
	_desc.fRotationPerSec = 8.f;
	_desc.fSpeedPerSec = 10.f;

	if (FAILED(__super::Initialize(&_desc))) {
		return E_FAIL;
	}

	if (FAILED(Ready_Components())) {
		return E_FAIL;
	}

	_fvector vPos{ 0.0f, 0.f, -20.0f, 1.0f };
	m_pTransformCom->Set_State(STATE::POSITION, vPos);

	// Tranform위치를 이동해준 뒤 콜라이더를 생성해서 맨 처음 시작할때 충돌안 됨
	if (FAILED(Ready_Collider())) {
		return E_FAIL;
	}

	return S_OK;
}

void CYGTrrigerWithoutModel::Priority_Update(_float fTimeDelta)
{
	if (m_bDead) {
		PxScene* pScene = m_pGameInstance->Get_Scene();
		if (pScene)
			pScene->removeActor(*m_pPhysXActorCom->Get_Actor());

		Safe_Release(m_pPhysXActorCom);
		m_pPhysXActorCom = nullptr;
	}

}

void CYGTrrigerWithoutModel::Update(_float fTimeDelta)
{

}

void CYGTrrigerWithoutModel::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CYGTrrigerWithoutModel::Render()
{
#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif


	return S_OK;
}

void CYGTrrigerWithoutModel::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	printf("YGTrigger 충돌 시작!\n");
}

void CYGTrrigerWithoutModel::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CYGTrrigerWithoutModel::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	printf("YGTrigger 충돌 종료!\n");
}

void CYGTrrigerWithoutModel::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	wprintf(L"YGTrigger Hit: %s\n", pOther->Get_Name().c_str());
}

void CYGTrrigerWithoutModel::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	wprintf(L"YGTrigger Trriger 시작: %s\n", pOther->Get_Name().c_str());
}

void CYGTrrigerWithoutModel::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	wprintf(L"YGTrigger Trriger 종료: %s\n", pOther->Get_Name().c_str());
}

HRESULT CYGTrrigerWithoutModel::Ready_Components()
{
	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CYGTrrigerWithoutModel::Ready_Collider()
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

	PxVec3 halfExtents = PxVec3(1.f, 2.f, 3.f);
	PxBoxGeometry  geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(false, true, false);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::TRIGGER);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

CYGTrrigerWithoutModel* CYGTrrigerWithoutModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CYGTrrigerWithoutModel* pInstance = new CYGTrrigerWithoutModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CYGTrrigerWithoutModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CYGTrrigerWithoutModel::Clone(void* pArg)
{
	CYGTrrigerWithoutModel* pInstance = new CYGTrrigerWithoutModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CYGTrrigerWithoutModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CYGTrrigerWithoutModel::Free()
{
	if (m_pPhysXActorCom) {
		PxScene* pScene = m_pGameInstance->Get_Scene();
		if (pScene)
			pScene->removeActor(*m_pPhysXActorCom->Get_Actor());
	}

	Safe_Release(m_pPhysXActorCom);

	__super::Free();
}
