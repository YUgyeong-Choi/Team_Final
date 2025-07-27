#include "YGCapsule.h"

#include "GameInstance.h"
#include "PhysX_IgnoreSelfCallback.h"

CYGCapsule::CYGCapsule(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CYGCapsule::CYGCapsule(const CYGCapsule& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CYGCapsule::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CYGCapsule::Initialize(void* pArg)
{
	CGameObject::GAMEOBJECT_DESC _desc{};
	lstrcpy(_desc.szName, TEXT("YGCapsule"));
	_desc.fRotationPerSec = 8.f;
	_desc.fSpeedPerSec = 10.f;

	if (FAILED(__super::Initialize(&_desc))) {
		return E_FAIL;
	}

	if (FAILED(Ready_Components())) {
		return E_FAIL;
	}

	_fvector vPos{-20.0f, 0.f, 10.0f, 1.0f };
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	m_pTransformCom->Rotation(0.f, XMConvertToRadians(180.f), 0.f);

	// Tranform위치를 이동해준 뒤 콜라이더를 생성해서 맨 처음 시작할때 충돌안 됨
	if (FAILED(Ready_Collider())) {
		return E_FAIL;
	}

	return S_OK;
}

void CYGCapsule::Priority_Update(_float fTimeDelta)
{
	if (m_bDead) {
		PxScene* pScene = m_pGameInstance->Get_Scene();
		if (pScene)
			pScene->removeActor(*m_pPhysXActorCom->Get_Actor());

		Safe_Release(m_pPhysXActorCom);
		m_pPhysXActorCom = nullptr;
	}

	if (m_pGameInstance->Key_Down(DIK_R))
	{

		m_pPhysXActorCom->Set_ColliderType(static_cast<COLLIDERTYPE>(rand() % 5 + 2));
	}

}

void CYGCapsule::Update(_float fTimeDelta)
{
	Update_ColliderPos();
	Ray();
}

void CYGCapsule::Late_Update(_float fTimeDelta)
{
	//if (m_pGameInstance->Is_In_Frustum(m_pPhysXActor)) {
	//	
	//}
	//m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_SHADOW, this);

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CYGCapsule::Render()
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




HRESULT CYGCapsule::Bind_ShaderResources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_World4x4())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

void CYGCapsule::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	printf("YGCapsule 충돌 시작!\n");

}

void CYGCapsule::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CYGCapsule::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	printf("YGCapsule 충돌 종료!\n");

}

void CYGCapsule::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	wprintf(L"YGCapsule Hit: %s\n", pOther->Get_Name().c_str());
}

HRESULT CYGCapsule::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Finoa"),TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CYGCapsule::Ready_Collider()
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

		PxCapsuleGeometry  geom = m_pGameInstance->CookCapsuleGeometry(physxVertices.data(), numVertices, 1.f);
		m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

		PxFilterData filterData{};
		filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
		m_pPhysXActorCom->Set_SimulationFilterData(filterData);
		m_pPhysXActorCom->Set_QueryFilterData(filterData);
		m_pPhysXActorCom->Set_Owner(this);
		m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::A);
		m_pPhysXActorCom->Set_Kinematic(true);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());
	}
	else
	{
		_tprintf(_T("%s 콜라이더 생성 실패\n"), m_szName);
	}


	return S_OK;
}

void CYGCapsule::Update_ColliderPos()
{	// 1. 월드 행렬 가져오기
	_matrix worldMatrix = m_pTransformCom->Get_WorldMatrix();

	// 2. 위치 추출
	_float4 vPos;
	XMStoreFloat4(&vPos, worldMatrix.r[3]);

	PxVec3 pos(vPos.x, vPos.y, vPos.z);
	pos.y += 0.5f;

	XMVECTOR boneQuat = XMQuaternionRotationMatrix(worldMatrix);
	XMFLOAT4 fQuat;
	XMStoreFloat4(&fQuat, boneQuat);
	PxQuat rot = PxQuat(fQuat.x, fQuat.y, fQuat.z, fQuat.w);

	// 4. PhysX Transform 적용
	m_pPhysXActorCom->Set_Transform(PxTransform(pos, rot));
}

void CYGCapsule::Ray()
{
	PxVec3 origin = m_pPhysXActorCom->Get_Actor()->getGlobalPose().p;
	XMFLOAT3 fLook;
	XMStoreFloat3(&fLook, m_pTransformCom->Get_State(STATE::LOOK));
	PxVec3 direction = PxVec3(fLook.x, fLook.y, fLook.z);
	direction.normalize();
	_float fRayLength = 10.f;

	PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
	PxRaycastBuffer hit;
	PxQueryFilterData filterData;
	filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	CIgnoreSelfCallback callback(m_pPhysXActorCom->Get_Actor());

	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, fRayLength, hit, hitFlags, filterData, &callback))
	{
		if (hit.hasBlock)
		{
			PxRigidActor* hitActor = hit.block.actor;

			//  자기 자신이면 무시
			if (hitActor == m_pPhysXActorCom->Get_Actor())
			{
				printf(" Ray hit myself  skipping\n");
				return;
			}

			PxVec3 hitPos = hit.block.position;
			PxVec3 hitNormal = hit.block.normal;

			CPhysXActor* pHitActor = static_cast<CPhysXActor*>(hitActor->userData);
			pHitActor->Get_Owner()->On_Hit(this,m_pPhysXActorCom->Get_ColliderType());

			printf("Ray충돌 했다!\n");
			printf("RayHitPos X: %f, Y: %f, Z: %f\n", hitPos.x, hitPos.y, hitPos.z);
			printf("RayHitNormal X: %f, Y: %f, Z: %f\n", hitNormal.x, hitNormal.y, hitNormal.z);
			m_bRayHit = true;
			m_vHitPos = hitPos;
			// 저기 hit.block.여기에 뭐 faceIndex, U, V 다양하게 있으니 궁금하면 보세여.. 
		}
	}

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		DEBUGRAY_DATA _data{};
		_data.vStartPos = m_pPhysXActorCom->Get_Actor()->getGlobalPose().p;
		XMFLOAT3 fLook;
		XMStoreFloat3(&fLook, m_pTransformCom->Get_State(STATE::LOOK));
		_data.vDirection = PxVec3(fLook.x, fLook.y, fLook.z);
		_data.fRayLength = 10.f;
		_data.bIsHit = m_bRayHit;
		_data.vHitPos = m_vHitPos;
		m_pPhysXActorCom->Add_RenderRay(_data);

		m_bRayHit = false;
		m_vHitPos = {};
	}
#endif

}

CYGCapsule* CYGCapsule::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CYGCapsule* pInstance = new CYGCapsule(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CYGCapsule");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CYGCapsule::Clone(void* pArg)
{
	CYGCapsule* pInstance = new CYGCapsule(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CYGCapsule");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CYGCapsule::Free()
{
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

	if (m_pPhysXActorCom) 
	{
		PxScene* pScene = m_pGameInstance->Get_Scene();
		if (pScene)
			pScene->removeActor(*m_pPhysXActorCom->Get_Actor());
	}
	
	Safe_Release(m_pPhysXActorCom);

	__super::Free();
}
