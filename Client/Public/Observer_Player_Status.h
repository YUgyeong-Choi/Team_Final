#pragma once


#include "Client_Defines.h"
#include "Observer.h"

NS_BEGIN(Client)

// ui에 표기할 stat을 저장하고, 값이 바뀔 때, notify를 player에서 호출해서 
// ui에서 등록한 push callback을 동작하게 한다

class   CObserver_Player_Status  : public CObserver
{
public:
	CObserver_Player_Status();
	virtual ~CObserver_Player_Status() = default;


private:
	_float m_fStaminaRatio = {1.f};
	_float m_fHpRatio = {1.f};
	_float m_fMpRatio = {1.f};

	_int m_iCurrentHP = {};
	_int m_iMaxHP = {};

	_int m_iCurrentStamina = {};
	_int m_iMaxStamina = {};

	_int m_iCurrentMana = {};
	_int m_iMaxMana = {};


public:
	virtual void Free() override;
};


NS_END