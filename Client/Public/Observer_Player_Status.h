#pragma once


#include "Client_Defines.h"
#include "Observer.h"

NS_BEGIN(Client)

// 값이 바뀔 때, notify를 player에서 호출해서 
// ui에서 등록한 push callback을 동작하게 한다

class   CObserver_Player_Status  : public CObserver
{
public:
	CObserver_Player_Status();
	virtual ~CObserver_Player_Status() = default;



	_float& Get_Hp() { return m_fHp; }

private:

	_float	m_fHp = {};
	_float  m_fMaxHp = {};
	_float  m_fMana = {};
	_float  m_fMaxMana = {};

public:
	virtual void Free() override;
};


NS_END