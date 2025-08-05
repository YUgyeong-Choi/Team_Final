#include "Observer_Manager.h"
#include "Observer.h"
#include "GameInstance.h"

CObserver_Manager::CObserver_Manager()
	: m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CObserver_Manager::Add_Observer(const _wstring strTag, CObserver* pObserver)
{
	if (m_pObservers.end() != m_pObservers.find(strTag))
		return S_OK;

	m_pObservers.emplace(strTag, pObserver);



	return S_OK;
}

HRESULT CObserver_Manager::Remove_Observer(const _wstring strTag)
{
	if (m_pObservers.end() == m_pObservers.find(strTag))
		return S_OK;

	CObserver* pObserver = m_pObservers[strTag];
	Safe_Release(pObserver);
	m_pObservers.erase(strTag);
	return S_OK;
}

void CObserver_Manager::Register_PullCallback(const _wstring& strTag, function<void(const _wstring& eventType, void* data)> callback)
{
	if (m_pObservers.end() == m_pObservers.find(strTag))
		return;

	m_pObservers[strTag]->Register_PullCallback(callback);
}



void CObserver_Manager::Register_PushCallback(const _wstring& strTag, function<void(const _wstring& eventType, void* data)> callback)
{
	if (m_pObservers.end() == m_pObservers.find(strTag))
		return;

	m_pObservers[strTag]->Register_PushCallback(callback);
}



void CObserver_Manager::Notify(const _wstring& strTag, const _wstring& eventType, void* pData)
{
	auto it = m_pObservers.find(strTag);
	if (it != m_pObservers.end() && nullptr != it->second)
	{
		it->second->OnNotify(eventType, pData);
	}
}

void CObserver_Manager::Notify_Pull(const _wstring& strTag, const _wstring& eventType, void* pData)
{
	auto it = m_pObservers.find(strTag);
	if (it != m_pObservers.end() && nullptr != it->second)
	{
		it->second->OnNotify_Pull(eventType, pData);
	}
}

void CObserver_Manager::Notify_Push(const _wstring& strTag, const _wstring& eventType, void* pData)
{
	auto it = m_pObservers.find(strTag);
	if (it != m_pObservers.end() && nullptr != it->second)
	{
		it->second->OnNotify_Push(eventType, pData);
	}
}

void CObserver_Manager::Reset(const _wstring& strTag)
{
	if (m_pObservers.end() == m_pObservers.find(strTag))
		return;

	m_pObservers[strTag]->Reset();
}

void CObserver_Manager::Reset_All()
{
	for (auto& pObserver : m_pObservers)
	{
		pObserver.second->Reset();
		Safe_Release(pObserver.second);
	}

	m_pObservers.clear();
}

CObserver_Manager* CObserver_Manager::Create()
{
	return new CObserver_Manager;
}

void CObserver_Manager::Free()
{
	__super::Free();

	for (auto& pObserver : m_pObservers)
	{
		pObserver.second->Reset();
		Safe_Release(pObserver.second);
	}
		

	m_pObservers.clear();

	Safe_Release(m_pGameInstance);
}
