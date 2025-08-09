#pragma once
#include "GameObject.h"

#include "Client_Defines.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_VolumeMesh;
NS_END

NS_BEGIN(Client)

class CDecal final : public CGameObject
{
public:
	typedef struct tagDecalDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float4x4	WorldMatrix = _float4x4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);

	}DECAL_DESC;

private:
	CDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDecal(const CDecal& Prototype);
	virtual ~CDecal() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta)override;
	virtual void Update(_float fTimeDelta)override;
	virtual void Late_Update(_float fTimeDelta)override;
	virtual HRESULT Render()override;

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_VolumeMesh* m_pVIBufferCom = { nullptr };
private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CDecal* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END