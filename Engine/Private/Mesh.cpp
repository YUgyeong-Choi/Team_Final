#include "Mesh.h"

#include "Bone.h"
#include "Shader.h"

CMesh::CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CMesh::CMesh(const CMesh& Prototype)
	: CVIBuffer( Prototype )
{
	strcpy_s(m_szName, Prototype.m_szName);


	m_iMaterialIndex = Prototype.m_iMaterialIndex;
	m_iNumBones = Prototype.m_iNumBones;

	m_pIndices = Prototype.m_pIndices;
	m_pVertexPositions = Prototype.m_pVertexPositions;

	m_OffsetMatrices = Prototype.m_OffsetMatrices;
	m_BoneIndices = Prototype.m_BoneIndices;
	memcpy(m_BoneMatrices, Prototype.m_BoneMatrices, sizeof(_float4x4) * g_iMaxNumBones);

	m_pLocalToGlobalBuffer = Prototype.m_pLocalToGlobalBuffer;
	m_pLocalToGlobalSRV = Prototype.m_pLocalToGlobalSRV;
	m_pOffsetsBuffer = Prototype.m_pOffsetsBuffer;
	m_pOffsetsSRV = Prototype.m_pOffsetsSRV;

	Safe_AddRef(m_pLocalToGlobalBuffer);
	Safe_AddRef(m_pLocalToGlobalSRV);
	Safe_AddRef(m_pOffsetsBuffer);
	Safe_AddRef(m_pOffsetsSRV);
}

HRESULT CMesh::Initialize_Prototype(MODEL eType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix)
{
	strcpy_s(m_szName, pAIMesh->mName.data);
	m_iMaterialIndex = pAIMesh->mMaterialIndex;
	m_iNumVertexBuffers = 1;
	m_iNumVertices = pAIMesh->mNumVertices;
	m_iNumIndices = pAIMesh->mNumFaces * 3;
	m_iIndexStride = sizeof(_uint);
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	HRESULT hr = eType == MODEL::NONANIM ? Ready_NonAnim_Mesh(pAIMesh, PreTransformMatrix) : Ready_Anim_Mesh(pAIMesh, Bones);

	if (FAILED(hr))
		return E_FAIL;

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

	for (size_t i = 0; i < pAIMesh->mNumFaces; i++)
	{
		m_pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[0];
		m_pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[1];
		m_pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[2];
	}

	D3D11_SUBRESOURCE_DATA		IBInitialData{};
	IBInitialData.pSysMem = m_pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBBufferDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMesh::Initialize_Prototype(MODEL eType, ifstream& ifs, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix)
{
	_uint NameLength = {};
	ifs.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));			// �޽� �̸� ���� 
	ifs.read(reinterpret_cast<char*>(m_szName), NameLength);				// �޽� �̸�
	ifs.read(reinterpret_cast<char*>(&m_iMaterialIndex), sizeof(_uint));	// ���׸��� �ε���
	ifs.read(reinterpret_cast<char*>(&m_iNumVertices), sizeof(_uint));		// ���ؽ� �
	ifs.read(reinterpret_cast<char*>(&m_iNumIndices), sizeof(_uint));		// �ε��� �
	m_iNumVertexBuffers = 1;
	m_iIndexStride = sizeof(_uint);
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


	HRESULT hr = eType == MODEL::NONANIM ? Ready_NonAnim_Mesh(ifs, PreTransformMatrix) : Ready_Anim_Mesh(ifs, Bones);

	if (FAILED(hr))
		return E_FAIL;


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

	ifs.read(reinterpret_cast<char*>(m_pIndices), sizeof(_uint) * m_iNumIndices);			// ���ؽ� ����ü �迭

	D3D11_SUBRESOURCE_DATA		IBInitialData{};
	IBInitialData.pSysMem = m_pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBBufferDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CMesh::Ready_NonAnim_Mesh(const aiMesh* pAIMesh, _fmatrix PreTransformMatrix)
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

HRESULT CMesh::Ready_NonAnim_Mesh(ifstream& ifs, _fmatrix PreTransformMatrix)
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
		ifs.read(reinterpret_cast<char*>(&pVertices[i].vPosition), sizeof(_float3));		
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));
	
		ifs.read(reinterpret_cast<char*>(&pVertices[i].vNormal), sizeof(_float3));			
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));
	
		ifs.read(reinterpret_cast<char*>(&pVertices[i].vTangent), sizeof(_float3));			
		ifs.read(reinterpret_cast<char*>(&pVertices[i].vTexcoord), sizeof(_float2));		
	}

	for (_uint i = 0; i < m_iNumVertices; ++i)
		m_pVertexPositions[i] = pVertices[i].vPosition;

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}
HRESULT CMesh::Ready_Anim_Mesh(const aiMesh* pAIMesh, const vector<class CBone*>& Bones)
{
	m_iVertexStride = sizeof(VTXANIMMESH);
	D3D11_BUFFER_DESC			VBBufferDesc{};
	VBBufferDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	VBBufferDesc.StructureByteStride = m_iVertexStride;
	VBBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA		VBInitialData{};

	VTXANIMMESH* pVertices = new VTXANIMMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);

	m_pVertexPositions = new _float3[m_iNumVertices];
	ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);

	for (size_t i = 0; i < m_iNumVertices; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(_float3));
		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(_float3));
		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(_float3));
		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(_float2));
	}

	/* �� ������ ������ �޴� ���� ������ ������ ��? */

	m_iNumBones = pAIMesh->mNumBones;

	for (_uint i = 0; i < m_iNumBones; i++)
	{
		/* �θ�� �ڽĵ��� ���� ���輺�� ǥ��(x) -> aiNode */
		/* �� �޽ÿ� � ������� ������ �ݴϴ�. and �󸶳� ������ �ݴϴ�. */
		aiBone* pAIBone = pAIMesh->mBones[i];

		_float4x4		OffsetMatrix;

		memcpy(&OffsetMatrix, &pAIBone->mOffsetMatrix, sizeof(_float4x4));

		XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));


		m_OffsetMatrices.push_back(OffsetMatrix);

		_uint	iBoneIndex = {};

		auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool
			{
				if(true == pBone->Compare_Name(pAIBone->mName.data))
					return true;				

				++iBoneIndex;

				return false;
			});

		m_BoneIndices.push_back(iBoneIndex);	

		/* i��° ���� � �������� ������ �ִµ�?*/
		_uint		iNumWeights = pAIBone->mNumWeights;
		
		for (_uint j = 0; j < iNumWeights; j++)
		{
			/* i��° ���� ���⤷�� �ִ� j��° ������ ���� */
			aiVertexWeight	AIWeight = pAIBone->mWeights[j];

			if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.x)
			{
				/* �� �޽ÿ��� ������ �ִ� ���� �� i��° ���� �� �������� ������ �ֳ�. */
				pVertices[AIWeight.mVertexId].vBlendIndices.x = i;
				pVertices[AIWeight.mVertexId].vBlendWeights.x = AIWeight.mWeight;
			}

			else if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.y)
			{
				pVertices[AIWeight.mVertexId].vBlendIndices.y = i;
				pVertices[AIWeight.mVertexId].vBlendWeights.y = AIWeight.mWeight;
			}

			else if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.z)
			{
				pVertices[AIWeight.mVertexId].vBlendIndices.z = i;
				pVertices[AIWeight.mVertexId].vBlendWeights.z = AIWeight.mWeight;
			}

			else if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.w)
			{
				pVertices[AIWeight.mVertexId].vBlendIndices.w = i;
				pVertices[AIWeight.mVertexId].vBlendWeights.w = AIWeight.mWeight;
			}
		}
	}

	if (0 == m_iNumBones)
	{
		m_iNumBones = 1;

		_uint	iBoneIndex = {};

		auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool
			{
				if (true == pBone->Compare_Name(m_szName))
					return true;

				++iBoneIndex;

				return false;
			});


		m_BoneIndices.push_back(iBoneIndex);

		_float4x4		OffsetMatrix;
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixIdentity());
		m_OffsetMatrices.push_back(OffsetMatrix);
	}

	for (_uint i = 0; i < m_iNumVertices; ++i)
		m_pVertexPositions[i] = pVertices[i].vPosition;

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CMesh::Ready_Anim_Mesh(ifstream& ifs, const vector<class CBone*>& Bones)
{
	m_iVertexStride = sizeof(VTXANIMMESH);
	D3D11_BUFFER_DESC			VBBufferDesc{};
	VBBufferDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VBBufferDesc.CPUAccessFlags = /*D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE*/0;
	VBBufferDesc.StructureByteStride = m_iVertexStride;
	VBBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA		VBInitialData{};

	VTXANIMMESH* pVertices = new VTXANIMMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);

	m_pVertexPositions = new _float3[m_iNumVertices];
	ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);



	/* �� ������ ������ �޴� ���� ������ ������ ��? */
	/* �������� ����? */

	ifs.read(reinterpret_cast<char*>(&m_iNumBones), sizeof(_uint));  // �� �޽��� ������ �ִ� ���� ��� 

	_uint offSize = {};
	ifs.read(reinterpret_cast<char*>(&offSize), sizeof(_uint)); 
	m_OffsetMatrices.resize(offSize);																// ������ ��� �迭 
	ifs.read(reinterpret_cast<char*>(m_OffsetMatrices.data()), sizeof(_float4x4) * offSize);		// ������ ��� �迭 

	_uint BoneIndicesSize = {};
	ifs.read(reinterpret_cast<char*>(&BoneIndicesSize), sizeof(_uint)); 
	m_BoneIndices.resize(BoneIndicesSize);															// ������ ������ ��ġ�� ������ �ε����� ��������� ���� �迭 
	ifs.read(reinterpret_cast<char*>(m_BoneIndices.data()), sizeof(_uint) * BoneIndicesSize);		// ������ ������ ��ġ�� ������ �ε����� ��������� ���� �迭 

	ifs.read(reinterpret_cast<char*>(pVertices), sizeof(VTXANIMMESH) * m_iNumVertices);			// ���ؽ� ����ü �迭

	for (_uint i = 0; i < m_iNumVertices; ++i)
		m_pVertexPositions[i] = pVertices[i].vPosition;

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);


	{
		// t1: Local��Global (uint)
		m_iNumBones = static_cast<_int>(m_BoneIndices.size());
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(_uint) * m_iNumBones;
		bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bd.StructureByteStride = sizeof(_uint);

		D3D11_SUBRESOURCE_DATA initL2G{ m_BoneIndices.data(), 0, 0 };
		if (FAILED((m_pDevice->CreateBuffer(&bd, &initL2G, &m_pLocalToGlobalBuffer))))
			return E_FAIL;

		D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
		sd.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		sd.Format = DXGI_FORMAT_UNKNOWN;
		sd.BufferEx.FirstElement = 0;
		sd.BufferEx.NumElements = m_iNumBones;
		if (FAILED((m_pDevice->CreateShaderResourceView(m_pLocalToGlobalBuffer, &sd, &m_pLocalToGlobalSRV))))
			return E_FAIL;
	}
	{
		// t2: Offsets (float4x4)
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(_float4x4) * offSize;
		bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bd.StructureByteStride = sizeof(_float4x4);

		D3D11_SUBRESOURCE_DATA initOff{ m_OffsetMatrices.data(), 0, 0 };
		if (FAILED(m_pDevice->CreateBuffer(&bd, &initOff, &m_pOffsetsBuffer)))
		return E_FAIL;

		D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
		sd.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		sd.Format = DXGI_FORMAT_UNKNOWN;
		sd.BufferEx.FirstElement = 0;
		sd.BufferEx.NumElements = offSize;
		if (FAILED(m_pDevice->CreateShaderResourceView(m_pOffsetsBuffer, &sd, &m_pOffsetsSRV)))
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CMesh::Bind_Bone_Matrices(CShader* pShader, const _char* pConstantName, const vector<class CBone*>& Bones)
{
	ZeroMemory(m_BoneMatrices, sizeof(_float4x4) * g_iMaxNumBones);

	for (size_t i = 0; i < m_iNumBones; i++)
	{
		XMStoreFloat4x4(&m_BoneMatrices[i],
			XMLoadFloat4x4(&m_OffsetMatrices[i]) * 
			XMLoadFloat4x4(Bones[m_BoneIndices[i]]->Get_CombinedTransformationMatrix()));
	}

	if (m_iNumBones > g_iMaxNumBones)
	{
		MSG_BOX("���� �ʹ� ����");
		return E_FAIL;
	}

	return pShader->Bind_Matrices(pConstantName, m_BoneMatrices, m_iNumBones);	
}

HRESULT CMesh::Bind_SkinningSRVs(CShader* pShader)
{
	if (FAILED(pShader->Bind_SRV("g_LocalToGlobal", m_pLocalToGlobalSRV))) 
		return E_FAIL; // VS:t1
	if (FAILED(pShader->Bind_SRV("g_Offsets", m_pOffsetsSRV)))       
		return E_FAIL; // VS:t2
	return S_OK;
}

CMesh* CMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix)
{
	CMesh* pInstance = new CMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, pAIMesh, Bones, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CMesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CMesh* CMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, ifstream& ifs, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix)
{
	CMesh* pInstance = new CMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, ifs, Bones, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CMesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CMesh::Clone(void* pArg)
{
	CMesh* pInstance = new CMesh(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMesh::Free()
{
	Safe_Release(m_pLocalToGlobalBuffer);
	Safe_Release(m_pLocalToGlobalSRV);
	Safe_Release(m_pOffsetsBuffer);
	Safe_Release(m_pOffsetsSRV);
	__super::Free();
}
