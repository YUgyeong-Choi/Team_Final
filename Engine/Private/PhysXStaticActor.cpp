#include "PhysXStaticActor.h"
#include "GameInstance.h"
CPhysXStaticActor::CPhysXStaticActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysXActor{ pDevice, pContext }
{
}

CPhysXStaticActor::CPhysXStaticActor(const CPhysXStaticActor& Prototype)
	: CPhysXActor(Prototype)
{
}

HRESULT CPhysXStaticActor::Initialize_Prototype()
{
#ifdef _DEBUG
    ReadyForDebugDraw(m_pDevice, m_pContext);
#endif
    return S_OK;
}

HRESULT CPhysXStaticActor::Initialize(void* pArg)
{
    return S_OK;
}


HRESULT CPhysXStaticActor::Create_Collision(PxPhysics* physics, const PxGeometry& geom, const PxTransform& pose, PxMaterial* material)
{
    m_pMaterial = material;

    m_pActor = physics->createRigidStatic(pose);
    if (!m_pActor)
        return E_FAIL;

    m_pShape = PxRigidActorExt::createExclusiveShape(*m_pActor, geom, *material);
    if (!m_pShape)
        return E_FAIL;

    m_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false); // OnEnter, OnStay, OnExit 활성화
    m_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false); //OnTriger 활성화
    m_pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false); // Ray충돌 활성화 

    m_pActor->userData = this;
    return S_OK;
}

HRESULT CPhysXStaticActor::Render()
{
#ifdef _DEBUG
    PxTransform pose = PxShapeExt::getGlobalPose(*m_pShape, *m_pActor);
    PxGeometryHolder geom = m_pShape->getGeometry();
    PxBounds3 bounds = PxShapeExt::getWorldBounds(*m_pShape, *m_pActor);

    DebugRender(m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ), pose, geom, bounds);

    for (auto& Ray : m_RenderRay) {
        DrawRay(m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ), Ray.vStartPos, Ray.vDirection, Ray.fRayLength, Ray.bIsHit, Ray.vHitPos);
    }
    m_RenderRay.clear();
#endif
    return S_OK;
}

void CPhysXStaticActor::Set_ShapeFlag(_bool bSimulation, _bool bTrigger, _bool bQuery)
{
    m_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, bSimulation); // OnEnter, OnStay, OnExit 활성화
    m_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, bTrigger); //OnTriger 활성화
    m_pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, bQuery); // Ray충돌 활성화 
}

void CPhysXStaticActor::Set_SimulationFilterData(PxFilterData _data)
{
    m_pShape->setSimulationFilterData(_data);
    m_filterData = _data;
#ifdef _DEBUG
    m_bReadyForDebugDraw = true;
#endif
}

void CPhysXStaticActor::Init_SimulationFilterData()
{
    PxFilterData filterData{};
    filterData.word0 = 0;
    filterData.word1 = 0;
    m_pShape->setSimulationFilterData(filterData);
#ifdef _DEBUG
    m_bReadyForDebugDraw = false;
#endif
}

void CPhysXStaticActor::Set_QueryFilterData(PxFilterData _data)
{
    m_pShape->setQueryFilterData(_data);
}

void CPhysXStaticActor::RemovePhysX()
{
    Get_Actor()->userData = nullptr;
    m_pGameInstance->Get_Scene()->removeActor(*Get_Actor());
}

void CPhysXStaticActor::Shape_Detach()
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


CPhysXStaticActor* CPhysXStaticActor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CPhysXStaticActor* pInstance = new CPhysXStaticActor(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CPhysXStaticActor");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CPhysXStaticActor::Clone(void* pArg)
{
    CPhysXStaticActor* pInstance = new CPhysXStaticActor(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CPhysXStaticActor");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CPhysXStaticActor::Free()
{
    if (m_pActor)
    {
        m_pActor->release();
        m_pActor = nullptr;
    }
    __super::Free();
}

