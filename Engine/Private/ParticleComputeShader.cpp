#include "ParticleComputeShader.h"

CParticleComputeShader::CParticleComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComputeShader(pDevice, pContext)
{
}

HRESULT CParticleComputeShader::Initialize_ParticleComputeShader(const _wstring& wstrFilePath, DESC* pDesc)
{
	if (FAILED(CComputeShader::Initialize(wstrFilePath))) return E_FAIL;

	m_iNumInstance = pDesc->iNumInstance;

	D3D11_BUFFER_DESC PPBufferDesc = {};
	PPBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	PPBufferDesc.ByteWidth = m_iNumInstance * sizeof(PPDESC);
	PPBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	PPBufferDesc.CPUAccessFlags = 0;
	PPBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	PPBufferDesc.StructureByteStride = sizeof(PPDESC);

	D3D11_SUBRESOURCE_DATA PPData = {};
	PPData.pSysMem = pDesc->pParticleParamDesc;

	HRESULT hr = m_pDevice->CreateBuffer(&PPBufferDesc, &PPData, &m_pPPBuffer);
	if (FAILED(hr))
		return hr;

	// ParticleDesc SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC PPsrvDesc = {};
	PPsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	PPsrvDesc.Format = DXGI_FORMAT_UNKNOWN; // StructuredBuffer는 반드시 UNKNOWN
	PPsrvDesc.Buffer.NumElements = m_iNumInstance;

	hr = m_pDevice->CreateShaderResourceView(m_pPPBuffer, &PPsrvDesc, &m_pPPSRV);
	if (FAILED(hr))
		return hr;

	D3D11_UNORDERED_ACCESS_VIEW_DESC PPuavDesc = {};
	PPuavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	PPuavDesc.Format = DXGI_FORMAT_UNKNOWN;  // StructuredBuffer는 반드시 UNKNOWN
	PPuavDesc.Buffer.NumElements = m_iNumInstance;

	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pPPBuffer, &PPuavDesc, &m_pPPUAV)))
		return E_FAIL;

	/* [ ParticleInitData ] */
	// InitInstance Buffer
	D3D11_BUFFER_DESC PIbufDesc{};
	PIbufDesc.Usage = D3D11_USAGE_DEFAULT;
	PIbufDesc.ByteWidth = sizeof(PPDESC) * m_iNumInstance;
	PIbufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // 읽기 전용
	PIbufDesc.CPUAccessFlags = 0;
	PIbufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	PIbufDesc.StructureByteStride = sizeof(PPDESC);

	D3D11_SUBRESOURCE_DATA PIinitData{};
	PIinitData.pSysMem = pDesc->pParticleParamDesc;

	hr = m_pDevice->CreateBuffer(&PIbufDesc, &PIinitData, &m_pPIBuffer);
	if (FAILED(hr))
		return hr;

	// InitInstance SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC PIsrvDesc{};
	PIsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	PIsrvDesc.Format = DXGI_FORMAT_UNKNOWN; // StructuredBuffer는 UNKNOWN
	//PIsrvDesc.Buffer.FirstElement = 0;
	PIsrvDesc.Buffer.NumElements = m_iNumInstance;

	hr = m_pDevice->CreateShaderResourceView(m_pPIBuffer, &PIsrvDesc, &m_pPISRV);
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

#pragma region StagingBuffer DEBUG
	/****** [ 디버그 용 staging buffer 입니다 ] ******/
	//D3D11_BUFFER_DESC desc = {};
	//desc.Usage = D3D11_USAGE_STAGING;
	//desc.ByteWidth = m_iNumInstance * sizeof(PPDESC);
	//desc.BindFlags = 0; // 반드시 0
	//desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	//desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	//desc.StructureByteStride = sizeof(PPDESC);

	//m_pDevice->CreateBuffer(&desc, nullptr, &m_pStaging);
#pragma endregion 

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
	_uint groups = (m_tParticleCBuffer.iNumInstances + iThreadX - 1) / iThreadX; // 올림 나눗셈

	Dispatch(groups, 1, 1);

	Unbind();

	return S_OK;
}


void CParticleComputeShader::Bind()
{
	__super::Bind();

	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	m_pContext->VSSetShaderResources(0, 1, nullSRV);

	m_pContext->UpdateSubresource(m_pCBuffer, 0, nullptr, &m_tParticleCBuffer, 0, 0);

	ID3D11UnorderedAccessView* uavs[] = { m_pPPUAV };
	UINT initialCounts[] = { 0 };
	m_pContext->CSSetUnorderedAccessViews(0, 1, uavs, initialCounts);
	// u0에 바인딩

	ID3D11ShaderResourceView* srvs[] = { m_pPISRV };
	m_pContext->CSSetShaderResources(0, 1, srvs);
	// → t0, t1
	// t0이 파티클 개별 속성
	// t1이 초기 상태

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

	__super::Unbind();
}

void CParticleComputeShader::Bind_InstanceSRV()
{
#pragma region StagingBuffer DEBUG
	//m_pContext->CopyResource(m_pStaging, m_pPPBuffer);

	//D3D11_MAPPED_SUBRESOURCE mapped{};
	//m_pContext->Map(m_pStaging, 0, D3D11_MAP_READ, 0, &mapped);

	//PPDESC* pData = reinterpret_cast<PPDESC*>(mapped.pData);


	//m_pContext->Unmap(m_pStaging, 0);
#pragma endregion

	ID3D11ShaderResourceView* srvs[] = { m_pPPSRV };
	m_pContext->VSSetShaderResources(0, 1, srvs);
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
	Safe_Release(m_pPPBuffer);
	Safe_Release(m_pPPSRV);
	Safe_Release(m_pPPUAV);
	Safe_Release(m_pPIBuffer);
	Safe_Release(m_pPISRV);
	Safe_Release(m_pCBuffer);

}
