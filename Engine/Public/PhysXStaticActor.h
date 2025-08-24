#pragma once
#include "PhysXActor.h"

NS_BEGIN(Engine)
class ENGINE_DLL CPhysXStaticActor : public CPhysXActor
{
private:
    CPhysXStaticActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CPhysXStaticActor(const CPhysXStaticActor& Prototype);
    virtual ~CPhysXStaticActor() = default;

public:
    virtual HRESULT Initialize_Prototype();
    virtual HRESULT Initialize(void* pArg);
    HRESULT Create_Collision(PxPhysics* physics, const PxGeometry& geom, const PxTransform& pose, PxMaterial* material);
    virtual HRESULT Render() override;

public:
    void Set_Transform(const PxTransform& pose) override
    {
        m_pActor->setGlobalPose(pose);
    }

    void Set_UserData(void* pData) {
        if (m_pActor)
            m_pActor->userData = pData;
    }

    void Set_ShapeFlag(_bool bSimulation, _bool bTrigger, _bool bQuery);
    void Set_SimulationFilterData(PxFilterData _data);
    virtual void Init_SimulationFilterData() override;
    void Set_QueryFilterData(PxFilterData _data);
    virtual void RemovePhysX() override;

    void Shape_Detach();
public:
    PxRigidActor* Get_Actor() override { return m_pActor; }

private:
    PxRigidActor* m_pActor = { nullptr };
public:
    static CPhysXStaticActor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END