#pragma once

#include "Client_Defines.h"
#include "EffectBase.h"
#include "VIBuffer_Point_Instance.h"

//NS_BEGIN(Engine)
//class CVIBuffer_Point_Instance;
//NS_END

NS_BEGIN(Client)

class CParticleEffect : public CEffectBase
{
public:
	typedef struct tagParticleEffectDesc : public CEffectBase::DESC
	{
		/* Instance Buffer Desc */
		_uint				iNumInstance = 1;
		_float3				vPivot;
		_float2				vLifeTime;
		_float2				vSpeed; 
		_float3				vRange;
		_float3				vCenter;
		_float2				vSize;
		PARTICLETYPE		ePType;

		/* Buffer Tag */
		_wstring			strBufferTag;
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

	virtual _float Ready_Death();

	void Set_PType(PARTICLETYPE ePType) { m_ePType = ePType; }
	void Set_Loop(_bool isLoop);

#ifdef USE_IMGUI
	PARTICLETYPE* Get_PType_Ptr() { return &m_ePType; }
	_bool* Get_Loop_Ptr() { return &m_isLoop; }
#endif

protected:
	CVIBuffer_Point_Instance*	m_pVIBufferCom = { nullptr };
	_wstring					m_strBufferTag;

	PARTICLETYPE				m_ePType = {}; // Particle Type
	_bool						m_bFirst = { true };
	_bool						m_bReadyDeath = { false };
protected:
	_uint						m_iNumInstance;
	_float						m_fMaxLifeTime = {};
	_bool						m_bLocal = { true };
	_float						m_fStretchFactor = { 0.015f };

protected:
	virtual HRESULT Ready_Components() override;
	virtual HRESULT Bind_ShaderResources() override;

public:
	static CParticleEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

public:
	virtual json Serialize() override;
	virtual void Deserialize(const json& j)override;
};

NS_END