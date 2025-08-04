#include "StaticMesh.h"
#include "GameInstance.h"

CStaticMesh::CStaticMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CStaticMesh::CStaticMesh(const CStaticMesh& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CStaticMesh::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CStaticMesh::Initialize(void* pArg)
{
	CStaticMesh::STATICMESH_DESC* StaicMeshDESC = static_cast<STATICMESH_DESC*>(pArg);

	m_eLevelID = StaicMeshDESC->m_eLevelID;

	m_szMeshID = StaicMeshDESC->szMeshID;

	m_iRender = StaicMeshDESC->iRender;

	//타일링 여부
	m_bUseTiling = StaicMeshDESC->bUseTiling;
	m_vTileDensity = StaicMeshDESC->vTileDensity;

	//충돌체 종류
	m_eColliderType = StaicMeshDESC->eColliderType;

	if (FAILED(__super::Initialize(StaicMeshDESC)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(StaicMeshDESC->WorldMatrix);

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	return S_OK;
}

void CStaticMesh::Priority_Update(_float fTimeDelta)
{

}

void CStaticMesh::Update(_float fTimeDelta)
{
}

void CStaticMesh::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CStaticMesh::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
			return E_FAIL;

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	//초기화가 이상함
	if (m_eColliderType == COLLIDER_TYPE::END)
		return E_FAIL; //치명적 오류

	//충돌체 없음
	if (m_eColliderType == COLLIDER_TYPE::NONE)
		return S_OK;

	if (m_pGameInstance->Get_RenderCollider())
	{
		if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
			return E_FAIL;
	}
#endif

	return S_OK;
}

HRESULT CStaticMesh::Ready_Components(void* pArg)
{
	CStaticMesh::STATICMESH_DESC* StaicMeshDESC = static_cast<STATICMESH_DESC*>(pArg);

	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(m_eLevelID), StaicMeshDESC->szModelPrototypeTag/*_wstring(TEXT("Prototype_Component_Model_")) + m_szMeshID*/,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	if (m_eColliderType == COLLIDER_TYPE::END)
		return E_FAIL;
	if (m_eColliderType != COLLIDER_TYPE::NONE)
	{
		/* For.Com_PhysX */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
			TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CStaticMesh::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	//타일링을 사용 하는가? 인스턴스된 애들은 타일링 하기 번거롭겠다.
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bTile", &m_bUseTiling, sizeof(_bool))))
		return E_FAIL;

	if (m_bUseTiling)
	{
		if (FAILED(m_pShaderCom->Bind_RawValue("g_TileDensity", &m_vTileDensity, sizeof(_float2))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CStaticMesh::Ready_Collider()
{
	if (m_pModelCom)
	{
		_uint numVertices = m_pModelCom->Get_Mesh_NumVertices(0);
		_uint numIndices = m_pModelCom->Get_Mesh_NumIndices(0);

		vector<PxVec3> physxVertices;
		physxVertices.reserve(numVertices);

		const _float3* pVertexPositions = m_pModelCom->Get_Mesh_pVertices(0);
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

		PxFilterData filterData{};
		filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;

		if (m_eColliderType == COLLIDER_TYPE::CONVEX)
		{
#pragma region 컨벡스 메쉬
			PxConvexMeshGeometry  ConvexGeom = m_pGameInstance->CookConvexMesh(physxVertices.data(), numVertices, meshScale);
			m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), ConvexGeom, pose, m_pGameInstance->GetMaterial(L"Default"));
			//m_pPhysXActorCom->Set_Kinematic(true);
			m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

			m_pPhysXActorCom->Set_SimulationFilterData(filterData);
			m_pPhysXActorCom->Set_QueryFilterData(filterData);
			m_pPhysXActorCom->Set_Owner(this);
			m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::B); // 이걸로 색깔을 바꿀 수 있다.
			m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());
#pragma endregion
		}
		else if (m_eColliderType == COLLIDER_TYPE::TRIANGLE)
		{
#pragma region 트라이앵글 메쉬
			// 인덱스 복사
			const _uint* pIndices = m_pModelCom->Get_Mesh_pIndices(0);
			vector<PxU32> physxIndices;
			physxIndices.reserve(numIndices);

			for (_uint i = 0; i < numIndices; ++i)
				physxIndices.push_back(static_cast<PxU32>(pIndices[i]));

			PxTriangleMeshGeometry  TriangleGeom = m_pGameInstance->CookTriangleMesh(physxVertices.data(), numVertices, physxIndices.data(), numIndices / 3, meshScale);
			m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), TriangleGeom, pose, m_pGameInstance->GetMaterial(L"Default"));
			m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

			m_pPhysXActorCom->Set_SimulationFilterData(filterData);
			m_pPhysXActorCom->Set_QueryFilterData(filterData);
			m_pPhysXActorCom->Set_Owner(this);
			m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::A);
			m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());
#pragma endregion
		}
	}
	else
	{
		_tprintf(_T("%s 콜라이더 생성 실패\n"), m_szName);
	}


	return S_OK;
}

CStaticMesh* CStaticMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStaticMesh* pGameInstance = new CStaticMesh(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CStaticMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CStaticMesh::Clone(void* pArg)
{
	CStaticMesh* pGameInstance = new CStaticMesh(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CStaticMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CStaticMesh::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pTextureCom);

	Safe_Release(m_pPhysXActorCom);
}
