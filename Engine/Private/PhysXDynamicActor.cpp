#include "PhysXDynamicActor.h"
#include "GameInstance.h"
CPhysXDynamicActor::CPhysXDynamicActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXActor{ pDevice, pContext }
	, m_bKinematic { m_bKinematic }
{
}

CPhysXDynamicActor::CPhysXDynamicActor(const CPhysXDynamicActor& Prototype)
	: CPhysXActor(Prototype),
	m_bKinematic(Prototype.m_bKinematic)
{
}

HRESULT CPhysXDynamicActor::Initialize_Prototype()
{
#ifdef _DEBUG
	ReadyForDebugDraw(m_pDevice, m_pContext);
#endif
	return S_OK;
}

HRESULT CPhysXDynamicActor::Initialize(void* pArg)
{

	return S_OK;
}

HRESULT CPhysXDynamicActor::Create_Collision(PxPhysics* physics, const PxGeometry& geom, const PxTransform& pose, PxMaterial* material)
{
    m_pMaterial = material;

    PxRigidDynamic* pDynamic = physics->createRigidDynamic(pose);
    if (!pDynamic)
        return E_FAIL;

    m_pShape = PxRigidActorExt::createExclusiveShape(*pDynamic, geom, *material);
    if (!m_pShape)
        return E_FAIL;


    // 충돌 속성 설정
    m_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
    m_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
    m_pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);

    m_pActor = pDynamic;
    pDynamic->userData = this;

    return S_OK;
}

HRESULT CPhysXDynamicActor::Render()
{
#ifdef _DEBUG
	PxTransform pose = PxShapeExt::getGlobalPose(*m_pShape, *m_pActor);
	PxGeometryHolder geom = m_pShape->getGeometry();
	PxBounds3 bounds = PxShapeExt::getWorldBounds(*m_pShape, *m_pActor);

	DebugRender(m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ),pose, geom, bounds);

	for (auto& Ray : m_RenderRay) {
		DrawRay(m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ), Ray.vStartPos, Ray.vDirection, Ray.fRayLength, Ray.bIsHit, Ray.vHitPos);
	}
	m_RenderRay.clear();
#endif
	return S_OK;
}

void CPhysXDynamicActor::Set_Kinematic(_bool bActiveKinematic)
{
	if (!m_pActor)
		return;

	PxRigidDynamic* pDynamic = m_pActor->is<PxRigidDynamic>();
	if (pDynamic)
		pDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, bActiveKinematic);

	m_bKinematic = bActiveKinematic;
}

void CPhysXDynamicActor::Set_ShapeFlag(_bool bSimulation, _bool bTrigger, _bool bQuery)
{
  	m_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, bSimulation); // OnEnter, OnStay, OnExit 활성화
	m_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, bTrigger); //OnTriger 활성화
	m_pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, bQuery); // Ray충돌 활성화 
#ifdef _DEBUG
	m_bReadyForDebugDraw = bSimulation || bTrigger;
#endif
}

void CPhysXDynamicActor::Set_SimulationFilterData(PxFilterData _data)
{
	if (!m_pShape)
	{
		return;
	}

	m_pShape->setSimulationFilterData(_data);

	// 모든 포인터 안전성 체크 후 즉시 적용
	if (Get_Actor() && Get_Actor()->getScene() && m_pGameInstance && m_pGameInstance->Get_Scene())
	{
		try
		{
			m_pGameInstance->Get_Scene()->resetFiltering(*Get_Actor());
		}
		catch (...)
		{
			// resetFiltering 실패 시에도 필터 데이터는 설정되었으므로 계속 진행
			OutputDebugString(L"[WARNING] resetFiltering failed, but filter data is set\n");
		}
	}

	m_filterData = _data;
#ifdef _DEBUG
	m_bReadyForDebugDraw = true;
#endif
}

void CPhysXDynamicActor::Init_SimulationFilterData()
{
	if (!m_pShape)
	{
		return;
	}

	PxFilterData filterData{};
	filterData.word0 = 0;
	filterData.word1 = 0;

	m_pShape->setSimulationFilterData(filterData);

	// 모든 포인터 안전성 체크 후 즉시 적용
	if (Get_Actor() && Get_Actor()->getScene() && m_pGameInstance && m_pGameInstance->Get_Scene())
	{
		try
		{
			m_pGameInstance->Get_Scene()->resetFiltering(*Get_Actor());
		}
		catch (...)
		{
			OutputDebugString(L"[WARNING] resetFiltering failed in Init\n");
		}
	}
#ifdef _DEBUG
	m_bReadyForDebugDraw = false;
#endif
}

void CPhysXDynamicActor::Set_QueryFilterData(PxFilterData _data)
{
	m_pShape->setQueryFilterData(_data);
}

void CPhysXDynamicActor::RemovePhysX()
{
	m_pGameInstance->Remove_TriggerRemoveActor(this, m_pTriggerEnterOthers);

	Get_Actor()->userData = nullptr;
	m_pGameInstance->Get_Scene()->removeActor(*Get_Actor());
}

void CPhysXDynamicActor::Shape_Detach()
{
	if (m_pActor == nullptr)
		return;

	PxShape* shapes[8];
	_uint count = m_pActor->getNbShapes();
	m_pActor->getShapes(shapes, count);

	for (_uint i = 0; i < count; ++i)
	{
		m_pActor->detachShape(*shapes[i]);
	}
}


CPhysXDynamicActor* CPhysXDynamicActor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPhysXDynamicActor* pInstance = new CPhysXDynamicActor(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPhysXDynamicActor");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CPhysXDynamicActor::Clone(void* pArg)
{
	CPhysXDynamicActor* pInstance = new CPhysXDynamicActor(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPhysXDynamicActor");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPhysXDynamicActor::Free()
{
	if (m_pActor)
	{
		m_pActor->release();
		m_pActor = nullptr;
	}
	__super::Free();
}
