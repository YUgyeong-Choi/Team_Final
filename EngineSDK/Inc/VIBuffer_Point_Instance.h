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
		_bool			bGravity = { false };
		_float			fGravity = { 9.8f };
		_float4			vDirection = {0.f, 1.f, 0.f, 0.f};
		_bool			isTool = { false };
	}DESC;
	
private:
	CVIBuffer_Point_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Point_Instance(const CVIBuffer_Point_Instance& Prototype);
	virtual ~CVIBuffer_Point_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype(const DESC* pDesc);
	virtual HRESULT Initialize(void* pArg);
	virtual void Update(_float fTimeDelta);
	virtual void Update_Tool(_float fCurTrackPos);

	virtual HRESULT Bind_Buffers() override;
	virtual HRESULT Render() override;

	virtual void Directional(_float fTimeDelta, _bool bTool = false);
	virtual void Spread(_float fTimeDelta, _bool bTool = false);

	void Set_Loop(_bool isLoop) { m_isLoop = isLoop; }

protected:
	VTXPOS_PARTICLE_INSTANCE*	m_pVertexInstances = { nullptr };
	_float3						m_vPivot = {};
	_bool						m_bGravity = { false };
	_float						m_fGravity = { 9.8f };
	_bool						m_isLoop = { false };
	_float4						m_vDirection = {};
	PARTICLETYPE				m_ePType;

	typedef struct tagCSMembers {
		_float4		vDirection = {};
		_float		vSpeeds;
		_float4		vRotationAxis_Speed = {};       // 자전용
		_float4		vOrbitAxis_Radius_Speed = {};   // 공전용
		_float4		vOrbitCenter = {};              // 공전 중심점 (선택적)

	}PARTICLEDESC;

	PARTICLEDESC*				m_pParticleDesc = { nullptr };

protected:
	// 잘 몰겠지만 일단 추가하는 변수들

	enum PSCALING{
		PSCALE_UP = 1,
		PSCALE_DOWN = 1 << 1,
		PSCALE_DEFAULT = 1 << 2
	};
	enum PFADE{
		PFADE_IN = 1,
		PFADE_OUT = 1 << 1,
		PFADE_DEFAULT = 1 << 2
	};



protected:
	HRESULT Make_InstanceBuffer(const DESC* pDesc);


public:
	static CVIBuffer_Point_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const DESC* pDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END