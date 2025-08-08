#include "Octree.h"

COctree::COctree(const _float3& vMin, const _float3& vMax, _uint depth)
    : m_vMin(vMin), m_vMax(vMax), m_iDepth(depth)
{
    m_vCenter = {
        (vMin.x + vMax.x) * 0.5f,
        (vMin.y + vMax.y) * 0.5f,
        (vMin.z + vMax.z) * 0.5f
    };
}

COctree* COctree::Create(const _float3& vMin, const _float3& vMax, _uint depth)
{
    return new COctree(vMin, vMax, depth);
}

COctree* COctree::Insert(CGameObject* pObj, const _float3& objMin, const _float3& objMax)
{
    if (!IsLeaf()) {
        for (int i = 0; i < 8; ++i) {
            COctree* pChild = m_pChildren[i];
            if (!pChild) continue;

            const _float3& cMin = pChild->Get_Min();
            const _float3& cMax = pChild->Get_Max();

            if (objMin.x >= cMin.x && objMax.x <= cMax.x &&
                objMin.y >= cMin.y && objMax.y <= cMax.y &&
                objMin.z >= cMin.z && objMax.z <= cMax.z)
            {
                return pChild->Insert(pObj, objMin, objMax); // 재귀적으로 삽입된 노드 리턴
            }
        }
    }

    m_Objects.push_back(pObj);

    if (m_Objects.size() > MAX_OBJECTS && m_iDepth < MAX_DEPTH)
    {
        if (IsLeaf())
            Subdivide();

        // TODO: 재배치 개선 필요
        std::vector<CGameObject*> temp = m_Objects;
        m_Objects.clear();
        for (CGameObject* obj : temp) {
            // FIXME: AABB 다시 받아야 함
            m_Objects.push_back(obj);
        }
    }

    return this; // 현재 노드가 최종 삽입된 노드
}

void COctree::Subdivide()
{
    for (int i = 0; i < 8; ++i) {
        _float3 newMin = m_vMin;
        _float3 newMax = m_vMax;

        if (i & 1) newMin.x = m_vCenter.x; else newMax.x = m_vCenter.x;
        if (i & 2) newMin.y = m_vCenter.y; else newMax.y = m_vCenter.y;
        if (i & 4) newMin.z = m_vCenter.z; else newMax.z = m_vCenter.z;

        m_pChildren[i] = COctree::Create(newMin, newMax, m_iDepth + 1);
    }
}

void COctree::Free()
{
    __super::Free();
    for (int i = 0; i < 8; ++i) {
        if (m_pChildren[i]) {
            Safe_Release(m_pChildren[i]);
        }
    }
    m_Objects.clear();
}