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
    ReadyForDebugDraw(m_pDevice, m_pContext);
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
    PxTransform pose = PxShapeExt::getGlobalPose(*m_pShape, *m_pActor);
    PxGeometryHolder geom = m_pShape->getGeometry();
    PxBounds3 bounds = PxShapeExt::getWorldBounds(*m_pShape, *m_pActor);

    DebugRender(m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ), pose, geom, bounds);

    for (auto& Ray : m_RenderRay) {
        DrawRay(m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ), Ray.vStartPos, Ray.vDirection, Ray.fRayLength, Ray.bIsHit, Ray.vHitPos);
    }
    m_RenderRay.clear();

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
}

void CPhysXStaticActor::Set_QueryFilterData(PxFilterData _data)
{
    m_pShape->setQueryFilterData(_data);
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

