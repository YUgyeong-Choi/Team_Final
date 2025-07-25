#pragma once

#include "Client_Defines.h"
#include "EffectBase.h"

NS_BEGIN(Engine)
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)
// 스프라이트 이펙트 부모
class CSpriteEffect abstract : public CEffectBase
{
public:
	typedef struct tagSpriteEffectDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_bool		bAnimation = { false };
		_uint		iFrame = {};
		_uint		iUVWidth = {};
		_uint		iUVHeight = {};
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
	CVIBuffer_Rect*		m_pVIBufferCom = { nullptr };

protected:
	_bool				m_bAnimation = { false };
	_float				m_fMaxFrame = { };
	_float				m_fFrame = { };

protected:
	virtual HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	virtual CGameObject* Clone(void* pArg) PURE;
	virtual void Free() override;

};

NS_END