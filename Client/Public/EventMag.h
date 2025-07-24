#pragma once
#include "Base.h"
#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI

NS_BEGIN(Client)
class CEventMag :public CBase
{
	DECLARE_SINGLETON(CEventMag)
private:
    CEventMag() = default;
    virtual ~CEventMag() = default;

public:
	void RegisterEvent(const string& eventName, function<void()> callback)
	{
		m_EventMap[eventName] = callback;
	}

	void DeregisterEvent(const string& eventName)
	{
		auto it = m_EventMap.find(eventName);
		if (it != m_EventMap.end())
		{
			m_EventMap.erase(it);
		}
	}

	const auto& GetEventMap() const
	{
		return m_EventMap;
	}



private:
	// first 이벤트 이름, second 이벤트 콜백 함수
	unordered_map<string, function<void()>> m_EventMap;

public:
    virtual void Free() override;
};
NS_END

