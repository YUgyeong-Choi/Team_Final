#include "YGDynamicObj.h"

#include "GameInstance.h"

CYGDynamicObj::CYGDynamicObj(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CYGDynamicObj::CYGDynamicObj(const CYGDynamicObj& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CYGDynamicObj::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CYGDynamicObj::Initialize(void* pArg)
{

	DYNAMICDATA_DESC* pDesc = static_cast<DYNAMICDATA_DESC*>(pArg);
	wcscmp(m_szName, pDesc->szName);
	m_colliderType = pDesc->colliderType;
	if (FAILED(__super::Initialize(pArg))) {
		return E_FAIL;
	}

	if (FAILED(Ready_Components())) {
		return E_FAIL;
	}

	if (m_colliderType == L"Box") {
		_fvector vPos{ -25.0f , -0.5f, -10.0f, 1.0f };
		m_pTransformCom->Set_State(STATE::POSITION, vPos);
	}
	else if (m_colliderType == L"Shpere") {
		_fvector vPos{ -15.0f , -0.5f, -10.0f, 1.0f };
		m_pTransformCom->Set_State(STATE::POSITION, vPos);
	}
	else if (m_colliderType == L"Convex") {
		_fvector vPos{ -10.0f , -0.5f, -10.0f, 1.0f };
		m_pTransformCom->Set_State(STATE::POSITION, vPos);
	}

	// Tranform위치를 이동해준 뒤 콜라이더를 생성해서 맨 처음 시작할때 충돌안 됨
	if (FAILED(Ready_Collider())) {
		return E_FAIL;
	}

	return S_OK;
}

void CYGDynamicObj::Priority_Update(_float fTimeDelta)
{

	// 1. PhysX 위치 및 회전 추출
	PxTransform physxPose = m_pPhysXActorCom->Get_Actor()->getGlobalPose();
	PxVec3 pos = physxPose.p;
	PxQuat rot = physxPose.q;

	// 2. 쿼터니언 → 행렬 변환
	XMFLOAT4 qFloat4(rot.x, rot.y, rot.z, rot.w);
	XMMATRIX rotMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&qFloat4));

	// 3. 모델 기준 오프셋 적용 (ex: Y-0.5 → local up 방향으로 올리기)
	XMVECTOR vOffset = XMVectorSet(0.f, -0.5f, 0.f, 0.f);  // 로컬 오프셋
	if (m_colliderType == L"Convex") {
		vOffset = {};
	}
	vOffset = XMVector3TransformNormal(vOffset, rotMatrix);  // 회전 적용된 offset

	XMVECTOR vPhysxPos = XMVectorSet(pos.x, pos.y, pos.z, 1.f);
	XMVECTOR vModelPos = XMVectorAdd(vPhysxPos, vOffset);

	// 4. 위치 및 회전 적용
	m_pTransformCom->Set_State(STATE::POSITION, vModelPos);
	m_pTransformCom->Set_State(STATE::RIGHT, rotMatrix.r[0]);
	m_pTransformCom->Set_State(STATE::UP, rotMatrix.r[1]);
	m_pTransformCom->Set_State(STATE::LOOK, rotMatrix.r[2]);
}

void CYGDynamicObj::Update(_float fTimeDelta)
{

}

void CYGDynamicObj::Late_Update(_float fTimeDelta)
{
	//if (m_pGameInstance->Is_In_Frustum(m_pPhysXActor)) {
	//	
	//}
	//m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_SHADOW, this);

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CYGDynamicObj::Render()
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




HRESULT CYGDynamicObj::Bind_ShaderResources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_World4x4())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

void CYGDynamicObj::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	printf("YGDynamicObj 충돌 시작!\n");
}

void CYGDynamicObj::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CYGDynamicObj::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	printf("YGDynamicObj 충돌 종료!\n");
}

void CYGDynamicObj::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	wprintf(L"YGDynamicObj Hit: %s\n", pOther->Get_Name().c_str());
}

HRESULT CYGDynamicObj::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Barrel"), TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CYGDynamicObj::Ready_Collider()
{

	if (m_pModelCom)
	{
		_uint numVertices = m_pModelCom->Get_Mesh_NumVertices(0);

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

		if (m_colliderType == L"Box") {
			PxBoxGeometry  geom = m_pGameInstance->CookBoxGeometry(physxVertices.data(), numVertices, 1.f);
			m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
		}
		else if (m_colliderType == L"Shpere") {
			PxSphereGeometry  geom = m_pGameInstance->CookSphereGeometry(physxVertices.data(), numVertices, 1.f);
			m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
		}
		else if (m_colliderType == L"Convex") {
			PxConvexMeshGeometry  geom = m_pGameInstance->CookConvexMesh(physxVertices.data(), numVertices, meshScale);
			m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
		}

		m_pPhysXActorCom->Set_ShapeFlag(true, false, false);

		PxFilterData filterData{};
		filterData.word0 = WORLDFILTER::FILTER_DYNAMICOBJ;
		filterData.word1 = WORLDFILTER::FILTER_MONSTERBODY | WORLDFILTER::FILTER_PLAYERBODY;
		m_pPhysXActorCom->Set_SimulationFilterData(filterData);
		m_pPhysXActorCom->Set_QueryFilterData(filterData);
		m_pPhysXActorCom->Set_Owner(this);
		m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::E);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());
	}
	else
	{
		_tprintf(_T("%s 콜라이더 생성 실패\n"), m_szName);
	}


	return S_OK;
}

CYGDynamicObj* CYGDynamicObj::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CYGDynamicObj* pInstance = new CYGDynamicObj(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CYGDynamicObj");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CYGDynamicObj::Clone(void* pArg)
{
	CYGDynamicObj* pInstance = new CYGDynamicObj(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CYGDynamicObj");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CYGDynamicObj::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pPhysXActorCom);
}
