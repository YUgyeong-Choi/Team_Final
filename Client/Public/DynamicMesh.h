#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
class CPhysXDynamicActor;
NS_END

NS_BEGIN(Client)

class CDynamicMesh : public CGameObject
{
public:
	typedef struct tagDynamicMeshDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar*	szMeshID;

		LEVEL			m_eMeshLevelID;

		_tchar		szModelPrototypeTag[MAX_PATH] = { 0 };
		_float4x4	WorldMatrix = _float4x4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);
		_float fColliderScale;
		_int   iColliderMeshIdx;
	}DYNAMICMESH_DESC;

protected:
	CDynamicMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDynamicMesh(const CDynamicMesh& Prototype);
	virtual ~CDynamicMesh() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	AABBBOX GetWorldAABB() const;

public:
	LEVEL Get_LevelID() const { return m_eMeshLevelID; }

protected:
	virtual void Update_ColliderPos();

protected: /* [ 초기화 변수 ] */
	const _tchar*	m_szMeshID = { nullptr };
	LEVEL			m_eMeshLevelID = { LEVEL::END };
	_bool			m_bDoOnce = {};
protected:

	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };
	CTexture*		m_pTextureCom = { nullptr };
	CTexture*		m_pEmissiveCom = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };

protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Collider(void* pArg);

public:
	static CDynamicMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END