#pragma once
#include "Component.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class ENGINE_DLL CPhysXActor : public CComponent
{
protected:
    CPhysXActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CPhysXActor(const CPhysXActor& Prototype);
    virtual ~CPhysXActor() = default;
public:
    virtual void Set_Transform(const PxTransform& pose) {};
    void Set_Owner(CGameObject* pOwner) { m_pOwner = pOwner; }

    virtual PxRigidActor* Get_Actor() const { return m_pActor; }
    PxShape* Get_Shape() { return m_pShape; }
    CGameObject* Get_Owner() { return m_pOwner; }

    void Set_ShapeFlag(_bool bSimulation, _bool bTrigger, _bool bQuery);
    void Set_SimulationFilterData(PxFilterData _data);
    void Set_QueryFilterData(PxFilterData _data);
public:
    virtual void On_Enter(CPhysXActor* pOther);
    virtual void On_Stay(CPhysXActor* pOther);
    virtual void On_Exit(CPhysXActor* pOther);
    virtual void On_Trigger(CPhysXActor* pOther);
    virtual HRESULT Render() override;
#ifdef _DEBUG
    // For Debug Render
    void Set_ColliderColor(_fvector vColor) { m_vRenderColor = vColor; }

    virtual void Add_RenderRay(DEBUGRAY_DATA _data);
    void DebugRender(_fmatrix view, _cmatrix proj, _float offSet = 0.f);
    void DrawRay(_fmatrix view, _cmatrix proj, const PxVec3& origin, const PxVec3& dir, float length, _bool drawHitBox = false, PxVec3 hitPos = { 0.f, 0.f, 0.f });

protected:
    list<DEBUGRAY_DATA> m_RenderRay;
    _vector m_vRenderColor;
#endif
protected:
    PxRigidActor* m_pActor = { nullptr };
    PxShape* m_pShape = { nullptr };
    PxMaterial* m_pMaterial = { nullptr };

    // 소유자 게임 오브젝트 Call Hit
    CGameObject* m_pOwner = { nullptr };

    // For Debug Render
    PrimitiveBatch<VertexPositionColor>* m_pBatch = { nullptr };
    BasicEffect* m_pEffect = { nullptr };
    ID3D11InputLayout* m_pInputLayout = { nullptr };
protected:
    HRESULT ReadyForDebugDraw(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
protected:
    void DrawDebugCapsule(PrimitiveBatch<VertexPositionColor>* pBatch, const PxTransform& pose, float radius, float halfHeight, FXMVECTOR color);
    void DrawTriangleMesh(PxTransform pose, PxGeometryHolder geom);
    void DrawConvexMesh(PxTransform pose, PxGeometryHolder geom);
public:
    virtual CComponent* Clone(void* pArg) = 0;
    virtual void Free() override;
};

NS_END

