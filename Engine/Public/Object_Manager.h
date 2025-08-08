#pragma once

#include "Base.h"

/* 레벨별로 구분하여 사본객체를 보관한다. */
/* 리스트들로 보관한다. 리스트 == 객체들 == 내가 나누고 싶은 기준에따라 그룹짓는다. */

NS_BEGIN(Engine)

class CObject_Manager final : public CBase
{
private:
	CObject_Manager();
	virtual ~CObject_Manager() = default;

public:
	class CComponent* Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strComponentTag, _uint iIndex);
	class CComponent* Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, _uint iPartID, const _wstring& strComponentTag, _uint iIndex);

public:
	HRESULT Initialize(_uint iNumLevels);
	HRESULT Add_GameObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLevelIndex, const _wstring& strLayerTag, void* pArg);
	HRESULT Add_GameObjectReturn(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLevelIndex, const _wstring& strLayerTag, class CGameObject** ppOut, void* pArg);
	void Priority_Update(_float fTimeDelta);
	void Update(_float fTimeDelta);
	void Late_Update(_float fTimeDelta);
	void Last_Update(_float fTimeDelta);
	void Clear(_uint iLevelIndex);

	list<class CGameObject*>& Get_ObjectList(_uint iLevelIndex, const _wstring& strLayerTag);
	class CGameObject* Find_LastObject(_uint iLevelIndex, const _wstring& strLayerTag);
	class CGameObject* Find_Object(_uint iLevelIndex, const _wstring& strLayerTag, _uint iIndex);

	//해당 이름을 포함한 레이어들의 태그를 모두 가져온다.
	vector<wstring> Find_LayerNamesContaining(_uint iLevelIndex, const wstring& SubString);

	//해당 레벨에 있는 레이어를 챙겨온다.
	const map<const _wstring, class CLayer*>& Get_Layers(_uint iLevelIndex) const {
		return m_pLayers[iLevelIndex]; 
	}

private:	
	_uint								m_iNumLevels = {};
	map<const _wstring, class CLayer*>* m_pLayers = { nullptr };
	class CGameInstance*				m_pGameInstance = { nullptr };

private:
	class CLayer* Find_Layer(_uint iLevelIndex, const _wstring& strLayerTag);

public:
	static CObject_Manager* Create(_uint iNumLevels);
	virtual void Free() override;
};

NS_END