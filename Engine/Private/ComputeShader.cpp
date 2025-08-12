#include "ComputeShader.h"

unordered_map<_wstring, ID3D11ComputeShader*> CComputeShader::m_ComputeShaderCache;
CComputeShader::CComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CComputeShader::Initialize(const _wstring& wstrFilePath)
{
	//ID3DBlob* blob{ nullptr };
	//if (FAILED(D3DReadFileToBlob(wstrFilePath.c_str(), &blob)))
	//	return E_FAIL;
	//if (FAILED(m_pDevice->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_pComputeShader)))
	//{
	//	MSG_BOX("Failed to create compute shader");
	//	return E_FAIL;
	//}

	auto it = m_ComputeShaderCache.find(wstrFilePath);
	if (it != m_ComputeShaderCache.end())
	{
		m_pComputeShader = it->second;
		Safe_AddRef(m_pComputeShader); // 인스턴스가 한 번 더 참조
		return S_OK;
	}

	//  없으면 로드 + CreateComputeShader
	ID3DBlob* blob = nullptr;
	HRESULT hr = D3DReadFileToBlob(wstrFilePath.c_str(), &blob);
	if (FAILED(hr)) return hr;

	ID3D11ComputeShader* pCS = nullptr;
	hr = m_pDevice->CreateComputeShader(
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		nullptr,
		&pCS);
	blob->Release();
	if (FAILED(hr)) return hr;

	//캐시에 넣고, 인스턴스용으로 AddRef
	m_ComputeShaderCache[wstrFilePath] = pCS;
	m_pComputeShader = pCS;
	Safe_AddRef(m_pComputeShader); // 캐시+인스턴스 = refcount 2

	return S_OK;
}

void CComputeShader::Bind()
{
	if (m_pComputeShader)
	{
		m_pContext->CSSetShader(m_pComputeShader, nullptr, 0);
	}
	else
	{
		MSG_BOX("Compute shader is not initialized.");
	}
}

void CComputeShader::Unbind()
{
	if (m_pComputeShader)
	{
		m_pContext->CSSetShader(nullptr, nullptr, 0);
	}
	else
	{
		MSG_BOX("Compute shader is not initialized.");
	}
}

void CComputeShader::ReleaseCache()
{
	for (auto& [path, pCS] : m_ComputeShaderCache)
		Safe_Release(pCS);
	m_ComputeShaderCache.clear();
}

CComputeShader* CComputeShader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& wstrFilePath)
{
	CComputeShader* pInstance = new CComputeShader(pDevice, pContext);
	if (FAILED(pInstance->Initialize(wstrFilePath)))
	{
		MSG_BOX("Failed to Created : CComputeShader");
		Safe_Release(pInstance);
		return nullptr;
	}
	return pInstance;
}

void CComputeShader::Free()
{
	Safe_Release(m_pComputeShader);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	__super::Free();
}
