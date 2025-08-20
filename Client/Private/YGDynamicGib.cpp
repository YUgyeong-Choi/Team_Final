#include "YGDynamicGib.h"

#include "GameInstance.h"

CYGDynamicGib::CYGDynamicGib(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CYGDynamicGib::CYGDynamicGib(const CYGDynamicGib& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CYGDynamicGib::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CYGDynamicGib::Initialize(void* pArg)
{

	if (nullptr == pArg)
		return E_FAIL;

	GIBDATA_DESC* pDesc = static_cast<GIBDATA_DESC*>(pArg);
	wcscmp(m_szName, pDesc->szName);

	if (FAILED(__super::Initialize(pArg))) {
		return E_FAIL;
	}

	if (FAILED(Ready_Components(pDesc))) {
		return E_FAIL;
	}

	_fvector vPos{ 20.0f , 0.f, -10.0f, 1.0f};
	m_pTransformCom->Set_State(STATE::POSITION, vPos);

	// Tranform위치를 이동해준 뒤 콜라이더를 생성해서 맨 처음 시작할때 충돌안 됨
	if (FAILED(Ready_Collider())) {
		return E_FAIL;
	}

	return S_OK;
}

void CYGDynamicGib::Priority_Update(_float fTimeDelta)
{
	// 위치/회전 동기화
	PxTransform physxPose = m_pPhysXActorCom->Get_Actor()->getGlobalPose();
	_vector vPos = XMVectorSet(physxPose.p.x, physxPose.p.y, physxPose.p.z, 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vPos);

	PxQuat q = physxPose.q;
	XMFLOAT4 qFloat4(q.x, q.y, q.z, q.w);
	_matrix rotMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&qFloat4));
	m_pTransformCom->Set_State(STATE::RIGHT, rotMatrix.r[0]);
	m_pTransformCom->Set_State(STATE::UP, rotMatrix.r[1]);
	m_pTransformCom->Set_State(STATE::LOOK, rotMatrix.r[2]);


	m_fLifeTime += fTimeDelta;
	if (m_fLifeTime > 2.f) {
		if (m_isNextFrame) {
			m_isNextFrame = false;
			m_fLifeTime = 0.f;

			m_pPhysXActorCom->Set_Kinematic(false);
			PxRigidDynamic* pDynamic = static_cast<PxRigidDynamic*>(m_pPhysXActorCom->Get_Actor());
			pDynamic->setLinearVelocity(PxVec3(0, 0, 0));
			pDynamic->setAngularVelocity(PxVec3(0, 0, 0));
			pDynamic->wakeUp();

		}else{
			m_pPhysXActorCom->Set_Kinematic(true);

			// 위치 초기화
			_fvector vPos{ 20.0f, 0.f, -10.0f, 1.0f };
			m_pTransformCom->Set_State(STATE::POSITION, vPos);
			m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSet(1.f, 0.f, 0.f, 0.f));
			m_pTransformCom->Set_State(STATE::UP, XMVectorSet(0.f, 1.f, 0.f, 0.f));
			m_pTransformCom->Set_State(STATE::LOOK, XMVectorSet(0.f, 0.f, 1.f, 0.f));

			// PhysX 액터에도 동기화
			PxVec3 pxPos = PxVec3(XMVectorGetX(vPos), XMVectorGetY(vPos), XMVectorGetZ(vPos));
			PxQuat pxRot = PxQuat(PxIdentity); // 필요 시 쿼터니언 회전값도 설정
			PxTransform pxTransform(pxPos, pxRot);
			m_pPhysXActorCom->Set_Transform(pxTransform);

			m_isNextFrame = true;
		}
	}
}

void CYGDynamicGib::Update(_float fTimeDelta)
{

}

void CYGDynamicGib::Late_Update(_float fTimeDelta)
{
	//if (m_pGameInstance->Is_In_Frustum(m_pPhysXActor)) {
	//	
	//}
	//m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_SHADOW, this);

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CYGDynamicGib::Render()
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




HRESULT CYGDynamicGib::Bind_ShaderResources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_World4x4())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

void CYGDynamicGib::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	printf("YGDynamicGib 충돌 시작!\n");
}

void CYGDynamicGib::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CYGDynamicGib::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	printf("YGDynamicGib 충돌 종료!\n");
}

void CYGDynamicGib::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	wprintf(L"YGDynamicGib Hit: %s\n", pOther->Get_Name().c_str());
}

HRESULT CYGDynamicGib::Ready_Components(GIBDATA_DESC* desc)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YG), desc->protoTag,TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CYGDynamicGib::Ready_Collider()
{

	if (m_pModelCom)
	{
		// 피오나 몸체가 2번째 메쉬라서
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

		PxBoxGeometry  geom = m_pGameInstance->CookBoxGeometry(physxVertices.data(), numVertices, 0.5f);
		m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorCom->Set_ShapeFlag(true, false, false);

		PxFilterData filterData{};
		filterData.word0 = WORLDFILTER::FILTER_EFFECTGIB;
		filterData.word1 = WORLDFILTER::FILTER_MONSTERBODY | WORLDFILTER::FILTER_EFFECTGIB;
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

CYGDynamicGib* CYGDynamicGib::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CYGDynamicGib* pInstance = new CYGDynamicGib(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CYGDynamicGib");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CYGDynamicGib::Clone(void* pArg)
{
	CYGDynamicGib* pInstance = new CYGDynamicGib(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CYGDynamicGib");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CYGDynamicGib::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pPhysXActorCom);
}
