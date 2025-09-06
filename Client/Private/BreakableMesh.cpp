//#include "BreakableMesh.h"
//#include "GameInstance.h"
//
//CBreakableMesh::CBreakableMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
//	:CGameObject(pDevice, pContext)
//	, m_pShaderCom(nullptr)
//{
//
//}
//
//CBreakableMesh::CBreakableMesh(const CBreakableMesh& Prototype)
//	:CGameObject(Prototype)
//	, m_pShaderCom(Prototype.m_pShaderCom)
//{
//
//	Safe_AddRef(m_pShaderCom);
//}
//
//HRESULT CBreakableMesh::Initialize_Prototype()
//{
//	return S_OK;
//}
//
//HRESULT CBreakableMesh::Initialize(void* pArg)
//{
//	if (FAILED(__super::Initialize(pArg)))
//		return E_FAIL;
//
//	if (FAILED(Ready_Components(pArg)))
//		return E_FAIL;
//
//	//if (FAILED(Ready_Collider()))
//	//	return E_FAIL;
//
//	return S_OK;
//}
//
//void CBreakableMesh::Priority_Update(_float fTimeDelta)
//{
//	
//
//}
//
//void CBreakableMesh::Update(_float fTimeDelta)
//{
//
//}
//
//void CBreakableMesh::Late_Update(_float fTimeDelta)
//{
//	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
//}
//
//HRESULT CBreakableMesh::Render()
//{
//	if (FAILED(Bind_ShaderResources()))
//		return E_FAIL;
//
//	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
//	for (_uint i = 0; i < iNumMesh; i++)
//	{
//		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
//		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);
//		m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0);
//
//
//		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);
//
//		if (FAILED(m_pShaderCom->Begin(0)))
//			return E_FAIL;
//
//		if (FAILED(m_pModelCom->Render(i)))
//			return E_FAIL;
//	}
//
//	if (m_pGameInstance->Get_RenderMapCollider())
//	{
//		if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
//			return E_FAIL;
//	}
//
//	return S_OK;
//}
//
//HRESULT CBreakableMesh::Bind_ShaderResources()
//{
//	/* [ 월드 스페이스 넘기기 ] */
//	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
//		return E_FAIL;
//
//	/* [ 뷰 , 투영 스페이스 넘기기 ] */
//	_float4x4 ViewMatrix, ProjViewMatrix;
//	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
//	XMStoreFloat4x4(&ProjViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));
//	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
//		return E_FAIL;
//	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjViewMatrix)))
//		return E_FAIL;
//
//	return S_OK;
//}
//
//HRESULT CBreakableMesh::Ready_Collider()
//{
//	// 3. Transform에서 S, R, T 분리
//	XMVECTOR S, R, T;
//	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());
//
//	// 3-1. 스케일, 회전, 위치 변환
//	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
//	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
//	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
//
//	PxTransform pose(positionVec, rotationQuat);
//	PxMeshScale meshScale(scaleVec);
//
//	PxVec3 halfExtents = {};
//
//	/*if (pArg != nullptr)
//	{
//		halfExtents = VectorToPxVec3(XMLoadFloat3(&pDesc->vExtent));
//	}
//	else
//	{
//		halfExtents = VectorToPxVec3(XMLoadFloat3(&m_vHalfExtents));
//	}*/
//
//	_float3 vHalf = _float3(0.5f, 0.5f, 0.5f);
//
//	halfExtents = VectorToPxVec3(XMLoadFloat3(&vHalf));
//	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
//	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
//	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);
//
//	PxFilterData filterData{};
//	filterData.word0 = WORLDFILTER::FILTER_PLAYERBODY;
//	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
//	m_pPhysXActorCom->Set_QueryFilterData(filterData);
//	m_pPhysXActorCom->Set_Owner(this);
//	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::A);
//	//m_pPhysXActorCom->Set_Kinematic(true);
//	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());
//
//	return S_OK;
//}
//
//HRESULT CBreakableMesh::Ready_Components(void* pArg)
//{
//	/* Com_Model */
//	if (FAILED(__super::Add_Component(m_iLevelID, _wstring(TEXT("Prototype_Component_Model_Stargazer_Destroyed")),
//		TEXT("Com_Model_Destroyed"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(STARGAZER_STATE::DESTROYED)]))))
//		return E_FAIL;
//
//	if (FAILED(__super::Add_Component(m_iLevelID, _wstring(TEXT("Prototype_Component_Model_Stargazer")),
//		TEXT("Com_Model_Functional"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(STARGAZER_STATE::FUNCTIONAL)]))))
//		return E_FAIL;
//
//	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
//		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
//		return E_FAIL;
//
//	for (_uint i = 0; i < ENUM_CLASS(STARGAZER_STATE::END); ++i)
//	{
//		m_pAnimator[i] = CAnimator::Create(m_pDevice, m_pContext);
//		if (nullptr == m_pAnimator)
//			return E_FAIL;
//
//		if (FAILED(m_pAnimator[i]->Initialize(m_pModelCom[i])))
//			return E_FAIL;
//	}
//
//	/* For.Com_PhysX */
//	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
//		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
//		return E_FAIL;
//
//
//	return S_OK;
//}
//
//CBreakableMesh* CBreakableMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
//{
//	CBreakableMesh* pGameInstance = new CBreakableMesh(pDevice, pContext);
//
//	if (FAILED(pGameInstance->Initialize_Prototype()))
//	{
//		MSG_BOX("Failed to Create : CStaticMesh");
//		Safe_Release(pGameInstance);
//	}
//
//	return pGameInstance;
//}
//
//CGameObject* CBreakableMesh::Clone(void* pArg)
//{
//	CBreakableMesh* pInstance = new CBreakableMesh(*this);
//	if (FAILED(pInstance->Initialize(pArg)))
//	{
//		MSG_BOX("Failed to Cloned : CBreakableMesh");
//		Safe_Release(pInstance);
//	}
//	return pInstance;
//}
//
//void CBreakableMesh::Free()
//{
//	__super::Free();
//
//	for (_uint i = 0; i < ENUM_CLASS(STARGAZER_STATE::END); ++i)
//	{
//		Safe_Release(m_pModelCom[i]);
//		Safe_Release(m_pAnimator[i]);
//	}
//
//	Safe_Release(m_pPlayer);
//	Safe_Release(m_pShaderCom);
//
//	Safe_Release(m_pScript);
//	Safe_Release(m_pPhysXActorCom);
//}
