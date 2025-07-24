#pragma once

#include "Client_Defines.h"
#include "EffectBase.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Point_Instance;
NS_END

NS_BEGIN(Client)

class CToolParticle final : public CEffectBase
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

	void Set_PType(_uint iPType) { m_iPType = iPType; }
	void Set_Loop(_bool isLoop);

private:
	CVIBuffer_Point_Instance*		m_pVIBufferCom = { nullptr };
	_uint							m_iPType = { 0 }; // Particle Type

private:
	HRESULT Ready_Components();

public:
	static CToolParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END