#pragma once
#include "ComputeShader.h"
NS_BEGIN(Engine)

class CParticleComputeShader : public CComputeShader
{
public:
	typedef struct tagParticleCSDesc {
		_uint						iNumInstance = {};
		PPDESC* pParticleParamDesc = { nullptr };
	}DESC;

private:
	CParticleComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticleComputeShader(const CParticleComputeShader& Prototype) = delete;
	virtual ~CParticleComputeShader() = default;

public:
	HRESULT Initialize_ParticleComputeShader(const _wstring& wstrFilePath, DESC* pDesc);
	HRESULT Update_CBuffer(const PARTICLECBUFFER& tCBuffer);
	HRESULT Dispatch_ParticleCS(const PARTICLECBUFFER& tCBuffer, _uint iGroupX, _uint iGroupY = 1, _uint iGroupZ = 1);
	virtual void Bind() override;
	virtual void Unbind() override;
	void Bind_InstanceSRV();
	HRESULT Bind_VtxMesh(ID3D11ShaderResourceView* pVtxAnimMesh);

private:
	// ParticleParameter
	// 파티클 개별의 속성들 모음 (속도 등) / 읽기전용
	ID3D11Buffer*				m_pPPBuffer = { nullptr };
	ID3D11ShaderResourceView*	m_pPPSRV = { nullptr };
	ID3D11UnorderedAccessView*	m_pPPUAV = { nullptr };

	// ParticleInit
	// 파티클 인스턴스의 초기 상태 저장 / 읽기전용
	ID3D11Buffer*				m_pPIBuffer = { nullptr };
	ID3D11ShaderResourceView*	m_pPISRV = { nullptr };

	// 파티클 전체의 속성 모음. (시간, 중력, 공전 축 등) / CPU쓰기, GPU읽기전용
	ID3D11Buffer*				m_pCBuffer = { nullptr };
	PARTICLECBUFFER				m_tParticleCBuffer = {};

#pragma region StagingBuffer DEBUG
	// 디버그 용 스테이징 버퍼
	ID3D11Buffer*				m_pStaging = { nullptr };
#pragma endregion
private:
	_uint						m_iNumInstance = {};

public:
	static CParticleComputeShader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& wstrFilePath, DESC* pDesc);
	virtual void Free() override;

};
NS_END

