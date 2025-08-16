#include "ParticleComputeShader.h"

CParticleComputeShader::CParticleComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComputeShader(pDevice, pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CParticleComputeShader::Initialize_ParticleComputeShader(const _wstring& wstrFilePath, ID3D11Buffer* pVBInstanceBuffer, PARTICLEDESC* pParticleDesc, _uint iNumInstance)
{
	if (pVBInstanceBuffer == nullptr) return E_FAIL;
	if (FAILED(CComputeShader::Initialize(wstrFilePath))) return E_FAIL;

	m_iNumInstance = iNumInstance;
	m_pVBInstance = pVBInstanceBuffer;
	Safe_AddRef(m_pVBInstance);

	/* [ InstanceBuffer ] */
	// Instance Buffer UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;  // StructuredBuffer는 반드시 UNKNOWN
	uavDesc.Buffer.NumElements = m_iNumInstance;

	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pVBInstance, &uavDesc, &m_pVBInstanceUAV)))
		return E_FAIL;

	//// Instance Buffer SRV
	//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	//srvDesc.Format = DXGI_FORMAT_UNKNOWN;  // StructuredBuffer는 반드시 UNKNOWN
	//srvDesc.Buffer.FirstElement = 0;
	//srvDesc.Buffer.NumElements = m_iNumInstance;

	//if (FAILED(m_pDevice->CreateShaderResourceView(m_pVBInstance, &srvDesc, &m_pVBInstanceSRV)))
	//	return E_FAIL;
	// 바인딩을 IA에서 해주는 방식 그대로 간다면 SRV를 굳이 만들어서 셰이더로 바인딩 할 필요 없음


	/* [ PARTICLEDESC ] */
	//ParticleDesc Buffer
	D3D11_BUFFER_DESC bufDesc = {};
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.ByteWidth = sizeof(PARTICLEDESC) * m_iNumInstance;
	bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufDesc.CPUAccessFlags = 0;
	bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufDesc.StructureByteStride = sizeof(PARTICLEDESC);

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = pParticleDesc;   // CPU에서 만든 배열 그대로 초기화

	HRESULT hr = m_pDevice->CreateBuffer(&bufDesc, &initData, &m_pParticleDescBuffer);
	if (FAILED(hr))
		return hr;

	// ParticleDesc SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; // StructuredBuffer는 반드시 UNKNOWN
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = m_iNumInstance;

	hr = m_pDevice->CreateShaderResourceView(m_pParticleDescBuffer, &srvDesc, &m_pParticleDescSRV);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

CParticleComputeShader* CParticleComputeShader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& wstrFilePath, ID3D11Buffer* pVBInstanceBuffer, PARTICLEDESC* pParticleDesc, _uint iNumInstance)
{
	CParticleComputeShader* pInstance = new CParticleComputeShader(pDevice, pContext);
	if (FAILED(pInstance->Initialize_ParticleComputeShader(wstrFilePath, pVBInstanceBuffer, pParticleDesc, iNumInstance)))
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
}
