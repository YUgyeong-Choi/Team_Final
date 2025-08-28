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

	/* [ 플링할 오브젝트를 추가 한다 ] */
	void Add_PoolObject(const _wstring& wsLayerName, CGameObject* pObj);

	/* [ 사용할 오브젝트를 오브젝트 매니저에서 돌 수 있도록 레이어에 추가한다 ] */
	/* [ 이때 m_ObjectPools에서는 삭제된다 ] */
	void Use_PoolObject(const _wstring& wsLayerName);
	void UseAll_PoolObjects(const _wstring& wsLayerName);

	/* [ 사용이 다 되었다면 다시 m_ObjectPools에 추가한다 ] */
	/* [ 이때 오브젝트 매니저에서 삭제해준다 ] */
	void Return_PoolObject(const _wstring& wsLayerName, CGameObject* pObj);
	
private:
	void Clear_Pools();
public:
	static CPulling_Manager* Create();
	virtual void Free() override;

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	
	unordered_map<wstring, queue<CGameObject*>> m_ObjectPools;
	
	//unordered_map<string, vector<CGameObject*>> m_ActiveObjects;
};

NS_END