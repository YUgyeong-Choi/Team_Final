#pragma once

#include "Client_Defines.h"
#include "EffectBase.h"

NS_BEGIN(Engine)
class CVIBuffer_Trail;
NS_END

NS_BEGIN(Client)

class CTrailEffect : public CEffectBase
{
public:
	typedef struct tagTrailEffectDesc : public CEffectBase::DESC
	{

	}DESC;

protected:
	CTrailEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTrailEffect(const CTrailEffect& Prototype);
	virtual ~CTrailEffect() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	CVIBuffer_Trail*		m_pVIBufferCom = { nullptr };

protected:
	virtual HRESULT Ready_Components() override;
	HRESULT Bind_ShaderResources();

public:
	static CTrailEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

public:
	virtual json Serialize()override;
	virtual void Deserialize(const json& j)override;
};

NS_END