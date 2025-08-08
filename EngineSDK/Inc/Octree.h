#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CGameObject;

class COctree : public CBase
{
private:
    COctree(const _float3& vMin, const _float3& vMax, _uint depth);
    virtual ~COctree() = default;
public:
    COctree* Insert(CGameObject* pObj, const _float3& objMin, const _float3& objMax);
    COctree* Insert_DistanceBased(CGameObject* pObj, const _float3& objMin, const _float3& objMax, const _float3& objCenter, const _float3& camPos);

    void Subdivide();

    void Set_Activated(_bool bFlag) { m_bActivated = bFlag; }
    _bool IsActivated() const { return m_bActivated; }

    _bool IsLeaf() const { return m_pChildren[0] == nullptr; }

    const std::vector<CGameObject*>& Get_Objects() const { return m_Objects; }
    COctree* Get_Child(_uint idx) const { return m_pChildren[idx]; }
    const _float3& Get_Min() const { return m_vMin; }
    const _float3& Get_Max() const { return m_vMax; }

private:
    _float3 m_vMin, m_vMax, m_vCenter;
    _uint   m_iDepth;
    vector<CGameObject*> m_Objects;
    COctree* m_pChildren[8] = { nullptr };
    _bool  m_bActivated = false;

    const _uint MAX_OBJECTS = 10;
    const _uint MAX_DEPTH = 5;
public:
    static COctree* Create(const _float3& vMin, const _float3& vMax, _uint depth);
    virtual void Free() override;
};

NS_END;