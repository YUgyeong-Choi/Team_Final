#pragma once

#include "Base.h"
#include "GameInstance.h"

NS_BEGIN(Engine)

class CPulling_Manager final : public CBase
{
private:
	struct RemoveObject {
		_wstring layerName;
		CGameObject* pObj;
		_bool bReset;
	};
private:
	CPulling_Manager();
	virtual ~CPulling_Manager() = default;

public:
	HRESULT Initialize();

	/* [ �ø��� ������Ʈ�� �߰� �Ѵ� ] */
	void Add_PoolObject(const _wstring& wsLayerName, CGameObject* pObj);

	/* [ ����� ������Ʈ�� ������Ʈ �Ŵ������� �� �� �ֵ��� ���̾ �߰��Ѵ� ] */
	/* [ �̶� m_ObjectPools������ �����ȴ� ] */
	void Use_PoolObject(const _wstring& wsLayerName, _bool bReset);
	void UseAll_PoolObjects(const _wstring& wsLayerName, _bool bReset);

	/* [ ����� �� �Ǿ��ٸ� �ٽ� m_ObjectPools�� �߰��Ѵ� ] */
	/* [ �̶� ������Ʈ �Ŵ������� �������ش� ] */
	void Return_PoolObject(const _wstring& wsLayerName, CGameObject* pObj, _bool bReset);
	
	// PriorityUpdate������ ������ �ֵ�
	void Push_WillRemove(const _wstring& wsLayerName, CGameObject* pObj, _bool bReset);
	void RemoveObjMagr_PushPullingMgr();
	void Clear_Pools();
public:
	static CPulling_Manager* Create();
	virtual void Free() override;

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	
	unordered_map<wstring, queue<CGameObject*>> m_ObjectPools;
	list<RemoveObject> m_RemoveObjects;
	//unordered_map<string, vector<CGameObject*>> m_ActiveObjects;
};

NS_END