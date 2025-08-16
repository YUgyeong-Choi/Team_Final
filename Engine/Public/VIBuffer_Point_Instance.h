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
		_float4			vDirection = {0.f, 1.f, 0.f, 0.f}; // ��������	
		_bool			bSpin = { false }; // ���� ����
		_bool			bOrbit = { false }; // ���� ����
		_float3			vRotationAxis = {};       // ������, xyz �� w �ӵ�
		_float3			vOrbitAxis = {};			// ������, xyz �� w �ӵ�
		_float2			vRotationSpeed = {}; // ���� �ӵ�
		_float2			vOrbitSpeed = {}; // ���� �ӵ�
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

	void Set_Loop(_bool isLoop) { m_tCBuffer.bIsLoop = isLoop; }


private:	
	VTXPOS_PARTICLE_INSTANCE*	m_pVertexInstances = { nullptr };
	//_float3						m_vPivot = {};
	//_bool						m_bGravity = { false };
	//_float						m_fGravity = { 9.8f };
	//_bool						m_isLoop = { false };
	//_float4						m_vDirection = {};
	//PARTICLETYPE				m_ePType;
	//_float3						m_vCenter = {};
	//_bool						m_isTool = { false };
	//_bool						m_bSpin = { false }; // ���� ����
	//_bool						m_bOrbit = { false }; // ���� ����
	//_float3						m_vRotationAxis = {};// ������, xyz �� w �ӵ�
	//_float3						m_vOrbitAxis = {};	// ������, xyz �� w �ӵ�
	//_float						m_fOrbitRadius = {};// ���� ������ (radius�� ������ �ʱ� ��ġ - �߽��� ���̷� ���)
	//_float4						m_vOrbitCenter = {};    // ���� �߽��� (������)

	PARTICLEDESC*				m_pParticleDesc = { nullptr };	
	PARTICLECBUFFER				m_tCBuffer = {};

private:
	/* [CS] */
	class CParticleComputeShader* m_pParticleCS = { nullptr };
		

private:


private:
	// �� �������� �ϴ� �߰��ϴ� ������

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