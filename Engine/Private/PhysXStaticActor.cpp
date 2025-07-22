#include "PhysXStaticActor.h"

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
    return S_OK;
}

HRESULT CPhysXStaticActor::Initialize(void* pArg)
{
    ReadyForDebugDraw(m_pDevice, m_pContext);
    return S_OK;
}
  

HRESULT CPhysXStaticActor::Create_Collision(PxPhysics* physics, const PxGeometry& geom, const PxTransform& pose, PxMaterial* material, WorldFilter _filter)
{
    m_pMaterial = material;

    m_pActor = physics->createRigidStatic(pose);
    if (!m_pActor)
        return E_FAIL;

    m_pShape = PxRigidActorExt::createExclusiveShape(*m_pActor, geom, *material);
    if (!m_pShape)
        return E_FAIL;

    m_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true); // OnEnter, OnStay, OnExit 활성화
    m_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false); //OnTriger 활성화
    m_pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true); // Ray충돌 활성화 

    // 여기 밑에 일단 무시 어떤 것 끼리 충돌할까 ~ 하는건데 정확히 잘 작동안함
    PxFilterData filterData;
    filterData.word0 = (1 << static_cast<_int>(_filter)); // 그룹
    filterData.word1 = (1 << static_cast<_int>(WorldFilter::PERSON)) | (1 << static_cast<_int>(WorldFilter::ITEMS)); // 마스크

    m_pShape->setSimulationFilterData(filterData);
    m_pShape->setQueryFilterData(filterData);
    m_pActor->userData = this;
    return S_OK;
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
    __super::Free();
}

