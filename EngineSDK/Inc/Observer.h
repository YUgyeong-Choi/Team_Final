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

	// 풀링되면 지우지 말고 처음 상태로 다시 만들고 다시 쓰기?
	virtual void Reset() = 0;

public:

	virtual void Free() override;
};

NS_END