#include "Observer.h"


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
	for (auto& callback : m_PullCallbacks)
	{
		callback(eventType, data);
	}
}

void CObserver::OnNotify_Push(const _wstring& eventType, void* data)
{
	for (auto& callback : m_PushCallbacks)
	{
		callback(eventType, data);
	}
}

void CObserver::Reset()
{
	Clear_PullCallback();
	Clear_PushCallback();
}




void CObserver::Free()
{
	__super::Free();

	Reset();
}
