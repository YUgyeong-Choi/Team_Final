#include "MapToolObject.h"

#include "GameInstance.h"

#include "EffectContainer.h"
#include "Effect_Manager.h"

CMapToolObject::CMapToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{

}

CMapToolObject::CMapToolObject(const CMapToolObject& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CMapToolObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapToolObject::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	MAPTOOLOBJ_DESC* pDesc = static_cast<MAPTOOLOBJ_DESC*>(pArg);

	m_iID = pDesc->iID;
	m_ModelName = WStringToString(pDesc->szModelName);

	//월드행렬
	m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);

	//타일링 여부
	m_bUseTiling = pDesc->bUseTiling;
	m_TileDensity[0] = pDesc->vTileDensity.x;
	m_TileDensity[1] = pDesc->vTileDensity.y;

	//콜라이더 종류
	m_eColliderType = pDesc->eColliderType;

	//자체 발광 여부
	m_iLightShape = pDesc->iLightShape;

	//인스턴싱 제외 여부
	m_bNoInstancing = pDesc->bNoInstancing;

	//오브젝트 타입
	m_eObjType = pDesc->eObjType;

	//별바라기 태그
	m_eStargazerTag = pDesc->eStargazerTag;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	if (m_eColliderType == COLLIDER_TYPE::TRIANGLE)
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorTriangleCom->Get_Actor());
	else
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorConvexCom->Get_Actor());


	if (m_eObjType == OBJ_TYPE::ERGO_ITEM)
	{
		Ready_Effect();
	}

	return S_OK;
}

void CMapToolObject::Priority_Update(_float fTimeDelta)
{

}

void CMapToolObject::Update(_float fTimeDelta)
{
	//현재 카메라와의 거리를 계산해서 어떤 lod를 사용할지 결정한다.
	const _float4* pCamPos = m_pGameInstance->Get_CamPosition();

	_float fDistance = XMVectorGetX(XMVector3Length(m_pTransformCom->Get_State(STATE::POSITION) - XMLoadFloat4(pCamPos)));

	if (fDistance < 20.f)
		m_eLOD = LOD::LOD0;
	else if (fDistance < 50.f)
		m_eLOD = LOD::LOD1;
	else
		m_eLOD = LOD::LOD2;

	//Update_ColliderPos();
}

void CMapToolObject::Late_Update(_float fTimeDelta)
{
	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorConvexCom))
	{
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
	}

	//if (m_pGameInstance->isIn_Frustum_WorldSpace(m_pTransformCom->Get_State(STATE::POSITION), 1.f))
	//{
	//	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
	//}
}

HRESULT CMapToolObject::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (m_pModelCom[ENUM_CLASS(m_eLOD)] == nullptr)
	{
		m_eLOD = LOD::LOD0; // 모델이 없으면 LOD0로 설정
	}

	_uint		iNumMesh = m_pModelCom[ENUM_CLASS(m_eLOD)]->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		_float Emissive = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &Emissive, sizeof(_float))))
			return E_FAIL;

		/*_float fGlass = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fGlass", &fGlass, sizeof(_float))))
			return E_FAIL;*/

		if (FAILED(m_pModelCom[ENUM_CLASS(m_eLOD)]->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
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

		m_pModelCom[ENUM_CLASS(m_eLOD)]->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);

		m_pModelCom[ENUM_CLASS(m_eLOD)]->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		m_pShaderCom->Begin(1);

		m_pModelCom[ENUM_CLASS(m_eLOD)]->Render(i);
	}

	return S_OK;
}

// 콜라이더 위치/회전/스케일을 월드 트랜스폼에 맞게 갱신
void CMapToolObject::Update_ColliderPos()
{
	// TRIANGLE 타입은 스케일 적용 대상이 아님 → 바로 리턴
	if (m_eColliderType == COLLIDER_TYPE::TRIANGLE)
		return;

	// ----------------------------------------------------
	// 1. 월드 행렬에서 스케일, 회전, 위치 분해
	// ----------------------------------------------------
	_matrix WorldMatrix = m_pTransformCom->Get_WorldMatrix();

	_vector vScale, vRotationQuat, vTranslation;
	XMMatrixDecompose(&vScale, &vRotationQuat, &vTranslation, WorldMatrix);

	// 위치
	_float3 vPos;
	XMStoreFloat3(&vPos, vTranslation);

	// 회전 (Quaternion)
	_float4 vRot;
	XMStoreFloat4(&vRot, vRotationQuat);

	// ----------------------------------------------------
	// 2. PxTransform(위치 + 회전) 적용
	//    (PhysX Transform에는 스케일이 들어가지 않음)
	// ----------------------------------------------------
	PxTransform physxTransform(
		PxVec3(vPos.x, vPos.y, vPos.z),
		PxQuat(vRot.x, vRot.y, vRot.z, vRot.w)
	);
	m_pPhysXActorConvexCom->Set_Transform(physxTransform);

	// ----------------------------------------------------
	// 3. MeshScale 생성 (스케일 벡터 적용)
	// ----------------------------------------------------
	PxVec3 scaleVec(
		XMVectorGetX(vScale),
		XMVectorGetY(vScale),
		XMVectorGetZ(vScale)
	);
	PxMeshScale meshScale(scaleVec);

	// ----------------------------------------------------
	// 4. 기존 Shape의 ConvexMeshGeometry 가져오기
	//    (getGeometry → PxGeometryHolder → ConvexMesh 꺼내기)
	// ----------------------------------------------------
	PxGeometryHolder holder = m_pPhysXActorConvexCom->Get_Shape()->getGeometry();
	PxConvexMeshGeometry oldConvex = holder.convexMesh();

	// ----------------------------------------------------
	// 5. ConvexMeshGeometry 새로 생성 (스케일 적용)
	// ----------------------------------------------------
	PxConvexMeshGeometry convexGeom(oldConvex.convexMesh, meshScale);

	// ----------------------------------------------------
	// 6. Shape 교체 (스케일 반영된 Geometry로 갱신)
	// ----------------------------------------------------
	m_pPhysXActorConvexCom->Modify_Shape(convexGeom);
}

void CMapToolObject::Set_Collider(COLLIDER_TYPE eColliderType)
{
	if (m_eColliderType == eColliderType)
		return;

	// 현재 콜라이더 제거
	if (m_eColliderType == COLLIDER_TYPE::CONVEX || m_eColliderType == COLLIDER_TYPE::NONE)
		m_pGameInstance->Get_Scene()->removeActor(*m_pPhysXActorConvexCom->Get_Actor());
	else if (m_eColliderType == COLLIDER_TYPE::TRIANGLE)
		m_pGameInstance->Get_Scene()->removeActor(*m_pPhysXActorTriangleCom->Get_Actor());

	// 새 콜라이더 추가
	if (eColliderType == COLLIDER_TYPE::CONVEX || eColliderType == COLLIDER_TYPE::NONE)
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorConvexCom->Get_Actor());
	else if (eColliderType == COLLIDER_TYPE::TRIANGLE)
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorTriangleCom->Get_Actor());

	m_eColliderType = eColliderType;
}


HRESULT CMapToolObject::Render_Collider()
{

#ifdef _DEBUG
	//초기화가 이상함
	if (m_eColliderType == COLLIDER_TYPE::END)
		return E_FAIL; //치명적 오류

	//충돌체 없음
	if (m_eColliderType == COLLIDER_TYPE::NONE)
		return S_OK;

	if (m_pGameInstance->Get_RenderCollider())
	{
		//컨벡스 충돌체
		if (m_eColliderType == COLLIDER_TYPE::CONVEX)
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorConvexCom)))
				return E_FAIL;
		}

		//트라이앵글 충돌체
		if (m_eColliderType == COLLIDER_TYPE::TRIANGLE)
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorTriangleCom)))
				return E_FAIL;
		}
	}
#endif

	return S_OK;
}

void CMapToolObject::Undo_WorldMatrix()
{
	if (m_bCanUndo == true)
	{
		m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&m_UndoWorldMatrix));
		m_bCanUndo = false;
	}
}

HRESULT CMapToolObject::Ready_Components(void* pArg)
{
	if (pArg == nullptr)
		return E_FAIL;

	MAPTOOLOBJ_DESC* pDesc = static_cast<MAPTOOLOBJ_DESC*>(pArg);

	m_ModelPrototypeTag = pDesc->szModelPrototypeTag;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPBRMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	//여기서 lod가 있으면 로드시켜놓자
	wstring strPrototypeTag_Lod1 = m_ModelPrototypeTag + L"_Lod1";
	wstring strPrototypeTag_Lod2 = m_ModelPrototypeTag + L"_Lod2";

	/* For.Com_LOD0 */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), pDesc->szModelPrototypeTag,
		TEXT("Com_LOD0"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(LOD::LOD0)]))))
		return E_FAIL;

	/* For.Com_LOD1 */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), strPrototypeTag_Lod1,
		TEXT("Com_LOD1"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(LOD::LOD1)]))))
	{
		//LOD1이 없을 수도 있다. 없으면 그냥 넘어가자.
	}

	/* For.Com_LOD2 */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), strPrototypeTag_Lod2,
		TEXT("Com_LOD2"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(LOD::LOD2)]))))
	{
		//LOD2이 없을 수도 있다. 없으면 그냥 넘어가자.
	}


	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX_Dynamic"), reinterpret_cast<CComponent**>(&m_pPhysXActorConvexCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX_Static"), reinterpret_cast<CComponent**>(&m_pPhysXActorTriangleCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapToolObject::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	_float fID = static_cast<_float>(m_iID);
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fID", &fID, sizeof(_float))))
		return E_FAIL;

	//타일링을 사용 하는가? 인스턴스된 애들은 타일링 하기 번거롭겠다.
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bTile", &m_bUseTiling, sizeof(_bool))))
		return E_FAIL;

	if (m_bUseTiling)
	{
		if (FAILED(m_pShaderCom->Bind_RawValue("g_TileDensity", &m_TileDensity, sizeof(m_TileDensity))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMapToolObject::Ready_Collider()
{
	if (!m_pModelCom[ENUM_CLASS(LOD::LOD0)])
	{
		_tprintf(_T("%s 콜라이더 생성 실패\n"), m_szName);
		return E_FAIL;
	}

	vector<PxVec3> physxVertices;
	vector<PxU32> physxIndices;

	_uint vertexBase = 0;
	_uint numMeshes = m_pModelCom[ENUM_CLASS(LOD::LOD0)]->Get_NumMeshes();

	for (_uint meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	{
		_uint numVertices = m_pModelCom[ENUM_CLASS(LOD::LOD0)]->Get_Mesh_NumVertices(meshIndex);
		_uint numIndices = m_pModelCom[ENUM_CLASS(LOD::LOD0)]->Get_Mesh_NumIndices(meshIndex);

		// --- 버텍스 ---
		const _float3* pVertexPositions = m_pModelCom[ENUM_CLASS(LOD::LOD0)]->Get_Mesh_pVertices(meshIndex);
		for (_uint i = 0; i < numVertices; ++i)
		{
			const _float3& v = pVertexPositions[i];
			physxVertices.emplace_back(v.x, v.y, v.z);
		}

		// --- 인덱스 ---
		const _uint* pIndices = m_pModelCom[ENUM_CLASS(LOD::LOD0)]->Get_Mesh_pIndices(meshIndex);
		for (_uint i = 0; i < numIndices; ++i)
		{
			physxIndices.push_back(static_cast<PxU32>(pIndices[i] + vertexBase));
		}

		vertexBase += numVertices;
	}

	// Transform 분리
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 scaleVec(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;

	// ─────────────── Convex Mesh ───────────────
	PxConvexMeshGeometry ConvexGeom =
		m_pGameInstance->CookConvexMesh(physxVertices.data(),
			static_cast<PxU32>(physxVertices.size()),
			meshScale);

	m_pPhysXActorConvexCom->Create_Collision(
		m_pGameInstance->GetPhysics(), ConvexGeom, pose,
		m_pGameInstance->GetMaterial(L"Default"));

	m_pPhysXActorConvexCom->Set_Kinematic(true);
	m_pPhysXActorConvexCom->Set_ShapeFlag(false, false, false);
	m_pPhysXActorConvexCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorConvexCom->Set_QueryFilterData(filterData);
	m_pPhysXActorConvexCom->Set_Owner(this);
	m_pPhysXActorConvexCom->Set_ColliderType(COLLIDERTYPE::B);

	// ─────────────── Triangle Mesh ───────────────
	PxTriangleMeshGeometry TriangleGeom =
		m_pGameInstance->CookTriangleMesh(
			physxVertices.data(),
			static_cast<PxU32>(physxVertices.size()),
			physxIndices.data(),
			static_cast<PxU32>(physxIndices.size() / 3),
			meshScale);

	m_pPhysXActorTriangleCom->Create_Collision(
		m_pGameInstance->GetPhysics(), TriangleGeom, pose,
		m_pGameInstance->GetMaterial(L"Default"));

	m_pPhysXActorTriangleCom->Set_ShapeFlag(false, false, false);
	m_pPhysXActorTriangleCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorTriangleCom->Set_QueryFilterData(filterData);
	m_pPhysXActorTriangleCom->Set_Owner(this);
	m_pPhysXActorTriangleCom->Set_ColliderType(COLLIDERTYPE::A);

	// AABB 디버그 박스 출력하지 않음
	m_pPhysXActorConvexCom->Set_DrawAABB(false);
	m_pPhysXActorTriangleCom->Set_DrawAABB(false);

	return S_OK;
}



HRESULT CMapToolObject::Ready_Effect()
{
	if (m_pEffect != nullptr)
	{
		m_pEffect->Set_isActive(true);
		//m_pEffect->Set_Loop(true);
		return S_OK;
	}


	CEffectContainer::DESC desc = {};
	desc.pSocketMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixIdentity());

	m_pEffect = dynamic_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_ErgoItem_M3P1_WB"), &desc));
	if (nullptr == m_pEffect)
		MSG_BOX("이펙트 생성 실패함");

	return S_OK;
}

CMapToolObject* CMapToolObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapToolObject* pInstance = new CMapToolObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMapToolObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapToolObject::Clone(void* pArg)
{
	CMapToolObject* pInstance = new CMapToolObject(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMapToolObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapToolObject::Free()
{
	__super::Free();

	for (_uint i = 0; i < ENUM_CLASS(LOD::END); ++i)
	{
		Safe_Release(m_pModelCom[i]);
	}

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pPhysXActorConvexCom);
	Safe_Release(m_pPhysXActorTriangleCom);
	Safe_Release(m_pTextureCom);
}
