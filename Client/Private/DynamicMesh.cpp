#include "DynamicMesh.h"
#include "GameInstance.h"

CDynamicMesh::CDynamicMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CDynamicMesh::CDynamicMesh(const CDynamicMesh& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CDynamicMesh::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CDynamicMesh::Initialize(void* pArg)
{
	CDynamicMesh::DYNAMICMESH_DESC* StaicMeshDESC = static_cast<DYNAMICMESH_DESC*>(pArg);

	m_eMeshLevelID = StaicMeshDESC->m_eMeshLevelID;
	m_vColliderOffSet = StaicMeshDESC->vColliderOffSet;
	m_szMeshID = StaicMeshDESC->szMeshID;

	if (FAILED(__super::Initialize(StaicMeshDESC)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(StaicMeshDESC->WorldMatrix);

	if (FAILED(Ready_Collider(StaicMeshDESC)))
		return E_FAIL;

	return S_OK;
}

void CDynamicMesh::Priority_Update(_float fTimeDelta)
{
	Update_ColliderPos();
}

void CDynamicMesh::Update(_float fTimeDelta)
{
}

void CDynamicMesh::Late_Update(_float fTimeDelta)
{
	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom))
	{
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
	}
}

HRESULT CDynamicMesh::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		_float Emissive = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &Emissive, sizeof(_float))))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
		{
			return E_FAIL;
		}

		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
		{
			if (!m_bDoOnce)
			{
				/* Com_Texture */
				if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Texture_DefaultARM")),
					TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
					return E_FAIL;
				m_bDoOnce = true;
			}

			if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_ARMTexture", 0)))
				return E_FAIL;
		}
		

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG

	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom))
	{
		if (m_pGameInstance->Get_RenderCollider())
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
				return E_FAIL;
		}
	}

#endif

	return S_OK;
}

AABBBOX CDynamicMesh::GetWorldAABB() const
{
	PxBounds3 wb = m_pPhysXActorCom->Get_Actor()->getWorldBounds();
	AABBBOX worldBox{ {wb.minimum.x, wb.minimum.y, wb.minimum.z},
					  {wb.maximum.x, wb.maximum.y, wb.maximum.z} };

	return worldBox;
}

void CDynamicMesh::Update_ColliderPos()
{
	_matrix WorldMatrix = m_pTransformCom->Get_WorldMatrix(); //월드행렬

	// 행렬 → 스케일, 회전, 위치 분해
	_vector vScale, vRotationQuat, vTranslation;
	XMMatrixDecompose(&vScale, &vRotationQuat, &vTranslation, WorldMatrix);

	// 위치 추출
	_float3 vPos;
	XMStoreFloat3(&vPos, vTranslation);
	vPos.x += XMVectorGetX(m_vColliderOffSet);
	vPos.y += XMVectorGetY(m_vColliderOffSet);
	vPos.z += XMVectorGetZ(m_vColliderOffSet);

	// 회전 추출
	_float4 vRot;
	XMStoreFloat4(&vRot, vRotationQuat);

	// PxTransform으로 생성
	PxTransform physxTransform(PxVec3(vPos.x, vPos.y, vPos.z), PxQuat(vRot.x, vRot.y, vRot.z, vRot.w));
	m_pPhysXActorCom->Set_Transform(physxTransform);
}

HRESULT CDynamicMesh::Ready_Components(void* pArg)
{
	CDynamicMesh::DYNAMICMESH_DESC* StaicMeshDESC = static_cast<DYNAMICMESH_DESC*>(pArg);

	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(m_eMeshLevelID), StaicMeshDESC->szModelPrototypeTag/*_wstring(TEXT("Prototype_Component_Model_")) + m_szMeshID*/,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CDynamicMesh::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;


	return S_OK;
}

HRESULT CDynamicMesh::Ready_Collider(void* pArg)
{
	CDynamicMesh::DYNAMICMESH_DESC* StaicMeshDESC = static_cast<DYNAMICMESH_DESC*>(pArg);

	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
	positionVec += VectorToPxVec3(StaicMeshDESC->vColliderOffSet);

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxVec3 halfExtents = VectorToPxVec3(StaicMeshDESC->vColliderSize);
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	//m_pPhysXActorCom->Set_ShapeFlag(true, false, true);
	m_pPhysXActorCom->Set_ShapeFlag(false, false, false);

	PxFilterData filterData{};
	filterData.word0 = 0;// WORLDFILTER::FILTER_MAP;
	filterData.word1 = 0;// WORLDFILTER::FILTER_PLAYERBODY;
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_Kinematic(true);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::C);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

CDynamicMesh* CDynamicMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDynamicMesh* pGameInstance = new CDynamicMesh(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CDynamicMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CDynamicMesh::Clone(void* pArg)
{
	CDynamicMesh* pGameInstance = new CDynamicMesh(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CDynamicMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CDynamicMesh::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pEmissiveCom);

	Safe_Release(m_pPhysXActorCom);
}
