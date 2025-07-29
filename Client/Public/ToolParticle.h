#pragma once

#include "Client_Defines.h"
#include "ParticleEffect.h"

NS_BEGIN(Client)

class CToolParticle final : public CParticleEffect
{

private:
	CToolParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CToolParticle(const CToolParticle& Prototype);
	virtual ~CToolParticle() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	virtual void Update_Tool(_float fTimeDelta, _float fCurFrame);

public:
	HRESULT Change_InstanceBuffer(void* pArg);

private:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();

public:
	static CToolParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END