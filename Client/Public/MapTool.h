#pragma once
#include "GameObject.h"

#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI

#define PATH_NONANIM "../Bin/Resources/Models/Bin_NonAnim"

NS_BEGIN(Client)

class CMapTool final : public CGameObject
{
private:
	CMapTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapTool(const CMapTool& Prototype);
	virtual ~CMapTool() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	HRESULT Ready_Model();
	HRESULT Save_Map();
	HRESULT Load_Map();

private:
	HRESULT Render_MapTool();

private:
	void Hierarchy();
	void Asset();

private:
	HRESULT Spawn_MapToolObject();
	HRESULT Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath);

private:
	void Add_ModelGroup(string ModelName, CGameObject* pMapToolObject);
	void Delete_ModelGroup(CGameObject* pMapToolObject);

	CGameObject* Get_Selected_GameObject();
private:
	//set하고싶었지만 imgui 선택이 인덱스로 접근해야해서 vector로
	vector<string>	m_ModelNames = {};
	_int			m_iSelectedModelIndex = { -1 };

private:
	map<string, list<CGameObject*>> m_ModelGroups;
	_int m_iSelectedHierarchyIndex = { -1 };

private:
	_uint m_iID = { 0 };

public:
	static CMapTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END