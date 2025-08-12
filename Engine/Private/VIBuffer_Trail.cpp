#include "VIBuffer_Trail.h"

CVIBuffer_Trail::CVIBuffer_Trail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_Trail::CVIBuffer_Trail(const CVIBuffer_Trail& Prototype)
	: CVIBuffer( Prototype )
	, m_iMaxNodeCount{ Prototype.m_iMaxNodeCount }
	, m_fLifeDuration{ Prototype.m_fLifeDuration }
	, m_bTrailActive{ Prototype.m_bTrailActive }
	, m_TrailNodes{ Prototype.m_TrailNodes }
{
}

HRESULT CVIBuffer_Trail::Initialize_Prototype()
{
	m_iNumVertexBuffers = 1;
	m_iNumVertices = m_iMaxNodeCount;
	m_iVertexStride = sizeof(VTXPOS_TRAIL);
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

	D3D11_BUFFER_DESC			VBBufferDesc{};
	VBBufferDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	VBBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VBBufferDesc.StructureByteStride = m_iVertexStride;
	VBBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA		VBInitialData{};

	VTXPOS_TRAIL* pVertices = new VTXPOS_TRAIL[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXPOS_TRAIL) * m_iNumVertices);

	// 굳이 저장 할 필요가 없을 것 같음
	//m_pVertexPositions = new _float3[m_iNumVertices];
	//ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);

	for (_uint i = 0; i < m_iNumVertices; i++)
	{
		pVertices[i].vInnerPos = _float3(0.f, 0.f, 0.f);
		pVertices[i].vOuterPos = _float3(0.f, 0.f, 0.f);
		pVertices[i].vLifeTime = _float2(0.f, 0.f);
		//m_pVertexPositions[i] = pVertices[i].vInnerPos;
	}

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CVIBuffer_Trail::Initialize(void* pArg)
{
	// 여기서 종류 정해야하나?
	return S_OK;
}

void CVIBuffer_Trail::Update_Trail(const _float3& vInnerPos, const _float3& vOuterPos, _float fTimeDelta)
{
	// 1. 시간 업데이트
	for (auto& node : m_TrailNodes)
		node.vLifeTime.y += fTimeDelta;

	// 2. 오래된 노드 제거
	m_TrailNodes.erase( // 제거 할 원소를 모두 모아서 뒤로 보낸 후 범위로 지정하는 algorithm 함수
		remove_if(m_TrailNodes.begin(), m_TrailNodes.end(),
			[](const VTXPOS_TRAIL& node) { return node.vLifeTime.y >= node.vLifeTime.x; }),
		m_TrailNodes.end()
	);

	// 3. 트레일 활성 상태일 경우에만 노드 추가
	if (m_bTrailActive)
	{
		VTXPOS_TRAIL newNode;
		newNode.vInnerPos = vInnerPos;
		newNode.vOuterPos = vOuterPos;
		newNode.vLifeTime = _float2(m_fLifeDuration, 0.f);
		m_TrailNodes.push_back(newNode);
	}

	Update_Buffers();
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
		pVertices[i].vInnerPos= m_TrailNodes[i].vInnerPos;
		pVertices[i].vOuterPos= m_TrailNodes[i].vOuterPos;
		pVertices[i].vLifeTime= m_TrailNodes[i].vLifeTime;
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
