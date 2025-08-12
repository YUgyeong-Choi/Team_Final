#pragma once
#include "GameObject.h"

#include "Client_Defines.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_VolumeMesh;
NS_END

NS_BEGIN(Client)

class CDecal abstract : public CGameObject
{
public:
	enum class TEXTURE_TYPE
	{
		ARMT,
		N,
		BC,
		END
	};

public:
	typedef struct tagDecalDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float4x4	WorldMatrix = _float4x4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);

		wstring PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::END)] = {};
	} DECAL_DESC;

protected:
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

protected:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::END)] = { nullptr };
	CVIBuffer_VolumeMesh* m_pVIBufferCom = { nullptr };
protected:
	virtual HRESULT Ready_Components(void* Arg);
	HRESULT Bind_ShaderResources();

public:
	virtual void Free() override;

};

NS_END