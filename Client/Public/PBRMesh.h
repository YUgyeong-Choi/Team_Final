#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
class CPhysXStaticActor;
NS_END

NS_BEGIN(Client)

class CPBRMesh final : public CGameObject
{
public:
	typedef struct tagStaticMeshDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar* szMeshID;

		LEVEL			m_eLevelID;
		_int			iRender = 0;
		_float3 		InitPos = { 0.f, 0.f, 0.f };
		_float3 		InitScale = { 1.f, 1.f, 1.f };

		_tchar		szModelPrototypeTag[MAX_PATH];
		_float4x4	WorldMatrix = _float4x4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);

	}STATICMESH_DESC;

private:
	CPBRMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPBRMesh(const CPBRMesh& Prototype);
	virtual ~CPBRMesh() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;

private:
	void SetCascadeShadow();


	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;

	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
public:
	LEVEL Get_LevelID() const { return m_eLevelID; }
	const _float3& Get_InitPos() const { return m_InitPos; }

public:
	void	SetbDead(_bool bDead) { m_bDead = bDead; }
	void	Toggleummy() { m_bDummyShow = !m_bDummyShow; }

private:

	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };
	CTexture*		m_pTextureCom = { nullptr };

	CPhysXStaticActor* m_pPhysXActorCom = { nullptr };

protected: /* [ 초기화 변수 ] */
	const _tchar*	m_szMeshID = { nullptr };
	LEVEL			m_eLevelID = { LEVEL::END };
	LEVEL			m_eLevelLight = { LEVEL::END };
	_float3			m_InitPos = {};
	_int			m_iRender = {};

protected:
	_bool			m_bDead = {};
	_bool			m_bDoOnce = {};
	_bool			m_bDummyShow = { true };

protected:
	SHADOW			m_eShadow = SHADOW::SHADOW_END;

private:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Collider();
public:
	static CPBRMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END