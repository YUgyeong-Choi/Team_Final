#include "PBRMesh.h"
#include "GameInstance.h"

CPBRMesh::CPBRMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CPBRMesh::CPBRMesh(const CPBRMesh& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CPBRMesh::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CPBRMesh::Initialize(void* pArg)
{
	CPBRMesh::STATICMESH_DESC* StaicMeshDESC = static_cast<STATICMESH_DESC*>(pArg);

	m_eMeshLevelID = StaicMeshDESC->m_eMeshLevelID;
	m_szMeshID = StaicMeshDESC->szMeshID;
	m_iRender = StaicMeshDESC->iRender;

	StaicMeshDESC->fSpeedPerSec = 0.f;
	StaicMeshDESC->fRotationPerSec = 0.f;

	if (FAILED(__super::Initialize(StaicMeshDESC)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(StaicMeshDESC->WorldMatrix);

	m_pTransformCom->Set_State(STATE::POSITION, 
		XMVectorSet(
			StaicMeshDESC->InitPos.x,
			StaicMeshDESC->InitPos.y,
			StaicMeshDESC->InitPos.z,
			1.f));

	m_pTransformCom->SetUp_Scale(
		StaicMeshDESC->InitScale.x,
		StaicMeshDESC->InitScale.y,
		StaicMeshDESC->InitScale.z
	);

	// Tranform위치를 이동해준 뒤 콜라이더를 생성해서 맨 처음 시작할때 충돌안 됨
	if (FAILED(Ready_Collider())) {
		return E_FAIL;
	}

	return S_OK;
}

void CPBRMesh::Priority_Update(_float fTimeDelta)
{
	if(KEY_DOWN(DIK_F7))
		Toggleummy();
}

void CPBRMesh::Update(_float fTimeDelta)
{
}

void CPBRMesh::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CPBRMesh::Render()
{
	if (!m_bDummyShow)
	{
		if (m_szMeshID == TEXT("Train") || m_szMeshID == TEXT("Station"))
			return S_OK; // 더미 오브젝트는 렌더링하지 않음
	}

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
		{
			if (!m_bDoOnce)
			{
				/* Com_Texture */
				if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Texture_DefaultARM")),
					TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
					return E_FAIL;

				if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_ARMTexture", 0)))
					return E_FAIL;
				m_bDoOnce = true;
			}
		}

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif
	return S_OK;
}


HRESULT CPBRMesh::Render_Shadow()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	SetCascadeShadow();
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Light_ViewMatrix(m_eShadow))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Light_ProjMatrix(m_eShadow))))
		return E_FAIL;

	_int iCascadeCount = ENUM_CLASS(m_eShadow);
	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		switch (iCascadeCount)
		{
		case 0: m_pShaderCom->Begin(2); break;
		case 1: m_pShaderCom->Begin(3); break;
		case 2: m_pShaderCom->Begin(4); break;
		}

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CPBRMesh::SetCascadeShadow()
{
	if (m_fViewZ < 20.f)
		m_eShadow = SHADOW::SHADOWA;
	else if (m_fViewZ < 40.f)
		m_eShadow = SHADOW::SHADOWB;
	else
		m_eShadow = SHADOW::SHADOWC;
}

void CPBRMesh::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	printf("CPBRMesh 충돌 시작!\n");
}

void CPBRMesh::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CPBRMesh::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	printf("CPBRMesh 충돌 종료!\n");
}

void CPBRMesh::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
//	wprintf(L"CPBRMesh Hit: %s\n", pOther->Get_Name().c_str());
}

HRESULT CPBRMesh::Ready_Components(void* pArg)
{
	CPBRMesh::STATICMESH_DESC* StaicMeshDESC = static_cast<STATICMESH_DESC*>(pArg);

	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(m_eMeshLevelID), _wstring(TEXT("Prototype_Component_Model_")) + m_szMeshID,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CPBRMesh::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

HRESULT CPBRMesh::Ready_Collider()
{
	if (m_pModelCom)
	{
		// 피오나 몸체가 2번째 메쉬라서
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

		// 2. 인덱스 복사
		const _uint* pIndices = m_pModelCom->Get_Mesh_pIndices(0);
		vector<PxU32> physxIndices;
		physxIndices.reserve(numIndices);

		for (_uint i = 0; i < numIndices; ++i)
			physxIndices.push_back(static_cast<PxU32>(pIndices[i]));

		// 3. Transform에서 S, R, T 분리
		XMVECTOR S, R, T;
		XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

		// 3-1. 스케일, 회전, 위치 변환
		PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
		PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
		PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

		PxTransform pose(positionVec, rotationQuat);
		PxMeshScale meshScale(scaleVec);

		PxTriangleMeshGeometry  geom = m_pGameInstance->CookTriangleMesh(physxVertices.data(), numVertices, physxIndices.data(), numIndices / 3, meshScale);
		m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

		PxFilterData filterData{};
		filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
		m_pPhysXActorCom->Set_SimulationFilterData(filterData);
		m_pPhysXActorCom->Set_QueryFilterData(filterData);
		m_pPhysXActorCom->Set_Owner(this);
		m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::B);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());
	}
	else
	{
		_tprintf(_T("%s 콜라이더 생성 실패\n"), m_szName);
	}


	return S_OK;
}

CPBRMesh* CPBRMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPBRMesh* pGameInstance = new CPBRMesh(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CPBRMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CPBRMesh::Clone(void* pArg)
{
	CPBRMesh* pGameInstance = new CPBRMesh(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CPBRMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CPBRMesh::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pPhysXActorCom);
}
