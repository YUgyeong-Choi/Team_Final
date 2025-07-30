#include "Mesh_Instance.h"

#include "Shader.h"

CMesh_Instance::CMesh_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer_Instance{ pDevice, pContext }
{
}

CMesh_Instance::CMesh_Instance(const CMesh_Instance& Prototype)
	: CVIBuffer_Instance(Prototype)
{
	//메쉬는 클론을 안하나?
	//_int a = 0;
}

//HRESULT CMesh_Instance::Initialize_Prototype(MODEL eType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix)
//{
//	strcpy_s(m_szName, pAIMesh->mName.data);
//	m_iMaterialIndex = pAIMesh->mMaterialIndex;
//	m_iNumVertexBuffers = 1;
//	m_iNumVertices = pAIMesh->mNumVertices;
//	m_iNumIndices = pAIMesh->mNumFaces * 3;
//	m_iIndexStride = sizeof(_uint);
//	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
//	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//
//	HRESULT hr = Ready_NonAnim_Mesh(pAIMesh, PreTransformMatrix);
//	//HRESULT hr = eType == MODEL::NONANIM ? Ready_NonAnim_Mesh(pAIMesh, PreTransformMatrix) : Ready_Anim_Mesh(pAIMesh, Bones);
//
//	if (FAILED(hr))
//		return E_FAIL;
//
//	D3D11_BUFFER_DESC			IBBufferDesc{};
//	IBBufferDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
//	IBBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	IBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
//	IBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
//	IBBufferDesc.StructureByteStride = m_iIndexStride;
//	IBBufferDesc.MiscFlags = 0;
//
//	m_pIndices = new _uint[m_iNumIndices];
//	ZeroMemory(m_pIndices, sizeof(_uint) * m_iNumIndices);
//
//	_uint	iNumIndices = { 0 };
//
//	for (size_t i = 0; i < pAIMesh->mNumFaces; i++)
//	{
//		m_pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[0];
//		m_pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[1];
//		m_pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[2];
//	}
//
//	D3D11_SUBRESOURCE_DATA		IBInitialData{};
//	IBInitialData.pSysMem = m_pIndices;
//
//	if (FAILED(m_pDevice->CreateBuffer(&IBBufferDesc, &IBInitialData, &m_pIB)))
//		return E_FAIL;
//
//	return S_OK;
//}

HRESULT CMesh_Instance::Initialize_Prototype(MODEL eType, ifstream& ifs, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix)
{
#pragma region 버텍스 버퍼
	_uint NameLength = {};
	ifs.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));			// 메쉬 이름 길이 
	ifs.read(reinterpret_cast<char*>(m_szName), NameLength);				// 메쉬 이름
	ifs.read(reinterpret_cast<char*>(&m_iMaterialIndex), sizeof(_uint));	// 머테리얼 인덱스
	ifs.read(reinterpret_cast<char*>(&m_iNumVertices), sizeof(_uint));		// 버텍스 몇개
	ifs.read(reinterpret_cast<char*>(&m_iNumIndices), sizeof(_uint));		// 인덱스 몇개
	m_iIndexStride = sizeof(_uint);
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


	HRESULT hr = Ready_NonAnim_Mesh(ifs, PreTransformMatrix);
	//HRESULT hr = eType == MODEL::NONANIM ? Ready_NonAnim_Mesh(ifs, PreTransformMatrix) : Ready_Anim_Mesh(ifs, Bones);

	if (FAILED(hr))
		return E_FAIL;
#pragma endregion

#pragma region 인덱스버퍼
	D3D11_BUFFER_DESC			IBBufferDesc{};
	IBBufferDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
	IBBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	IBBufferDesc.StructureByteStride = m_iIndexStride;
	IBBufferDesc.MiscFlags = 0;

	m_pIndices = new _uint[m_iNumIndices];
	ZeroMemory(m_pIndices, sizeof(_uint) * m_iNumIndices);

	_uint	iNumIndices = { 0 };

	ifs.read(reinterpret_cast<char*>(m_pIndices), sizeof(_uint) * m_iNumIndices);			// 버텍스 구조체 배열

	D3D11_SUBRESOURCE_DATA		IBInitialData{};
	IBInitialData.pSysMem = m_pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBBufferDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;
#pragma endregion

#pragma region 인스턴싱을 위한 준비
	m_iNumVertexBuffers = 2; //두개 해야해 버텍스 + 인스턴스버퍼(이거 땜에 응애응애)
	//인스턴스 버퍼 스트라이드
	m_iVertexInstanceStride = sizeof(VTXMESH_INSTANCE);
	m_iNumIndexPerInstance = m_iNumIndices;
	//m_iNumInstance
#pragma endregion


	return S_OK;
}

HRESULT CMesh_Instance::Initialize(void* pArg)
{
	MESHINSTANCE_DESC* pDesc = static_cast<MESHINSTANCE_DESC*>(pArg);
	m_iNumInstance = pDesc->iNumInstance;

#pragma region INSTANCEBUFFER

	m_VBInstanceDesc.ByteWidth = m_iNumInstance * m_iVertexInstanceStride;
	m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_VBInstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_VBInstanceDesc.StructureByteStride = m_iVertexInstanceStride;
	m_VBInstanceDesc.MiscFlags = 0;

 	m_pVertexInstances = new VTXMESH_INSTANCE[m_iNumInstance];

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		memcpy_s(&m_pVertexInstances[i].vRight, sizeof(_float4), &(*pDesc->pInstanceMatrixs)[i].m[0], sizeof(_float4));
		memcpy_s(&m_pVertexInstances[i].vUp, sizeof(_float4), &(*pDesc->pInstanceMatrixs)[i].m[1], sizeof(_float4));
		memcpy_s(&m_pVertexInstances[i].vLook, sizeof(_float4), &(*pDesc->pInstanceMatrixs)[i].m[2], sizeof(_float4));
		memcpy_s(&m_pVertexInstances[i].vTranslation, sizeof(_float4), &(*pDesc->pInstanceMatrixs)[i].m[3], sizeof(_float4));
	}

	m_VBInstanceSubresourceData.pSysMem = m_pVertexInstances;

	if (FAILED(m_pDevice->CreateBuffer(&m_VBInstanceDesc, &m_VBInstanceSubresourceData, &m_pVBInstance)))
		return E_FAIL;

#pragma endregion 

	return S_OK;
}

HRESULT CMesh_Instance::Ready_NonAnim_Mesh(const aiMesh* pAIMesh, _fmatrix PreTransformMatrix)
{
	m_iVertexStride = sizeof(VTXMESH);
	D3D11_BUFFER_DESC			VBBufferDesc{};
	VBBufferDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	VBBufferDesc.StructureByteStride = m_iVertexStride;
	VBBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA		VBInitialData{};

	VTXMESH* pVertices = new VTXMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXMESH) * m_iNumVertices);

	m_pVertexPositions = new _float3[m_iNumVertices];
	ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);

	for (size_t i = 0; i < m_iNumVertices; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(_float3));
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));

		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(_float3));
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));

		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(_float3));
		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(_float2));
	}

	for (_uint i = 0; i < m_iNumVertices; ++i)
		m_pVertexPositions[i] = pVertices[i].vPosition;

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CMesh_Instance::Ready_NonAnim_Mesh(ifstream& ifs, _fmatrix PreTransformMatrix)
{
	m_iVertexStride = sizeof(VTXMESH);
	D3D11_BUFFER_DESC			VBBufferDesc{};
	VBBufferDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	VBBufferDesc.StructureByteStride = m_iVertexStride;
	VBBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA		VBInitialData{};

	VTXMESH* pVertices = new VTXMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXMESH) * m_iNumVertices);

	m_pVertexPositions = new _float3[m_iNumVertices];
	ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);

	for (size_t i = 0; i < m_iNumVertices; i++)
	{
		ifs.read(reinterpret_cast<char*>(&pVertices[i].vPosition), sizeof(_float3));		// 포지숑
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));

		ifs.read(reinterpret_cast<char*>(&pVertices[i].vNormal), sizeof(_float3));			// 잉덱스
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));

		ifs.read(reinterpret_cast<char*>(&pVertices[i].vTangent), sizeof(_float3));			// 땅젱뜨
		ifs.read(reinterpret_cast<char*>(&pVertices[i].vTexcoord), sizeof(_float2));		// 뗶,꾸 얼마나귀엽습ㄴ까
	}

	for (_uint i = 0; i < m_iNumVertices; ++i)
		m_pVertexPositions[i] = pVertices[i].vPosition;

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

//CMesh_Instance* CMesh_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix)
//{
//	CMesh_Instance* pInstance = new CMesh_Instance(pDevice, pContext);
//
//	if (FAILED(pInstance->Initialize_Prototype(eType, pAIMesh, Bones, PreTransformMatrix)))
//	{
//		MSG_BOX("Failed to Created : CMesh_Instance");
//		Safe_Release(pInstance);
//	}
//
//	return pInstance;
//}

CMesh_Instance* CMesh_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, ifstream& ifs, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix)
{
	CMesh_Instance* pInstance = new CMesh_Instance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, ifs, Bones, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CMesh_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CMesh_Instance::Clone(void* pArg)
{
	CMesh_Instance* pInstance = new CMesh_Instance(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMesh_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMesh_Instance::Free()
{
	__super::Free();

	Safe_Delete_Array(m_pVertexInstances);
}
