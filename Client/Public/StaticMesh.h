#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
class CPhysXStaticActor;
class COctree;
NS_END

NS_BEGIN(Client)

class CStaticMesh : public CGameObject
{
public:
	typedef struct tagStaticMeshDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar*	szMeshID;

		LEVEL			m_eLevelID;
		_int			iRender = 0;

		_tchar		szModelPrototypeTag[MAX_PATH] = { 0 };
		_float4x4	WorldMatrix = _float4x4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);


		_bool		bUseOctoTree = { true };
		_bool		bUseTiling = { false };
		_float2		vTileDensity = { 1.f, 1.f };
		COLLIDER_TYPE eColliderType = { COLLIDER_TYPE::NONE };
		_int		iLightShape = { 0 };

	}STATICMESH_DESC;

protected:
	CStaticMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStaticMesh(const CStaticMesh& Prototype);
	virtual ~CStaticMesh() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	LEVEL Get_LevelID() const { return m_eLevelID; }
	const _float3& Get_InitPos() const { return m_InitPos; }

private:
	void Update_ColliderPos();

protected: /* [ 초기화 변수 ] */
	const _tchar*	m_szMeshID = { nullptr };
	const _tchar*	m_szMeshFullID = { nullptr };
	LEVEL			m_eLevelID = { LEVEL::END };
	LEVEL			m_eLevelLight = { LEVEL::END };
	_float3			m_InitPos = {};
	_int			m_iRender = {};
	_bool			m_bDoOnce = {};

private:
	_bool	m_bUseOctoTree = { true };
	_bool	m_bUseTiling = { false };
	_float2	m_vTileDensity = { 1.0f, 1.0f };

private:
	COLLIDER_TYPE m_eColliderType = { COLLIDER_TYPE::NONE };

private:
	_int m_iLightShape = { 0 };

protected:

	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };
	CTexture*		m_pTextureCom = { nullptr };
	CTexture*		m_pEmissiveCom = { nullptr };

protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Collider();

public:
	static CStaticMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END