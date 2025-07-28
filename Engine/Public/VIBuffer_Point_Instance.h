#pragma once

#include "VIBuffer_Instance.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Point_Instance final : public CVIBuffer_Instance
{
public:
	typedef struct tagPointInstance : public CVIBuffer_Instance::INSTANCE_DESC
	{
		_float3			vPivot;
		_float2			vLifeTime;
		_float2			vSpeed;
		_bool			isLoop;
		PARTICLETYPE	ePType;
		_float3			vRange;
		_float2			vSize;
		_float3			vCenter;

	}DESC;
	
private:
	CVIBuffer_Point_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Point_Instance(const CVIBuffer_Point_Instance& Prototype);
	virtual ~CVIBuffer_Point_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype(const INSTANCE_DESC* pDesc);
	virtual HRESULT Initialize(void* pArg);
	virtual void Update(_float fTimeDelta);

	virtual HRESULT Bind_Buffers() override;
	virtual HRESULT Render() override;

	virtual void Drop(_float fTimeDelta);
	virtual void Spread(_float fTimeDelta);

	void Set_Loop(_bool isLoop) { m_isLoop = isLoop; }

protected:
	VTXPOS_PARTICLE_INSTANCE*	m_pVertexInstances = { nullptr };
	_float*						m_pSpeeds = { nullptr };
	_float3						m_vPivot = {};
	_bool						m_isLoop = { false };
	PARTICLETYPE				m_ePType;


public:
	static CVIBuffer_Point_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const INSTANCE_DESC* pDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END