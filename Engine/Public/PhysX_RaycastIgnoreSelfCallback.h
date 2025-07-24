#include "PxSimulationEventCallback.h"
#include "PxPhysicsAPI.h"
#include "PhysXActor.h"

using namespace physx;

class CRaycastIgnoreSelfCallback : public PxQueryFilterCallback
{
public:
    CRaycastIgnoreSelfCallback(PxActor* pIgnoreActor)
        : m_pIgnoreActor(pIgnoreActor) {
    }

    virtual PxQueryHitType::Enum preFilter(
        const PxFilterData& filterData,
        const PxShape* shape,
        const PxRigidActor* actor,
        PxHitFlags& queryFlags) override
    {
        PxFilterData shapeFilter = shape->getQueryFilterData();

        // 필터 마스크 체크
        if ((filterData.word0 & shapeFilter.word1) == 0 &&
            (shapeFilter.word0 & filterData.word1) == 0)
            return PxQueryHitType::eNONE;

        // 자기 자신 무시
        if (actor == m_pIgnoreActor)
            return PxQueryHitType::eNONE;

        return PxQueryHitType::eBLOCK;
    }

    virtual PxQueryHitType::Enum postFilter(
        const PxFilterData&,
        const PxQueryHit&,
        const PxShape*,
        const PxRigidActor*) override
    {
        return PxQueryHitType::eBLOCK;
    }

private:
    PxActor* m_pIgnoreActor;
};