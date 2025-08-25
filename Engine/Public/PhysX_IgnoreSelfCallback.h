#pragma once

#include "PxPhysicsAPI.h"
#include "PhysXActor.h"

using namespace physx;

class CIgnoreSelfCallback : public PxQueryFilterCallback
{
public:
    CIgnoreSelfCallback(const unordered_set<PxActor*>& ignoreActors)
        : m_IgnoreActors(ignoreActors) {}

    virtual PxQueryHitType::Enum preFilter(const PxFilterData&, const PxShape*, const PxRigidActor* actor, PxHitFlags&) override
    {
        if (m_IgnoreActors.find(const_cast<PxActor*>(static_cast<const PxActor*>(actor))) != m_IgnoreActors.end())
            return PxQueryHitType::eNONE; // 公矫
        return PxQueryHitType::eBLOCK;   // 面倒 贸府
    }

    virtual PxQueryHitType::Enum postFilter(const PxFilterData&, const PxQueryHit&, const PxShape*, const PxRigidActor*) override
    {
        return PxQueryHitType::eBLOCK;
    }

private:
    unordered_set<PxActor*> m_IgnoreActors;
};