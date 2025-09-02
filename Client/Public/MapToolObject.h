#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CPhysXDynamicActor;
class CPhysXStaticActor;
class CTexture;
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
		_int		iLightShape = { 0 }; //����Ʈ ���
		_bool		bNoInstancing = { false }; //�ν��Ͻ� ���� ����

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
	void Set_Collider(COLLIDER_TYPE eColliderType);
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
	//�� ������Ʈ�� �� ���� �ĺ� ��ȣ
	_uint	m_iID = { 0 };

private:
	_bool	m_bUseTiling = { false };
	_float	m_TileDensity[2] = { 0.3f, 0.3f };

private:
	COLLIDER_TYPE m_eColliderType = { COLLIDER_TYPE::NONE };

private:
	_int	m_iLightShape = { 0 }; // ����Ʈ ���

private:
	_bool	m_bNoInstancing = { false }; //�ν��Ͻ��� ���ϰ� �Ѵ�.

private:
	LOD m_eLOD = { LOD::LOD0 }; // ���� LOD ����

private:
	_bool			m_bDoOnce = {};

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom[ENUM_CLASS(LOD::END)] = {nullptr};
	CPhysXDynamicActor* m_pPhysXActorConvexCom = { nullptr };
	CPhysXStaticActor*	m_pPhysXActorTriangleCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };

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