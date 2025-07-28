#pragma once

#include "Client_Defines.h"
#include "EffectBase.h"

NS_BEGIN(Engine)
class CVIBuffer_Point_Instance;
NS_END

NS_BEGIN(Client)

class CParticleEffect : public CEffectBase
{
public:
	typedef struct tagParticleEffectDesc : public CEffectBase::DESC
	{
		/* Instance Buffer Desc */
		_uint				iNumInstance;
		_float3				vRange;
		_float2				vSize;
		_float3				vCenter;
		_float3				vPivot;
		_float2				vLifeTime;
		_float2				vSpeed; 
		_bool				isLoop;
		PARTICLETYPE		ePType;
	}DESC;

protected:
	CParticleEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticleEffect(const CParticleEffect& Prototype);
	virtual ~CParticleEffect() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Set_PType(PARTICLETYPE ePType) { m_ePType = ePType; }
	void Set_Loop(_bool isLoop);

protected:
	CVIBuffer_Point_Instance*		m_pVIBufferCom = { nullptr };
	PARTICLETYPE					m_ePType = { }; // Particle Type

protected:
	_uint				m_iNumInstance;
	_float3				m_vPivot = {};
	_float				m_fMaxLifeTime = {};
	_bool				m_isLoop = { false };

protected:
	HRESULT Ready_Components(void* pArg);

public:
	static CParticleEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END