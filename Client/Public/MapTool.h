#pragma once
#include "GameObject.h"

#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI

#define PATH_NONANIM "../Bin/Resources/Models/Bin_NonAnim"
#define PRE_TRANSFORMMATRIX_SCALE 0.01f

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
	//맵툴 조작
	void Control(_float fTimeDelta);

private:
	HRESULT Ready_Model();
	HRESULT Save_Map();
	HRESULT Load_Map();

private:
	HRESULT Render_MapTool();

private:
	void Render_Hierarchy();
	void Render_Asset();
	void Render_Detail();
	void Render_Preview();

private:
	HRESULT Spawn_MapToolObject();
	HRESULT Duplicate_Selected_Object();
	HRESULT Undo_Selected_Object();
	void	DeleteMapToolObject();
	
	HRESULT Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath);

private:
	void Add_ModelGroup(string ModelName, CGameObject* pMapToolObject);
	void Delete_ModelGroup(CGameObject* pMapToolObject);

	CGameObject* Get_Selected_GameObject();
	_int		Find_HierarchyIndex_By_ID(_uint iID);

private:
	void Picking();

private:
	void Control_PreviewObject(_float fTimeDelta);

private:
	//이전 프레임에 기즈모를 사용 중인가?
	bool m_bWasUsingGizmoLastFrame = false;

private:
	_bool m_bPreviewHovered = { false };

private:
	//set하고싶었지만 imgui 선택이 인덱스로 접근해야해서 vector로
	vector<string>	m_ModelNames = {};
	_int			m_iSelectedModelIndex = { -1 };

private:
	map<string, list<CGameObject*>> m_ModelGroups;
	_int m_iSelectedHierarchyIndex = { -1 };

private:
	_uint m_iID = { 0 };

private:
	ImGuizmo::OPERATION m_currentOperation = { ImGuizmo::TRANSLATE };

private:
	class CPreviewObject* m_pPreviewObject = { nullptr };

public:
	static CMapTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END