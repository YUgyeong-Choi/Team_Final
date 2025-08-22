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

    // �浹�� �߻��� �� �ڵ� ȣ���
    void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override
    {
        if (g_bSceneChanging) return;


        auto* actorA = static_cast<CPhysXActor*>(pairHeader.actors[0]->userData);
        auto* actorB = static_cast<CPhysXActor*>(pairHeader.actors[1]->userData);

        for (PxU32 i = 0; i < nbPairs; ++i)
        {
            const PxContactPair& cp = pairs[i];

            // ������ ����
            PxContactPairPoint pts[64];
            PxU32 count = cp.extractContacts(pts, PX_ARRAY_SIZE(pts));

            // ��ǥ ������ ����: separation�� ���� ����(=���� ���� �İ��) ��
            bool hasPoint = false;
            PxContactPairPoint best{};
            float bestSep = FLT_MAX; // (�� ���� ���� "�� ����"���⼱ ������ �۴�)

            for (PxU32 c = 0; c < count; ++c)
            {
                const auto& p = pts[c];
                if (!hasPoint || p.separation < bestSep) {
                    best = p;
                    bestSep = p.separation;
                    hasPoint = true;
                }
            }

            // �̺�Ʈ���� �ݹ� ȣ��
            if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
            {
                if (hasPoint) {
                    if (actorA) actorA->On_Enter(actorB, best.position, best.normal /*, best.separation*/);
                    if (actorB) actorB->On_Enter(actorA, best.position, -best.normal /* �ݴ��� ��� */ /*, best.separation*/);
                }
                else {
                    if (actorA) actorA->On_Enter(actorB);
                    if (actorB) actorB->On_Enter(actorA);
                }
            }

            if (cp.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
            {
                if (hasPoint) {
                    if (actorA) actorA->On_Stay(actorB, best.position, best.normal /*, best.separation*/);
                    if (actorB) actorB->On_Stay(actorA, best.position, -best.normal /*, best.separation*/);
                }
                else {
                    if (actorA) actorA->On_Stay(actorB);
                    if (actorB) actorB->On_Stay(actorA);
                }
            }

            if (cp.events & PxPairFlag::eNOTIFY_TOUCH_LOST)
            {
                if (hasPoint) {
                    if (actorA) actorA->On_Exit(actorB, best.position, best.normal /*, best.separation*/);
                    if (actorB) actorB->On_Exit(actorA, best.position, -best.normal /*, best.separation*/);
                }
                else {
                    if (actorA) actorA->On_Exit(actorB);
                    if (actorB) actorB->On_Exit(actorA);
                }
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

            // ������ ���� ����
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