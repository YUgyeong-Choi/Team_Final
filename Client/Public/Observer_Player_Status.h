#pragma once


#include "Client_Defines.h"
#include "Observer.h"

NS_BEGIN(Client)

// ���� �ٲ� ��, notify�� player���� ȣ���ؼ� 
// ui���� ����� push callback�� �����ϰ� �Ѵ�

class   CObserver_Player_Status  : public CObserver
{
public:
	CObserver_Player_Status();
	virtual ~CObserver_Player_Status() = default;


private:


public:
	virtual void Free() override;
};


NS_END