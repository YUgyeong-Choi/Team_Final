#include "ParticleComputeShader.h"

CParticleComputeShader::CParticleComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComputeShader(pDevice, pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CParticleComputeShader::Initialize_ParticleComputeShader(const _wstring& wstrFilePath, DESC* pDesc)
{
	if (pDesc->pVBInstanceBuffer == nullptr) return E_FAIL;
	if (FAILED(CComputeShader::Initialize(wstrFilePath))) return E_FAIL;

	m_iNumInstance = pDesc->iNumInstance;
	m_pVBInstance = pDesc->pVBInstanceBuffer;
	Safe_AddRef(m_pVBInstance);

	/* [ InstanceBuffer ] */
	// Instance Buffer UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;  // StructuredBuffer는 반드시 UNKNOWN
	uavDesc.Buffer.NumElements = m_iNumInstance;

	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pVBInstance, &uavDesc, &m_pVBInstanceUAV)))
		return E_FAIL;

	// Instance Buffer SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;  // StructuredBuffer는 반드시 UNKNOWN
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = m_iNumInstance;

	if (FAILED(m_pDevice->CreateShaderResourceView(m_pVBInstance, &srvDesc, &m_pVBInstanceSRV)))
		return E_FAIL;
	//바인딩을 IA에서 해주는 방식 그대로 간다면 SRV를 굳이 만들어서 셰이더로 바인딩 할 필요 없음
	//근데 이제 IA에서 안넣을거임..

	/* [ PARTICLEDESC ] */
	// ParticleDesc Buffer
	D3D11_BUFFER_DESC PDbufDesc = {};
	PDbufDesc.Usage = D3D11_USAGE_DEFAULT;
	PDbufDesc.ByteWidth = sizeof(PARTICLEDESC) * m_iNumInstance;
	PDbufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	PDbufDesc.CPUAccessFlags = 0;
	PDbufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	PDbufDesc.StructureByteStride = sizeof(PARTICLEDESC);

	D3D11_SUBRESOURCE_DATA PDinitData = {};
	PDinitData.pSysMem = pDesc->pParticleDesc;   // CPU에서 만든 배열 그대로 초기화

	HRESULT hr = m_pDevice->CreateBuffer(&PDbufDesc, &PDinitData, &m_pParticleDescBuffer);
	if (FAILED(hr))
		return hr;

	// ParticleDesc SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC PDsrvDesc = {};
	PDsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	PDsrvDesc.Format = DXGI_FORMAT_UNKNOWN; // StructuredBuffer는 반드시 UNKNOWN
	PDsrvDesc.Buffer.FirstElement = 0;
	PDsrvDesc.Buffer.NumElements = m_iNumInstance;

	hr = m_pDevice->CreateShaderResourceView(m_pParticleDescBuffer, &PDsrvDesc, &m_pParticleDescSRV);
	if (FAILED(hr))
		return hr;

	/* [ ParticleInitData ] */
	// InitInstance Buffer
	D3D11_BUFFER_DESC PIbufDesc{};
	PIbufDesc.Usage = D3D11_USAGE_DEFAULT;
	PIbufDesc.ByteWidth = sizeof(VTXPOS_PARTICLE_INSTANCE) * m_iNumInstance;
	PIbufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // 읽기 전용
	PIbufDesc.CPUAccessFlags = 0;
	PIbufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	PIbufDesc.StructureByteStride = sizeof(VTXPOS_PARTICLE_INSTANCE);

	D3D11_SUBRESOURCE_DATA PIinitData{};
	PIinitData.pSysMem = pDesc->pVertexInstances;

	hr = m_pDevice->CreateBuffer(&PIbufDesc, &PIinitData, &m_pInitInstanceBuffer);
	if (FAILED(hr))
		return hr;

	// InitInstance SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC PIsrvDesc{};
	PIsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	PIsrvDesc.Format = DXGI_FORMAT_UNKNOWN; // StructuredBuffer는 UNKNOWN
	PIsrvDesc.Buffer.FirstElement = 0;
	PIsrvDesc.Buffer.NumElements = m_iNumInstance;

	hr = m_pDevice->CreateShaderResourceView(m_pInitInstanceBuffer, &PIsrvDesc, &m_pInitInstanceSRV);
	if (FAILED(hr))
		return hr;

	/* [ Constant Buffer ] */
	D3D11_BUFFER_DESC cbDesc{};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;            // 매 프레임 UpdateSubresource
	cbDesc.ByteWidth = sizeof(PARTICLECBUFFER);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;

	hr = m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_pCBuffer);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT CParticleComputeShader::Update_CBuffer(const PARTICLECBUFFER& tCBuffer)
{
	m_tParticleCBuffer = tCBuffer; // 안되나요,

	return S_OK;
}

HRESULT CParticleComputeShader::Dispatch_ParticleCS(const PARTICLECBUFFER& tCBuffer, _uint iGroupX, _uint iGroupY, _uint iGroupZ)
{
	//Update_CBuffer(tCBuffer);
	m_tParticleCBuffer = tCBuffer;
	Bind();

	const _uint iThreadX = 128;
	UINT groups = (m_iNumInstance + iThreadX - 1) / iThreadX; // 올림 나눗셈

	Dispatch(groups, 1, 1);

	Unbind();

	return S_OK;
}


void CParticleComputeShader::Bind()
{
	__super::Bind();
	ID3D11UnorderedAccessView* uavs[] = { m_pVBInstanceUAV };
	UINT initialCounts[] = { 0 };
	m_pContext->CSSetUnorderedAccessViews(0, 1, uavs, initialCounts);
	// u0에 바인딩

	ID3D11ShaderResourceView* srvs[] = { m_pParticleDescSRV, m_pInitInstanceSRV };
	m_pContext->CSSetShaderResources(0, _countof(srvs), srvs);
	// → t0, t1
	// t0이 파티클 개별 속성
	// t1이 초기 상태

	m_pContext->UpdateSubresource(m_pCBuffer, 0, nullptr, &m_tParticleCBuffer, 0, 0);
	m_pContext->CSSetConstantBuffers(0, 1, &m_pCBuffer);
	// b0에 바인딩

}

void CParticleComputeShader::Unbind()
{
	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
	ID3D11Buffer* nullCB[1] = { nullptr };

	m_pContext->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
	m_pContext->CSSetShaderResources(0, 2, nullSRV);
	m_pContext->CSSetConstantBuffers(0, 1, nullCB);
	m_pContext->CSSetShader(nullptr, nullptr, 0);

	__super::Unbind();
}

void CParticleComputeShader::Bind_InstanceSRV()
{
	ID3D11ShaderResourceView* srvs[] = { m_pVBInstanceSRV };
	m_pContext->VSSetShaderResources(0, _countof(srvs), srvs);
}

CParticleComputeShader* CParticleComputeShader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& wstrFilePath, DESC* pDesc)
{
	CParticleComputeShader* pInstance = new CParticleComputeShader(pDevice, pContext);
	if (FAILED(pInstance->Initialize_ParticleComputeShader(wstrFilePath, pDesc)))
	{
		MSG_BOX("Failed to Created : CParticleComputeShader");
		Safe_Release(pInstance);
		return nullptr;
	}
	return pInstance;
}

void CParticleComputeShader::Free()
{
	__super::Free();
	Safe_Release(m_pVBInstance);
	Safe_Release(m_pVBInstanceUAV);
	Safe_Release(m_pVBInstanceSRV);
	Safe_Release(m_pParticleDescBuffer);
	Safe_Release(m_pParticleDescSRV);
	Safe_Release(m_pCBuffer);

}
