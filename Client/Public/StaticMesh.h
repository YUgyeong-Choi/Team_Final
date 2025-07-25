#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
NS_END

NS_BEGIN(Client)

class CStaticMesh final : public CGameObject
{
public:
	typedef struct tagStaticMeshDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar*	szMeshID;
		LEVEL			m_eLevelID;
		_int			iRender = 0;

		_float3			m_vInitPos = { 0.f, 0.f, 0.f };
		_float3			m_vInitScale = { 1.f, 1.f, 1.f };

	}STATICMESH_DESC;

private:
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

public:
	void	SetbDead(_bool bDead) { m_bDead = bDead; }

private:

	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };
	CTexture*		m_pTextureCom = { nullptr };

protected: /* [ 초기화 변수 ] */
	const _tchar*	m_szMeshID = { nullptr };
	LEVEL			m_eLevelID = { LEVEL::END };
	LEVEL			m_eLevelLight = { LEVEL::END };
	_float3			m_InitPos = {};
	_int			m_iRender = {};

protected:
	_bool			m_bDead = {};

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CStaticMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END