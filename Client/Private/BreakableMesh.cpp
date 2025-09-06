#include "BreakableMesh.h"
#include "GameInstance.h"

CBreakableMesh::CBreakableMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CDynamicMesh(pDevice, pContext)
{

}

CBreakableMesh::CBreakableMesh(const CBreakableMesh& Prototype)
	:CDynamicMesh(Prototype)
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

	//if (FAILED(Ready_Collider()))
	//	return E_FAIL;

	return S_OK;
}

void CBreakableMesh::Priority_Update(_float fTimeDelta)
{
	

}

void CBreakableMesh::Update(_float fTimeDelta)
{

}

void CBreakableMesh::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CBreakableMesh::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(Render_Model()))
		return E_FAIL;

	if (FAILED(Render_PartModels()))
		return E_FAIL;

	//if (m_pGameInstance->Get_RenderMapCollider())
	//{
	//	if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
	//		return E_FAIL;
	//}

	return S_OK;
}

HRESULT CBreakableMesh::Render_Model()
{
	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CBreakableMesh::Render_PartModels()
{
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
	// 3. Transform에서 S, R, T 분리
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 3-1. 스케일, 회전, 위치 변환
	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxVec3 halfExtents = {};

	/*if (pArg != nullptr)
	{
		halfExtents = VectorToPxVec3(XMLoadFloat3(&pDesc->vExtent));
	}
	else
	{
		halfExtents = VectorToPxVec3(XMLoadFloat3(&m_vHalfExtents));
	}*/

	_float3 vHalf = _float3(0.5f, 0.5f, 0.5f);

	halfExtents = VectorToPxVec3(XMLoadFloat3(&vHalf));
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_PLAYERBODY;
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::A);
	//m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

HRESULT CBreakableMesh::Ready_Components(void* pArg)
{
	BREAKABLEMESH_DESC* pDesc = static_cast<BREAKABLEMESH_DESC*>(pArg);

	//파트 메쉬의 갯수 받아서 늘려놓기
	m_iPartModelCount = pDesc->iPartModelCount;
	m_pPartModelComs.resize(m_iPartModelCount);

	wstring BaseTag = TEXT("Prototype_Component_Model_");

	//wstring MainTag = BaseTag + pDesc->ModelName;

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

	Safe_Release(m_pModelCom);

	Safe_Release(m_pShaderCom);

	Safe_Release(m_pPhysXActorCom);
}
