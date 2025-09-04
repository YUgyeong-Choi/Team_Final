#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
class COctree;
NS_END

NS_BEGIN(Client)

class CWaterPuddle : public CGameObject
{
public:
	typedef struct tagWaterDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float4x4	WorldMatrix = _float4x4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);
	}WATER_DESC;

protected:
	CWaterPuddle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWaterPuddle(const CWaterPuddle& Prototype);
	virtual ~CWaterPuddle() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

protected: /* [ 초기화 변수 ] */
	LEVEL			m_eLevelLight = { LEVEL::END };
	_float3			m_InitPos = {};
	_bool			m_bDoOnce = {};

protected:

	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };

protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();

public:
	static CWaterPuddle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END