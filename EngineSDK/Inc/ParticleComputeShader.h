#pragma once
#include "ComputeShader.h"
NS_BEGIN(Engine)

class CParticleComputeShader : public CComputeShader
{
public:
	typedef struct tagParticleCSDesc {
		_uint						iNumInstance = {};
		ID3D11Buffer*				pVBInstanceBuffer = { nullptr };
		PARTICLEDESC*				pParticleDesc = { nullptr };
		VTXPOS_PARTICLE_INSTANCE*	pVertexInstances = { nullptr };
	}DESC;

private:
	CParticleComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticleComputeShader(const CParticleComputeShader& Prototype) = delete;
	virtual ~CParticleComputeShader() = default;

public:
	HRESULT Initialize_ParticleComputeShader(const _wstring& wstrFilePath, DESC* pDesc);
	//void Initialize_ParticleDesc(PARTICLEDESC* pParticleDesc);
	HRESULT Update_CBuffer(const PARTICLECBUFFER& tCBuffer);
	HRESULT Dispatch_ParticleCS(const PARTICLECBUFFER& tCBuffer, _uint iGroupX, _uint iGroupY = 1, _uint iGroupZ = 1);
	virtual void Bind() override;
	virtual void Unbind() override;
	void Bind_InstanceSRV();

private:
	// 매번 CS에서 수정 당하실 분 (VIBuffer_PointInstance에서 만든 것을 주소 참조만 함)
	ID3D11Buffer*				m_pVBInstance = { nullptr };
	ID3D11UnorderedAccessView*	m_pVBInstanceUAV = { nullptr };
	ID3D11ShaderResourceView*	m_pVBInstanceSRV = { nullptr };

	// 파티클 개별의 속성들 모음 (속도 등) / 읽기전용
	ID3D11Buffer*				m_pParticleDescBuffer = { nullptr };
	ID3D11ShaderResourceView*	m_pParticleDescSRV = { nullptr };

	// 파티클 인스턴스의 초기 상태 저장 / 읽기전용
	ID3D11Buffer*				m_pInitInstanceBuffer = { nullptr };
	ID3D11ShaderResourceView*	m_pInitInstanceSRV = { nullptr };

	// 파티클 전체의 속성 모음. (시간, 중력, 공전 축 등) / CPU쓰기, GPU읽기전용
	ID3D11Buffer*				m_pCBuffer = { nullptr };
	PARTICLECBUFFER				m_tParticleCBuffer = {};

private:
	_uint						m_iNumInstance = {};

public:
	static CParticleComputeShader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& wstrFilePath, DESC* pDesc);
	virtual void Free() override;

};
NS_END

