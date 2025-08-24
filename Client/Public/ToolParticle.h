#pragma once

#include "Client_Defines.h"
#include "ParticleEffect.h"

NS_BEGIN(Engine)
#include "VIBuffer_Point_Instance.h"
NS_END

NS_BEGIN(Client)

class CToolParticle final : public CParticleEffect
{
public:
	typedef struct tagToolPEDesc : public CParticleEffect::DESC
	{
		_bool bLoadingInTool = { false };
	}DESC;
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
	const CVIBuffer_Point_Instance::DESC Get_InstanceBufferDesc();
	void Set_CBuffer(const PARTICLECBUFFER& tCBuffer) { m_pVIBufferCom->Set_CBuffer(tCBuffer); }
	const PARTICLECBUFFER& Get_CBuffer() { return m_pVIBufferCom->Get_CBuffer(); }
	_float* Get_StretchFactor_Ptr() { return &m_fStretchFactor; }

private:
	_bool m_bLoadingInTool = { false };


	CVIBuffer_Point_Instance::DESC m_tDesc = {};

	//_float3				m_vRange;
	//_float2				m_vSize;
	//_float3				m_vCenter;
	//_float3				m_vPivot;
	//_float2				m_vLifeTime;
	//_float2				m_vSpeed;
	//_bool				m_isLoop;
	//_bool				m_bGravity;
	//_float				m_fGravity;
	//
	//_bool				m_bSpin = { false }; 
	//_bool				m_bOrbit = { false };
	//_float3				m_vRotationAxis = {};
	//_float3				m_vOrbitAxis = {};	
	//_float2				m_vRotationSpeed = {}; 
	//_float2				m_vOrbitSpeed = {}; 
	//_float2				m_vAccel = {};
	//_float				m_fMaxSpeed = {};
	//_float				m_fMinSpeed = {};

private:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();

public:
	static CToolParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

public:
	virtual json Serialize() override;
	virtual void Deserialize(const json& j)override;
};

NS_END