#pragma once

#include "Base.h"
#include "Observer.h"


NS_BEGIN(Engine)

class CObserver_Manager final : public CBase
{
private:
	CObserver_Manager();
	virtual ~CObserver_Manager() = default;


public:
	HRESULT Add_Observer(const _wstring strTag, CObserver* pObserver);
	HRESULT Remove_Observer(const _wstring strTag);

	void Register_PullCallback(const _wstring& strTag, function<void(const _wstring& eventType, void* data)> callback);
	
	void Register_PushCallback(const _wstring& strTag, function<void(const _wstring& eventType, void* data)> callback);

	// pull push 둘다
	void Notify(const _wstring& strTag, const _wstring& eventType, void* pData);

	// 필요하면 쓰기
	void Notify_Pull(const _wstring& strTag, const _wstring& eventType, void* pData);

	void Notify_Push(const _wstring& strTag, const _wstring& eventType, void* pData);

	void Reset(const _wstring& strTag);

	CObserver* Find_Observer(const _wstring& strTag)
	{
		auto iter = m_pObservers.find(strTag);
		if (iter != m_pObservers.end())
			return iter->second;
		return nullptr;
	}

	void Reset_All();

private:

	map<_wstring, CObserver*> m_pObservers = { };
	class CGameInstance* m_pGameInstance = { nullptr };



private:

public:
	static CObserver_Manager* Create();
	virtual void Free() override;
};

NS_END