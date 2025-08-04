#pragma once

#include "Base.h"

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
	void Register_PullCallback(function<void(const _wstring& eventType, void* data)> callback) { m_PullCallbacks.push_back(callback); }
	void Register_PushCallback(function<void(const _wstring& eventType, void* data)> callback) { m_PushCallbacks.push_back(callback); }

	void Clear_PullCallback() { m_PullCallbacks.clear(); }
	void Clear_PushCallback() { m_PushCallbacks.clear(); }

protected:
	
	vector<function<void(const _wstring& eventType, void* data)>> m_PullCallbacks;
	vector<function<void(const _wstring& eventType, void* data)>> m_PushCallbacks;

public:

	virtual void Free() override;
};

NS_END