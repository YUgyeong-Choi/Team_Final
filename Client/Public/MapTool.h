#pragma once
#include "YWTool.h"
#include "Client_Defines.h"

#include "MapToolObject.h"

NS_BEGIN(Client)

class CMapTool final : public CYWTool
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
	virtual HRESULT	Render_ImGui() override;
	virtual HRESULT Load(const _char* Map) override;
	virtual HRESULT Save(const _char* Map) override;

private:
	HRESULT Load_StaticMesh(const _char* Map);
	HRESULT Load_Stargazer(const _char* Map);
	HRESULT Load_ErgoItem(const _char* Map);
	HRESULT Load_Breakable(const _char* Map);

private:
	//���� ����
	void Control(_float fTimeDelta);
private:
	//ERT�� ���� ����
	void Change_Operation();
	//���� ����
	void DeselectObject();
	//�ݶ��̴� ����
	void Change_ColliderType();
	//���� �̵�
	void SnapTo();

private:
	HRESULT Ready_Model(const _char* Map);

	HRESULT Save_Favorite();
	HRESULT Load_Favorite();

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
	void	Clear_Map();
	
	HRESULT Load_Mesh(const wstring& strPrototypeTag, const _char* pModelFilePath);

private:
	//�� �׷쿡 �߰� ����
	void Add_ModelGroup(string ModelName, CGameObject* pMapToolObject);
	void Delete_ModelGroup(CGameObject* pMapToolObject);

	//���� ��Ŀ���� ������Ʈ ��������
	CGameObject* Get_Focused_Object();

	//���̵�� ���̾��Ű �ε��� ã��
	_int			Find_HierarchyIndex_By_ID(_uint iID);

	//�ε����� ������Ʈ ã��
	class CMapToolObject*	Find_Object_By_Index(_int iIndex);

private:
	//Ŭ������ ������Ʈ �����ϱ�
	void SelectByClick();
	//�巡�׷� ������Ʈ ���� �����ϱ�
	void SelectByDrag(const _float2& vMouseDragEnd);
private:
	void Control_PreviewObject(_float fTimeDelta);

private:
	void Detail_Name();
	void Detail_ObjType();
	void Detail_Transform();
	void Detail_Tile();
	//���齺 �Ǵ� Ʈ���̾ޱ� ����(�⺻ ���齺)
	void Detail_Collider();
	void Detail_LightShape();
	void Detail_NoInstancing();
	void Detail_IsFloor();
	void Detail_CullNone();

	//�� �ٶ�� �±�
	void Detail_StargazerTag();

	//������ �±�
	void Detail_ItemTag();

	//�߼Ҹ� �±�
	void Detail_FootStepSound();

private:
	HRESULT Add_Favorite(const string& ModelName, _bool bSave);

private:
	const _char* m_ObjType[ENUM_CLASS(CMapToolObject::OBJ_TYPE::END)] = {
		"STATIC_MESH",
		"STARGAZER",
		"ERGO_ITEM",
		"BREAKABLE",
	};

private:
	const _char* m_StargazerTag[ENUM_CLASS(STARGAZER_TAG::END)] = {
		"OUTER",
		"FESTIVAL_LEADER",
		"FIRE_EATER",
		"FESTIVAL_LEADER_IN",
	};

private:
	const _char* m_ItemTag[ENUM_CLASS(ITEM_TAG::END)] = {
		"COGWHEEL", //��Ϲ���
		"PULSE_CELL", //�޽� ����
		"ERGO_SHARD", //����� ������ ����
		"PASSENGER_NOTE", //�°��� ����
		"KEY",
		"KRAT_TIMES" //ũ��Ʈ Ÿ�ӽ�
	};

private:
	const _char* m_FootStepSound[ENUM_CLASS(FOOTSTEP_SOUND::END)] = {
	"CARPET", "SKIN", "WOOD", "DIRT", "STONE", "WATER",
	};

private:
	//������ �ִ� ������ ��������?
	_bool m_bMaxFragment = { true };

private:
	//���콺 �巢 ���� ����Ʈ
	_float2 m_vMouseDragStart = {};
	_bool	m_bDragging = { false };

private:
	//���� �����ӿ� ����� ��� ���ΰ�?
	_bool m_bWasUsingGizmoLastFrame = false;

private:
	_bool m_bPreviewHovered = { false };

private:
	//set�ϰ�;����� imgui ������ �ε����� �����ؾ��ؼ� vector��
	vector<string>	m_ModelNames = {};
	_int			m_iSelectedModelIndex = { -1 };
private:
	//���ã�� ���
	vector<string>	m_FavoriteModelNames = {};
	_int			m_iSelectedFavoriteModelIndex = { -1 };
private:
	//���̾��Ű ����
	map<string, list<CGameObject*>> m_ModelGroups;

	//���õ� �͵�
	set<_int> m_SelectedIndexies;
	set<class CMapToolObject*> m_SelectedObjects = {};

	//��Ŀ��
	_int m_iFocusIndex = { -1 };
	class CMapToolObject* m_pFocusObject = { nullptr };

private:
	ImGuizmo::OPERATION m_currentOperation = { ImGuizmo::TRANSLATE };

private:
	class CPreviewObject*	m_pPreviewObject = { nullptr };
	class CCamera_Free*		m_pCamera_Free = { nullptr };

private:
	//������ ���� ��� �����
	_float4x4 m_CopyWorldMatrix = {};

private:
	//��� �ݶ��̴��� ���� ���� ����
	_bool	m_bRenderAllCollider = { false };


public:
	static CMapTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END