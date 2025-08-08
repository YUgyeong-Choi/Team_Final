#include "Octree_Manager.h"
#include "Octree.h"
#include "Frustum.h"
#include "GameInstance.h"

COctree_Manager::COctree_Manager()
{
    m_pGameInstance = CGameInstance::Get_Instance();
    Safe_AddRef(m_pGameInstance);
}

void COctree_Manager::Build(const _float3& vWorldMin, const _float3& vWorldMax)
{
    m_pRoot = COctree::Create(vWorldMin, vWorldMax, 0);
    m_vecActivatedNodes.clear();
    m_mapObjToNode.clear();

    const _float4* pCam = m_pGameInstance->Get_CamPosition();
    m_vLastCamPos = { pCam->x, pCam->y, pCam->z };
}

COctree* COctree_Manager::Insert(CGameObject* pObj, const _float3& objMin, const _float3& objMax)
{
    if (!m_pRoot)
        return nullptr;

    _float3 vObjCenter = {
        (objMin.x + objMax.x) * 0.5f,
        (objMin.y + objMax.y) * 0.5f,
        (objMin.z + objMax.z) * 0.5f
    };

    const _float4* pCamPos4 = m_pGameInstance->Get_CamPosition();
    _float3 vCamPos = { pCamPos4->x, pCamPos4->y, pCamPos4->z };

    COctree* pNode = m_pRoot->Insert_DistanceBased(pObj, objMin, objMax, vObjCenter, vCamPos);
    if (pNode)
        m_mapObjToNode[pObj] = pNode;
    return pNode;
}

void COctree_Manager::CollectVisibleObjects(CFrustum* pFrustum)
{
    m_vecActivatedNodes.clear();

    if (m_pRoot)
        Traverse(m_pRoot, pFrustum);
}

void COctree_Manager::Traverse(COctree* pNode, CFrustum* pFrustum)
{
    if (!pFrustum->Is_AABB_InFrustum(pNode->Get_Min(), pNode->Get_Max()))
        return;

    pNode->Set_Activated(true);
    m_vecActivatedNodes.push_back(pNode);

    if (!pNode->IsLeaf()) {
        for (int i = 0; i < 8; ++i) {
            COctree* pChild = pNode->Get_Child(i);
            if (pChild)
                Traverse(pChild, pFrustum);
        }
    }
}

void COctree_Manager::Update_Dynamic()
{
    /*
    const _float4* pCam = m_pGameInstance->Get_CamPosition();
    _float3 vCurrCam = { pCam->x, pCam->y, pCam->z };
    float dist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&vCurrCam) - XMLoadFloat3(&m_vLastCamPos)));

    if (dist < 10.f)
        return;

    m_vLastCamPos = vCurrCam;

    std::vector<std::tuple<CGameObject*, _float3, _float3>> objects;

    for (const auto& pair : m_mapObjToNode) {
        CGameObject* pObj = pair.first;
        _float3 min, max;
        pObj->Get_AABB_MinMax(&min, &max); // 사용자가 정의한 AABB 반환 함수 필요
        objects.emplace_back(pObj, min, max);
    }

    if (m_pRoot)
        m_pRoot->Clear();

    m_mapObjToNode.clear();

    for (auto& [pObj, min, max] : objects) {
        _float3 center = {
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f,
            (min.z + max.z) * 0.5f
        };

        m_pRoot->Insert_DistanceBased(pObj, min, max, center, m_vLastCamPos);
        m_mapObjToNode[pObj] = m_pRoot; // 업데이트 필요 시 정확한 노드 반환값 저장 가능
    }
    */
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
    Safe_Release(m_pGameInstance);
}
