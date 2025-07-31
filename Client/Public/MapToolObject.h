#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CPhysXDynamicActor;
class CPhysXStaticActor;
NS_END

NS_BEGIN(Client)

class CMapToolObject final : public CGameObject
{
	friend class CMapTool;

public:
	typedef struct tagMapToolObjectDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_tchar		szModelName[MAX_PATH];
		_tchar		szModelPrototypeTag[MAX_PATH];
		_float4x4	WorldMatrix = _float4x4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);
		_uint		iID = { 0 };
		_bool		bUseTiling = { false };
		_float2		vTileDensity = { 1.f, 1.f };
		COLLIDER_TYPE eColliderType = { COLLIDER_TYPE::NONE };
	}MAPTOOLOBJ_DESC;

private:
	CMapToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapToolObject(const CMapToolObject& Prototype);
	virtual ~CMapToolObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	HRESULT Render_Collider();
public:
	void Undo_WorldMatrix();
	void Update_ColliderPos();
	void Set_UndoWorldMatrix(const _fmatrix WorldMatrix) {
		m_bCanUndo = true;
		XMStoreFloat4x4(&m_UndoWorldMatrix, WorldMatrix);
	}

public:
	const wstring& Get_ModelPrototypeTag() const {
		return m_ModelPrototypeTag;
	}

	_uint Get_ID() const {
		return m_iID;
	}

	const string& Get_ModelName() const {
		return m_ModelName;
	}

private:
	_bool		m_bCanUndo = { false };
	_float4x4	m_UndoWorldMatrix = {};

private:
	wstring m_ModelPrototypeTag = {};
	string	m_ModelName = {};
	//맵 오브젝트들 간 고유 식별 번호
	_uint	m_iID = { 0 };

private:
	_bool	m_bUseTiling = { false };
	_float	m_TileDensity[2] = { 1.0f, 1.0f };

private:
	COLLIDER_TYPE m_eColliderType = { COLLIDER_TYPE::NONE };

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorConvexCom = { nullptr };
	CPhysXStaticActor*	m_pPhysXActorTriangleCom = { nullptr };

private:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Collider();
public:
	static CMapToolObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END