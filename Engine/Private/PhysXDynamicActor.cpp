#include "PhysXDynamicActor.h"

CPhysXDynamicActor::CPhysXDynamicActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXActor{ pDevice, pContext }
{
}

CPhysXDynamicActor::CPhysXDynamicActor(const CPhysXDynamicActor& Prototype)
	: CPhysXActor(Prototype)
{
}

HRESULT CPhysXDynamicActor::Initialize_Prototype()
{
	ReadyForDebugDraw(m_pDevice, m_pContext);
	return S_OK;
}

HRESULT CPhysXDynamicActor::Initialize(void* pArg)
{

	return S_OK;
}

HRESULT CPhysXDynamicActor::Create_Collision(PxPhysics* physics, const PxGeometry& geom, const PxTransform& pose, PxMaterial* material, WorldFilter _filter)
{
    m_pMaterial = material;

    PxRigidDynamic* pDynamic = physics->createRigidDynamic(pose);
    if (!pDynamic)
        return E_FAIL;


    m_pShape = PxRigidActorExt::createExclusiveShape(*pDynamic, geom, *material);
    if (!m_pShape)
        return E_FAIL;


    // 충돌 속성 설정
    m_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
    m_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
    m_pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);

    PxFilterData filterData;
    filterData.word0 = static_cast<_int>(_filter); // 그룹
    filterData.word1 = static_cast<_int>(_filter); // 마스크
    m_pShape->setSimulationFilterData(filterData);
    m_pShape->setQueryFilterData(filterData);

    m_pActor = pDynamic;
    pDynamic->userData = this;

    return S_OK;
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
	__super::Free();
}
