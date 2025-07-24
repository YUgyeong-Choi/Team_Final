#include "YGObject.h"

#include "GameInstance.h"

CYGObject::CYGObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CYGObject::CYGObject(const CYGObject& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CYGObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CYGObject::Initialize(void* pArg)
{
	CGameObject::GAMEOBJECT_DESC _desc{};
	lstrcpy(_desc.szName, TEXT("YGObject"));
	_desc.fRotationPerSec = 8.f;
	_desc.fSpeedPerSec = 10.f;

	if (FAILED(__super::Initialize(&_desc))) {
		return E_FAIL;
	}

	if (FAILED(Ready_Components())) {
		return E_FAIL;
	}

	if (FAILED(Ready_Collider())) {
		return E_FAIL;
	}

	return S_OK;
}

void CYGObject::Priority_Update(_float fTimeDelta)
{
	if (m_bDead) {
		PxScene* pScene = m_pGameInstance->Get_Scene();
		if (pScene)
			pScene->removeActor(*m_pPhysXActorCom->Get_Actor());

		Safe_Release(m_pPhysXActorCom);
		m_pPhysXActorCom = nullptr;
	}

	if (m_pGameInstance->Key_Pressing(DIK_A))
	{
		m_pTransformCom->Go_Left(fTimeDelta);
	}

	if (m_pGameInstance->Key_Pressing(DIK_D))
	{
		m_pTransformCom->Go_Right(fTimeDelta);
	}
	if (m_pGameInstance->Key_Pressing(DIK_W))
	{
		m_pTransformCom->Go_Straight(fTimeDelta);
	}
	if (m_pGameInstance->Key_Pressing(DIK_S))
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}

	if (m_pGameInstance->Key_Pressing(DIK_E))
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f),  fTimeDelta * 0.1f);
	}

	if (m_pGameInstance->Key_Pressing(DIK_Q))
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f),  -fTimeDelta * 0.1f);
	}

}

void CYGObject::Update(_float fTimeDelta)
{
	// 1. 월드 행렬 가져오기
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

void CYGObject::Late_Update(_float fTimeDelta)
{
	//if (m_pGameInstance->Is_In_Frustum(m_pPhysXActor)) {
	//	
	//}
	//m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_SHADOW, this);

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CYGObject::Render()
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

		DEBUGRAY_DATA _data{};
		_data.vStartPos = m_pPhysXActorCom->Get_Actor()->getGlobalPose().p;
		_data.vDirection = PxVec3(0.f, 1.f, 0.f);
		_data.fRayLength = 10.f;
		m_pPhysXActorCom->Add_RenderRay(_data);
	}
#endif


	return S_OK;
}




HRESULT CYGObject::Bind_ShaderResources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_World4x4())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

void CYGObject::On_CollisionEnter(CGameObject* pOther)
{
}

void CYGObject::On_CollisionStay(CGameObject* pOther)
{
}

void CYGObject::On_CollisionExit(CGameObject* pOther)
{
}

void CYGObject::On_Hit(_int iDamage, _float3 HitPos)
{
}

HRESULT CYGObject::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Finoa"),TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Kinematic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CYGObject::Ready_Collider()
{
	if (m_pModelCom)
	{
		// 피오나 몸체가 2번째 메쉬라서
		_uint numVertices = m_pModelCom->Get_Mesh_NumVertices(2);
		_uint numIndices = m_pModelCom->Get_Mesh_NumIndices(2);

		vector<PxVec3> physxVertices;
		physxVertices.reserve(numVertices);

		const _float3* pVertexPositions = m_pModelCom->Get_Mesh_pVertices(2);
		for (_uint i = 0; i < numVertices; ++i)
		{
			const _float3& v = pVertexPositions[i];
			physxVertices.emplace_back(v.x, v.y, v.z);
		}

		// 2. 인덱스 복사
		const _uint* pIndices = m_pModelCom->Get_Mesh_pIndices(2);
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

		PxCapsuleGeometry  geom = m_pGameInstance->CookCapsuleGeometry(physxVertices.data(), numVertices, 1.f);
		m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(),geom, pose, m_pGameInstance->GetMaterial(L"Default"),WorldFilter::ANIMAL);

		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());
	}
	else
	{
		_tprintf(_T("%s 콜라이더 생성 실패\n"), m_szName);
	}


	return S_OK;
}

CYGObject* CYGObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CYGObject* pInstance = new CYGObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CYGObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CYGObject::Clone(void* pArg)
{
	CYGObject* pInstance = new CYGObject(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CYGObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CYGObject::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pPhysXActorCom);
}
