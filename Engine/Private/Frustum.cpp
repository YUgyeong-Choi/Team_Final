#include "Frustum.h"

#include "GameInstance.h"

CFrustum::CFrustum()
	: m_pGameInstance { CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CFrustum::Initialize()
{
	m_vOriginalPoints[0] = _float4(-1.f, 1.f, 0.f, 1.f);
	m_vOriginalPoints[1] = _float4(1.f, 1.f, 0.f, 1.f);
	m_vOriginalPoints[2] = _float4(1.f, -1.f, 0.f, 1.f);
	m_vOriginalPoints[3] = _float4(-1.f, -1.f, 0.f, 1.f);

	m_vOriginalPoints[4] = _float4(-1.f, 1.f, 1.f, 1.f);
	m_vOriginalPoints[5] = _float4(1.f, 1.f, 1.f, 1.f);
	m_vOriginalPoints[6] = _float4(1.f, -1.f, 1.f, 1.f);
	m_vOriginalPoints[7] = _float4(-1.f, -1.f, 1.f, 1.f);

	return S_OK;
}

void CFrustum::Transform_ToWorldSpace()
{
	_matrix		ProjMatrixInverse = m_pGameInstance->Get_Transform_Matrix_Inv(D3DTS::PROJ);
	_matrix		ViewMatrixInverse = m_pGameInstance->Get_Transform_Matrix_Inv(D3DTS::VIEW);

	_vector		vPoints[8];

	for (size_t i = 0; i < 8; i++)
	{
		vPoints[i] = XMVector3TransformCoord(XMLoadFloat4(&m_vOriginalPoints[i]), ProjMatrixInverse);
		XMStoreFloat4(&m_vWorldPoints[i], XMVector3TransformCoord(vPoints[i], ViewMatrixInverse));
	}

	Make_Plane(m_vWorldPoints, m_vWorldPlanes);
	
}

void CFrustum::Transform_ToLocalSpace(_fmatrix WorldMatrix)
{
	_matrix		WorldMatrixInverse = XMMatrixInverse(nullptr, WorldMatrix);
	_float4		vPoints[8];

	for (size_t i = 0; i < 8; i++)
	{
		XMStoreFloat4(&vPoints[i], XMVector3TransformCoord(XMLoadFloat4(&m_vWorldPoints[i]), WorldMatrixInverse));
	}

	Make_Plane(vPoints, m_vLocalPlanes);

}

_bool CFrustum::isIn_WorldSpace(_fvector vWorldPos, _float fRange)
{
	/*ax+ by + cz + d > 0*/

	for (size_t i = 0; i < 6; i++)
	{
		if (fRange < XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_vWorldPlanes[i]), vWorldPos)))
			return false;
	}

	return true;
}

_bool CFrustum::isIn_LocalSpace(_fvector vLocalPos, _float fRange)
{
	/*ax+ by + cz + d > 0*/

	for (size_t i = 0; i < 6; i++)
	{
		if (fRange < XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_vLocalPlanes[i]), vLocalPos)))
			return false;
	}

	return true;
}



void CFrustum::Make_Plane(const _float4* p, _float4* out)
{
	// RIGHT   (near top-right, far bottom-right, far top-right)
	XMStoreFloat4(&out[0], XMPlaneFromPoints(XMLoadFloat4(&p[1]), XMLoadFloat4(&p[6]), XMLoadFloat4(&p[5])));

	// LEFT    (near top-left, far top-left, far bottom-left)
	XMStoreFloat4(&out[1], XMPlaneFromPoints(XMLoadFloat4(&p[0]), XMLoadFloat4(&p[4]), XMLoadFloat4(&p[7])));

	// BOTTOM  (far bottom-left, far bottom-right, near bottom-right)
	XMStoreFloat4(&out[2], XMPlaneFromPoints(XMLoadFloat4(&p[7]), XMLoadFloat4(&p[6]), XMLoadFloat4(&p[2])));

	// TOP     (near top-left, far top-right, far top-left)
	XMStoreFloat4(&out[3], XMPlaneFromPoints(XMLoadFloat4(&p[0]), XMLoadFloat4(&p[5]), XMLoadFloat4(&p[4])));

	// FAR     (far top-left, far top-right, far bottom-right)
	XMStoreFloat4(&out[4], XMPlaneFromPoints(XMLoadFloat4(&p[4]), XMLoadFloat4(&p[5]), XMLoadFloat4(&p[6])));

	// NEAR    (near top-left, near bottom-right, near top-right)
	XMStoreFloat4(&out[5], XMPlaneFromPoints(XMLoadFloat4(&p[0]), XMLoadFloat4(&p[2]), XMLoadFloat4(&p[1])));
}

_bool CFrustum::isIn_PhysXAABB(CPhysXActor* pPhysXActor)
{
	if (pPhysXActor == nullptr || pPhysXActor->Get_Actor() == nullptr)
		return false;

	PxBounds3 bounds = pPhysXActor->Get_Actor()->getWorldBounds();

	PxVec3 min = bounds.minimum;
	PxVec3 max = bounds.maximum;

	// 그냥 바로 변환
	_float3 vMin = { min.x, min.y, min.z };
	_float3 vMax = { max.x, max.y, max.z };

	return Is_AABB_InFrustum(vMin, vMax);
}

_bool CFrustum::Is_AABB_InFrustum(const _float3& vMin, const _float3& vMax)
{
	for (int i = 0; i < 6; ++i)
	{
		const _vector& plane = XMLoadFloat4(&m_vWorldPlanes[i]);

		// AABB의 negative vertex 선택 (가장 바깥쪽 점)
		_float3 nVertex = {
			(plane.m128_f32[0] < 0.f) ? vMax.x : vMin.x,
			(plane.m128_f32[1] < 0.f) ? vMax.y : vMin.y,
			(plane.m128_f32[2] < 0.f) ? vMax.z : vMin.z,
		};

		// 만약 negative vertex가 평면 바깥에 있다면 완전히 바깥
		if (XMVectorGetX(XMPlaneDotCoord(plane, XMLoadFloat3(&nVertex))) < 0.f)
			return false;
	}

	return true;
}
CFrustum* CFrustum::Create()
{
	CFrustum* pInstance = new CFrustum();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CFrustum");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFrustum::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
}
