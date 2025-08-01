#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL  CObserver abstract : public CBase
{
protected:
	CObserver();
	virtual ~CObserver() = default;

public:
	virtual void OnNotify(const _wstring& eventType, void* data = nullptr) = 0;

	// Ǯ���Ǹ� ������ ���� ó�� ���·� �ٽ� ����� �ٽ� ����?
	virtual void Reset() = 0;

	// �ݹ�� 
	void Register_Callback(function<void(const _wstring& eventType, void* data)> callback) { m_Callbacks.push_back(callback); }
	void Clear_Callback() { m_Callbacks.clear(); }

private:
	vector<function<void(const _wstring& eventType, void* data)>> m_Callbacks;

public:

	virtual void Free() override;
};

NS_END