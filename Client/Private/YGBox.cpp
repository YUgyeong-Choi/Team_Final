#include "YGBox.h"

#include "GameInstance.h"
#include "PhysX_IgnoreSelfCallback.h"

CYGBox::CYGBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CYGBox::CYGBox(const CYGBox& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CYGBox::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CYGBox::Initialize(void* pArg)
{
	CGameObject::GAMEOBJECT_DESC _desc{};
	lstrcpy(_desc.szName, TEXT("YGBox"));
	_desc.fRotationPerSec = 8.f;
	_desc.fSpeedPerSec = 10.f;

	if (FAILED(__super::Initialize(&_desc))) {
		return E_FAIL;
	}

	if (FAILED(Ready_Components())) {
		return E_FAIL;
	}

	_fvector vPos{ 10.0f, 0.f, 10.0f, 1.0f };
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	m_pTransformCom->Rotation(0.f, XMConvertToRadians(180.f),0.f);

	// Tranform위치를 이동해준 뒤 콜라이더를 생성해서 맨 처음 시작할때 충돌안 됨
	if (FAILED(Ready_Collider())) {
		return E_FAIL;
	}

	return S_OK;
}

void CYGBox::Priority_Update(_float fTimeDelta)
{
}

void CYGBox::Update(_float fTimeDelta)
{

}

void CYGBox::Late_Update(_float fTimeDelta)
{
	if (m_pGameInstance->isIn_Frustum_WorldSpace(m_pTransformCom->Get_State(STATE::POSITION), 1.f)) {
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
	}

}

HRESULT CYGBox::Render()
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




HRESULT CYGBox::Bind_ShaderResources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_World4x4())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

void CYGBox::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	printf("YGBox 충돌 시작!\n");

}

void CYGBox::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CYGBox::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	printf("YGBox 충돌 종료!\n");

}

void CYGBox::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	wprintf(L"YGBox Hit: %s\n", pOther->Get_Name().c_str());
}

HRESULT CYGBox::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Finoa"),TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CYGBox::Ready_Collider()
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

		// **************************
		// 플레이어 상자 Offset 필요함
		positionVec.y += 0.5f;

		PxTransform pose(positionVec, rotationQuat);
		PxMeshScale meshScale(scaleVec);

		PxBoxGeometry  geom = m_pGameInstance->CookBoxGeometry(physxVertices.data(), numVertices, 1.f);
		m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

		PxFilterData filterData{};
		filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
		m_pPhysXActorCom->Set_SimulationFilterData(filterData);
		m_pPhysXActorCom->Set_QueryFilterData(filterData);
		m_pPhysXActorCom->Set_Owner(this);
		m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::D);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());
	}
	else
	{
		_tprintf(_T("%s 콜라이더 생성 실패\n"), m_szName);
	}


	return S_OK;
}

CYGBox* CYGBox::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CYGBox* pInstance = new CYGBox(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CYGBox");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CYGBox::Clone(void* pArg)
{
	CYGBox* pInstance = new CYGBox(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CYGBox");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CYGBox::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pPhysXActorCom);
}
