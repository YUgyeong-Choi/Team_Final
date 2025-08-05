#pragma once

#include "Client_Defines.h"
#include "Observer.h"

// ui에게 현재 있는 아이템의 정보를 알려주고, ui는 아이템 프로토타입에 맞춰서 텍스처를 바꿔서 적용하도록
// 하이라이트 되고 있는 칸을 알려준다?
//

NS_BEGIN(Client)

class CObserver_Slot : public CObserver
{
public:
	CObserver_Slot();
	virtual ~CObserver_Slot() = default;

private:

	vector<ITEM_DESC> m_BeltUpDescs;
	vector<ITEM_DESC> m_BeltDownDescs;

	

public:
	virtual void Free() override;


};

NS_END