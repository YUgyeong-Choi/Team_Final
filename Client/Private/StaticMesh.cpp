#include "StaticMesh.h"
#include "GameInstance.h"



CStaticMesh::CStaticMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CStaticMesh::CStaticMesh(const CStaticMesh& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CStaticMesh::Initialize_Prototype()
{
	/* �ܺ� �����ͺ��̽��� ���ؼ� ���� ä���. */

	return S_OK;
}

HRESULT CStaticMesh::Initialize(void* pArg)
{
	CStaticMesh::STATICMESH_DESC* StaicMeshDESC = static_cast<STATICMESH_DESC*>(pArg);

	m_bUseOctoTree = StaicMeshDESC->bUseOctoTree;

	m_strMeshName = StaicMeshDESC->szModelPrototypeTag;

	m_iRender = StaicMeshDESC->iRender;

	//Ÿ�ϸ� ����
	m_bUseTiling = StaicMeshDESC->bUseTiling;
	m_vTileDensity = StaicMeshDESC->vTileDensity;

	//�浹ü ����
	m_eColliderType = StaicMeshDESC->eColliderType;

	//����Ʈ ���
	m_iLightShape = StaicMeshDESC->iLightShape;

	//�ٴ��ΰ�?
	m_bIsFloor = StaicMeshDESC->bIsFloor;

	//�ø� ����
	m_bCullNone = StaicMeshDESC->bCullNone;

	//ǲ���� ����
	m_eFS_Sound = StaicMeshDESC->eFS_Sound;

	if (FAILED(__super::Initialize(StaicMeshDESC)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(StaicMeshDESC->WorldMatrix);

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	return S_OK;
}

void CStaticMesh::Priority_Update(_float fTimeDelta)
{

}

void CStaticMesh::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CStaticMesh::Late_Update(_float fTimeDelta)
{
	/* [ ����Ʈ���� ����������� ��� ����ü�� ���� ���Ѵ�. ] */
	if (!m_bUseOctoTree)
	{
		//������ ����ü �̰Ŵ�ſ�

		if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom))
		{
			m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
		}
	}

	/* [ LOD ���� ] */
	if (m_bIsLOD)
	{
		LOD_Update();
		m_bIsLOD = false;
	}
}

HRESULT CStaticMesh::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (m_bIsDissolve)
	{
		if (FAILED(m_pDissolveMap->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0)))
			return E_FAIL;

		_bool vDissolve = true;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bDissolve", &vDissolve, sizeof(_bool))))
			return E_FAIL;

		if (m_vecDissolveMeshNum.empty())
		{
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vDissolveGlowColor", &m_vDissolveGlowColor, sizeof(_float3))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolveAmount", &m_fDissolve, sizeof(_float))))
				return E_FAIL;
		}
	}
	else
	{
		_bool vDissolve = false;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bDissolve", &vDissolve, sizeof(_bool))))
			return E_FAIL;
	}

	if (m_pModelCom[ENUM_CLASS(m_eLOD)] == nullptr)
		m_eLOD = LOD::LOD0;

	_uint		iNumMesh = m_pModelCom[ENUM_CLASS(m_eLOD)]->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		_bool bIsDiffuse = true;
		_bool bIsNormal = true;
		_bool bIsARM = true;
		_bool bIsEmissive = true;

		if (m_bIsDissolve && m_vecDissolveMeshNum.size() > 0)
		{
			auto iter = find(m_vecDissolveMeshNum.begin(), m_vecDissolveMeshNum.end(), i);
			if (iter != m_vecDissolveMeshNum.end())
			{
				if (FAILED(m_pShaderCom->Bind_RawValue("g_vDissolveGlowColor", &m_vDissolveGlowColor, sizeof(_float3))))
					return E_FAIL;
				if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolveAmount", &m_fDissolve, sizeof(_float))))
					return E_FAIL;
			}
			else
			{
				_float fDissolve = 1.f;
				if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolveAmount", &fDissolve, sizeof(_float))))
					return E_FAIL;
			}
		}

		_float fEmissive = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
			return E_FAIL;

		if (FAILED(m_pModelCom[ENUM_CLASS(m_eLOD)]->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			bIsDiffuse = false;

		if (FAILED(m_pModelCom[ENUM_CLASS(m_eLOD)]->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
			bIsNormal = false;

		if (FAILED(m_pModelCom[ENUM_CLASS(m_eLOD)]->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
			bIsARM = false;

		if (FAILED(m_pModelCom[ENUM_CLASS(m_eLOD)]->Bind_Material(m_pShaderCom, "g_Emissive", i, aiTextureType_EMISSIVE, 0)))
			bIsEmissive = false;

		_bool bIsGlass = m_pModelCom[ENUM_CLASS(m_eLOD)]->HasTexture(i, aiTextureType_AMBIENT);

		/* [ ��ǻ�� , �̹̽ú�, �۷��� �� ������ �����϶� ] */
		if (!bIsDiffuse && !bIsEmissive && !bIsGlass)
			continue;

		/* [ ���� �Ž���� ���� �����϶� ] */
		if (bIsGlass)
		{
			_float fGlass = 1.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fGlass", &fGlass, sizeof(_float))))
				return E_FAIL;

			m_pShaderCom->Begin(5);
			m_pModelCom[ENUM_CLASS(m_eLOD)]->Render(i);

			continue;
		}
		else
		{
			_float fGlass = 0.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fGlass", &fGlass, sizeof(_float))))
				return E_FAIL;

			m_pShaderCom->Begin(5);
			m_pModelCom[ENUM_CLASS(m_eLOD)]->Render(i);
		}

		/* [ ARM ���� ���ٸ� �⺻�� ����϶� ] */
		if (!bIsARM)
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

		/* [ �̹̽ú� ���� �ִٸ� ����϶� ] */
		if (bIsEmissive)
		{
			SetEmissive(m_iLightShape);
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &m_fEmissive, sizeof(_float))))
				return E_FAIL;
		}

		if(m_bCullNone == false)
			m_pShaderCom->Begin(0);
		else if(m_bCullNone)
 			m_pShaderCom->Begin(7);

		m_pModelCom[ENUM_CLASS(m_eLOD)]->Render(i);
	}


#ifdef _DEBUG
	//�ʱ�ȭ�� �̻���
	if (m_eColliderType == COLLIDER_TYPE::END)
		return E_FAIL; //ġ���� ����

	//�浹ü ����
	//if (m_eColliderType == COLLIDER_TYPE::NONE)
	//	return S_OK;

	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom))
	{
		if (m_pGameInstance->Get_RenderMapCollider())
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
				return E_FAIL;
		}
	}

#endif

	return S_OK;
}

void CStaticMesh::LOD_Update()
{
	//���� ī�޶���� �Ÿ��� ����ؼ� � lod�� ������� �����Ѵ�.
	const _float4* pCamPos = m_pGameInstance->Get_CamPosition();

	_float fDistance = XMVectorGetX(XMVector3Length(m_pTransformCom->Get_State(STATE::POSITION) - XMLoadFloat4(pCamPos)));

	if (fDistance < 20.f)
		m_eLOD = LOD::LOD0;
	else if (fDistance < 50.f)
		m_eLOD = LOD::LOD1;
	else
		m_eLOD = LOD::LOD2;
}

HRESULT CStaticMesh::SetEmissive(_int LightShape)
{
	switch (LightShape)
	{
	case 1: //Station_Light_02
	{
		m_fEmissive = 1.f;
		break;
	}
	case 2: //Underdark_Lamp_01
	{
		m_fEmissive = 1.f;
		break;
	}
	case 3: //StreetLamp_02
	{
		m_fEmissive = 0.3f;
		break;
	}
	case 4: //SpotLight_01
	{
		m_fEmissive = 2.f;
		break;
	}
	case 5: //FloorLight_02
	{
		m_fEmissive = 0.5f;
		break;
	}
	case 6: //LightProp_Light_02
	{
		m_fEmissive = 1.f;
		break;
	}
	case 7: //StreetLamp_04
	{
		m_fEmissive = 3.f;
		break;
	}
	case 8: //Interior_Lamp_02
	{
		m_fEmissive = 1.f;
		break;
	}
	case 9: //StreetLamp_02
	{
		m_fEmissive = 1.f;
		break;
	}


	default:
		m_fEmissive = 1.f;
		break;
	}
	return S_OK;
}

HRESULT CStaticMesh::Add_Actor()
{
	if (m_eColliderType == COLLIDER_TYPE::END)
		return E_FAIL; //ġ���� ����


	HRESULT hr = { S_OK };
	//�浹ü�� �ִ� �͸�
	if (m_eColliderType != COLLIDER_TYPE::NONE)
	{
		//m_pGameInstance->Get_Scene()->lockWrite();
		hr = m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

		//m_pPhysXActorCom->Get_Actor()->userData = this;
		//���Ϳ� ����ƽ �޽��� �����ص���
		//m_pGameInstance->Get_Scene()->unlockWrite();
	}

	return hr;
}

void CStaticMesh::Update_ColliderPos()
{
	/*
		�������� �ٲٷ��� m_pPhysXActorConvexCom ������Ʈ�� ���� ������ �Ѵ�.
		�׷��ٰ� �Ѵ�. ���� ������ִ°ŷ� �غ���
	*/
	if (m_eColliderType == COLLIDER_TYPE::TRIANGLE)
		return;

	_matrix WorldMatrix = m_pTransformCom->Get_WorldMatrix(); //�������

	// ��� �� ������, ȸ��, ��ġ ����
	_vector vScale, vRotationQuat, vTranslation;
	XMMatrixDecompose(&vScale, &vRotationQuat, &vTranslation, WorldMatrix);

	// ��ġ ����
	_float3 vPos;
	XMStoreFloat3(&vPos, vTranslation);

	// ȸ�� ����
	_float4 vRot;
	XMStoreFloat4(&vRot, vRotationQuat);

	// PxTransform���� ����
	PxTransform physxTransform(PxVec3(vPos.x, vPos.y, vPos.z), PxQuat(vRot.x, vRot.y, vRot.z, vRot.w));
	m_pPhysXActorCom->Set_Transform(physxTransform);
}

HRESULT CStaticMesh::Ready_Components(void* pArg)
{
	CStaticMesh::STATICMESH_DESC* StaicMeshDESC = static_cast<STATICMESH_DESC*>(pArg);
	m_szMeshFullID = StaicMeshDESC->szModelPrototypeTag;

	wstring ModelPrototypeTag = StaicMeshDESC->szModelPrototypeTag;

	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	//���⼭ lod�� ������ �ε���ѳ���
	wstring strPrototypeTag_Lod1 = ModelPrototypeTag + L"_Lod1";
	wstring strPrototypeTag_Lod2 = ModelPrototypeTag + L"_Lod2";

	/* For.Com_LOD0 */
	if (FAILED(__super::Add_Component(StaicMeshDESC->iLevelID, ModelPrototypeTag,
		TEXT("Com_LOD0"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(LOD::LOD0)]))))
		return E_FAIL;

	/* For.Com_LOD1 */
	if (FAILED(__super::Add_Component(StaicMeshDESC->iLevelID, strPrototypeTag_Lod1,
		TEXT("Com_LOD1"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(LOD::LOD1)]))))
	{
		//LOD1�� ���� ���� �ִ�. ������ �׳� �Ѿ��.
	}

	/* For.Com_LOD2 */
	if (FAILED(__super::Add_Component(StaicMeshDESC->iLevelID, strPrototypeTag_Lod2,
		TEXT("Com_LOD2"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(LOD::LOD2)]))))
	{
		//LOD2�� ���� ���� �ִ�. ������ �׳� �Ѿ��.
	}

	if (m_eColliderType == COLLIDER_TYPE::END)
		return E_FAIL;

	//������ AABB �ø��� ���ؼ��� �浹�� �ʿ��� �����߰���?
	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CStaticMesh::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	//Ÿ�ϸ��� ��� �ϴ°�? �ν��Ͻ��� �ֵ��� Ÿ�ϸ� �ϱ� ���ŷӰڴ�.
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bTile", &m_bUseTiling, sizeof(_bool))))
		return E_FAIL;

	if (m_bUseTiling)
	{
		if (FAILED(m_pShaderCom->Bind_RawValue("g_TileDensity", &m_vTileDensity, sizeof(_float2))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CStaticMesh::Ready_Collider()
{
	if (!m_pModelCom)
	{
		_tprintf(_T("%s �ݶ��̴� ���� ����\n"), m_szName);
		return E_FAIL;
	}

	_uint numMeshes = m_pModelCom[ENUM_CLASS(m_eLOD)]->Get_NumMeshes();

	vector<PxVec3> physxVertices;
	vector<PxU32> physxIndices;

	_uint vertexBase = 0;

	for (_uint meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	{
		_uint numVertices = m_pModelCom[ENUM_CLASS(m_eLOD)]->Get_Mesh_NumVertices(meshIndex);
		_uint numIndices = m_pModelCom[ENUM_CLASS(m_eLOD)]->Get_Mesh_NumIndices(meshIndex);

		// --- ���ؽ� ---
		const _float3* pVertexPositions = m_pModelCom[ENUM_CLASS(m_eLOD)]->Get_Mesh_pVertices(meshIndex);
		for (_uint i = 0; i < numVertices; ++i)
		{
			const _float3& v = pVertexPositions[i];
			physxVertices.emplace_back(v.x, v.y, v.z);
		}

		// --- �ε��� ---
		const _uint* pIndices = m_pModelCom[ENUM_CLASS(m_eLOD)]->Get_Mesh_pIndices(meshIndex);
		for (_uint i = 0; i < numIndices; ++i)
		{
			physxIndices.push_back(static_cast<PxU32>(pIndices[i] + vertexBase));
		}

		vertexBase += numVertices;
	}

	// ������������������������������������������������������������
	// Transform �� Scale/Rotation/Position
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	

	PxFilterData filterData{};
	
	filterData.word0 = WORLDFILTER::FILTER_MAP; // �⺻�� ��

	// �ٴ��̸� �ٴ� �÷��� �߰�
	if (m_bIsFloor)
		filterData.word0 |= WORLDFILTER::FILTER_FLOOR;

	// �߼Ҹ� ������ ��ȿ�ϸ� FOOTSTEP �÷��� �߰�
	if (m_eFS_Sound != FOOTSTEP_SOUND::END)
		filterData.word0 |= WORLDFILTER::FILTER_FOOTSTEP;


	//filterData.word1 = WORLDFILTER::FILTER_DYNAMICOBJ;

	// ������������������������������������������������������������
	if (m_eColliderType == COLLIDER_TYPE::CONVEX || m_eColliderType == COLLIDER_TYPE::NONE)
	{
		// Convex�� ���ؽ� ���� ���� (255��)
		PxConvexMeshGeometry ConvexGeom =
			m_pGameInstance->CookConvexMesh(
				physxVertices.data(),
				static_cast<PxU32>(physxVertices.size()),
				meshScale);

		m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), ConvexGeom, pose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorCom->Set_ShapeFlag(true, false, true);
		m_pPhysXActorCom->Set_SimulationFilterData(filterData);
		m_pPhysXActorCom->Set_QueryFilterData(filterData);
		m_pPhysXActorCom->Set_Owner(this);
		m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::ENVIRONMENT_CONVEX);
	}
	else if (m_eColliderType == COLLIDER_TYPE::TRIANGLE)
	{
		PxTriangleMeshGeometry TriangleGeom =
			m_pGameInstance->CookTriangleMesh(
				physxVertices.data(),
				static_cast<PxU32>(physxVertices.size()),
				physxIndices.data(),
				static_cast<PxU32>(physxIndices.size() / 3),
				meshScale);

		m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), TriangleGeom, pose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorCom->Set_ShapeFlag(true, false, true);
		m_pPhysXActorCom->Set_SimulationFilterData(filterData);
		m_pPhysXActorCom->Set_QueryFilterData(filterData);
		m_pPhysXActorCom->Set_Owner(this);
		m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::ENVIRONMENT_TRI);
	}

	return S_OK;
}


CStaticMesh* CStaticMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStaticMesh* pGameInstance = new CStaticMesh(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CStaticMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CStaticMesh::Clone(void* pArg)
{
	CStaticMesh* pGameInstance = new CStaticMesh(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CStaticMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CStaticMesh::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	for (_uint i = 0; i < ENUM_CLASS(LOD::END); ++i)
	{
		Safe_Release(m_pModelCom[i]);
	}
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pEmissiveCom);

	Safe_Release(m_pPhysXActorCom);
}
