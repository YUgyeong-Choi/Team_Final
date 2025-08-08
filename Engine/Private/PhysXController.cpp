#include "PhysXController.h"
#include "GameInstance.h"
CPhysXController::CPhysXController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysXActor{ pDevice, pContext }
{
}

CPhysXController::CPhysXController(const CPhysXController& Prototype)
    : CPhysXActor(Prototype)
{
}

HRESULT CPhysXController::Initialize_Prototype()
{
    ReadyForDebugDraw(m_pDevice, m_pContext);
    return S_OK;
}

HRESULT CPhysXController::Initialize(void* pArg)
{
    return S_OK;
}

HRESULT CPhysXController::Create_Controller(PxControllerManager* pManager, PxMaterial* pMaterial, const PxExtendedVec3& pos, float radius, float height, CPhysXControllerHitReport* pReport)
{
    PxCapsuleControllerDesc desc;
    desc.material = pMaterial;
    desc.height = height;
    desc.radius = radius;
    desc.position = pos;
    desc.upDirection = PxVec3(0, 1, 0);
    desc.stepOffset = 0.005f;
    desc.contactOffset = 0.05f;
    desc.slopeLimit = cosf(PxPi / 4); // 45��

    desc.userData = this;

    if (pReport)
        desc.reportCallback = pReport;

    m_pController = pManager->createController(desc);

    if (!m_pController)
        return E_FAIL;

    // [�߿�] �ݹ鿡�� ����� �� �ֵ��� ���� ���Ϳ��� userData ����
    m_pController->getActor()->userData = this;

    return S_OK;
}


HRESULT CPhysXController::Render()
{
    if (!m_pController)
        return E_FAIL;

    PxExtendedVec3 extPos = m_pController->getPosition();
    PxCapsuleController* pCapsule = static_cast<PxCapsuleController*>(m_pController);

    float radius = pCapsule->getRadius();
    float halfHeight = pCapsule->getHeight() * 0.5f;

    PxTransform pose(PxVec3((float)extPos.x, (float)extPos.y, (float)extPos.z), PxQuat(PxIdentity));
    PxCapsuleGeometry capsuleGeom(radius, halfHeight);
    PxBounds3 bounds = PxBounds3::empty();

    DebugRender(m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ), pose, capsuleGeom, bounds);

    for (auto& Ray : m_RenderRay) {
        DrawRay(m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ), Ray.vStartPos, Ray.vDirection, Ray.fRayLength, Ray.bIsHit, Ray.vHitPos);
    }
    m_RenderRay.clear();

    return S_OK;
}

void CPhysXController::Set_ShapeFlag(_bool bSimulation, _bool bTrigger, _bool bQuery)
{
    if (!m_pController)
        return;

    PxRigidActor* pActor = m_pController->getActor();
    if (!pActor)
        return;

    PxShape* pShape = nullptr;
    PxU32 shapeCount = pActor->getNbShapes();
    
    if (shapeCount > 0)
    {
        pActor->getShapes(&pShape, 1);
        if (pShape)
        {
            pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, bSimulation);
            pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, bTrigger);
            pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, bQuery);
        }
    }
}

void CPhysXController::Set_SimulationFilterData(PxFilterData filter)
{
    if (!m_pController)
        return;

    PxRigidActor* pActor = m_pController->getActor();
    if (!pActor)
        return;

    PxShape* pShape = nullptr;
    if (pActor->getNbShapes() > 0)
    {
        pActor->getShapes(&pShape, 1);
        if (pShape)
            pShape->setSimulationFilterData(filter);
    }
}

void CPhysXController::Set_QueryFilterData(PxFilterData filter)
{
    if (!m_pController)
        return;

    PxRigidActor* pActor = m_pController->getActor();
    if (!pActor)
        return;

    PxShape* pShape = nullptr;
    if (pActor->getNbShapes() > 0)
    {
        pActor->getShapes(&pShape, 1);
        if (pShape)
            pShape->setQueryFilterData(filter);
    }
}

void CPhysXController::Move(_float fDeltaTime, const PxVec3& vDirection, _float fSpeed)
{
    PxVec3 displacement = vDirection * fSpeed * fDeltaTime;

    PxControllerFilters filters;
    PxControllerCollisionFlags result = m_pController->move(displacement, 0.001f, fDeltaTime, filters);

    // ����: ���� ��Ҵ��� Ȯ��
    if (result & PxControllerCollisionFlag::eCOLLISION_DOWN)
    {
        // ���� ���� ���� ��
    }
}

CPhysXController* CPhysXController::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CPhysXController* pInstance = new CPhysXController(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CPhysXController");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CPhysXController::Clone(void* pArg)
{
    CPhysXController* pInstance = new CPhysXController(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CPhysXController");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CPhysXController::Free()
{
    if (m_pController)
    {
        m_pController->release();
        m_pController = nullptr;
    }
    __super::Free();
}

