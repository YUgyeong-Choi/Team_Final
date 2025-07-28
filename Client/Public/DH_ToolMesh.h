#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
class CLight;
NS_END

NS_BEGIN(Client)

class CDH_ToolMesh final : public CGameObject
{
public:
	typedef struct tagDH_ToolDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar* szMeshID;
		_float3		m_vInitPos = { 0.f, 0.f, 0.f };

		_int		iID = { 0 };
		LEVEL		eLEVEL = { LEVEL::END };
	}DHTOOL_DESC;

private:
	CDH_ToolMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDH_ToolMesh(const CDH_ToolMesh& Prototype);
	virtual ~CDH_ToolMesh() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	_uint Get_ID() const { return m_iID; }

private:
	_uint m_iID = { 0 };
	LEVEL m_eTargetLevel = { LEVEL::END };

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

protected: /* [ 초기화 변수 ] */
	const _tchar*	m_szMeshID = { nullptr };
	_float3			m_InitPos = {};

private:
	CLight* m_pLight = { nullptr };

private:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Light();

public:
	static CDH_ToolMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END