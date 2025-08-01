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

public:

	virtual void Free() override;
};

NS_END