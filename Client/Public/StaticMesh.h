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
		_int			iRender = 0;

		_tchar		szModelPrototypeTag[MAX_PATH] = { 0 };
		//_float4x4	WorldMatrix = _float4x4(
		//	1.f, 0.f, 0.f, 0.f,
		//	0.f, 1.f, 0.f, 0.f,
		//	0.f, 0.f, 1.f, 0.f,
		//	0.f, 0.f, 0.f, 1.f
		//);


		_bool		bUseOctoTree = { true }; // 병합 할때 true 필요, false
		_bool		bUseTiling = { false };
		_float2		vTileDensity = { 1.f, 1.f };
		COLLIDER_TYPE eColliderType = { COLLIDER_TYPE::NONE };
		_int		iLightShape = { 0 };
		_bool		bIsFloor = { false }; //바닥 여부
		_bool		bCullNone = { false }; //컬링 여부
		FOOTSTEP_SOUND eFS_Sound = { FOOTSTEP_SOUND::END };
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
	void LOD_Update();

private:
	HRESULT SetEmissive(_int LightShape);

public:
	HRESULT Add_Actor();

public:
	const _float3& Get_InitPos() const { return m_InitPos; }

private:
	void Update_ColliderPos();
public:
	const wstring& Get_MeshName() const {
		return m_strMeshName;
	}
public:
	FOOTSTEP_SOUND Get_FootStepSound() const {
		return m_eFS_Sound;
	}

protected: /* [ 초기화 변수 ] */
	const _tchar*	m_szMeshID = { nullptr };
	const _tchar*	m_szMeshFullID = { nullptr };
	LEVEL			m_eLevelLight = { LEVEL::END };
	_float3			m_InitPos = {};
	_int			m_iRender = {};
	_bool			m_bDoOnce = {};
	wstring			m_strMeshName = {};

private:
	_bool	m_bUseOctoTree = { true };
	_bool	m_bUseTiling = { false };
	_float2	m_vTileDensity = { 1.0f, 1.0f };
	_float  m_fEmissive = {};

private:
	COLLIDER_TYPE m_eColliderType = { COLLIDER_TYPE::NONE };

protected:
	_int m_iLightShape = { 0 };

	unordered_set<_int> m_mapVisibleLight = { 1, 3, 4, 6 };

private:
	//바닥 여부
	_bool m_bIsFloor = { false };

private:
	//컬링 여부
	_bool m_bCullNone = { false };

private:
	LOD m_eLOD = { LOD::LOD0 }; // 현재 LOD 상태

private:
	//풋스텝 사운드
	FOOTSTEP_SOUND m_eFS_Sound = { FOOTSTEP_SOUND::END };

protected:

	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom[ENUM_CLASS(LOD::END)] = { nullptr };
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