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

void CPulling_Manager::Add_PoolObject(string strLayerName, CGameObject* pObj)
{
	m_ObjectPools[strLayerName].push(pObj);
}

void CPulling_Manager::Use_PoolObject(string strLayerName)
{
    auto it = m_ObjectPools.find(strLayerName);
    if (it == m_ObjectPools.end() || it->second.empty())
        return; 

    auto& q = it->second;
    CGameObject* pObj = q.front(); 
    q.pop(); 

    if (!pObj) return;

    const int levelIdx = m_pGameInstance->GetCurrentLevelIndex();
    wstring layerName(strLayerName.begin(), strLayerName.end());
    m_pGameInstance->Push_GameObject(pObj, levelIdx, layerName);
}

void CPulling_Manager::UseAll_PoolObjects(string strLayerName)
{
    auto it = m_ObjectPools.find(strLayerName);
    if (it == m_ObjectPools.end() || it->second.empty())
        return; 

    auto& queueObjs = it->second;
    const _int levelIdx = m_pGameInstance->GetCurrentLevelIndex();
    wstring layerName(strLayerName.begin(), strLayerName.end());

    // 큐가 빌 때까지 전부 사용
    while (!queueObjs.empty())
    {
        CGameObject* pObj = queueObjs.front();
        queueObjs.pop();

        if (!pObj) continue; 

        m_pGameInstance->Push_GameObject(pObj, levelIdx, layerName);
    }
}


void CPulling_Manager::Return_PoolObject(string strLayerName, CGameObject* pObj)
{
    const _int levelIdx = m_pGameInstance->GetCurrentLevelIndex();
    wstring layerName(strLayerName.begin(), strLayerName.end());
    CGameObject* pReturnObj = m_pGameInstance->Recycle_GameObject(pObj, levelIdx, layerName);
    if (!pObj)
        return;

    pReturnObj->Reset();
    m_ObjectPools[strLayerName].push(pReturnObj);
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
