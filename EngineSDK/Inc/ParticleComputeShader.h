#pragma once
#include "ComputeShader.h"
NS_BEGIN(Engine)

class CParticleComputeShader : public CComputeShader
{
private:
	// CS 구조체 선언
	typedef struct tagParticleCBuffer {
		_float DeltaTime;       // dt (tool이면 무시)
		_float TrackTime;       // tool 절대시간(초) = curTrackPos/60.f
		_uint  ParticleType;    // 0:SPREAD, 1:DIRECTIONAL
		_uint  IsTool;			// uint == bool

		_uint  IsLoop;
		_uint  UseGravity;
		_uint  UseSpin;
		_uint  UseOrbit;

		_float  Gravity;        // e.g. 9.8
		_float3 Pivot;          // vPivot

		_float3 Center;         // vCenter
		_float  _pad0;

		_float3 OrbitAxis;      // normalized
		_float  _pad1;

		_float3 RotationAxis;   // normalized
		_float  _pad2;
	}PARTICLECBUFFER;

private:
	CParticleComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticleComputeShader(const CParticleComputeShader& Prototype) = delete;
	virtual ~CParticleComputeShader() = default;

public:
	HRESULT Initialize_ParticleComputeShader(const _wstring& wstrFilePath, ID3D11Buffer* pVBInstanceBuffer, PARTICLEDESC* pParticleDesc, _uint iNumInstance);
	//void Initialize_ParticleDesc(PARTICLEDESC* pParticleDesc);
	HRESULT Update_CBuffer(const PARTICLECBUFFER& tCBuffer);
	//HRESULT 

private:
	ID3D11Buffer*				m_pVBInstance = { nullptr };
	ID3D11Buffer*				m_pParticleDescBuffer = { nullptr };

	ID3D11UnorderedAccessView*	m_pVBInstanceUAV = { nullptr };
	ID3D11ShaderResourceView*	m_pVBInstanceSRV = { nullptr };

	ID3D11ShaderResourceView*	m_pParticleDescSRV = { nullptr };

private:
	_uint						m_iNumInstance = {};

public:
	static CParticleComputeShader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& wstrFilePath, ID3D11Buffer* pVBInstanceBuffer, PARTICLEDESC* pParticleDesc, _uint iNumInstance);
	virtual void Free() override;

};
NS_END

