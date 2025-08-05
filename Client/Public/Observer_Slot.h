#pragma once

#include "Client_Defines.h"
#include "Observer.h"

// ui���� ���� �ִ� �������� ������ �˷��ְ�, ui�� ������ ������Ÿ�Կ� ���缭 �ؽ�ó�� �ٲ㼭 �����ϵ���
// ���̶���Ʈ �ǰ� �ִ� ĭ�� �˷��ش�?
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