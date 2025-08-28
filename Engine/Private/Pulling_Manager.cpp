#include "Pulling_Manager.h"
#include "GameInstance.h"

CPulling_Manager::CPulling_Manager()
	: m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CPulling_Manager::Initialize()
{
	return S_OK;
}

void CPulling_Manager::Add_PoolObject(const _wstring& wsLayerName, CGameObject* pObj)
{
	m_ObjectPools[wsLayerName].push(pObj);
}

void CPulling_Manager::Use_PoolObject(const _wstring& wsLayerName)
{
    auto it = m_ObjectPools.find(wsLayerName);
    if (it == m_ObjectPools.end() || it->second.empty())
        return; 

    auto& q = it->second;
    CGameObject* pObj = q.front(); 
    q.pop(); 

    if (!pObj) return;

    const int levelIdx = m_pGameInstance->GetCurrentLevelIndex();
    m_pGameInstance->Push_GameObject(pObj, levelIdx, wsLayerName);
}

void CPulling_Manager::UseAll_PoolObjects(const _wstring& wsLayerName)
{
    auto it = m_ObjectPools.find(wsLayerName);
    if (it == m_ObjectPools.end() || it->second.empty())
        return; 

    auto& queueObjs = it->second;
    const _int levelIdx = m_pGameInstance->GetCurrentLevelIndex();

    // 큐가 빌 때까지 전부 사용
    while (!queueObjs.empty())
    {
        CGameObject* pObj = queueObjs.front();
        queueObjs.pop();

        if (!pObj) continue; 

        m_pGameInstance->Push_GameObject(pObj, levelIdx, wsLayerName);
    }
}


void CPulling_Manager::Return_PoolObject(const _wstring& wsLayerName, CGameObject* pObj)
{
    const _int levelIdx = m_pGameInstance->GetCurrentLevelIndex();
    CGameObject* pReturnObj = m_pGameInstance->Recycle_GameObject(pObj, levelIdx, wsLayerName);
    if (!pObj)
        return;

    pReturnObj->Reset();
    m_ObjectPools[wsLayerName].push(pReturnObj);
}

void CPulling_Manager::Push_WillRemove(const _wstring& wsLayerName, CGameObject* pObj)
{
    RemoveObject desc{};
    desc.layerName = wsLayerName;
    desc.pObj = pObj;
    m_RemoveObjects.push_back(desc);
}

void CPulling_Manager::RemoveObjMagr_PushPullingMgr()
{
    for (auto& obj : m_RemoveObjects)
        Return_PoolObject(obj.layerName, obj.pObj);

    m_RemoveObjects.clear();
}

void CPulling_Manager::Clear_Pools()
{
    for (auto& pair : m_ObjectPools)
    {
        auto& q = pair.second;
        while (!q.empty())
        {
            CGameObject* pObj = q.front();
            q.pop();

            Safe_Release(pObj); // 참조 카운트 줄이고 0이면 delete
        }
    }

    m_ObjectPools.clear(); // 모든 key-value 제거
}

CPulling_Manager* CPulling_Manager::Create()
{
	CPulling_Manager* pInstance = new CPulling_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CPulling_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}



void CPulling_Manager::Free()
{
	__super::Free();
    Clear_Pools();
	Safe_Release(m_pGameInstance);
}
