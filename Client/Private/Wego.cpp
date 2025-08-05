#include "Wego.h"

#include "GameInstance.h"
CWego::CWego(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUnit(pDevice, pContext)
{
}

CWego::CWego(const CWego& Prototype)
	: CUnit(Prototype)
{
}

HRESULT CWego::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWego::Initialize(void* pArg)
{
	UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	/* [ 초기화 위치값 ] */
	m_pTransformCom->Set_State(STATE::POSITION, _vector{ m_InitPos.x, m_InitPos.y, m_InitPos.z });
	//m_pTransformCom->Rotation(XMConvertToRadians(0.f), XMConvertToRadians(90.f), XMConvertToRadians(0.f));
	m_pTransformCom->SetUp_Scale(pDesc->InitScale.x, pDesc->InitScale.y, pDesc->InitScale.z);

	/* [ 트리거 생성 ] */
	if (FAILED(Ready_Trigger()))
		return E_FAIL;

	/* [ 충돌체 생성 ] */
	if (FAILED(Ready_Collider()))
		return E_FAIL;

	

	return S_OK;
}

void CWego::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CWego::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CWego::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CWego::Render()
{
	__super::Render();

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		m_pGameInstance->Add_DebugComponent(m_pTriggerBoxCom);
	}
#endif

	return S_OK;
}

HRESULT CWego::Ready_Components()
{
	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Static"),
		TEXT("Com_PhysXTrigger"), reinterpret_cast<CComponent**>(&m_pTriggerBoxCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Static"),
		TEXT("Com_PhysXCollider"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CWego::Ready_Trigger()
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
	m_pTriggerBoxCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pTriggerBoxCom->Set_ShapeFlag(false, true, false);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
	m_pTriggerBoxCom->Set_SimulationFilterData(filterData);
	m_pTriggerBoxCom->Set_QueryFilterData(filterData);
	m_pTriggerBoxCom->Set_Owner(this);
	m_pTriggerBoxCom->Set_ColliderType(COLLIDERTYPE::TRIGGER);
	m_pGameInstance->Get_Scene()->addActor(*m_pTriggerBoxCom->Get_Actor());

	return S_OK;
}

HRESULT CWego::Ready_Collider()
{
	return S_OK;
}

CWego* CWego::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWego* pInstance = new CWego(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWego");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CWego::Clone(void* pArg)
{
	CWego* pInstance = new CWego(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CWego");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CWego::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAnimator);
	Safe_Release(m_pShaderCom);

	Safe_Release(m_pTriggerBoxCom);
	Safe_Release(m_pPhysXActorCom);

}
