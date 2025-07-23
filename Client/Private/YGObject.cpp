#include "YGObject.h"

#include "GameInstance.h"
#include "Mesh.h"

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


	if (FAILED(__super::Initialize(&_desc))) {
		return E_FAIL;
	}

	if (FAILED(Ready_Components())) {
		return E_FAIL;
	}



	//CModel* _model = nullptr;
	///* For.Com_Model */
	//if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::GAMEPLAY), m_prototypeColTag,
	//	TEXT("Com_Model2"), reinterpret_cast<CComponent**>(&_model))))
	//	return E_FAIL;

	//if (_model)
	//{
	//	_uint numVertices = _model->Get_Mesh(0)->Get_NumVertices();
	//	_uint numIndices = _model->Get_Mesh(0)->Get_NumIndices();

	//	// 1. 정점 추출 (★ 스케일 미적용)
	//	vector<PxVec3> physxVertices;
	//	physxVertices.reserve(numVertices);

	//	const _float3* pVertexPositions = _model->Get_Mesh(0)->Get_Vertices();
	//	for (_uint i = 0; i < numVertices; ++i)
	//	{
	//		const _float3& v = pVertexPositions[i];
	//		physxVertices.emplace_back(v.x, v.y, v.z);
	//	}

	//	// 2. 인덱스 복사
	//	const _uint* pIndices = _model->Get_Mesh(0)->Get_Indices();
	//	std::vector<PxU32> physxIndices;
	//	physxIndices.reserve(numIndices);

	//	for (_uint i = 0; i < numIndices; ++i)
	//		physxIndices.push_back(static_cast<PxU32>(pIndices[i]));

	//	// 3. Transform에서 S, R, T 분리
	//	XMVECTOR S, R, T;
	//	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	//	// 3-1. 스케일, 회전, 위치 변환
	//	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	//	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	//	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	//	PxTransform pose(positionVec, rotationQuat);
	//	PxMeshScale meshScale(scaleVec);

	//	m_pPhysXActor = m_pGameInstance->Create_StaticConvexMeshActor(physxVertices.data(), numVertices, pose, meshScale, WorldFilter::RESOURCES);
	//	m_pPhysXActor->Set_Owner(this);
	//	m_pPhysXActor->ReadyForDebugDraw(m_pDevice, m_pContext);
	//}
	//else
	//{
	//	_tprintf(_T("%s 콜라이더 생성 실패\n"), m_szName);
	//}

	//Safe_Release(_model);

	return S_OK;
}

void CYGObject::Priority_Update(_float fTimeDelta)
{

}

void CYGObject::Update(_float fTimeDelta)
{
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

	//if (g_bRenderDebug)
	//	m_pPhysXActor->DebugRender(m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));

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
