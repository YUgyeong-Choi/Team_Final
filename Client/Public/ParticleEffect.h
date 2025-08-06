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
		_float3				vRange;
		_float2				vSize;
		_float3				vCenter;
		_float3				vPivot;
		_float2				vLifeTime;
		_float2				vSpeed; 
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

	void Set_PType(PARTICLETYPE ePType) { m_ePType = ePType; }
	void Set_Loop(_bool isLoop);

#ifdef USE_IMGUI
	PARTICLETYPE* Get_PType_Ptr() { return &m_ePType; }
	_bool* Get_Loop_Ptr() { return &m_isLoop; }
#endif

protected:
	CVIBuffer_Point_Instance*		m_pVIBufferCom = { nullptr };
	_wstring						m_strBufferTag;

	PARTICLETYPE					m_ePType = {}; // Particle Type
	// 로컬/월드 분리할건지
	// 변수로 처리할건지
	// 일단 변수로

protected:
	_uint				m_iNumInstance;
	_float				m_fMaxLifeTime = {};
	_bool				m_bLocal = { true };

	_float3				m_vRange;
	_float2				m_vSize;
	_float3				m_vCenter;
	_float3				m_vPivot;
	_float2				m_vLifeTime;
	_float2				m_vSpeed;
	_bool				m_isLoop;
	_bool				m_bGravity;
	_float				m_fGravity;

protected:
	virtual HRESULT Ready_Components() override;
	HRESULT Bind_ShaderResources();

public:
	static CParticleEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

public:
	virtual json Serialize() override;
	virtual void Deserialize(const json& j)override;
};

NS_END