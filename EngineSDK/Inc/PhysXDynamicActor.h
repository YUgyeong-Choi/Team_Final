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
    void Set_Kinematic(_bool bActiveDynamic);

    void Set_Transform(const PxTransform& pose) override
    {
        static_cast<PxRigidDynamic*>(m_pActor)->setKinematicTarget(pose);
    }
private:
    _bool m_bKinematic = false;
public:
    static CPhysXDynamicActor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END