#pragma once
#include "PhysXActor.h"
#include "PhysX_ControllerReport.h"
NS_BEGIN(Engine)
class ENGINE_DLL CPhysXController : public CPhysXActor
{
private:
    CPhysXController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CPhysXController(const CPhysXController& Prototype);
    virtual ~CPhysXController() = default;

public:
    virtual HRESULT Initialize_Prototype();
    virtual HRESULT Initialize(void* pArg);
    HRESULT Create_Controller(PxControllerManager* pManager, PxMaterial* pMaterial, const PxExtendedVec3& pos, float radius = 0.4f, float height = 1.7f, CPhysXControllerHitReport* pReport = nullptr);
    virtual HRESULT Render() override;
public:
    void Set_Transform(const PxTransform& pose)
    {
        if (m_pController)
            m_pController->setPosition(PxExtendedVec3(pose.p.x, pose.p.y, pose.p.z));
    }
    void Set_Transform(const PxVec3& pose)
    {
        if (m_pController)
            m_pController->setPosition(PxExtendedVec3(pose.x, pose.y, pose.z));
    }
    void Set_ShapeFlag(_bool bSimulation, _bool bTrigger, _bool bQuery);
    void Set_SimulationFilterData(PxFilterData _data);
    virtual void Init_SimulationFilterData() override;
    void Set_QueryFilterData(PxFilterData filter);
    PxControllerCollisionFlags Move(_float fDeltaTime, const PxVec3& vDirection, _float fSpeed, _float intensity = 1.f);

    PxController* Get_Controller() const { return m_pController; }
public:
    PxRigidActor* Get_Actor() override
    {
        return m_pController ? m_pController->getActor() : nullptr;
    }

private:
    PxController* m_pController = nullptr;
public:
    static CPhysXController* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END