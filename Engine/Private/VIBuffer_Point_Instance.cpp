#include "VIBuffer_Point_Instance.h"

#include "GameInstance.h"

CVIBuffer_Point_Instance::CVIBuffer_Point_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer_Instance{ pDevice, pContext }
{
}

CVIBuffer_Point_Instance::CVIBuffer_Point_Instance(const CVIBuffer_Point_Instance& Prototype)
	: CVIBuffer_Instance( Prototype )
	, m_pVertexInstances{ Prototype.m_pVertexInstances }
	, m_pParticleDesc(Prototype.m_pParticleDesc)
	, m_vPivot{ Prototype.m_vPivot }
	, m_isLoop{ Prototype.m_isLoop }
	, m_bGravity{ Prototype.m_bGravity }
	, m_fGravity{ Prototype.m_fGravity }
	, m_vDirection{ Prototype.m_vDirection }
	, m_ePType{ Prototype.m_ePType }
{
}

HRESULT CVIBuffer_Point_Instance::Initialize_Prototype(const DESC* pArg)
{
	m_iNumIndexPerInstance = 1;
	m_iVertexInstanceStride = sizeof(VTXPOS_PARTICLE_INSTANCE);

	m_iNumVertexBuffers = 2;
	m_iNumVertices = 1;
	m_iVertexStride = sizeof(VTXPOS);
	m_iNumIndices = m_iNumIndexPerInstance;

	m_iIndexStride = sizeof(_ushort);
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

#pragma region VERTEXBUFFER
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

	pVertices[0].vPosition = _float3(0.0f, 0.0f, 0.f);

	for (_uint i = 0; i < m_iNumVertices; ++i)
		m_pVertexPositions[i] = pVertices[i].vPosition;

	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBBufferDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

#pragma endregion 

	if(pArg->isTool == false)
		Make_InstanceBuffer(pArg);

	return S_OK;
}

HRESULT CVIBuffer_Point_Instance::Initialize(void* pArg)
{
	DESC* pDesc = static_cast<DESC*>(pArg);
	if (pDesc->isTool == true)
		Make_InstanceBuffer(pDesc);

	if (FAILED(m_pDevice->CreateBuffer(&m_VBInstanceDesc, &m_VBInstanceSubresourceData, &m_pVBInstance)))
		return E_FAIL;

	return S_OK;
}

void CVIBuffer_Point_Instance::Update(_float fTimeDelta)
{
	switch (m_ePType)
	{
	case Engine::PTYPE_SPREAD:
		Spread(fTimeDelta);
		break;
	case Engine::PTYPE_DIRECTIONAL:
		Directional(fTimeDelta);
		break;
	case Engine::PTYPE_END:
		break;
	default:
		break;
	}
}
void CVIBuffer_Point_Instance::Update_Tool(_float fCurTrackPos)
{
	switch (m_ePType)
	{
	case Engine::PTYPE_SPREAD:
		Spread(fCurTrackPos / 60.f, true);
		break;
	case Engine::PTYPE_DIRECTIONAL:
		Directional(fCurTrackPos / 60.f, true);
		break;
	case Engine::PTYPE_END:
		break;
	default:
		break;
	}
}

HRESULT CVIBuffer_Point_Instance::Bind_Buffers()
{
	// ����� cs�� �����ϱ� 
	ID3D11Buffer* pVertexBuffers[] = {
		   m_pVB,
		   m_pVBInstance,
	};

	_uint		iVertexStrides[] = {
		m_iVertexStride,
		m_iVertexInstanceStride

	};

	_uint		iOffsets[] = {
		0,
		0
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	//m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0); // ��ƼŬ Indexbuffer ���� ����
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);
	
	return S_OK;
}

HRESULT CVIBuffer_Point_Instance::Render()
{
	m_pContext->DrawInstanced(m_iNumIndexPerInstance, m_iNumInstance, 0, 0); 
	return S_OK;
}

void CVIBuffer_Point_Instance::Directional(_float fTimeDelta, _bool bTool)
{
	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXPOS_PARTICLE_INSTANCE* pVertices = static_cast<VTXPOS_PARTICLE_INSTANCE*>(SubResource.pData);


	if (bTool)
	{
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			_float trackTime = fTimeDelta; // fTimeDelta�� ���� �ð����� ���� (trackPos / 60.0f)

			// ���� ���� ��: trackTime�� �ֱ� ���� ����
			if (m_isLoop)
			{
				trackTime = fmodf(trackTime, m_pVertexInstances[i].vLifeTime.x);
			}
			_vector vStart = XMLoadFloat4(&m_pVertexInstances[i].vTranslation);
			_vector vDir = XMLoadFloat4(&m_pVertexInstances[i].vDirection);
			_vector vNew = vStart + vDir * m_pParticleDesc[i].vSpeeds * trackTime;

			XMStoreFloat4(&pVertices[i].vTranslation, vNew);

			pVertices[i].vLifeTime.y = trackTime;

			// �ʱ� ��ġ - �ӵ� * �ð�
			//pVertices[i].vTranslation = m_pVertexInstances[i].vTranslation;
			//pVertices[i].vTranslation.y -= m_pSpeeds[i] * trackTime;
		}
	}
	else
	{
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			pVertices[i].vLifeTime.y += fTimeDelta;

			//pVertices[i].vTranslation.y -= m_pSpeeds[i] * fTimeDelta;

			_vector vDir = XMLoadFloat4(&m_pVertexInstances[i].vDirection);
			_vector vPos = XMLoadFloat4(&pVertices[i].vTranslation);
			vPos += vDir * m_pParticleDesc[i].vSpeeds * fTimeDelta;
			XMStoreFloat4(&pVertices[i].vTranslation, vPos);


			if (true == m_isLoop &&
				pVertices[i].vLifeTime.y >= pVertices[i].vLifeTime.x)
			{
				pVertices[i].vLifeTime.y = 0.f;
				pVertices[i].vTranslation = m_pVertexInstances[i].vTranslation;
			}
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Point_Instance::Spread(_float fTimeDelta, _bool bTool)
{
	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXPOS_PARTICLE_INSTANCE* pVertices = static_cast<VTXPOS_PARTICLE_INSTANCE*>(SubResource.pData);

	_vector vDir = {};

	if (bTool)
	{
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			_float trackTime = fTimeDelta;
			if (m_isLoop)
				trackTime = fmodf(trackTime, m_pVertexInstances[i].vLifeTime.x);

			pVertices[i].vLifeTime.y = trackTime;

			_vector vStart = XMLoadFloat4(&m_pVertexInstances[i].vTranslation);
			_vector vDir = XMLoadFloat4(&m_pVertexInstances[i].vDirection);
			_vector vNew = vStart + vDir * m_pParticleDesc[i].vSpeeds * trackTime;

			if (m_bGravity)
			{
				_float gOffset = 0.5f * m_fGravity * trackTime * trackTime;
				vNew = XMVectorSetY(vNew, XMVectorGetY(vNew) - gOffset);
			}

			// ����
			if (m_pParticleDesc[i].vRotationAxis_Speed.w != 0.f)
			{
				_float angle = m_pParticleDesc[i].vRotationAxis_Speed.w * trackTime;
				_vector axis = XMLoadFloat3((_float3*)&m_pParticleDesc[i].vRotationAxis_Speed);
				_matrix rot = XMMatrixRotationAxis(axis, angle);
				//_vector rot = XMMatrixRotationAxis(axis, angle);

				_vector vRight = XMVector3TransformNormal(XMLoadFloat4(&pVertices[i].vRight), rot);
				_vector vUp = XMVector3TransformNormal(XMLoadFloat4(&pVertices[i].vUp), rot);
				_vector vLook = XMVector3TransformNormal(XMLoadFloat4(&pVertices[i].vLook), rot);

				XMStoreFloat4(&pVertices[i].vRight, vRight);
				XMStoreFloat4(&pVertices[i].vUp, vUp);
				XMStoreFloat4(&pVertices[i].vLook, vLook);
			}
			// ����
			if (m_pParticleDesc[i].vOrbitAxis_Radius_Speed.w != 0.f)
			{
				_float angle = m_pParticleDesc[i].vOrbitAxis_Radius_Speed.w * trackTime;
				_vector axis = XMLoadFloat3((_float3*)&m_pParticleDesc[i].vOrbitAxis_Radius_Speed);
				_matrix orbitRot = XMMatrixRotationAxis(axis, angle);

				_vector center = XMLoadFloat4(&m_pParticleDesc[i].vOrbitCenter);
				_vector pos = XMLoadFloat4(&m_pVertexInstances[i].vTranslation);
				_vector offset = pos - center;
				offset = XMVector3TransformNormal(offset, orbitRot);
				pos = center + offset;

				XMStoreFloat4(&pVertices[i].vTranslation, pos);
			}




			XMStoreFloat4(&pVertices[i].vTranslation, vNew);
		}
	}
	else
	{
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			pVertices[i].vLifeTime.y += fTimeDelta;

			_vector vDir = XMLoadFloat4(&m_pVertexInstances[i].vDirection);
			_vector vPos = XMLoadFloat4(&pVertices[i].vTranslation);
			vPos += vDir * m_pParticleDesc[i].vSpeeds * fTimeDelta;

			if (m_bGravity)
			{
				_float t = pVertices[i].vLifeTime.y;
				_float gOffset = 0.5f * m_fGravity * fTimeDelta * fTimeDelta;
				vPos = XMVectorSetY(vPos, XMVectorGetY(vPos) - gOffset);
			}

			XMStoreFloat4(&pVertices[i].vTranslation, vPos);

			if (m_isLoop && pVertices[i].vLifeTime.y >= pVertices[i].vLifeTime.x)
			{
				pVertices[i].vLifeTime.y = 0.f;
				pVertices[i].vTranslation = m_pVertexInstances[i].vTranslation;
			}
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

HRESULT CVIBuffer_Point_Instance::Make_InstanceBuffer(const DESC* pDesc)
{
	// desc ���� ����
	m_vPivot = pDesc->vPivot;
	m_isLoop = pDesc->isLoop;
	m_iNumInstance = pDesc->iNumInstance;
	m_ePType = pDesc->ePType;
	m_bGravity = pDesc->bGravity;
	m_fGravity = pDesc->fGravity;
	m_vDirection = pDesc->vDirection;


#pragma region INSTANCEBUFFER
	m_VBInstanceDesc.ByteWidth = m_iNumInstance * m_iVertexInstanceStride;
	m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_VBInstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_VBInstanceDesc.StructureByteStride = m_iVertexInstanceStride;
	m_VBInstanceDesc.MiscFlags = 0;

	m_pVertexInstances = new VTXPOS_PARTICLE_INSTANCE[m_iNumInstance];
	//m_pSpeeds = new _float[m_iNumInstance];
	m_pParticleDesc = new PARTICLEDESC[m_iNumInstance];
	ZeroMemory(m_pParticleDesc, sizeof(PARTICLEDESC) * m_iNumInstance);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		m_pParticleDesc[i].vSpeeds = m_pGameInstance->Compute_Random(pDesc->vSpeed.x, pDesc->vSpeed.y);
		_float	fSize = m_pGameInstance->Compute_Random(pDesc->vSize.x, pDesc->vSize.y);

		m_pVertexInstances[i].vRight = _float4(fSize, 0.f, 0.f, 0.f);
		m_pVertexInstances[i].vUp = _float4(0.f, fSize, 0.f, 0.f);
		m_pVertexInstances[i].vLook = _float4(0.f, 0.f, fSize, 0.f);

		m_pVertexInstances[i].vTranslation = _float4(
			m_pGameInstance->Compute_Random(pDesc->vCenter.x - pDesc->vRange.x * 0.5f, pDesc->vCenter.x + pDesc->vRange.x * 0.5f),
			m_pGameInstance->Compute_Random(pDesc->vCenter.y - pDesc->vRange.y * 0.5f, pDesc->vCenter.y + pDesc->vRange.y * 0.5f),
			m_pGameInstance->Compute_Random(pDesc->vCenter.z - pDesc->vRange.z * 0.5f, pDesc->vCenter.z + pDesc->vRange.z * 0.5f),
			1.f
		);

		m_pVertexInstances[i].vLifeTime = _float2(
			m_pGameInstance->Compute_Random(pDesc->vLifeTime.x, pDesc->vLifeTime.y),
			0.f
		);
		_vector vDir = {};


		switch (m_ePType)
		{
		case Engine::PTYPE_SPREAD:
			_vector vStart = XMLoadFloat4(&m_pVertexInstances[i].vTranslation);
			_vector vPivot = XMLoadFloat3(&m_vPivot);
			vDir = XMVectorSetW(XMVector3Normalize(vStart - vPivot), 0.f);
			break;
		case Engine::PTYPE_DIRECTIONAL:
			vDir = XMVector3Normalize(XMLoadFloat4(&m_vDirection));
			break;
		case Engine::PTYPE_ALLRANDOM:
			vDir = XMVector3Normalize(
				XMVectorSet(
					m_pGameInstance->Compute_Random(-1.f, 1.f),
					m_pGameInstance->Compute_Random(-1.f, 1.f),
					m_pGameInstance->Compute_Random(-1.f, 1.f),
					0.f)
			);
			break;
		default:
			break;
		}

		

		XMStoreFloat4(&m_pVertexInstances[i].vDirection, vDir);
		//XMStoreFloat4(&m_pParticleDesc[i].vDirection, vDir);
	}

	m_VBInstanceSubresourceData.pSysMem = m_pVertexInstances;

#pragma endregion 

	return S_OK;
}

CVIBuffer_Point_Instance* CVIBuffer_Point_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const DESC* pArg)
{
	CVIBuffer_Point_Instance* pInstance = new CVIBuffer_Point_Instance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pArg)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Point_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_Point_Instance::Clone(void* pArg)
{
	CVIBuffer_Point_Instance* pInstance = new CVIBuffer_Point_Instance(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CVIBuffer_Point_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Point_Instance::Free()
{
	__super::Free();


	if (false == m_isCloned)
	{
		Safe_Delete_Array(m_pVertexInstances);
		Safe_Delete_Array(m_pParticleDesc);
	}
}
