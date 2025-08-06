#pragma once

#include "Client_Defines.h"
#include "Observer.h"

// ui에게 현재 착용 중인 아이템, 각 스킬 정보, 내구도를 알려줌

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