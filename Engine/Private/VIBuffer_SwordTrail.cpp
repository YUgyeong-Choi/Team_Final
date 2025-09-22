#include "VIBuffer_SwordTrail.h"

CVIBuffer_SwordTrail::CVIBuffer_SwordTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_SwordTrail::CVIBuffer_SwordTrail(const CVIBuffer_SwordTrail& Prototype)
	: CVIBuffer( Prototype )
	, m_iMaxNodeCount{ Prototype.m_iMaxNodeCount }
	, m_fLifeDuration{ Prototype.m_fLifeDuration }
	, m_bTrailActive{ Prototype.m_bTrailActive }
	, m_TrailNodes{ Prototype.m_TrailNodes }
	, m_Subdivisions{ Prototype.m_Subdivisions }
	, m_fNodeInterval{ Prototype.m_fNodeInterval }
{
}

HRESULT CVIBuffer_SwordTrail::Initialize_Prototype(const DESC* pDesc)
{
	if (pDesc != nullptr)
	{
		m_fLifeDuration = pDesc->fLifeDuration;
		m_fNodeInterval = pDesc->fNodeInterval;
		m_Subdivisions = pDesc->Subdivisions;
	}
	m_iMaxNodeCount = 500;
	m_iNumVertexBuffers = 1;
	m_iNumVertices = m_iMaxNodeCount;
	m_iVertexStride = sizeof(VTXPOS_TRAIL);
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

	return S_OK;
}

HRESULT CVIBuffer_SwordTrail::Initialize_Prototype(const _wstring& strJsonFilePath)
{
	json j;
	ifstream ifs(strJsonFilePath);
	if (!ifs.is_open())
	{
		MSG_BOX("Failed to open JSON file for CVIBuffer_SwordTrail");
		return E_FAIL;
	}
	ifs >> j;
	ifs.close();



	return S_OK;
}

HRESULT CVIBuffer_SwordTrail::Initialize(void* pArg)
{

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

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);


	// 여기서 종류 정해야하나?
	m_InterpolatedNewNodes.reserve(m_Subdivisions + 1);

	return S_OK;
}

void CVIBuffer_SwordTrail::Update_Trail(const _float3& vInnerPos, const _float3& vOuterPos, _float fTimeDelta)
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


	m_bNewNode = false;
	m_InterpolatedNewNodes.clear();

	m_fNodeAccTime += fTimeDelta;
	// 3. 트레일 활성 상태일 경우에만 노드 추가
	if (m_bTrailActive && m_fNodeAccTime >= m_fNodeInterval)
	{
		VTXPOS_TRAIL newNode;
		newNode.vInnerPos = vInnerPos;
		newNode.vOuterPos = vOuterPos;
		newNode.vLifeTime = _float2(m_fLifeDuration, 0.f);
		m_TrailNodes.push_back(newNode);
		m_bNewNode = true;

		m_fNodeAccTime = 0.f;
	}
	
	Update_Buffers();
}

HRESULT CVIBuffer_SwordTrail::Update_Buffers()
{
	m_iNumVertices = (_uint)m_TrailNodes.size();
	if (m_iNumVertices == 0)
		return S_OK;

	if (FAILED(Interpolate_TrailNodes()))
		return E_FAIL;

	return S_OK;
}


HRESULT CVIBuffer_SwordTrail::Bind_Buffers()
{
	ID3D11Buffer* pVertexBuffers[] = { m_pVB };
	_uint		iVertexStrides[] = { m_iVertexStride };
	_uint		iOffsets[] = { 0 };

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);

	return S_OK;
}


HRESULT CVIBuffer_SwordTrail::Render()
{
	if (m_TrailNodes.size() < 2)
		return S_OK;

	m_pContext->Draw(m_iNumVertices, 0);

	return S_OK;
}

HRESULT CVIBuffer_SwordTrail::Interpolate_TrailNodes()
{
	// 원본 노드가 너무 적으면 그릴 필요 없음
	if (m_TrailNodes.size() < 4)
	{
		m_iNumVertices = 0;
		return S_OK;
	}

	// 보간 후 예상 정점 수 계산
	_uint iSmoothCount = (_uint)(m_TrailNodes.size() - 3) * (m_Subdivisions + 1);

	// 1) 버퍼 크기 부족 시 재생성
	if (iSmoothCount > m_iMaxNodeCount)
	{
		m_iMaxNodeCount = (_uint)iSmoothCount;

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

	// 2) 보간된 노드 생성
	vector<VTXPOS_TRAIL> SmoothNodes;
	SmoothNodes.reserve(iSmoothCount);

	_uint outIndex = 0;
	const _float invDen = 1.0f / max(1, (iSmoothCount - 1));

	for (size_t i = 0; i + 3 < m_TrailNodes.size(); ++i)
	{
		const auto& P0 = m_TrailNodes[i + 0];
		const auto& P1 = m_TrailNodes[i + 1];
		const auto& P2 = m_TrailNodes[i + 2];
		const auto& P3 = m_TrailNodes[i + 3];

		for (_int s = 0; s <= m_Subdivisions; ++s)
		{
			 _float t = (_float)s / (_float)m_Subdivisions;

			// Inner pos 보간
			_vector i0 = XMLoadFloat3(&P0.vInnerPos);
			_vector i1 = XMLoadFloat3(&P1.vInnerPos);
			_vector i2 = XMLoadFloat3(&P2.vInnerPos);
			_vector i3 = XMLoadFloat3(&P3.vInnerPos);
			_float3 inner;
			XMStoreFloat3(&inner, XMVectorCatmullRom(i0, i1, i2, i3, t));

			// Outer pos 보간
			_vector o0 = XMLoadFloat3(&P0.vOuterPos);
			_vector o1 = XMLoadFloat3(&P1.vOuterPos);
			_vector o2 = XMLoadFloat3(&P2.vOuterPos);
			_vector o3 = XMLoadFloat3(&P3.vOuterPos);
			_float3 outer;
			XMStoreFloat3(&outer, XMVectorCatmullRom(o0, o1, o2, o3, t));

			// LifeTime (간단히 P1 기준)
			VTXPOS_TRAIL node;
			node.vInnerPos = inner;
			node.vOuterPos = outer;
			node.vLifeTime = P1.vLifeTime;
			node.fVCoord = (_float)outIndex * invDen;
			++outIndex;
			SmoothNodes.push_back(node);
			if (m_bNewNode == true && i + 4 >= m_TrailNodes.size())
			{ //새 노드가 추가됐고 지금 그 위치일 때(제일 마지막 세트)
				m_InterpolatedNewNodes.push_back(outer);
			}
		}
	}


	m_iNumVertices = (_uint)SmoothNodes.size();

	// 3) GPU 버퍼에 쓰기
	D3D11_MAPPED_SUBRESOURCE subres;
	if (FAILED(m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres)))
		return E_FAIL;

	memcpy(subres.pData, SmoothNodes.data(), sizeof(VTXPOS_TRAIL) * m_iNumVertices);

	m_pContext->Unmap(m_pVB, 0);

	return S_OK;
}

CVIBuffer_SwordTrail* CVIBuffer_SwordTrail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& strJsonFilePath)
{
	CVIBuffer_SwordTrail* pInstance = new CVIBuffer_SwordTrail(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(strJsonFilePath)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_SwordTrail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CVIBuffer_SwordTrail* CVIBuffer_SwordTrail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const DESC* pDesc)
{
	CVIBuffer_SwordTrail* pInstance = new CVIBuffer_SwordTrail(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pDesc)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_SwordTrail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_SwordTrail::Clone(void* pArg)
{
	CVIBuffer_SwordTrail* pInstance = new CVIBuffer_SwordTrail(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CVIBuffer_SwordTrail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_SwordTrail::Free()
{
	__super::Free();

}
