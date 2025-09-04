#pragma once

#include "VIBuffer_Instance.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Point_Instance final : public CVIBuffer_Instance
{
public:
	typedef struct tagPointInstance : public CVIBuffer_Instance::INSTANCE_DESC
	{
		/* --- CBuffer Parameters --- */
		// numinstances
		PARTICLETYPE	ePType;
		_bool			isTool = { false };
		_bool			isLoop;
		_bool			bGravity = { false };
		_bool			bSpin = { false }; // ���� ����
		_bool			bOrbit = { false }; // ���� ����
		_float			fGravity = { 9.8f };
		_float3			vPivot;
		_float3			vCenter;
		_float3			vOrbitAxis = {};			// ������, xyz �� w �ӵ�
		_float3			vRotationAxis = {};       // ������, xyz �� w �ӵ�
		_bool			isTileLoop = { false };
		_float2			vTileCnt = { 1.f,1.f };
		_float			fTileTickPerSec = { 60.f };

		/* --- Particle Parameters --- */
		_float2			vLifeTime;
		_float2			vSpeed;
		_float3			vRange;
		_float2			vSize;
		_float2			vRotationSpeed = {}; // ���� �ӵ�
		_float2			vOrbitSpeed = {}; // ���� �ӵ�

		_float2			vAccel = {};        // ���ӵ� (+�� ����, -�� ����)
		_float			fMaxSpeed = { 1000.f };	// �ʿ�����
		_float			fMinSpeed = { 0.f };

		//_float4			vDirection = {0.f, 1.f, 0.f, 0.f}; // ��������	
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

	void Set_Loop(_bool isLoop) { m_tCBuffer.bIsLoop = isLoop ? 1 : 0; m_tCBuffer.isTileLoop = isLoop ? 1 : 0; }
	void Set_Center(const _float3& vCenter) { m_tCBuffer.vCenter = vCenter; }
	void Set_Center(const _float4x4& matWorld) { XMStoreFloat3(&m_tCBuffer.vCenter, XMVector3TransformCoord(XMLoadFloat3(&m_tCBuffer.vCenter), XMLoadFloat4x4(&matWorld))); }
	void Set_SocketRotation(const _float4& vRot) { m_tCBuffer.vSocketRot = vRot; }
	void Set_CombinedMatrix(const _float4x4& matCombined) { m_bFirst = true; m_tCBuffer.g_CombinedMatrix = matCombined; }
	void Set_First(_bool bFirst) { m_bFirst = bFirst; }
	void Set_InitRotation(_fmatrix matRot);

#ifdef USE_IMGUI
	void Set_CBuffer(const PARTICLECBUFFER& tCBuffer) { m_tCBuffer = tCBuffer; }
	const PARTICLECBUFFER& Get_CBuffer() { return m_tCBuffer; }
#endif

private:
	HRESULT Make_InstanceBuffer(const DESC* pDesc);

private:	
	/* PARTICLECBUFFER ����ü�� ���յ� !! */
	PARTICLECBUFFER				m_tCBuffer = {};

	//VTXPOS_PARTICLE_INSTANCE*	m_pVertexInstances = { nullptr };
	//PARTICLEDESC*				m_pParticleDesc = { nullptr };	
	// + => PPDESC ������
	PPDESC* m_pParticleParamDesc = { nullptr };

private:
	/* [CS] */
	class CParticleComputeShader*	m_pParticleCS = { nullptr };
	_bool							m_bFirst = { true };

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

public:
	static CVIBuffer_Point_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const DESC* pDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END