#pragma once
#include "Base.h"
#include "Octree.h"

NS_BEGIN(Engine)

class CFrustum;

class COctree_Manager : public CBase
{
private:
    COctree_Manager();
    virtual ~COctree_Manager() = default;
public:
    void Build(const _float3& vWorldMin, const _float3& vWorldMax);
    COctree* Insert(CGameObject* pObj, const _float3& objMin, const _float3& objMax);

    void CollectVisibleObjects(CFrustum* pFrustum);

private:
    void Traverse(COctree* pNode, CFrustum* pFrustum);

private:
    COctree* m_pRoot = nullptr;
    vector<COctree*> m_vecActivatedNodes;

    unordered_map<CGameObject*, COctree*> m_mapObjToNode;
    class CGameInstance* m_pGameInstance = nullptr;
public:
    static COctree_Manager* Create();
    virtual void Free() override;
};

NS_END

