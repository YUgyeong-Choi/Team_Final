#pragma once

#include "PxPhysicsAPI.h"
#include "PhysXActor.h"
#include <unordered_set>

using namespace physx;

// ��Ʈ�ѷ� �浹 �ݹ� ���� Ŭ����
class CPhysXControllerHitReport : public PxUserControllerHitReport
{
public:
    CPhysXControllerHitReport() = default;
    virtual ~CPhysXControllerHitReport() = default;

    // ===== Ignore ���� =====
    void AddIgnoreActor(PxActor* actor)
    {
        if (actor)
            m_IgnoreActors.insert(actor);
    }

    void RemoveIgnoreActor(PxActor* actor)
    {
        m_IgnoreActors.erase(actor);
    }

    void ClearIgnoreActors()
    {
        m_IgnoreActors.clear();
    }

    bool IsIgnored(PxActor* actor) const
    {
        return (actor && m_IgnoreActors.find(actor) != m_IgnoreActors.end());
    }

    // ===== �浹 �ݹ� =====
    void onShapeHit(const PxControllerShapeHit& hit) override
    {
        PxRigidActor* pHitActor = hit.actor;
        PxController* pController = hit.controller;

        if (!pHitActor || !pController)
            return;

        // Ignore ��Ͽ� ������ ����
        if (IsIgnored(pHitActor))
            return;

        CPhysXActor* pOther = static_cast<CPhysXActor*>(pHitActor->userData);
        CPhysXActor* pSelf = static_cast<CPhysXActor*>(pController->getActor()->userData);

        if (pSelf && pOther)
        {
            pSelf->On_Enter(pOther);
        }
    }

    void onControllerHit(const PxControllersHit& hit) override
    {
        PxController* pA = hit.controller;
        PxController* pB = hit.other;

        if (!pA || !pB)
            return;

        // Ignore ��Ͽ� ������ ����
        if (IsIgnored(pB->getActor()))
            return;

        CPhysXActor* actorA = static_cast<CPhysXActor*>(pA->getActor()->userData);
        CPhysXActor* actorB = static_cast<CPhysXActor*>(pB->getActor()->userData);

        if (actorA && actorB)
        {
            actorA->On_Enter(actorB);
            actorB->On_Enter(actorA);
        }
    }

    void onObstacleHit(const PxControllerObstacleHit& hit) override
    {
        // �ʿ� �� ����
    }

private:
    unordered_set<PxActor*> m_IgnoreActors;
};