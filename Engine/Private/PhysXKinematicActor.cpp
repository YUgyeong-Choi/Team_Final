#include "PhysXKinematicActor.h"


CPhysXKinematicActor::CPhysXKinematicActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysXActor{ pDevice, pContext }
{
}

CPhysXKinematicActor::CPhysXKinematicActor(const CPhysXKinematicActor& Prototype)
    : CPhysXActor(Prototype)
{
}

HRESULT CPhysXKinematicActor::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CPhysXKinematicActor::Initialize(void* pArg)
{
    ReadyForDebugDraw(m_pDevice, m_pContext);
    return S_OK;
}

HRESULT CPhysXKinematicActor::Create_Collision(PxPhysics* physics, const PxGeometry& geom, const PxTransform& pose, PxMaterial* material, WorldFilter _filter)
{
    m_pMaterial = material;

    PxRigidDynamic* pDynamic = physics->createRigidDynamic(pose);
    if (!pDynamic)
        return E_FAIL;

    pDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

    m_pShape = PxRigidActorExt::createExclusiveShape(*pDynamic, geom, *material);
    if (!m_pShape)
        return E_FAIL;

    m_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true); 
    m_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);  
    m_pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);

    PxFilterData filterData;
    filterData.word0 = (1 << static_cast<_int>(_filter)); // 그룹
    filterData.word1 = (1 << static_cast<_int>(WorldFilter::BUILDING)) | (1 << static_cast<_int>(WorldFilter::STATIC)) | (1 << static_cast<_int>(WorldFilter::LOOT)) | (1 << static_cast<_int>(WorldFilter::RESOURCES)); // 마스크

    m_pShape->setSimulationFilterData(filterData);

    m_pActor = pDynamic;
    m_pActor->userData = this;

    return S_OK;
}

CPhysXKinematicActor* CPhysXKinematicActor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CPhysXKinematicActor* pInstance = new CPhysXKinematicActor(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CPhysXDynamicActor");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CPhysXKinematicActor::Clone(void* pArg)
{
    CPhysXKinematicActor* pInstance = new CPhysXKinematicActor(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CPhysXDynamicActor");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CPhysXKinematicActor::Free()
{
    __super::Free();
}
