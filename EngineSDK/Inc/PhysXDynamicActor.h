#pragma once
#include "PhysXActor.h"

NS_BEGIN(Engine)
class ENGINE_DLL CPhysXDynamicActor : public CPhysXActor
{
private:
    CPhysXDynamicActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CPhysXDynamicActor(const CPhysXDynamicActor& Prototype);
    virtual ~CPhysXDynamicActor() = default;

public:
    virtual HRESULT Initialize_Prototype();
    virtual HRESULT Initialize(void* pArg);
    HRESULT Create_Collision(PxPhysics* physics, const PxGeometry& geom, const PxTransform& pose, PxMaterial* material);
    virtual HRESULT Render() override;

public:
    void Set_Kinematic(_bool bActiveDynamic);

    void Set_ShapeFlag(_bool bSimulation, _bool bTrigger, _bool bQuery);
    void Set_SimulationFilterData(PxFilterData _data);
    virtual void Init_SimulationFilterData() override;
    void Set_QueryFilterData(PxFilterData _data);

    void Set_Transform(const PxTransform& pose) override
    {
        static_cast<PxRigidDynamic*>(m_pActor)->setKinematicTarget(pose);
    }
    virtual void RemovePhysX() override;
    void Shape_Detach();
public:
    PxRigidActor* Get_Actor() override { return m_pActor; }
    PxShape* Get_Shape() { return m_pShape; }
private:
    _bool m_bKinematic = false;
    PxRigidActor* m_pActor = { nullptr };
    PxShape* m_pShape = { nullptr };
public:
    static CPhysXDynamicActor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END