#pragma once

#pragma once

#include "Client_Defines.h"
#include "Observer.h"

NS_BEGIN(Client)

class   CObserver_Player_Status  : public CObserver
{
protected:
	CObserver_Player_Status();
	virtual ~CObserver_Player_Status() = default;

public:
	virtual void OnNotify(const _wstring& eventType, void* data = nullptr);

	
	virtual void Reset();


private:
	_float m_fStaminaRatio = {};
	_float m_fHpRatio = {};
	_float m_fMpRatio = {};


public:
	virtual void Free() override;
};


NS_END