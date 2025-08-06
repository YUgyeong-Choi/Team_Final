#include "VIBuffer_VolumeMesh.h"

CVIBuffer_VolumeMesh::CVIBuffer_VolumeMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_VolumeMesh::CVIBuffer_VolumeMesh(const CVIBuffer_VolumeMesh& Prototype)
	: CVIBuffer(Prototype)
{
}

HRESULT CVIBuffer_VolumeMesh::Initialize_Prototype()
{

	m_iNumVertexBuffers = 1;
	m_iNumVertices = 8;
	m_iVertexStride = sizeof(VTXPOS);
	m_iNumIndices = 36;
	m_iIndexStride = sizeof(_ushort);
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D11_BUFFER_DESC			VBBufferDesc{};
	VBBufferDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	VBBufferDesc.StructureByteStride = m_iVertexStride;
	VBBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA		VBInitialData{};

	VTXPOS* pVertices = new VTXPOS[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXPOS) * m_iNumVertices);

	m_pVertexPositions = new _float3[m_iNumVertices];
	ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);

	pVertices[0].vPosition = _float3(-0.5f, 0.5f, -0.5f);
	pVertices[1].vPosition = _float3(0.5f, 0.5f, -0.5f);
	pVertices[2].vPosition = _float3(0.5f, -0.5f, -0.5f);
	pVertices[3].vPosition = _float3(-0.5f, -0.5f, -0.5f);
	pVertices[4].vPosition = _float3(-0.5f, 0.5f, 0.5f);
	pVertices[5].vPosition = _float3(0.5f, 0.5f, 0.5f);
	pVertices[6].vPosition = _float3(0.5f, -0.5f, 0.5f);
	pVertices[7].vPosition = _float3(-0.5f, -0.5f, 0.5f);

	for (_uint i = 0; i < m_iNumVertices; ++i)
		m_pVertexPositions[i] = pVertices[i].vPosition;

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	D3D11_BUFFER_DESC			IBBufferDesc{};
	IBBufferDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
	IBBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	IBBufferDesc.StructureByteStride = m_iIndexStride;
	IBBufferDesc.MiscFlags = 0;

	_ushort* pIndices = new _ushort[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);

	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;

	pIndices[3] = 0;
	pIndices[4] = 2;
	pIndices[5] = 3;

	pIndices[6] = 5;
	pIndices[7] = 4;
	pIndices[8] = 7;

	pIndices[9] = 5;
	pIndices[10] = 7;
	pIndices[11] = 6;

	pIndices[12] = 1;
	pIndices[13] = 5;
	pIndices[14] = 6;

	pIndices[15] = 1;
	pIndices[16] = 6;
	pIndices[17] = 2;

	pIndices[18] = 4;
	pIndices[19] = 0;
	pIndices[20] = 3;

	pIndices[21] = 4;
	pIndices[22] = 3;
	pIndices[23] = 7;

	pIndices[24] = 4;
	pIndices[25] = 5;
	pIndices[26] = 1;

	pIndices[27] = 4;
	pIndices[28] = 1;
	pIndices[29] = 0;

	pIndices[30] = 6;
	pIndices[31] = 7;
	pIndices[32] = 3;

	pIndices[33] = 6;
	pIndices[34] = 3;
	pIndices[35] = 2;

	D3D11_SUBRESOURCE_DATA		IBInitialData{};
	IBInitialData.pSysMem = pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBBufferDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

	return S_OK;
}

HRESULT CVIBuffer_VolumeMesh::Initialize(void* pArg)
{
	return S_OK;
}

CVIBuffer_VolumeMesh* CVIBuffer_VolumeMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVIBuffer_VolumeMesh* pInstance = new CVIBuffer_VolumeMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CVIBuffer_VolumeMesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_VolumeMesh::Clone(void* pArg)
{
	CVIBuffer_VolumeMesh* pInstance = new CVIBuffer_VolumeMesh(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CVIBuffer_VolumeMesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_VolumeMesh::Free()
{
	__super::Free();


}
