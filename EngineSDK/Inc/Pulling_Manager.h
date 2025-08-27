#pragma once

#include "Base.h"
#include "GameInstance.h"

NS_BEGIN(Engine)

class CPulling_Manager final : public CBase
{
private:
	CPulling_Manager();
	virtual ~CPulling_Manager() = default;

public:
	HRESULT Initialize();

	/* [ �ø��� ������Ʈ�� �߰� �Ѵ� ] */
	void Add_PoolObject(string strLayerName, CGameObject* pObj);

	/* [ ����� ������Ʈ�� ������Ʈ �Ŵ������� �� �� �ֵ��� ���̾ �߰��Ѵ� ] */
	/* [ �̶� m_ObjectPools������ �����ȴ� ] */
	void Use_PoolObject(string strLayerName);
	void UseAll_PoolObjects(string strLayerName);

	/* [ ����� �� �Ǿ��ٸ� �ٽ� m_ObjectPools�� �߰��Ѵ� ] */
	/* [ �̶� ������Ʈ �Ŵ������� �������ش� ] */
	void Return_PoolObject(string strLayerName, CGameObject* pObj);
	
private:
	void Clear_Pools();
public:
	static CPulling_Manager* Create();
	virtual void Free() override;

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	
	unordered_map<string, queue<CGameObject*>> m_ObjectPools;
	
	//unordered_map<string, vector<CGameObject*>> m_ActiveObjects;
};

NS_END