#pragma once
#include "GameObject.h"

#include "Client_Defines.h"

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
	HRESULT	Render_ImGui();


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
	void Render_Favorite();
	void Render_Detail();
	void Render_Preview();

private:
	HRESULT Spawn_MapToolObject();
	HRESULT Spawn_MapToolObject(string ModelName);
	HRESULT Duplicate_Selected_Object();
	HRESULT Undo_Selected_Object();
	void	DeleteMapToolObject();
	
	HRESULT Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath);

private:
	//모델 그룹에 추가 제거
	void Add_ModelGroup(string ModelName, CGameObject* pMapToolObject);
	void Delete_ModelGroup(CGameObject* pMapToolObject);

	//현재 포커스된 오브젝트 가져오기
	CGameObject* Get_Focused_Object();

	//아이디로 하이어라키 인덱스 찾기
	_int			Find_HierarchyIndex_By_ID(_uint iID);

	//인덱스로 오브젝트 찾기
	class CMapToolObject*	Find_Object_By_Index(_int iIndex);

private:
	//클릭으로 오브젝트 선택하기
	void SelectByClick();
	//드래그로 오브젝트 다중 선택하기
	void SelectByDrag(const _float2& vMouseDragEnd);

	//마우스 드랙 시작 포인트
	_float2 m_vMouseDragStart = {};
	_bool	m_bDragging = { false };
private:
	void Control_PreviewObject(_float fTimeDelta);

private:
	void Detail_Name();
	void Detail_Transform();
	void Detail_Tile();
	//컨백스 또는 트라이앵글 선택(기본 컨백스)
	void Detail_Collider();

private:
	//이전 프레임에 기즈모를 사용 중인가?
	_bool m_bWasUsingGizmoLastFrame = false;

private:
	_bool m_bPreviewHovered = { false };

private:
	//set하고싶었지만 imgui 선택이 인덱스로 접근해야해서 vector로
	vector<string>	m_ModelNames = {};
	_int			m_iSelectedModelIndex = { -1 };
private:
	//즐겨찾기 목록
	vector<string>	m_FavoriteModelNames = {};
	_int			m_iSelectedFavoriteModelIndex = { -1 };
private:
	//하이어라키 관련
	map<string, list<CGameObject*>> m_ModelGroups;

	//선택된 것들
	set<_int> m_SelectedIndexies;
	set<class CMapToolObject*> m_SelectedObjects = {};

	//포커스
	_int m_iFocusIndex = { -1 };
	class CMapToolObject* m_pFocusObject = { nullptr };

private:
	_uint m_iID = { 0 };

private:
	ImGuizmo::OPERATION m_currentOperation = { ImGuizmo::TRANSLATE };

private:
	class CPreviewObject*	m_pPreviewObject = { nullptr };
	class CCamera_Free*		m_pCamera_Free = { nullptr };

private:
	//복사한 월드 행렬 저장용
	_float4x4 m_CopyWorldMatrix = {};

private:
	//모든 콜라이더를 렌더 할지 말지
	_bool	m_bRenderAllCollider = { false };


public:
	static CMapTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END