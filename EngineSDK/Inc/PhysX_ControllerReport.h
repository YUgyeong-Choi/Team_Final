#pragma once

#include "PxPhysicsAPI.h"
#include "PhysXActor.h"
using namespace physx;


// 컨트롤러 충돌 콜백 전용 클래스
class CPhysXControllerHitReport : public PxUserControllerHitReport
{
public:
    CPhysXControllerHitReport() = default;
    virtual ~CPhysXControllerHitReport() = default;

    // 컨트롤러 vs 액터 충돌
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
            // On_Stay/Exit는 사용자 코드에서 추적
        }
    }

    // 컨트롤러 vs 컨트롤러 충돌
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

    // 컨트롤러 vs 장애물 (옵션 사용 시)
    void onObstacleHit(const PxControllerObstacleHit& hit) override
    {
        // 기본은 무시. 필요 시 처리 가능.
    }
};