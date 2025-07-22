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

    HRESULT Create_Collision(PxPhysics* physics, const PxGeometry& geom, const PxTransform& pose, PxMaterial* material, WorldFilter _filter);

    void Set_Transform(const PxTransform& pose) override
    {
        m_pActor->setGlobalPose(pose);
    }

    void Set_UserData(void* pData) {
        if (m_pActor)
            m_pActor->userData = pData;
    }

public:
    static CPhysXStaticActor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END