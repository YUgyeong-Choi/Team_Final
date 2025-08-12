#include "AnimComputeShader.h"
#include <stack>
#include <iostream>
#include <queue>


CAnimComputeShader::CAnimComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComputeShader(pDevice, pContext)
{
}


HRESULT CAnimComputeShader::Initialize_AnimComputeShader(const _wstring& wstrFilePath, _uint iBoneCount)
{

	if (FAILED(CComputeShader::Initialize(wstrFilePath))) return E_FAIL;
	m_iBoneCount = iBoneCount;
	if (m_iBoneCount == 0)
		return E_FAIL;

	// 애니메이션 로컬 행렬 입력 버퍼 및 SRV 생성 (t0)
	D3D11_BUFFER_DESC inputBufferDesc = {};
	inputBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	inputBufferDesc.ByteWidth = sizeof(_float4x4) * m_iBoneCount;
	inputBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inputBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	inputBufferDesc.StructureByteStride = sizeof(_float4x4);
	if (FAILED(m_pDevice->CreateBuffer(&inputBufferDesc, nullptr, &m_pInputBuffer))) 
		return E_FAIL;
	D3D11_SHADER_RESOURCE_VIEW_DESC inputSrvDesc = {};
	inputSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	inputSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	inputSrvDesc.BufferEx.NumElements = m_iBoneCount;
	inputSrvDesc.BufferEx.FirstElement = 0;
	inputSrvDesc.BufferEx.Flags = 0;
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pInputBuffer, &inputSrvDesc, &m_pSourceBoneSRV)))
		return E_FAIL;

	//  최종 행렬 출력 버퍼 (GPU 쓰기, VS 읽기) 및 UAV(u0), SRV(VS용) 생성
	D3D11_BUFFER_DESC outputBufferDesc = {};
	outputBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	outputBufferDesc.ByteWidth = sizeof(_float4x4) * m_iBoneCount;
	outputBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	outputBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	outputBufferDesc.StructureByteStride = sizeof(_float4x4);

	if (FAILED(m_pDevice->CreateBuffer(&outputBufferDesc, nullptr, &m_pOutputBuffer))) 
		return E_FAIL;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.NumElements = m_iBoneCount;
	uavDesc.Buffer.FirstElement = 0;

	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pOutputBuffer, &uavDesc, &m_pOutputBoneUAV))) 
		return E_FAIL;

	D3D11_SHADER_RESOURCE_VIEW_DESC outputSrvDesc = {};
	outputSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	outputSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	outputSrvDesc.BufferEx.NumElements = m_iBoneCount;
	outputSrvDesc.BufferEx.FirstElement = 0;
	outputSrvDesc.BufferEx.Flags = 0;
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pOutputBuffer, &outputSrvDesc, &m_pOutputBoneSRVForVS)))
		return E_FAIL;

	vector<_float4x4> identityList(m_iBoneCount);
	_matrix id = XMMatrixIdentity();
	for (_uint i = 0; i < m_iBoneCount; ++i)
		XMStoreFloat4x4(&identityList[i], id);
	m_pContext->UpdateSubresource(m_pOutputBuffer, 0, nullptr,
		identityList.data(), 0, 0);


	// 상수 버퍼 생성 (b0)
	D3D11_BUFFER_DESC cbd = {};
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(ANIM_CS_DESC);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	if (FAILED(m_pDevice->CreateBuffer(&cbd, nullptr, &m_pCSParamBuffer))) return E_FAIL;

	//  부모 인덱스 버퍼 및 SRV 생성 (t2)
	D3D11_BUFFER_DESC pbDesc = {};
	pbDesc.Usage = D3D11_USAGE_DEFAULT;
	pbDesc.ByteWidth = sizeof(_int) * m_iBoneCount;
	pbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	pbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	pbDesc.StructureByteStride = sizeof(_int);
	if (FAILED(m_pDevice->CreateBuffer(&pbDesc, nullptr, &m_pParentIndexBuffer))) 
		return E_FAIL;

	D3D11_SHADER_RESOURCE_VIEW_DESC psrvDesc = {};
	psrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	psrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	psrvDesc.BufferEx.NumElements = m_iBoneCount;
	psrvDesc.BufferEx.FirstElement = 0;
	psrvDesc.BufferEx.Flags = 0;
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pParentIndexBuffer, &psrvDesc, &m_pParentIndexSRV))) 
		return E_FAIL;

	//  뼈 마스크 버퍼 및 SRV 생성 (t3)


	D3D11_BUFFER_DESC mbDesc = pbDesc;
	mbDesc.ByteWidth = sizeof(_float) * m_iBoneCount;
	mbDesc.StructureByteStride = sizeof(_float);
	if (FAILED(m_pDevice->CreateBuffer(&mbDesc, nullptr, &m_pBoneMaskBuffer)))
		return E_FAIL;
	D3D11_SHADER_RESOURCE_VIEW_DESC msrvDesc = {};
	msrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	msrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	msrvDesc.BufferEx.FirstElement = 0;
	msrvDesc.BufferEx.NumElements = m_iBoneCount;
	msrvDesc.BufferEx.Flags = 0;
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pBoneMaskBuffer, &msrvDesc, &m_pBoneMaskSRV)))
		return E_FAIL;

	D3D11_BUFFER_DESC vsBufDesc = {};
	vsBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vsBufDesc.ByteWidth = sizeof(_float4x4) * m_iBoneCount;
	vsBufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	vsBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	vsBufDesc.StructureByteStride = sizeof(_float4x4);
	if (FAILED(m_pDevice->CreateBuffer(&vsBufDesc, nullptr, &m_pOutputBufferForVS)))
	    return E_FAIL;

	D3D11_SHADER_RESOURCE_VIEW_DESC vsSrvDesc = {};
	vsSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	vsSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	vsSrvDesc.BufferEx.FirstElement = 0;
	vsSrvDesc.BufferEx.NumElements = m_iBoneCount;
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pOutputBufferForVS, &vsSrvDesc, &m_pOutputBoneSRV_VSOnly)))
		return E_FAIL;

	m_pContext->UpdateSubresource(m_pOutputBufferForVS, 0, nullptr, identityList.data(), 0, 0);

	return S_OK;
}

// 로컬 뼈 행렬을 GPU 입력 버퍼로 업로드
void CAnimComputeShader::UploadBoneMatrices(const _float4x4* pBoneMatrices)
{
	if (m_pInputBuffer == nullptr || pBoneMatrices == nullptr)
		return;
	m_pContext->UpdateSubresource(m_pInputBuffer, 0, nullptr, pBoneMatrices, 0, 0);



}
// 계층적 애니메이션 행렬 계산 (레벨 순서대로)
void CAnimComputeShader::ExecuteHierarchical(const _float4x4& preTransform)
{

	_uint iThreadGroupX = (m_iBoneCount + 63) / 64;
	__super::Bind();
	m_pContext->CSSetConstantBuffers(0, 1, &m_pCSParamBuffer);

	// SRV 바인딩: 입력(t0), 부모(t2), 마스크(t3), 레벨(t4)
	ID3D11ShaderResourceView* srvs[5] = {
		m_pSourceBoneSRV,
		nullptr, // t1: 블렌딩 미사용
		m_pParentIndexSRV,
		m_pBoneMaskSRV,
		m_pBoneLevelSRV
	};
	m_pContext->CSSetShaderResources(0, 5, srvs);
	
	m_pContext->CSSetUnorderedAccessViews(0, 1, &m_pOutputBoneUAV, nullptr);

	

	// 각 레벨별로 순차적으로 Dispatch
	for (_uint level = 0; level <= m_iMaxLevel; level++)
	{

		ANIM_CS_DESC desc{};
		desc.blendWeight = 0.0f;
		desc.blendFactor = 1.0f;
		desc.boneCount = m_iBoneCount;
		desc.currentLevel = level;
		desc.preTransformMatrix = preTransform;
		desc.isMasked = 0;
		SetCSParams(desc);

	


		// UAV 배리어
		//UAV 배리어(UAV Barrier)란
		// “UAV(Unordered Access View)에 대한 쓰기 작업이 완료된 뒤에야, 
		// 그 결과를 다음 단계에서 읽을 수 있게 보장”해 주는 메모리 동기화 지점
		__super::Dispatch(iThreadGroupX, 1, 1);
	
	}
	//// 모든 리소스 언바인딩
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	m_pContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
//	m_pContext->Flush();
	ID3D11ShaderResourceView* nullSRVs[5] =
	{ nullptr, nullptr, nullptr, nullptr, nullptr };
	m_pContext->CSSetShaderResources(0, 5, nullSRVs);

	__super::Unbind();
}

// GPU의 계산 결과를 CPU로 다운로드
HRESULT CAnimComputeShader::DownloadBoneMatrices(_float4x4* pOutBoneMatrices, _uint iCount)
{
	  if (!pOutBoneMatrices || iCount == 0)
    {
        cout << "ERROR: Invalid parameters for download" << endl;
        return E_INVALIDARG;
    }
    
    if (!m_pOutputBuffer)
    {
        cout << "ERROR: Output buffer is null" << endl;
        return E_FAIL;
    }
    
    // 스테이징 버퍼 생성
    D3D11_BUFFER_DESC stagingBufferDesc = {};
    m_pOutputBuffer->GetDesc(&stagingBufferDesc);
    
  
    stagingBufferDesc.Usage = D3D11_USAGE_STAGING;
    stagingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingBufferDesc.BindFlags = 0;
    stagingBufferDesc.MiscFlags = 0;
    
    ID3D11Buffer* pStagingBuffer = nullptr;
    HRESULT hr = m_pDevice->CreateBuffer(&stagingBufferDesc, nullptr, &pStagingBuffer);
    if (FAILED(hr))
    {
        cout << "ERROR: Failed to create staging buffer, HR: " << hex << hr << endl;
        return hr;
    }
    
    // 데이터 복사
    m_pContext->CopyResource(pStagingBuffer, m_pOutputBuffer);
    
    // 강제 완료 대기
    m_pContext->Flush();
    
    // 매핑
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = m_pContext->Map(pStagingBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
    if (FAILED(hr))
    {
        cout << "ERROR: Failed to map staging buffer, HR: " << hex << hr << endl;
        Safe_Release(pStagingBuffer);
        return hr;
    }
    
    // 데이터 복사
    _uint copySize = min(iCount, m_iBoneCount) * sizeof(_float4x4);
    memcpy(pOutBoneMatrices, mappedResource.pData, copySize);
    
    m_pContext->Unmap(pStagingBuffer, 0);
    Safe_Release(pStagingBuffer);
    
    cout << "Successfully downloaded " << min(iCount, m_iBoneCount) << " matrices" << endl;
    return S_OK;
}

// 뼈의 계층 레벨 계산 및 GPU 버퍼 생성
void CAnimComputeShader::BuildHierarchyLevels()
{

	m_BoneLevels.resize(m_iBoneCount, -1);
	m_iMaxLevel = 0;
	queue<_int> q;
	for (_uint i = 0; i < m_iBoneCount; ++i) {
		if (m_ParentIndices[i] < 0) {
			m_BoneLevels[i] = 0;
			q.push(i);
		}
	}


	while (!q.empty()) 
	{
		int bone = q.front(); q.pop();
		int level = m_BoneLevels[bone];
		m_iMaxLevel = max(m_iMaxLevel, level);

		// 자식 찾기: m_ParentIndices에서 bone이 부모인 인덱스들
		for (_uint i = 0; i < m_iBoneCount; ++i)
		{
			if (m_ParentIndices[i] == bone) 
			{
				m_BoneLevels[i] = level + 1;
				q.push(i);
			}
		}
	}
	// 계산된 레벨 정보를 GPU 버퍼로 업로드
	D3D11_BUFFER_DESC levelDesc{};
	levelDesc.Usage = D3D11_USAGE_DEFAULT;
	levelDesc.ByteWidth = sizeof(_int) * m_iBoneCount;
	levelDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	levelDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	levelDesc.StructureByteStride = sizeof(_int);
	D3D11_SUBRESOURCE_DATA levelData{};
	levelData.pSysMem = m_BoneLevels.data();
	if (FAILED(m_pDevice->CreateBuffer(&levelDesc, &levelData, &m_pBoneLevelBuffer)))
		return;
	D3D11_SHADER_RESOURCE_VIEW_DESC levelSrvDesc{};
	levelSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	levelSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	levelSrvDesc.BufferEx.NumElements = m_iBoneCount;
	levelSrvDesc.BufferEx.FirstElement = 0;
	levelSrvDesc.BufferEx.Flags = 0;
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pBoneLevelBuffer, &levelSrvDesc, &m_pBoneLevelSRV)))
		return;
}

// 컴퓨트 셰이더 상수 버퍼 데이터 업데이트
void CAnimComputeShader::SetCSParams(const ANIM_CS_DESC& desc)
{
	m_pContext->UpdateSubresource(m_pCSParamBuffer, 0, nullptr, &desc, 0, 0);
}

// 부모 인덱스 배열을 GPU 버퍼로 업로드
void CAnimComputeShader::SetParentIndices(const vector<_int>& parents)
{
	m_ParentIndices = parents;
	m_pContext->UpdateSubresource(m_pParentIndexBuffer, 0, nullptr, parents.data(), 0, 0);
}

// 뼈 마스크 배열을 GPU 버퍼로 업로드
void CAnimComputeShader::SetBoneMask(const vector<_float>& mask)
{
	m_pContext->UpdateSubresource(m_pBoneMaskBuffer, 0, nullptr, mask.data(), 0, 0);
}

CAnimComputeShader* CAnimComputeShader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& wstrFilePath, _uint iBoneCount)
{
	CAnimComputeShader* pInstance = new CAnimComputeShader(pDevice, pContext);
	if (FAILED(pInstance->Initialize_AnimComputeShader(wstrFilePath, iBoneCount)))
	{
		MSG_BOX("Failed to Created : CAnimComputeShader");
		Safe_Release(pInstance);
		return nullptr;
	}
	return pInstance;
}

void CAnimComputeShader::Free()
{
	__super::Free();
	Safe_Release(m_pCSParamBuffer);
	Safe_Release(m_pParentIndexBuffer);
	Safe_Release(m_pInputBuffer);
	Safe_Release(m_pOutputBuffer);
	Safe_Release(m_pBoneMaskBuffer);
	Safe_Release(m_pParentIndexSRV);
	Safe_Release(m_pBoneMaskSRV);
	Safe_Release(m_pSourceBoneSRV);
	Safe_Release(m_pTargetBoneSRV);
	Safe_Release(m_pOutputBoneSRVForVS);
	Safe_Release(m_pOutputBoneUAV);
	Safe_Release(m_pBoneLevelBuffer);
	Safe_Release(m_pBoneLevelSRV);
	Safe_Release(m_pOutputBufferForVS);
	Safe_Release(m_pOutputBoneSRV_VSOnly);
	m_iBoneCount = 0;
}