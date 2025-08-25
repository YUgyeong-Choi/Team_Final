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
    CGameObject* Get_Owner() { return m_pOwner; }

    void Set_ColliderType(COLLIDERTYPE eColliderType);
    COLLIDERTYPE Get_ColliderType() { return m_eColliderType; }


    virtual PxRigidActor* Get_Actor() = 0;
    PxShape* Get_Shape() { return m_pShape; }
    void Add_IngoreActors(PxActor* pActor) { m_ignoreActors.insert(pActor); }
    void Remove_IgnoreActors(PxActor* pActor);
    unordered_set<PxActor*> Get_IngoreActors() { return m_ignoreActors; }

    void Modify_Shape(const PxGeometry& geom, PxMaterial* material = nullptr);
    void ReCreate_Shape(class PxRigidActor* pRigidActor, const PxGeometry& geom, PxMaterial* material = nullptr);

    virtual void RemovePhysX() {};

    virtual void Init_SimulationFilterData() {};
	PxFilterData Get_FilterData() const { return m_filterData; }
public:
    virtual void On_Enter(CPhysXActor* pOther, PxVec3 HitPos = {}, PxVec3 HitNormal = {});
    virtual void On_Stay(CPhysXActor* pOther, PxVec3 HitPos = {}, PxVec3 HitNormal = {});
    virtual void On_Exit(CPhysXActor* pOther, PxVec3 HitPos = {}, PxVec3 HitNormal = {});
    virtual void On_TriggerEnter(CPhysXActor* pOther);
    virtual void On_TriggerExit(CPhysXActor* pOther);
    virtual HRESULT Render() = 0;

#ifdef _DEBUG
    // For Debug Render
    virtual void Add_RenderRay(DEBUGRAY_DATA _data);
    void DebugRender(_fmatrix view, _cmatrix proj, PxTransform pose, PxGeometryHolder geom, PxBounds3 bounds, _float offSet = 0.f);
    void DrawRay(_fmatrix view, _cmatrix proj, const PxVec3& origin, const PxVec3& dir, float length, _bool drawHitBox = false, PxVec3 hitPos = { 0.f, 0.f, 0.f });
	_bool Get_ReadyForDebugDraw() const { return m_bReadyForDebugDraw; }
protected:
    list<DEBUGRAY_DATA> m_RenderRay;
    _vector m_vRenderColor;
    void Set_RenderColor();
#endif
protected:
    PxMaterial* m_pMaterial = { nullptr };

    // 소유자 게임 오브젝트 Call Hit
    CGameObject* m_pOwner = { nullptr };

    // 어떤 콜라이더 타입인지
    COLLIDERTYPE m_eColliderType = COLLIDERTYPE::A;

    // 컨트롤러에서 무시할 자기 Actor 넣는 곳
    unordered_set<PxActor*> m_ignoreActors;

    // 필터 설정
    PxFilterData m_filterData{};

    PxShape* m_pShape = { nullptr };

    // TriggerRemove용 사용 할 거
    unordered_set<CPhysXActor*> m_pTriggerEnterOthers;

#ifdef _DEBUG
    // For Debug Render
    PrimitiveBatch<VertexPositionColor>* m_pBatch = { nullptr };
    BasicEffect* m_pEffect = { nullptr };
    ID3D11InputLayout* m_pInputLayout = { nullptr };
	_bool m_bReadyForDebugDraw = { false };
protected:
    HRESULT ReadyForDebugDraw(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
protected:
    void DrawDebugCapsule(PrimitiveBatch<VertexPositionColor>* pBatch, const PxTransform& pose, float radius, float halfHeight, FXMVECTOR color);
    void DrawTriangleMesh(PxTransform pose, PxGeometryHolder geom, PxBounds3 bounds);
    void DrawConvexMesh(PxTransform pose, PxGeometryHolder geom, PxBounds3 bounds);
#endif
public:
    virtual CComponent* Clone(void* pArg) = 0;
    virtual void Free() override;
};

NS_END

