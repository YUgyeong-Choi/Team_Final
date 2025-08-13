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
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

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

	// ���� ���� �� �ʿ䰡 ���� �� ����
	//m_pVertexPositions = new _float3[m_iNumVertices];
	//ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);

	//for (_uint i = 0; i < m_iNumVertices; i++)
	//{
	//	pVertices[i].vInnerPos = _float3(0.f, 0.f, 0.f);
	//	pVertices[i].vOuterPos = _float3(0.f, 0.f, 0.f);
	//	pVertices[i].vLifeTime = _float2(0.f, 0.f);
	//	//m_pVertexPositions[i] = pVertices[i].vInnerPos;
	//}

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CVIBuffer_Trail::Initialize(void* pArg)
{
	// ���⼭ ���� ���ؾ��ϳ�?
	return S_OK;
}

void CVIBuffer_Trail::Update_Trail(const _float3& vInnerPos, const _float3& vOuterPos, _float fTimeDelta)
{
	// 1. �ð� ������Ʈ
	for (auto& node : m_TrailNodes)
		node.vLifeTime.y += fTimeDelta;

	// 2. ������ ��� ����
	m_TrailNodes.erase( // ���� �� ���Ҹ� ��� ��Ƽ� �ڷ� ���� �� ������ �����ϴ� algorithm �Լ�
		remove_if(m_TrailNodes.begin(), m_TrailNodes.end(),
			[](const VTXPOS_TRAIL& node) { return node.vLifeTime.y >= node.vLifeTime.x; }),
		m_TrailNodes.end()
	);

	m_fNodeAccTime += fTimeDelta;
	// 3. Ʈ���� Ȱ�� ������ ��쿡�� ��� �߰�
	if (m_bTrailActive && m_fNodeAccTime >= m_fNodeInterval)
	{
//		if (m_TrailNodes.empty())
//		{
//			VTXPOS_TRAIL firstNode;
//			firstNode.vInnerPos = vInnerPos;
//			firstNode.vOuterPos = vOuterPos;
//			firstNode.vLifeTime = _float2(m_fLifeDuration, 0.f);
//			m_TrailNodes.push_back(firstNode);
//		}
//		else
//		{
			VTXPOS_TRAIL newNode;
			newNode.vInnerPos = vInnerPos;
			newNode.vOuterPos = vOuterPos;
			newNode.vLifeTime = _float2(m_fLifeDuration, 0.f);
			m_TrailNodes.push_back(newNode);
//		}

		m_fNodeAccTime = 0.f;
	}
	
	Update_Buffers();
}

HRESULT CVIBuffer_Trail::Update_Buffers()
{
	m_iNumVertices = (_uint)m_TrailNodes.size();
	if (m_iNumVertices == 0)
		return S_OK;

	if (FAILED(Interpolate_TrailNodes()))
		return E_FAIL;


	//D3D11_MAPPED_SUBRESOURCE subres;
	//if (FAILED(m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres)))
	//	return E_FAIL;

	//VTXPOS_TRAIL* pVertices = static_cast<VTXPOS_TRAIL*>(subres.pData);
	//for (_uint i = 0; i < m_iNumVertices; ++i)
	//{
	//	pVertices[i].vInnerPos= m_TrailNodes[i].vInnerPos;
	//	pVertices[i].vOuterPos= m_TrailNodes[i].vOuterPos;
	//	pVertices[i].vLifeTime= m_TrailNodes[i].vLifeTime;
	//}

	//m_pContext->Unmap(m_pVB, 0);


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
	if (m_TrailNodes.size() < 2)
		return S_OK;

	m_pContext->Draw(m_iNumVertices, 0);

	return S_OK;
}

HRESULT CVIBuffer_Trail::Interpolate_TrailNodes()
{
	// ���� ��尡 �ʹ� ������ �׸� �ʿ� ����
	if (m_TrailNodes.size() < 4)
	{
		m_iNumVertices = 0;
		return S_OK;
	}

	// ���� �� ���� ���� �� ���
	_uint smoothCount = (_uint)(m_TrailNodes.size() - 3) * (m_Subdivisions + 1);

	// 1) ���� ũ�� ���� �� �����
	if (smoothCount > m_iMaxNodeCount)
	{
		m_iMaxNodeCount = (_uint)smoothCount;

		Safe_Release(m_pVB);

		D3D11_BUFFER_DESC vbDesc = {};
		vbDesc.ByteWidth = (_uint)(m_iMaxNodeCount * sizeof(VTXPOS_TRAIL));
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbDesc.Usage = D3D11_USAGE_DYNAMIC;
		vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vbDesc.StructureByteStride = sizeof(VTXPOS_TRAIL);
		
		if (FAILED(m_pDevice->CreateBuffer(&vbDesc, nullptr, &m_pVB)))
			return E_FAIL;
	}

	// 2) ������ ��� ����
	vector<VTXPOS_TRAIL> smoothNodes;
	smoothNodes.reserve(smoothCount);

	for (size_t i = 0; i + 3 < m_TrailNodes.size(); ++i)
	{
		const auto& P0 = m_TrailNodes[i + 0];
		const auto& P1 = m_TrailNodes[i + 1];
		const auto& P2 = m_TrailNodes[i + 2];
		const auto& P3 = m_TrailNodes[i + 3];

		for (_int s = 0; s <= m_Subdivisions; ++s)
		{
			 _float t = (_float)s / (_float)m_Subdivisions;

			// Inner pos ����
			XMVECTOR i0 = XMLoadFloat3(&P0.vInnerPos);
			XMVECTOR i1 = XMLoadFloat3(&P1.vInnerPos);
			XMVECTOR i2 = XMLoadFloat3(&P2.vInnerPos);
			XMVECTOR i3 = XMLoadFloat3(&P3.vInnerPos);
			XMFLOAT3 inner;
			XMStoreFloat3(&inner, XMVectorCatmullRom(i0, i1, i2, i3, t));

			// Outer pos ����
			XMVECTOR o0 = XMLoadFloat3(&P0.vOuterPos);
			XMVECTOR o1 = XMLoadFloat3(&P1.vOuterPos);
			XMVECTOR o2 = XMLoadFloat3(&P2.vOuterPos);
			XMVECTOR o3 = XMLoadFloat3(&P3.vOuterPos);
			XMFLOAT3 outer;
			XMStoreFloat3(&outer, XMVectorCatmullRom(o0, o1, o2, o3, t));

			// LifeTime (������ P1 ����)
			VTXPOS_TRAIL node;
			node.vInnerPos = inner;
			node.vOuterPos = outer;
			node.vLifeTime = P1.vLifeTime;

			smoothNodes.push_back(node);
		}
	}

	m_iNumVertices = (_uint)smoothNodes.size();

	// 3) GPU ���ۿ� ����
	D3D11_MAPPED_SUBRESOURCE subres;
	if (FAILED(m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres)))
		return E_FAIL;
	VTXPOS_TRAIL* pVertices = static_cast<VTXPOS_TRAIL*>(subres.pData);
	memcpy(subres.pData, smoothNodes.data(), sizeof(VTXPOS_TRAIL) * m_iNumVertices);

	m_pContext->Unmap(m_pVB, 0);

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
