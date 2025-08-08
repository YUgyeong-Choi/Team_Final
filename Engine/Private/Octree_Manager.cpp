#include "Octree_Manager.h"
#include "Octree.h"
#include "Frustum.h"
COctree_Manager::COctree_Manager()
{
}

void COctree_Manager::Build(const _float3& vWorldMin, const _float3& vWorldMax)
{
    m_pRoot = COctree::Create(vWorldMin, vWorldMax, 0);
    m_vecActivatedNodes.clear();
    m_mapObjToNode.clear();
}

COctree* COctree_Manager::Insert(CGameObject* pObj, const _float3& objMin, const _float3& objMax)
{
    if (!m_pRoot)
        return nullptr;

    COctree* pNode = m_pRoot->Insert(pObj, objMin, objMax);
    if (pNode)
        m_mapObjToNode[pObj] = pNode;
    return pNode;
}

void COctree_Manager::CollectVisibleObjects(CFrustum* pFrustum, std::vector<CGameObject*>& outObjects)
{
    m_vecActivatedNodes.clear();
    if (m_pRoot)
        Traverse(m_pRoot, pFrustum, outObjects);
}

void COctree_Manager::Traverse(COctree* pNode, CFrustum* pFrustum, std::vector<CGameObject*>& outObjects)
{
    if (!pFrustum->Is_AABB_InFrustum(pNode->Get_Min(), pNode->Get_Max()))
        return;

    pNode->Set_Activated(true);
    m_vecActivatedNodes.push_back(pNode);

    if (pNode->IsLeaf()) {
        const auto& objs = pNode->Get_Objects();
        outObjects.insert(outObjects.end(), objs.begin(), objs.end());
    }
    else {
        for (int i = 0; i < 8; ++i) {
            COctree* pChild = pNode->Get_Child(i);
            if (pChild)
                Traverse(pChild, pFrustum, outObjects);
        }
    }
}

COctree_Manager* COctree_Manager::Create()
{
    COctree_Manager* pInstance = new COctree_Manager();
    return pInstance;
}

void COctree_Manager::Free()
{
    __super::Free();
    Safe_Release(m_pRoot);
    m_vecActivatedNodes.clear();
    m_mapObjToNode.clear();
}
