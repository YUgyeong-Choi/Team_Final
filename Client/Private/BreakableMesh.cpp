#include "BreakableMesh.h"
#include "GameInstance.h"
#include "Client_Calculation.h"

CBreakableMesh::CBreakableMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
{

}

CBreakableMesh::CBreakableMesh(const CBreakableMesh& Prototype)
	:CGameObject(Prototype)
{

}

HRESULT CBreakableMesh::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBreakableMesh::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	BREAKABLEMESH_DESC* pDesc = static_cast<BREAKABLEMESH_DESC*>(pArg);
	m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);

	//if (FAILED(Ready_Collider()))
	//	return E_FAIL;

	if (FAILED(Ready_PartColliders()))
		return E_FAIL;
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, 10.f, 0.f, 1.f));
	return S_OK;
}

void CBreakableMesh::Priority_Update(_float fTimeDelta)
{
	

}

void CBreakableMesh::Update(_float fTimeDelta)
{
	if (auto pActor = m_pPartPhysXActorComs[0]->Get_Actor())
	{
		PxTransform pose = pActor->getGlobalPose();
		PxMat44 mat44(pose);
		m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(reinterpret_cast<const _float4x4*>(&mat44)));
	}

	static _bool bTest = false;

	if (m_pGameInstance->Key_Down(DIK_L))
	{
		bTest = !bTest;

		CPhysXDynamicActor* pActorCom = m_pPartPhysXActorComs[0];
		PxRigidDynamic* pRigid = static_cast<PxRigidDynamic*>(pActorCom->Get_Actor());
		if (bTest == false)
		{
		//	pRigid->setLinearVelocity(PxVec3(0.0f, -5.0f, 0.0f));
			// 지금은 무게 중심이 중앙이라 떨어지면 뚝 떨어지는 느낌이 나서
			// 부딪혀서 날라갈 때 속도와 회전 속도를 줘서 처리하면 됨.(장원)
			pRigid->setAngularVelocity(PxVec3(GetRandomFloat(-2, 2), GetRandomFloat(-2, 2), GetRandomFloat(-2, 2)));
		}
		pRigid->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, bTest);
		pRigid->wakeUp();
	}

}

void CBreakableMesh::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CBreakableMesh::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (m_pGameInstance->Key_Pressing(DIK_K))
	{
		/*if (FAILED(Render_Model()))
			return E_FAIL;*/
	}
	else
	{
		if (FAILED(Render_PartModels()))
			return E_FAIL;
	}


#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderMapCollider())
	{
		/*if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
			return E_FAIL;*/

		for (CPhysXDynamicActor* pPartActor : m_pPartPhysXActorComs)
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(pPartActor)))
				return E_FAIL;
		}
	}
#endif // _DEBUG




	return S_OK;
}

HRESULT CBreakableMesh::Render_Model()
{
	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CBreakableMesh::Render_PartModels()
{
	for (CModel* pPartModel : m_pPartModelComs)
	{
		_uint		iNumMesh = pPartModel->Get_NumMeshes();
		for (_uint i = 0; i < iNumMesh; i++)
		{
			if (FAILED(pPartModel->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
				return E_FAIL;

			if (FAILED(pPartModel->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
				return E_FAIL;

			if (FAILED(pPartModel->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Begin(0)))
				return E_FAIL;

			if (FAILED(pPartModel->Render(i)))
				return E_FAIL;
		}
	}
	
	return S_OK;
}

HRESULT CBreakableMesh::Bind_ShaderResources()
{
	/* [ 월드 스페이스 넘기기 ] */
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;

	/* [ 뷰 , 투영 스페이스 넘기기 ] */
	_float4x4 ViewMatrix, ProjViewMatrix;
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjViewMatrix)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBreakableMesh::Ready_Collider()
{
	if (!m_pModelCom)
	{
		_tprintf(_T("%s 콜라이더 생성 실패\n"), m_szName);
		return E_FAIL;
	}

	_uint numMeshes = m_pModelCom->Get_NumMeshes();

	vector<PxVec3> physxVertices;
	vector<PxU32> physxIndices;

	_uint vertexBase = 0;

	for (_uint meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	{
		_uint numVertices = m_pModelCom->Get_Mesh_NumVertices(meshIndex);
		_uint numIndices = m_pModelCom->Get_Mesh_NumIndices(meshIndex);

		// --- 버텍스 ---
		const _float3* pVertexPositions = m_pModelCom->Get_Mesh_pVertices(meshIndex);
		for (_uint i = 0; i < numVertices; ++i)
		{
			const _float3& v = pVertexPositions[i];
			physxVertices.emplace_back(v.x, v.y, v.z);
		}

		// --- 인덱스 ---
		const _uint* pIndices = m_pModelCom->Get_Mesh_pIndices(meshIndex);
		for (_uint i = 0; i < numIndices; ++i)
		{
			physxIndices.push_back(static_cast<PxU32>(pIndices[i] + vertexBase));
		}

		vertexBase += numVertices;
	}

	// ──────────────────────────────
	// Transform → Scale/Rotation/Position
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_MAP;

	// ──────────────────────────────
	// Convex는 버텍스 제한 있음 (255개)
	PxConvexMeshGeometry ConvexGeom =
		m_pGameInstance->CookConvexMesh(
			physxVertices.data(),
			static_cast<PxU32>(physxVertices.size()),
			meshScale);

	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), ConvexGeom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::ENVIRONMENT_CONVEX);

	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}
HRESULT CBreakableMesh::Ready_PartColliders()
{
	// 각 파트 모델을 ConvexMesh로 쿠킹해서 Dynamic Actor 생성
	for (_uint i = 0; i < m_iPartModelCount; ++i)
	{
		CModel* pPartModel = m_pPartModelComs[i];
		if (!pPartModel)
			continue;

		// 1) 파트 버텍스 수집
		vector<PxVec3> verts;
		_uint numMeshes = pPartModel->Get_NumMeshes();
		for (_uint m = 0; m < numMeshes; ++m)
		{
			_uint numVertices = pPartModel->Get_Mesh_NumVertices(m);
			const _float3* pv = pPartModel->Get_Mesh_pVertices(m);

			for (_uint v = 0; v < numVertices; ++v)
				verts.emplace_back(pv[v].x, pv[v].y, pv[v].z);
		}

		if (verts.empty())
			continue;

		// 2) 스케일/포즈 설정 (본체 트랜스폼 기준)
		_vector S, R, T;
		XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());
		PxMeshScale scale(PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S)));
		PxTransform pose(
			PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T)),
			PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R))
		);

		// 3) ConvexMesh 쿠킹
		PxConvexMeshGeometry geom = m_pGameInstance->CookConvexMesh(verts.data(), (PxU32)verts.size(), scale);
		if (!geom.convexMesh)
			continue;

		// 4) Dynamic Actor 생성
		CPhysXDynamicActor* pActorCom = m_pPartPhysXActorComs[i];
		if (!pActorCom)
			continue;

		if (FAILED(pActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"))))
			continue;

		PxRigidDynamic* pRigid = static_cast<PxRigidDynamic*>(pActorCom->Get_Actor());
		if (!pRigid)
			continue;

		// 5) Kinematic 끄기, 중력 적용, 질량/관성, CCD, wakeUp
		pActorCom->Set_Kinematic(false);
		//pRigid->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);     // 확실히 Kinematic 끄기
		pRigid->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);        // 중력 켜기
		PxRigidBodyExt::updateMassAndInertia(*pRigid, 0.5f);              // 질량/관성 적용
		pRigid->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, false);     // CCD 활성화
		//pRigid->wakeUp();                                                  // 잠든 상태면 깨우기

		// 6) 필터 설정
		PxFilterData fd{};
		fd.word0 = WORLDFILTER::FILTER_MAP;
		fd.word1 = WORLDFILTER::FILTER_MAP;
		pActorCom->Set_ShapeFlag(true, false, true);
		pActorCom->Set_SimulationFilterData(fd);
		pActorCom->Set_QueryFilterData(fd);
		pActorCom->Set_Owner(this);
		pActorCom->Set_ColliderType(COLLIDERTYPE::ENVIRONMENT_CONVEX);
		// 7) 씬에 등록
		m_pGameInstance->Get_Scene()->addActor(*pRigid);
	}

	return S_OK;
}


HRESULT CBreakableMesh::Ready_Components(void* pArg)
{
	BREAKABLEMESH_DESC* pDesc = static_cast<BREAKABLEMESH_DESC*>(pArg);

	//파트 메쉬의 갯수 받아서 늘려놓기
	m_iPartModelCount = pDesc->iPartModelCount;
	m_pPartModelComs.resize(m_iPartModelCount);
	m_pPartPhysXActorComs.resize(m_iPartModelCount);

	wstring BaseTag = TEXT("Prototype_Component_Model_");

	wstring MainTag = BaseTag + pDesc->ModelName;

	/* Com_Model */ //본 모델
	//if (FAILED(__super::Add_Component(m_iLevelID, MainTag,
	//	TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	//	return E_FAIL;

	//파트모델
	for (_uint i = 0; i < m_iPartModelCount; ++i)
	{
		//Com_PartModel0...1...2...
		wstring ComTag = TEXT("Com_PartModel") + to_wstring(i);

		wstring PartTag = BaseTag + pDesc->PartModelNames[i];

		if (FAILED(__super::Add_Component(m_iLevelID, PartTag,
			ComTag.c_str(), reinterpret_cast<CComponent**>(&m_pPartModelComs[i]))))
			return E_FAIL;
	}

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPBRMesh"),
		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	//if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
	//	TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
	//	return E_FAIL;

	//파트 모델
	for (_uint i = 0; i < m_iPartModelCount; ++i)
	{
		////Com_PhysX_Part0...1...2...
		wstring PartPhysXTag = TEXT("Com_PhysX_Part") + to_wstring(i);

		/* For.Com_PhysX */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
			TEXT("Prototype_Component_PhysX_Dynamic"), PartPhysXTag, reinterpret_cast<CComponent**>(&m_pPartPhysXActorComs[i]))))
			return E_FAIL;
	}


	return S_OK;
}

CBreakableMesh* CBreakableMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBreakableMesh* pGameInstance = new CBreakableMesh(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CStaticMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

CGameObject* CBreakableMesh::Clone(void* pArg)
{
	CBreakableMesh* pInstance = new CBreakableMesh(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBreakableMesh");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBreakableMesh::Free()
{
	__super::Free();

	for (CModel* pPartModel : m_pPartModelComs)
	{
		Safe_Release(pPartModel);
	}

	for (CPhysXDynamicActor* pPartActor : m_pPartPhysXActorComs)
	{
		Safe_Release(pPartActor);
	}

	Safe_Release(m_pModelCom);

	Safe_Release(m_pShaderCom);

	Safe_Release(m_pPhysXActorCom);
}
