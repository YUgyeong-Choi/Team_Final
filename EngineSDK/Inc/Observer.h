#pragma once

#include "Base.h"
#include "GameObject.h"

// 값을 받아오는 쪽에서 콜백 등록해야 편함
// 람다로 할때 this로 변수 가져 올 수 있어서

// 값을 저장하고 알려줄 옵저버를 구현하면 될듯

NS_BEGIN(Engine)

class ENGINE_DLL  CObserver abstract : public CBase
{
public:
	CObserver();
	virtual ~CObserver() = default;

public:
	void OnNotify(const _wstring& eventType, void* data);

	// 값 받아오는
	void OnNotify_Pull(const _wstring& eventType, void* data = nullptr);
	// 값 전달하는
	void OnNotify_Push(const _wstring& eventType, void* data = nullptr);

	
	void Reset();

	// 콜백 등록
	void Register_PullCallback(CGameObject* pOwner, function<void(const _wstring& eventType, void* data)> callback) { m_PullCallbacks.push_back({ pOwner,callback }); }
	void Register_PushCallback(CGameObject* pOwner, function<void(const _wstring& eventType, void* data)> callback) { m_PushCallbacks.push_back({ pOwner,callback }); }
	
	void Clear_PullCallback() { m_PullCallbacks.clear(); }
	void Clear_PushCallback() { m_PushCallbacks.clear(); }

	void Remove_PullCallback(CGameObject* pOwner);
	void Remove_PushCallback(CGameObject* pOwner);

protected:
	
	list<pair<CGameObject*, function<void(const _wstring& eventType, void* data)>>> m_PullCallbacks;
	list< pair<CGameObject*, function<void(const _wstring& eventType, void* data)>>> m_PushCallbacks;

public:

	virtual void Free() override;
};

NS_END