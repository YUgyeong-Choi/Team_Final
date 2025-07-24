#pragma once

#include "Client_Defines.h"
#include "BlendObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CSpriteEffect abstract : public CBlendObject
{
public:
	typedef struct tagSpriteEffectDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_bool				bAnimation = { false };
		_uint				iFrame = {};
		const _float4x4*	pSocketMatrix = { nullptr };
	}DESC;

protected:
	CSpriteEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSpriteEffect(const CSpriteEffect& Prototype);
	virtual ~CSpriteEffect() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	CShader*			m_pShaderCom = { nullptr };
	CTexture*			m_pTextureCom = { nullptr };
	CVIBuffer_Rect*		m_pVIBufferCom = { nullptr };

protected:
	_bool				m_bAnimation = { false };
	_float				m_fMaxFrame = { };
	_float				m_fFrame = { };

protected:
	virtual HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	//static CSpriteEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;

};

NS_END