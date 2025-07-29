#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CEffectContainer final : public CGameObject
{
private:
	CEffectContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffectContainer(const CEffectContainer& Prototype);
	virtual ~CEffectContainer() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:

private:
	_float				m_fFrame = { };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CEffectContainer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END