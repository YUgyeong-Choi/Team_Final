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
	// ��ƼŬ ������ �Ӽ��� ���� (�ӵ� ��) / �б�����
	ID3D11Buffer*				m_pPPBuffer = { nullptr };
	ID3D11ShaderResourceView*	m_pPPSRV = { nullptr };
	ID3D11UnorderedAccessView*	m_pPPUAV = { nullptr };

	// ParticleInit
	// ��ƼŬ �ν��Ͻ��� �ʱ� ���� ���� / �б�����
	ID3D11Buffer*				m_pPIBuffer = { nullptr };
	ID3D11ShaderResourceView*	m_pPISRV = { nullptr };

	// ��ƼŬ ��ü�� �Ӽ� ����. (�ð�, �߷�, ���� �� ��) / CPU����, GPU�б�����
	ID3D11Buffer*				m_pCBuffer = { nullptr };
	PARTICLECBUFFER				m_tParticleCBuffer = {};

#pragma region StagingBuffer DEBUG
	// ����� �� ������¡ ����
	ID3D11Buffer*				m_pStaging = { nullptr };
#pragma endregion
private:
	_uint						m_iNumInstance = {};

public:
	static CParticleComputeShader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& wstrFilePath, DESC* pDesc);
	virtual void Free() override;

};
NS_END

