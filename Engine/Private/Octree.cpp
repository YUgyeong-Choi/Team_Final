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

COctree* COctree::Insert_DistanceBased(CGameObject* pObj, const _float3& objMin, const _float3& objMax, const _float3& objCenter, const _float3& camPos)
{
    float dist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&objCenter) - XMLoadFloat3(&camPos)));

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
                return pChild->Insert_DistanceBased(pObj, objMin, objMax, objCenter, camPos);
            }
        }
    }

    m_Objects.push_back({ pObj, objMin, objMax });

    if (dist < 100.f && m_iDepth < MAX_DEPTH) {
        if (IsLeaf())
            Subdivide(camPos);

        vector<ObjectEntry> temp = m_Objects;
        m_Objects.clear();

        for (auto& entry : temp) {
            _float3 center = {
                (entry.vMin.x + entry.vMax.x) * 0.5f,
                (entry.vMin.y + entry.vMax.y) * 0.5f,
                (entry.vMin.z + entry.vMax.z) * 0.5f
            };

            Insert_DistanceBased(entry.pObj, entry.vMin, entry.vMax, center, camPos);
        }
    }

    return this;
}

void COctree::Subdivide(const _float3& camPos)
{
    for (int i = 0; i < 8; ++i) {
        _float3 newMin = m_vMin;
        _float3 newMax = m_vMax;

        if (i & 1) newMin.x = m_vCenter.x; else newMax.x = m_vCenter.x;
        if (i & 2) newMin.y = m_vCenter.y; else newMax.y = m_vCenter.y;
        if (i & 4) newMin.z = m_vCenter.z; else newMax.z = m_vCenter.z;

        _float3 center = {
            (newMin.x + newMax.x) * 0.5f,
            (newMin.y + newMax.y) * 0.5f,
            (newMin.z + newMax.z) * 0.5f
        };

        float dist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&center) - XMLoadFloat3(&camPos)));

        // 거리 기준 필터링
        if (dist < 200.f) {
            m_pChildren[i] = COctree::Create(newMin, newMax, m_iDepth + 1);
        }
    }
}

void COctree::Clear()
{
    for (int i = 0; i < 8; ++i) {
        if (m_pChildren[i]) {
            m_pChildren[i]->Clear();
            Safe_Release(m_pChildren[i]);
            m_pChildren[i] = nullptr;
        }
    }
    m_Objects.clear();
    m_bActivated = false;
}

void COctree::Free()
{
    __super::Free();
    Clear();
}
