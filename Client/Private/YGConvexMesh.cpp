#include "YGConvexMesh.h"

#include "GameInstance.h"
#include "LockOn_Manager.h"

CYGConvexMesh::CYGConvexMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CYGConvexMesh::CYGConvexMesh(const CYGConvexMesh& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CYGConvexMesh::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CYGConvexMesh::Initialize(void* pArg)
{
	CGameObject::GAMEOBJECT_DESC _desc{};
	lstrcpy(_desc.szName, TEXT("YGBox"));
	_desc.fRotationPerSec = 8.f;
	_desc.fSpeedPerSec = 10.f;

	if (FAILED(__super::Initialize(&_desc))) {
		return E_FAIL;
	}

	if (FAILED(Ready_Components())) {
		return E_FAIL;
	}

	_fvector vPos{ 0.0f, 0.f, 10.0f, 1.0f };
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	m_pTransformCom->Rotation(0.f, XMConvertToRadians(180.f),0.f);

	// Tranform위치를 이동해준 뒤 콜라이더를 생성해서 맨 처음 시작할때 충돌안 됨
	if (FAILED(Ready_Collider())) {
		return E_FAIL;
	}

	return S_OK;
}

void CYGConvexMesh::Priority_Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_B))
	{
		ChangeColliderSize();
	}
}

void CYGConvexMesh::Update(_float fTimeDelta)
{

}

void CYGConvexMesh::Late_Update(_float fTimeDelta)
{
	//if (m_pGameInstance->Is_In_Frustum(m_pPhysXActor)) {
	//	
	//}
	//m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_SHADOW, this);

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CYGConvexMesh::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			continue;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_SPECULAR, 0))) {}

		if (FAILED(m_pShaderCom->Begin(0)))
			continue;

		if (FAILED(m_pModelCom->Render(i)))
			continue;
	}

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif


	return S_OK;
}




HRESULT CYGConvexMesh::Bind_ShaderResources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_World4x4())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

void CYGConvexMesh::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	printf("YGConvex 충돌 시작!\n");

}

void CYGConvexMesh::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CYGConvexMesh::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	printf("YGConvex 충돌 종료!\n");

}

void CYGConvexMesh::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	wprintf(L"YGConvex Hit: %s\n", pOther->Get_Name().c_str());
}

HRESULT CYGConvexMesh::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Finoa"),TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CYGConvexMesh::Ready_Collider()
{
	if (m_pModelCom)
	{
		// 피오나 몸체가 2번째 메쉬라서
		_uint numVertices = m_pModelCom->Get_Mesh_NumVertices(2);

		vector<PxVec3> physxVertices;
		physxVertices.reserve(numVertices);

		const _float3* pVertexPositions = m_pModelCom->Get_Mesh_pVertices(2);
		for (_uint i = 0; i < numVertices; ++i)
		{
			const _float3& v = pVertexPositions[i];
			physxVertices.emplace_back(v.x, v.y, v.z);
		}

		// 3. Transform에서 S, R, T 분리
		XMVECTOR S, R, T;
		XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

		// 3-1. 스케일, 회전, 위치 변환
		PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
		PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
		PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

		PxTransform pose(positionVec, rotationQuat);
		PxMeshScale meshScale(scaleVec);

		PxConvexMeshGeometry  geom = m_pGameInstance->CookConvexMesh(physxVertices.data(), numVertices,meshScale);
		m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

		PxFilterData filterData{};
		filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
		m_pPhysXActorCom->Set_SimulationFilterData(filterData);
		m_pPhysXActorCom->Set_QueryFilterData(filterData);
		m_pPhysXActorCom->Set_Owner(this);
		m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::C);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());
	}
	else
	{
		_tprintf(_T("%s 콜라이더 생성 실패\n"), m_szName);
	}


	return S_OK;
}

void CYGConvexMesh::ChangeColliderSize()
{
	if (m_pPhysXActorCom == nullptr || m_pModelCom == nullptr)
		return;

	// 기존 shape 제거
	m_pPhysXActorCom->Shape_Detach();

	// 정점 가져오기
	_uint numVertices = m_pModelCom->Get_Mesh_NumVertices(2);
	const _float3* pVertexPositions = m_pModelCom->Get_Mesh_pVertices(2);

	vector<_float3> scaledVertices(numVertices);
	for (_uint i = 0; i < numVertices; ++i)
	{
		const _float3& v = pVertexPositions[i];
		scaledVertices[i] = { v.x * 2.0f, v.y * 2.0f, v.z * 2.0f }; // 2배 스케일
	}

	// 3. Transform에서 S, R, T 분리
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 3-1. 스케일, 회전, 위치 변환
	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	// 쿠킹 및 콜라이더 재생성
	PxConvexMeshGeometry newGeom = m_pGameInstance->CookConvexMesh(reinterpret_cast<const PxVec3*>(scaledVertices.data()), numVertices, meshScale);

	PxShape* pNewShape = PxRigidActorExt::createExclusiveShape(*m_pPhysXActorCom->Get_Actor(), newGeom, *m_pGameInstance->GetMaterial(L"Default"));
	if (!pNewShape)
		return; // 실패 처리

	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

	// 필터 등 정보 재설정
	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY; 
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
}


CYGConvexMesh* CYGConvexMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CYGConvexMesh* pInstance = new CYGConvexMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CYGConvexMesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CYGConvexMesh::Clone(void* pArg)
{
	CYGConvexMesh* pInstance = new CYGConvexMesh(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CYGConvexMesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CYGConvexMesh::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pPhysXActorCom);
}
