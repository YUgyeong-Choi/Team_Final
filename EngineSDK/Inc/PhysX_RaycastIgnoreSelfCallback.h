#pragma once

#include "PxQueryFiltering.h"
#include "PxPhysicsAPI.h"

using namespace physx;
#include <unordered_set>

class CRaycastIgnoreSelfCallback : public PxQueryFilterCallback
{
public:
    CRaycastIgnoreSelfCallback(const std::unordered_set<PxActor*>& ignoreActors)
        : m_IgnoreActors(ignoreActors) {
    }

    // 정확한 시그니처로 override
    virtual PxQueryHitType::Enum preFilter(
        const PxFilterData&, const PxShape*, const PxRigidActor* pActor, PxHitFlags& /*queryFlags*/) override
    {
        if (m_IgnoreActors.find((PxActor*)pActor) != m_IgnoreActors.end())
            return PxQueryHitType::eNONE;
        return PxQueryHitType::eBLOCK;
    }

    virtual PxQueryHitType::Enum postFilter(
        const PxFilterData&, const PxQueryHit&, const PxShape*, const PxRigidActor*) override
    {
        return PxQueryHitType::eBLOCK;
    }

private:
    std::unordered_set<PxActor*> m_IgnoreActors;
};