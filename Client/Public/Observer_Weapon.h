#pragma once

#include "Client_Defines.h"
#include "Observer.h"

// ui���� ���� ���� ���� ������, �� ��ų ����, �������� �˷���

NS_BEGIN(Client)

class CObserver_Weapon : public CObserver
{
public:
	CObserver_Weapon();
	virtual ~CObserver_Weapon() = default;

public:
	virtual void Free() override;


};

NS_END