#pragma once

#include "Base.h"
//#include <mutex>

/* 객체들을 모아놓는다. */

NS_BEGIN(Engine)

class CLayer final : public CBase
{
private:
	CLayer();
	virtual ~CLayer() = default;

public:
	class CComponent* Get_Component(const _wstring& strComponentTag, _uint iIndex);
	class CComponent* Get_Component(_uint iPartID, const _wstring& strComponentTag, _uint iIndex);
	//게임 오브젝트들 가져온다.
	const list<class CGameObject*>& Get_GameObjects() const { 
		return m_GameObjects; 
	}
public:
	HRESULT Add_GameObject(class CGameObject* pGameObject);
	CGameObject* Remove_GameObject(class CGameObject* pGameObject);

	void Priority_Update(_float fTimeDelta);
	void Update(_float fTimeDelta);
	void Late_Update(_float fTimeDelta);
	void Last_Update(_float fTimeDelta);

	CGameObject* Get_Object(_uint iIndex);
	CGameObject* Get_LastObject();
	list<class CGameObject*>& Get_ObjectList();
private:
	list<class CGameObject*>			m_GameObjects;

	//mutex m_mtx = {};

public:
	static CLayer* Create();
	virtual void Free();
};

NS_END