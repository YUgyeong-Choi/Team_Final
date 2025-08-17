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
	// �Ź� CS���� ���� ���Ͻ� �� (VIBuffer_PointInstance���� ���� ���� �ּ� ������ ��)
	ID3D11Buffer*				m_pVBInstance = { nullptr };
	ID3D11UnorderedAccessView*	m_pVBInstanceUAV = { nullptr };
	ID3D11ShaderResourceView*	m_pVBInstanceSRV = { nullptr };

	// ��ƼŬ ������ �Ӽ��� ���� (�ӵ� ��) / �б�����
	ID3D11Buffer*				m_pParticleDescBuffer = { nullptr };
	ID3D11ShaderResourceView*	m_pParticleDescSRV = { nullptr };

	// ��ƼŬ �ν��Ͻ��� �ʱ� ���� ���� / �б�����
	ID3D11Buffer*				m_pInitInstanceBuffer = { nullptr };
	ID3D11ShaderResourceView*	m_pInitInstanceSRV = { nullptr };

	// ��ƼŬ ��ü�� �Ӽ� ����. (�ð�, �߷�, ���� �� ��) / CPU����, GPU�б�����
	ID3D11Buffer*				m_pCBuffer = { nullptr };
	PARTICLECBUFFER				m_tParticleCBuffer = {};

private:
	_uint						m_iNumInstance = {};

public:
	static CParticleComputeShader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& wstrFilePath, DESC* pDesc);
	virtual void Free() override;

};
NS_END

