#include "Cell.h"
#include "GameInstance.h"

CCell::CCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CCell::Initialize(const _float3* pPoints, _int iIndex)
{
	m_iIndex = iIndex;

	memcpy(m_vPoints, pPoints, sizeof(_float3) * POINT_END);

	_vector		vLine = XMVectorSet(0.f, 0.f, 0.f, 0.f);

	vLine = XMLoadFloat3(&m_vPoints[POINT_B]) - XMLoadFloat3(&m_vPoints[POINT_A]);
	m_vNormals[LINE_AB] = _float3(vLine.m128_f32[2] * -1.f, 0.f, vLine.m128_f32[0]);

	vLine = XMLoadFloat3(&m_vPoints[POINT_C]) - XMLoadFloat3(&m_vPoints[POINT_B]);
	m_vNormals[LINE_BC] = _float3(vLine.m128_f32[2] * -1.f, 0.f, vLine.m128_f32[0]);

	vLine = XMLoadFloat3(&m_vPoints[POINT_A]) - XMLoadFloat3(&m_vPoints[POINT_C]);
	m_vNormals[LINE_CA] = _float3(vLine.m128_f32[2] * -1.f, 0.f, vLine.m128_f32[0]);


#ifdef _DEBUG
	m_pVIBuffer = CVIBuffer_Cell::Create(m_pDevice, m_pContext, pPoints);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;
#endif


	return S_OK;
}

//현재 이것은 높이를 고려하지 않음
_bool CCell::isIn(_fvector vLocalPos, _int* pNeighborIndex, _float* pDist)
{
	for (size_t i = 0; i < LINE_END; i++)
	{
		_vector		vDir = vLocalPos - XMLoadFloat3(&m_vPoints[i]);

		if (0 < XMVectorGetX(XMVector3Dot(XMVector3Normalize(vDir), XMVector3Normalize(XMLoadFloat3(&m_vNormals[i])))))
		{
			if(pNeighborIndex)
				*pNeighborIndex = m_iNeighborIndices[i];

			return false;
		}
			
	}
	//거리를 반환 받을 변수가 전달 되었다면
	//pOut으로 삼각형 중점의 거리와, 던진 로컬포즈의 거리를 구해서 던져주자
	if (pDist)
	{
		_float3 vCentroid;//삼각형의 중점

		vCentroid.x = (m_vPoints[0].x + m_vPoints[1].x + m_vPoints[2].x) / 3.f;
		vCentroid.y = (m_vPoints[0].y + m_vPoints[1].y + m_vPoints[2].y) / 3.f;
		vCentroid.z = (m_vPoints[0].z + m_vPoints[1].z + m_vPoints[2].z) / 3.f;

		*pDist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&vCentroid) - vLocalPos));
	}

	return true;
}

NavigationEdge* CCell::FindEdge(_fvector vPosition)
{
	_float fMaxDot = -FLT_MAX;
	_int iBestIndex = -1;

	for (size_t i = 0; i < LINE_END; i++)
	{
		_vector vDir = vPosition - XMLoadFloat3(&m_vPoints[i]);
		_float fDot = XMVectorGetX(XMVector3Dot(XMVector3Normalize(vDir), XMLoadFloat3(&m_vNormals[i])));

		if (fDot > 0.f && fDot > fMaxDot)
		{
			fMaxDot = fDot;
			iBestIndex = (_int)i;
		}
	}

	if (iBestIndex != -1)
	{
		// 침범한 Edge 중 가장 깊은 것 하나만 기준으로 반환
		m_LastEdge.vDir = _vector{ -m_vNormals[iBestIndex].z, 0.f, m_vNormals[iBestIndex].x, 0.f };
		m_LastEdge.vNormal = XMLoadFloat3(&m_vNormals[iBestIndex]);

		return &m_LastEdge;
	}

	return nullptr;
}

_bool CCell::Compare(_fvector vSour, _fvector vDest)
{
	/*XMVectorEqual(vSour, vDest);*/
	if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_A]), vSour))
	{
		if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_B]), vDest))
			return true;
		if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_C]), vDest))
			return true;		
	}

	if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_B]), vSour))
	{
		if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_C]), vDest))
			return true;
		if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_A]), vDest))
			return true;
	}

	if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_C]), vSour))
	{
		if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_A]), vDest))
			return true;
		if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_B]), vDest))
			return true;
	}

	return false;
}

_float CCell::Compute_Height(_fvector vLocalPos)
{
	_vector		vPlane = XMPlaneFromPoints(XMLoadFloat3(&m_vPoints[POINT_A]), XMLoadFloat3(&m_vPoints[POINT_B]), XMLoadFloat3(&m_vPoints[POINT_C]));

	return (-vPlane.m128_f32[0] * vLocalPos.m128_f32[0] - vPlane.m128_f32[2] * vLocalPos.m128_f32[2] - vPlane.m128_f32[3]) / vPlane.m128_f32[1];

	// y = (-ax - cz - d) / b


	
}

#ifdef _DEBUG
HRESULT CCell::Render()
{
	m_pVIBuffer->Bind_Buffers();
	m_pVIBuffer->Render();

	return S_OK;
}
#endif

CCell* CCell::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _float3* pPoints, _int iIndex)
{
	CCell* pInstance = new CCell(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pPoints, iIndex)))
	{
		MSG_BOX("Failed to Created : CCell");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CCell::Free()
{
	__super::Free();

#ifdef _DEBUG
	Safe_Release(m_pVIBuffer);
#endif
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
