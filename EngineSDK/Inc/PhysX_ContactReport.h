#pragma once

#include "PxSimulationEventCallback.h"
#include "PxPhysicsAPI.h"
#include "PhysXActor.h"

using namespace physx;

class CPhysX_ContactReport : public PxSimulationEventCallback
{
public:
    CPhysX_ContactReport() = default;
    virtual ~CPhysX_ContactReport() = default;

    // 충돌이 발생할 때 자동 호출됨
    void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override
    {
        if (g_bSceneChanging)
            return;

        void* userDataA = pairHeader.actors[0]->userData;
        void* userDataB = pairHeader.actors[1]->userData;

        CPhysXActor* actorA = static_cast<CPhysXActor*>(userDataA);
        CPhysXActor* actorB = static_cast<CPhysXActor*>(userDataB);

        for (PxU32 i = 0; i < nbPairs; ++i)
        {
            const PxContactPair& cp = pairs[i];

            if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
            {
                if (actorA)
                    actorA->On_Enter(actorB);

                if (actorB)
                    actorB->On_Enter(actorA);
            }

            if (cp.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
            {
                if (actorA)
                    actorA->On_Stay(actorB);

                if (actorB)
                    actorB->On_Stay(actorA);
            }

            if (cp.events & PxPairFlag::eNOTIFY_TOUCH_LOST)
            {
                if (actorA)
                    actorA->On_Exit(actorB);

                if (actorB)
                    actorB->On_Exit(actorA);
            }
        }
    }

    void onTrigger(PxTriggerPair* pairs, PxU32 count) override
    {
        if (g_bSceneChanging)
            return;

        for (PxU32 i = 0; i < count; ++i)
        {
            const PxTriggerPair& pair = pairs[i];

            // 삭제된 형태 무시
            if ((pair.status & physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER) ||
                (pair.status & physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
            {
                continue;
            }

            void* userDataA = pair.triggerActor->userData;
            void* userDataB = pair.otherActor->userData;

            CPhysXActor* actorA = static_cast<CPhysXActor*>(userDataA);
            CPhysXActor* actorB = static_cast<CPhysXActor*>(userDataB);

            if (pair.status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
            {
                if (actorA) actorA->On_TriggerEnter(actorB);
                if (actorB) actorB->On_TriggerEnter(actorA);
            }

            if (pair.status & PxPairFlag::eNOTIFY_TOUCH_LOST)
            {
                if (actorA) actorA->On_TriggerExit(actorB);
                if (actorB) actorB->On_TriggerExit(actorA);
            }
        }
    }
    void onConstraintBreak(PxConstraintInfo*, PxU32) override {}
    void onWake(PxActor**, PxU32) override {}
    void onSleep(PxActor**, PxU32) override {}
    void onAdvance(const PxRigidBody* const*, const PxTransform*, const PxU32) override {}
};