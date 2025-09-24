#include "Observer.h"
#include "GameObject.h"

CObserver::CObserver()
{
}

void CObserver::OnNotify(const _wstring& eventType, void* data)
{
	OnNotify_Pull(eventType, data);
	OnNotify_Push(eventType, data);
}

void CObserver::OnNotify_Pull(const _wstring& eventType, void* data)
{
	for (auto& pair : m_PullCallbacks)
	{
		if(pair.first && !pair.first->Get_bDead())
			pair.second(eventType, data);
	}
}

void CObserver::OnNotify_Push(const _wstring& eventType, void* data)
{
	for (auto& pair : m_PushCallbacks)
	{
		if (pair.first && !pair.first->Get_bDead())
			pair.second(eventType, data);
	}
}

void CObserver::Reset()
{
	Clear_PullCallback();
	Clear_PushCallback();
}

void CObserver::Remove_PullCallback(CGameObject* pOwner)
{
	m_PullCallbacks.remove_if([pOwner](const auto& entry) {
		return entry.first == pOwner;
		});
}

void CObserver::Remove_PushCallback(CGameObject* pOwner)
{
	m_PushCallbacks.remove_if([pOwner](const auto& entry) {
		return entry.first == pOwner;
		});
}




void CObserver::Free()
{
	__super::Free();

	Reset();
}
