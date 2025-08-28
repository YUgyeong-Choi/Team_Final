#pragma once

#include "Client_Defines.h"
#include "EffectBase.h"

NS_BEGIN(Engine)
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)
// 스프라이트 이펙트 부모
class CSpriteEffect : public CEffectBase
{
public:
	typedef struct tagSpriteEffectDesc : public CEffectBase::DESC
	{

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
	virtual _float Ready_Death();
	virtual void Pause() { m_isEffectActive = false; }

protected:
	CVIBuffer_Rect*		m_pVIBufferCom = { nullptr };

protected:
	virtual HRESULT Ready_Components() override;
	virtual HRESULT Bind_ShaderResources() override;

public:
	static CSpriteEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

public:
	virtual json Serialize()override;
	virtual void Deserialize(const json& j)override;
};

NS_END