#include "VIBuffer_Point_Instance.h"

#include "GameInstance.h"
#include "ParticleComputeShader.h"

CVIBuffer_Point_Instance::CVIBuffer_Point_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer_Instance{ pDevice, pContext }
{
}

CVIBuffer_Point_Instance::CVIBuffer_Point_Instance(const CVIBuffer_Point_Instance& Prototype)
	: CVIBuffer_Instance( Prototype )
	, m_pParticleParamDesc{ Prototype.m_pParticleParamDesc }
	, m_tCBuffer{ Prototype.m_tCBuffer }
{
}

HRESULT CVIBuffer_Point_Instance::Initialize_Prototype(const DESC* pArg)
{
	m_iNumIndexPerInstance = 1;
	m_iVertexInstanceStride = sizeof(VTXPOS_PARTICLE_INSTANCE);

	m_iNumVertexBuffers = 1;
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
	ZeroMemory(&m_tCBuffer, sizeof(PARTICLECBUFFER));
	m_tCBuffer.bIsTool = pArg->isTool?1:0;
	if(m_tCBuffer.bIsTool == 0) // 클라이언트 생성 시 
		Make_InstanceBuffer(pArg);


	return S_OK;
}

HRESULT CVIBuffer_Point_Instance::Initialize(void* pArg)
{
	if (pArg != nullptr)	
	{
 		DESC* pDesc = static_cast<DESC*>(pArg);
		m_tCBuffer.bIsTool = pDesc->isTool?1:0;

		if (m_tCBuffer.bIsTool == 1)
			Make_InstanceBuffer(pDesc); // 툴에서 생성 시
	}

	//if (FAILED(m_pDevice->CreateBuffer(&m_VBInstanceDesc, &m_VBInstanceSubresourceData, &m_pVBInstance)))
	//	return E_FAIL;

	CParticleComputeShader::DESC csDesc = {};
	csDesc.iNumInstance = m_iNumInstance;
	//csDesc.pParticleDesc = m_pParticleDesc;			//이거랑
	//csDesc.pVertexInstances = m_pVertexInstances;	// 이건 initdata
	csDesc.pParticleParamDesc = m_pParticleParamDesc;

	m_pParticleCS = CParticleComputeShader::Create(m_pDevice, m_pContext,TEXT("../Bin/ShaderFiles/Shader_Compute_Particle.cso"), &csDesc);
	if (!m_pParticleCS){
		return E_FAIL;
	}
	//if (FAILED(m_pParticleCS->))
	//	return E_FAIL;	

	return S_OK;
}

void CVIBuffer_Point_Instance::Update(_float fTimeDelta)
{
	m_tCBuffer.isFirst = 0;
	if (m_bFirst == true)
	{
		m_tCBuffer.isFirst = 1;
		m_tCBuffer.fAccTime = m_pGameInstance->Get_GlobalAccTime();
		m_tCBuffer.iEffectSeed = rand() % 100000;
		m_bFirst = false;
	}

	m_tCBuffer.fDeltaTime = fTimeDelta;
	m_tCBuffer.bIsTool = false;
	m_tCBuffer.fAccTime = m_pGameInstance->Get_GlobalAccTime();
	/* [ CS ] */
	m_pParticleCS->Dispatch_ParticleCS(m_tCBuffer, 128);
}

void CVIBuffer_Point_Instance::Update_Tool(_float fCurTrackPos)
{

	m_tCBuffer.fTrackTime = fCurTrackPos;
	m_tCBuffer.fAccTime = m_pGameInstance->Get_GlobalAccTime();
	m_tCBuffer.bIsTool = true;
	m_pParticleCS->Dispatch_ParticleCS(m_tCBuffer, 128);

}

HRESULT CVIBuffer_Point_Instance::Bind_Buffers()
{
	ID3D11Buffer* pVertexBuffers[] = {
		   m_pVB,
		   //m_pVBInstance,
	};

	_uint		iVertexStrides[] = {
		m_iVertexStride,
		//m_iVertexInstanceStride

	};

	_uint		iOffsets[] = {
		0,
		//0
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);

	m_pParticleCS->Bind_InstanceSRV();
	
	return S_OK;
}

HRESULT CVIBuffer_Point_Instance::Render()
{
	m_pContext->DrawInstanced(m_iNumIndexPerInstance, m_iNumInstance, 0, 0); 
	return S_OK;
}

void CVIBuffer_Point_Instance::Set_InitRotation(_fmatrix matRot)
{
	_vector vOrigOrbitAxis = XMLoadFloat3(&m_tCBuffer.vOrbitAxis);
	_vector vResOrbitAxis = XMVector3Normalize(XMVector3TransformNormal(vOrigOrbitAxis, matRot));
	XMStoreFloat3(&m_tCBuffer.vOrbitAxis, vResOrbitAxis);
}

HRESULT CVIBuffer_Point_Instance::Make_InstanceBuffer(const DESC* pDesc)
{
	// desc 변수 저장

	m_tCBuffer.iNumInstances = m_iNumInstance = pDesc->iNumInstance;
	m_tCBuffer.iParticleType = pDesc->ePType;
	m_tCBuffer.bIsTool = pDesc->isTool ? 1 : 0;
	m_tCBuffer.bIsLoop = pDesc->isLoop ? 1 : 0;
	m_tCBuffer.bUseGravity = pDesc->bGravity ? 1 : 0;
	m_tCBuffer.bUseSpin = pDesc->bSpin ? 1 : 0;
	m_tCBuffer.bUseOrbit = pDesc->bOrbit ? 1 : 0;
	m_tCBuffer.fGravity = pDesc->fGravity;
	m_tCBuffer.vCenter = pDesc->vCenter;
	m_vLocalCenter = pDesc->vCenter;
	//m_tCBuffer.vPivot = pDesc->vPivot;
	// 절대 위치 Pivot으로 저장
	XMStoreFloat3(&m_tCBuffer.vPivot, XMLoadFloat3(&pDesc->vPivot) - XMLoadFloat3(&pDesc->vCenter));
	m_tCBuffer.vOrbitAxis = pDesc->vOrbitAxis;
	m_tCBuffer.isTileLoop = pDesc->isTileLoop ? 1 : 0;
	m_tCBuffer.vTileCnt = pDesc->vTileCnt; // m_iTileX, Y인데..
	m_tCBuffer.fTileTickPerSec = pDesc->fTileTickPerSec;
	m_tCBuffer.vRange = pDesc->vRange;
	m_tCBuffer.fShrinkThreshold = pDesc->fShrinkThreshold;
	XMStoreFloat4x4(&m_tCBuffer.g_CombinedMatrix, XMMatrixIdentity());
	m_tCBuffer.isCircleRange = pDesc->isCircleRange ? 1 : 0;

#pragma region INSTANCEBUFFER
	/* [ CS ] */
	// 버퍼 생성용 정보들 전부 CS.cpp쪽으로 옮김

	m_pParticleParamDesc = new PPDESC[m_iNumInstance];
	ZeroMemory(m_pParticleParamDesc, sizeof(PPDESC) * m_iNumInstance);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		m_pParticleParamDesc[i].bFirstLoopDiscard = m_tCBuffer.bIsLoop;
		m_pParticleParamDesc[i].fMaxSpeed = pDesc->fMaxSpeed;
		m_pParticleParamDesc[i].fMinSpeed = pDesc->fMinSpeed;
		m_pParticleParamDesc[i].fSpeed = m_pGameInstance->Compute_Random(pDesc->vSpeed.x, pDesc->vSpeed.y);
		m_pParticleParamDesc[i].fAccel = m_pGameInstance->Compute_Random(pDesc->vAccel.x, pDesc->vAccel.y);
		m_pParticleParamDesc[i].fRotationSpeed = XMConvertToRadians(m_pGameInstance->Compute_Random(pDesc->vRotationSpeed.x, pDesc->vRotationSpeed.y));
		m_pParticleParamDesc[i].fOrbitSpeed = m_pGameInstance->Compute_Random(pDesc->vOrbitSpeed.x, pDesc->vOrbitSpeed.y);
		_float	fSize = m_pGameInstance->Compute_Random(pDesc->vSize.x, pDesc->vSize.y);
		m_pParticleParamDesc[i].vRight = _float4(fSize, 0.f, 0.f, 0.f);
		m_pParticleParamDesc[i].vUp = _float4(0.f, fSize, 0.f, 0.f);
		m_pParticleParamDesc[i].vLook = _float4(0.f, 0.f, fSize, 0.f);

		m_pParticleParamDesc[i].vLifeTime = _float2(
			m_pGameInstance->Compute_Random(pDesc->vLifeTime.x, pDesc->vLifeTime.y),
			0.f
		);

		/**** Position이랑 Direction 이제 최초 스폰 시에 초기화 하므로 주석처리함 ****/
		
		//m_pParticleParamDesc[i].vTranslation = _float4(
		//	m_pGameInstance->Compute_Random(pDesc->vCenter.x - pDesc->vRange.x * 0.5f, pDesc->vCenter.x + pDesc->vRange.x * 0.5f),
		//	m_pGameInstance->Compute_Random(pDesc->vCenter.y - pDesc->vRange.y * 0.5f, pDesc->vCenter.y + pDesc->vRange.y * 0.5f),
		//	m_pGameInstance->Compute_Random(pDesc->vCenter.z - pDesc->vRange.z * 0.5f, pDesc->vCenter.z + pDesc->vRange.z * 0.5f),
		//	1.f
		//);

		//m_pParticleParamDesc[i].vInitOffset = _float3(
		//	m_pParticleParamDesc[i].vTranslation.x - pDesc->vCenter.x,
		//	m_pParticleParamDesc[i].vTranslation.y - pDesc->vCenter.y,
		//	m_pParticleParamDesc[i].vTranslation.z - pDesc->vCenter.z
		//);

		//_vector vDir = {};
		//switch (m_tCBuffer.iParticleType)
		//{
		//case Engine::PTYPE_SPREAD:
		//{
		//	_vector vStart = XMLoadFloat4(&m_pParticleParamDesc[i].vTranslation);
		//	_vector vPivot = XMLoadFloat3(&pDesc->vPivot);
		//	vDir = XMVectorSetW(XMVector3Normalize(vStart - vPivot), 0.f);
		//}
		//	break;
		//case Engine::PTYPE_DIRECTIONAL:
		//{
		//	_vector vPivot = XMLoadFloat3(&pDesc->vPivot);
		//	_vector vCenter = XMLoadFloat3(&pDesc->vCenter);
		//	vDir = XMVectorSetW(XMVector3Normalize(vCenter - vPivot), 0.f);
		//	//vDir = XMVector3Normalize(XMLoadFloat4(&m_vDirection));
		//}
		//	break;
		//case Engine::PTYPE_ALLRANDOM:
		//{
		//	vDir = XMVectorSetW(XMVector3Normalize(
		//		XMVectorSet(
		//			m_pGameInstance->Compute_Random(-1.f, 1.f),
		//			m_pGameInstance->Compute_Random(-1.f, 1.f),
		//			m_pGameInstance->Compute_Random(-1.f, 1.f),
		//			0.f)), 0.f);
		//}
		//	break;
		//case Engine::PTYPE_SHRINK:
		//{
		//	_vector vStart = XMLoadFloat4(&m_pParticleParamDesc[i].vTranslation);
		//	_vector vPivot = XMLoadFloat3(&pDesc->vPivot);
		//	vDir = XMVectorSetW(XMVector3Normalize(vPivot - vStart), 0.f);
		//}
		//break;

		//default:
		//	break;
		//}

		//XMStoreFloat4(&m_pParticleParamDesc[i].vDirection, vDir);
		
		/*************************************************************************/


	}

	m_VBInstanceSubresourceData.pSysMem = m_pParticleParamDesc;

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

	// 클라일 때 프로토타입에서 삭제
	// 툴일 땐 클론에서 삭제

	if (false == m_tCBuffer.bIsTool && false == m_isCloned)
	{
		Safe_Delete_Array(m_pParticleParamDesc);
	}
	if (m_tCBuffer.bIsTool && m_isCloned)
	{	
		Safe_Delete_Array(m_pParticleParamDesc);
	}
	if (m_isCloned)
		Safe_Release(m_pParticleCS);
}
