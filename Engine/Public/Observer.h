#pragma once

#include "Base.h"
#include "GameObject.h"

// ���� �޾ƿ��� �ʿ��� �ݹ� ����ؾ� ����
// ���ٷ� �Ҷ� this�� ���� ���� �� �� �־

// ���� �����ϰ� �˷��� �������� �����ϸ� �ɵ�

NS_BEGIN(Engine)

class ENGINE_DLL  CObserver abstract : public CBase
{
public:
	CObserver();
	virtual ~CObserver() = default;

public:
	void OnNotify(const _wstring& eventType, void* data);

	// �� �޾ƿ���
	void OnNotify_Pull(const _wstring& eventType, void* data = nullptr);
	// �� �����ϴ�
	void OnNotify_Push(const _wstring& eventType, void* data = nullptr);

	
	void Reset();

	// �ݹ� ���
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