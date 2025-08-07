#include "VIBuffer_Trail.h"

CVIBuffer_Trail::CVIBuffer_Trail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_Trail::CVIBuffer_Trail(const CVIBuffer_Trail& Prototype)
	: CVIBuffer( Prototype )
{
}

HRESULT CVIBuffer_Trail::Initialize_Prototype()
{
	m_iNumVertexBuffers = 1;
	m_iNumVertices = m_iMaxNodeCount;
	m_iVertexStride = sizeof(VTXPOS_TRAIL);
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

	D3D11_BUFFER_DESC			VBBufferDesc{};
	VBBufferDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	VBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	VBBufferDesc.StructureByteStride = m_iVertexStride;
	VBBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA		VBInitialData{};

	VTXPOS_TRAIL* pVertices = new VTXPOS_TRAIL[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXPOS_TRAIL) * m_iNumVertices);

	m_pVertexPositions = new _float3[m_iNumVertices];
	ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);

	for (_uint i = 0; i < m_iNumVertices; i++)
	{
		pVertices[i].vPosition = _float3(0.f, 0.f, 0.f);
		m_pVertexPositions[i] = pVertices[i].vPosition;
	}

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CVIBuffer_Trail::Initialize(void* pArg)
{
	return S_OK;
}

void CVIBuffer_Trail::Update_Trail(const _float3& vPosition, _float fTimeDelta)
{

}

HRESULT CVIBuffer_Trail::Update_Buffers()
{
	m_iNumVertices = (_uint)m_TrailNodes.size();
	if (m_iNumVertices == 0)
		return S_OK;

	D3D11_MAPPED_SUBRESOURCE subres;
	if (FAILED(m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres)))
		return E_FAIL;

	VTXPOS_TRAIL* pVertices = static_cast<VTXPOS_TRAIL*>(subres.pData);
	for (_uint i = 0; i < m_iNumVertices; ++i)
	{
		pVertices[i].vPosition = m_TrailNodes[i].vPosition;
		pVertices[i].vPSize = m_TrailNodes[i].vPSize;
		pVertices[i].vLifeTime = m_TrailNodes[i].vLifeTime;

		m_pVertexPositions[i] = m_TrailNodes[i].vPosition; // 디버깅/선택용
	}

	m_pContext->Unmap(m_pVB, 0);
	return S_OK;
}


HRESULT CVIBuffer_Trail::Bind_Buffers()
{
	ID3D11Buffer* pVertexBuffers[] = { m_pVB };
	_uint		iVertexStrides[] = { m_iVertexStride };
	_uint		iOffsets[] = { 0 };

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);

	return S_OK;
}


HRESULT CVIBuffer_Trail::Render()
{
	m_pContext->Draw(m_iNumVertices, 0);

	return S_OK;
}

CVIBuffer_Trail* CVIBuffer_Trail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVIBuffer_Trail* pInstance = new CVIBuffer_Trail(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Trail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_Trail::Clone(void* pArg)
{
	CVIBuffer_Trail* pInstance = new CVIBuffer_Trail(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CVIBuffer_Trail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Trail::Free()
{
	__super::Free();


}
