#pragma once

#include "PxPhysicsAPI.h"
#include "PhysXActor.h"
using namespace physx;


// ��Ʈ�ѷ� �浹 �ݹ� ���� Ŭ����
class CPhysXControllerHitReport : public PxUserControllerHitReport
{
public:
    CPhysXControllerHitReport() = default;
    virtual ~CPhysXControllerHitReport() = default;

    // ��Ʈ�ѷ� vs ���� �浹
    void onShapeHit(const PxControllerShapeHit& hit) override
    {
        PxRigidActor* pHitActor = hit.actor;
        PxController* pController = hit.controller;

        if (!pHitActor || !pController)
            return;

        CPhysXActor* pOther = static_cast<CPhysXActor*>(pHitActor->userData);
        CPhysXActor* pSelf = static_cast<CPhysXActor*>(pController->getActor()->userData);

        if (pSelf && pOther)
        {
            pSelf->On_Enter(pOther);
            // On_Stay/Exit�� ����� �ڵ忡�� ����
        }
    }

    // ��Ʈ�ѷ� vs ��Ʈ�ѷ� �浹
    void onControllerHit(const PxControllersHit& hit) override
    {
        PxController* pA = hit.controller;
        PxController* pB = hit.other;

        if (!pA || !pB)
            return;

        CPhysXActor* actorA = static_cast<CPhysXActor*>(pA->getActor()->userData);
        CPhysXActor* actorB = static_cast<CPhysXActor*>(pB->getActor()->userData);

        if (actorA && actorB)
        {
            actorA->On_Enter(actorB);
            actorB->On_Enter(actorA);
        }
    }

    // ��Ʈ�ѷ� vs ��ֹ� (�ɼ� ��� ��)
    void onObstacleHit(const PxControllerObstacleHit& hit) override
    {
        // �⺻�� ����. �ʿ� �� ó�� ����.
    }
};