#include "PxPhysicsAPI.h"
#include "PhysXActor.h"

using namespace physx;

class CIgnoreSelfCallback : public PxQueryFilterCallback
{
public:
    CIgnoreSelfCallback(PxActor* pIgnoreActor) : m_pIgnoreActor(pIgnoreActor) {}

    virtual PxQueryHitType::Enum preFilter(const PxFilterData&, const PxShape*, const PxRigidActor* actor, PxHitFlags&) override
    {
        if (actor == m_pIgnoreActor)
            return PxQueryHitType::eNONE;
        return PxQueryHitType::eBLOCK;
    }

    virtual PxQueryHitType::Enum postFilter(const PxFilterData&, const PxQueryHit&, const PxShape*, const PxRigidActor*) override
    {
        return PxQueryHitType::eBLOCK;
    }

private:
    PxActor* m_pIgnoreActor;
};