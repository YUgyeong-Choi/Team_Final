#include "PhysXDynamicActor.h"

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
	ReadyForDebugDraw(m_pDevice, m_pContext);
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

void CPhysXDynamicActor::Set_Kinematic(_bool bActiveKinematic)
{
	if (!m_pActor)
		return;

	PxRigidDynamic* pDynamic = m_pActor->is<PxRigidDynamic>();
	if (pDynamic)
		pDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, bActiveKinematic);

	m_bKinematic = bActiveKinematic;
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
