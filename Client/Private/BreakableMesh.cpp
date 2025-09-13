#include "BreakableMesh.h"

#include "Cell.h"
#include "Fuoco.h"
#include "Player.h"
#include "Navigation.h"
#include "GameInstance.h"
#include "Client_Calculation.h"

CBreakableMesh::CBreakableMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
{

}

CBreakableMesh::CBreakableMesh(const CBreakableMesh& Prototype)
	:CGameObject(Prototype)
{

}

HRESULT CBreakableMesh::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBreakableMesh::Initialize(void* pArg)
{
	BREAKABLEMESH_DESC* pDesc = static_cast<BREAKABLEMESH_DESC*>(pArg);

	//Ǫ���� ���� ��ո� ��Ŀ������ �� �޶� �̷��� ó���ع����߰ڴ�. ���ο� Ŭ���� �ı� �ʹ� ���ŷο� ��
	m_bFireEaterBossPipe = pDesc->bFireEaterBossPipe;
	
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	//��Ʈ������Ʈ �ʱ� ������� ����
	m_PartInitWorldMatrixs.resize(pDesc->iPartModelCount);
	_vector vMainPos =  m_pTransformCom->Get_State(STATE::POSITION);
	for (_uint i = 0; i < m_iPartModelCount; ++i)
	{
		m_pPartTransformComs[i]->Set_State(STATE::POSITION, vMainPos + XMLoadFloat3(&pDesc->vOffsets[i]));
		m_PartInitWorldMatrixs[i] = m_pPartTransformComs[i]->Get_World4x4();
	}

	if (FAILED(Ready_PartColliders()))
		return E_FAIL;

	//Ǫ���� ��ո� �׺� ��Ȱ��ȭ ��Ų��.
	if (m_bFireEaterBossPipe)
	{
		if (pDesc->wsNavName.empty() == false)
		{
			//�ڽ��� aabb ��ŭ�� �׺� �ε����� �����´�.
			Store_NavIndices();
		}
	}

	return S_OK;
}

void CBreakableMesh::Priority_Update(_float fTimeDelta)
{
	//Ǫ���� ��ո� �÷��̾ �����ϱ� ���� ã�´�.
	if (m_bFireEaterBossPipe)
	{
		Find_Player();
	}

	//if (m_pGameInstance->Key_Down(DIK_L))
	//{
	//	Reset();
	//}
}

void CBreakableMesh::Update(_float fTimeDelta)
{
	for (_uint i = 0; i < m_iPartModelCount; ++i)
	{
		if (auto pActor = m_pPartPhysXActorComs[i]->Get_Actor())
		{
			PxTransform pose = pActor->getGlobalPose();
			PxMat44 mat44(pose);
			m_pPartTransformComs[i]->Set_WorldMatrix(XMLoadFloat4x4(reinterpret_cast<const _float4x4*>(&mat44)));
		}
	}

	if (m_bBreakTriggered)
	{
		Break();
	}

	if (m_bIsBroken)
	{
		if (m_fTimeAcc > m_fTime_Invisible)
		{
			Invisible();
		}
		else
		{
			m_fTimeAcc += fTimeDelta;
		}
	}

}

void CBreakableMesh::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CBreakableMesh::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (m_bIsBroken == false)
	{
		if (FAILED(Render_Model()))
			return E_FAIL;
	}
	else
	{
		if (m_bInvisible == false)
		{
			if (FAILED(Render_PartModels()))
				return E_FAIL;
		}		
	}


#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderMapCollider())
	{
		if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
			return E_FAIL;

		for (CPhysXDynamicActor* pPartActor : m_pPartPhysXActorComs)
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(pPartActor)))
				return E_FAIL;
		}
	}
#endif // _DEBUG




	return S_OK;
}

void CBreakableMesh::Reset()
{
	m_bInvisible = false;
	m_fTimeAcc = 0.f;

	m_bBreakTriggered = false;
	m_bIsBroken = false;

	Set_Active_StoreCells(false);

	//�� �� ���� �ʱ�ȭ
	PxActor* pActor = m_pPhysXActorCom->Get_Actor();
	if (!pActor->getScene()) // nullptr�̸� ���� ����
	{
		m_pGameInstance->Get_Scene()->addActor(*pActor);
	}

	//��Ʈ �� ���� �ʱ�ȭ
	//��Ʈ �� ��ġ �ǵ�����
	for (_uint i = 0; i < m_iPartModelCount; ++i)
	{
		CPhysXDynamicActor* pActorCom = m_pPartPhysXActorComs[i];
		PxRigidDynamic* pRigid = static_cast<PxRigidDynamic*>(pActorCom->Get_Actor());

		if (!pRigid)
			continue;

		// 1) �ٽ� Kinematic ���·�
		m_pPartPhysXActorComs[i]->Set_Kinematic(true);


		// 2) ��ġ �ʱ�ȭ
		// m_PartInitWorldMatrixs[i] : XMFLOAT4X4 �Ǵ� XMMATRIX
		_matrix world = XMLoadFloat4x4(&m_PartInitWorldMatrixs[i]); // �Ǵ� XMLoadFloat4x4(&m_PartInitWorldMatrixs[i])
		_vector S, R, T;
		XMMatrixDecompose(&S, &R, &T, world);

		// PhysX�� Transform ����
		PxTransform pose(
			PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T)),
			PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R))
		);

		// ���� Actor ��ġ/ȸ�� �ʱ�ȭ
		pRigid->setGlobalPose(pose);

		// 3) �� �ʱ�ȭ
		/*pRigid->setLinearVelocity(PxVec3(0.f, 0.f, 0.f));
		pRigid->setAngularVelocity(PxVec3(0.f, 0.f, 0.f));
		pRigid->clearForce();*/

		// 4) �߷� ����
		pRigid->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

		// 6) Transform ������Ʈ
		m_pPartTransformComs[i]->Set_WorldMatrix(m_PartInitWorldMatrixs[i]);
	}

}

void CBreakableMesh::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (m_bFireEaterBossPipe) //Ǫ���� ����̸� Ǫ���ڿ� ���ؼ��� �μ�����
	{
		//Ǫ���ڰ� ǻ�������� ��
		//pOther->ǻ���� �� Ʈ���� Ʈ��
		if ((eColliderType == COLLIDERTYPE::MONSTER || eColliderType == COLLIDERTYPE::BOSS_WEAPON)
			&& m_bBreakTriggered == false)
		{
			if (auto pFuoco = dynamic_cast<CFuoco*>(pOther))
			{
				if (pFuoco->GetFuryState() == CBossUnit::EFuryState::Fury)
				{
					m_bBreakTriggered = true;
				}
			}
		}
	}
	else //�ƴϸ� �׳� ��ο��� �μ������Ѵ�.
	{
		if (m_bBreakTriggered == false)
		{
			m_bBreakTriggered = true;
		}
	}


}

void CBreakableMesh::Break()
{
	if (m_bIsBroken == true)
		return;

	m_bIsBroken = true;

	Set_Active_StoreCells(true);

	m_pGameInstance->Get_Scene()->removeActor(*m_pPhysXActorCom->Get_Actor());

	for (CPhysXDynamicActor* pPartActor : m_pPartPhysXActorComs)
	{
		CPhysXDynamicActor* pActorCom = pPartActor;
		PxRigidDynamic* pRigid = static_cast<PxRigidDynamic*>(pActorCom->Get_Actor());
		if (!pRigid)
			continue;

		pActorCom->Set_Kinematic(false);

		IgnorePlayerCollider(pPartActor);

		//pRigid->setLinearVelocity(PxVec3(0.0f, -5.0f, 0.0f));
		// ������ ���� �߽��� �߾��̶� �������� �� �������� ������ ����
		// �ε����� ���� �� �ӵ��� ȸ�� �ӵ��� �༭ ó���ϸ� ��.(���)
		pRigid->setAngularVelocity(PxVec3(GetRandomFloat(-5.f, 5.f), GetRandomFloat(-5.f, 5.f), GetRandomFloat(-5.f, 5.f)));
		pRigid->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !m_bIsBroken);

		PxActor* pActor = pPartActor->Get_Actor();
		if (!pActor->getScene()) // nullptr�̸� ���� ����
		{
			m_pGameInstance->Get_Scene()->addActor(*pActor);
		}

		//AddActor ���� ����ũ���ؾ��Ѵ�
		pRigid->wakeUp();
	}
}

HRESULT CBreakableMesh::Render_Model()
{
	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
			return E_FAIL;
			
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CBreakableMesh::Render_PartModels()
{
	for (_uint i = 0; i < m_iPartModelCount; ++i)
	{
		/* [ ���� �����̽� �ѱ�� ] */
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pPartTransformComs[i]->Get_WorldMatrix_Ptr())))
			return E_FAIL;

		_uint		iNumMesh = m_pPartModelComs[i]->Get_NumMeshes();
		for (_uint j = 0; j < iNumMesh; j++)
		{
			if (FAILED(m_pPartModelComs[i]->Bind_Material(m_pShaderCom, "g_DiffuseTexture", j, aiTextureType_DIFFUSE, 0)))
				return E_FAIL;

			if (FAILED(m_pPartModelComs[i]->Bind_Material(m_pShaderCom, "g_NormalTexture", j, aiTextureType_NORMALS, 0)))
				return E_FAIL;

			if (FAILED(m_pPartModelComs[i]->Bind_Material(m_pShaderCom, "g_ARMTexture", j, aiTextureType_SPECULAR, 0)))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Begin(0)))
				return E_FAIL;

			if (FAILED(m_pPartModelComs[i]->Render(j)))
				return E_FAIL;
		}
	}
	
	return S_OK;
}

HRESULT CBreakableMesh::Find_Player()
{
	if (m_pPlayer != nullptr)
		return S_OK;

	CGameObject* pObj = m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player"));

	if (nullptr != pObj)
	{
		m_pPlayer = static_cast<CPlayer*>(pObj);
		Safe_AddRef(m_pPlayer);
	}

	if (m_pPlayer == nullptr)
		return E_FAIL;

	return S_OK;
}

void CBreakableMesh::IgnorePlayerCollider(CPhysXDynamicActor* pActor)
{
	//pActor->Init_SimulationFilterData();
	if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
	{
		if (auto pController = pPlayer->Get_Controller())
		{
			pController->Add_IngoreActors(pActor->Get_Actor());
		}
	}
}

void CBreakableMesh::Invisible()
{
	if (m_bInvisible == true)
		return;

	m_bInvisible = true;

	for (CPhysXDynamicActor* pActor : m_pPartPhysXActorComs)
	{
		m_pGameInstance->Get_Scene()->removeActor(*pActor->Get_Actor());
	}
}

bool CBreakableMesh::IsPointInsideXZ(const PxVec3& point, const PxBounds3& bounds)
{
	return (point.x >= bounds.minimum.x && point.x <= bounds.maximum.x) &&
		(point.z >= bounds.minimum.z && point.z <= bounds.maximum.z);
}

void CBreakableMesh::Store_NavIndices()
{
	PxRigidActor* pActor = m_pPhysXActorCom->Get_Actor();
	if (pActor)
	{
		PxShape* pShapes[8];
		PxU32 shapeCount = pActor->getShapes(pShapes, 8);

		for (PxU32 i = 0; i < shapeCount; ++i)
		{
			PxShape* pShape = pShapes[i];

			// ���� ���� ���
			PxTransform localPose = pShape->getLocalPose();
			PxTransform actorPose = pActor->getGlobalPose();
			PxTransform worldPose = actorPose.transform(localPose);

			// Shape Geometry
			const PxGeometry& geom = pShape->getGeometry();

			PxBounds3 bounds;
			PxGeometryQuery::computeGeomBounds(bounds, geom, worldPose, 0.0f);

			vector<CCell*>& Cells = m_pNaviCom->Get_Cells();

			//�� �˻�
			for (CCell* pCell : Cells)
			{
				_bool isIn = true;

				//���� ������ ���ؼ� �˻�
				for (_uint i = 0; i < CCell::POINT::POINT_END; ++i)
				{
					_vector vPoint = pCell->Get_Point(static_cast<CCell::POINT>(i));

					_float3 Point = {};
					XMStoreFloat3(&Point, vPoint);

					isIn = IsPointInsideXZ(PxVec3(Point.x, 0, Point.z), bounds);

					//�ϳ��� �ȵ��� ������ �׸�
					if (isIn == false)
						break;
				}
				
				//������ ��� ���Դٸ� �ε��� ����
				if (isIn)
				{
					m_NavIndices.push_back(pCell->Get_Index());
				}

			}
		}
	}

}

void CBreakableMesh::Set_Active_StoreCells(_bool bActive)
{
	for (_int iIndex : m_NavIndices)
	{
		CCell* pCell = m_pNaviCom->Get_Cell(iIndex);

		if (pCell)
		{
			pCell->Set_Active(bActive);
		}

	}
}

HRESULT CBreakableMesh::Bind_ShaderResources()
{
	/* [ ���� �����̽� �ѱ�� ] */
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;

	/* [ �� , ���� �����̽� �ѱ�� ] */
	_float4x4 ViewMatrix, ProjViewMatrix;
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjViewMatrix)))
		return E_FAIL;

	_bool bUseTiling = false;
	//Ÿ�ϸ��� ��� �ϴ°�? �ν��Ͻ��� �ֵ��� Ÿ�ϸ� �ϱ� ���ŷӰڴ�.
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bTile", &bUseTiling, sizeof(_bool))))
		return E_FAIL;

	//�̹̽ú� ����
	_float fEmissive = 0.f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
		return E_FAIL;

	//�۷��� ����
	_float fGlass = 0.f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fGlass", &fGlass, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

HRESULT CBreakableMesh::Ready_Collider()
{
	if (!m_pModelCom)
	{
		_tprintf(_T("%s �ݶ��̴� ���� ����\n"), m_szName);
		return E_FAIL;
	}

	_uint numMeshes = m_pModelCom->Get_NumMeshes();

	vector<PxVec3> physxVertices;
	vector<PxU32> physxIndices;

	_uint vertexBase = 0;

	for (_uint meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	{
		_uint numVertices = m_pModelCom->Get_Mesh_NumVertices(meshIndex);
		_uint numIndices = m_pModelCom->Get_Mesh_NumIndices(meshIndex);

		// --- ���ؽ� ---
		const _float3* pVertexPositions = m_pModelCom->Get_Mesh_pVertices(meshIndex);
		for (_uint i = 0; i < numVertices; ++i)
		{
			const _float3& v = pVertexPositions[i];
			physxVertices.emplace_back(v.x, v.y, v.z);
		}

		// --- �ε��� ---
		const _uint* pIndices = m_pModelCom->Get_Mesh_pIndices(meshIndex);
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

	if (m_bFireEaterBossPipe)
	{
		filterData.word0 = WORLDFILTER::FILTER_MAP;
		//Ǫ���ڿ� ���ؼ��� �μ����� �Ѵ�.
		filterData.word1 = WORLDFILTER::FILTER_MONSTERBODY;
	}
	else
	{
		filterData.word0 = WORLDFILTER::FILTER_MAP;
		//����, �÷��̾�, ����(�÷��̾�, ����)�� ���ؼ� �μ������Ѵ�.
		filterData.word1 = WORLDFILTER::FILTER_MONSTERBODY | WORLDFILTER::FILTER_PLAYERBODY | WORLDFILTER::FILTER_PLAYERWEAPON | WORLDFILTER::FILTER_MONSTERWEAPON;
	}


	// ������������������������������������������������������������
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
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::BREAKABLE_OBJECT);

	//�������� ȣ��ǹ�����
	//m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

HRESULT CBreakableMesh::Ready_PartColliders()
{
	// �� ��Ʈ ���� ConvexMesh�� ��ŷ�ؼ� Dynamic Actor ����
	for (_uint i = 0; i < m_iPartModelCount; ++i)
	{
		CModel* pPartModel = m_pPartModelComs[i];
		if (!pPartModel)
			continue;

		// 1) ��Ʈ ���ؽ� ����
		vector<PxVec3> verts;
		_uint numMeshes = pPartModel->Get_NumMeshes();
		for (_uint m = 0; m < numMeshes; ++m)
		{
			_uint numVertices = pPartModel->Get_Mesh_NumVertices(m);
			const _float3* pv = pPartModel->Get_Mesh_pVertices(m);

			for (_uint v = 0; v < numVertices; ++v)
				verts.emplace_back(pv[v].x, pv[v].y, pv[v].z);
		}

		if (verts.empty())
			continue;

		// 2) ������/���� ���� (��ü Ʈ������ ����)
		_vector S, R, T;
		XMMatrixDecompose(&S, &R, &T, m_pPartTransformComs[i]->Get_WorldMatrix());
		PxMeshScale scale(PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S)));
		PxTransform pose(
			PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T)),
			PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R))
		);

		// 3) ConvexMesh ��ŷ
		PxConvexMeshGeometry geom = m_pGameInstance->CookConvexMesh(verts.data(), (PxU32)verts.size(), scale);
		if (!geom.convexMesh)
			continue;

		// 4) Dynamic Actor ����
		CPhysXDynamicActor* pActorCom = m_pPartPhysXActorComs[i];
		if (!pActorCom)
			continue;

		if (FAILED(pActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"))))
			continue;

		PxRigidDynamic* pRigid = static_cast<PxRigidDynamic*>(pActorCom->Get_Actor());
		if (!pRigid)
			continue;

		// 5) Kinematic ����, �߷� ����, ����/����, CCD, wakeUp
		pActorCom->Set_Kinematic(true);
		//pRigid->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);     // Ȯ���� Kinematic ����
		pRigid->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);        // �߷� �ѱ�
		PxRigidBodyExt::updateMassAndInertia(*pRigid, 0.5f);              // ����/���� ����
		pRigid->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, false);     // CCD Ȱ��ȭ
		//pRigid->wakeUp();                                                  // ��� ���¸� �����

		if (m_bFireEaterBossPipe)
		{
			//�÷��̾� ����
			IgnorePlayerCollider(pActorCom);
		}
		else
		{
			//������� ���Ͷ�, �÷��̾����� �߷� ���̸� ������
		}

		// 6) ���� ����
		PxFilterData fd{};
		fd.word0 = WORLDFILTER::FILTER_DYNAMICOBJ;

		if (m_bFireEaterBossPipe)
		{
			fd.word1 = WORLDFILTER::FILTER_FLOOR | WORLDFILTER::FILTER_DYNAMICOBJ | WORLDFILTER::FILTER_MONSTERBODY;
		}
		else
		{
			//������� ���Ͷ�, �÷��̾����� �߷� ���̸� ������
			fd.word1 = WORLDFILTER::FILTER_FLOOR | WORLDFILTER::FILTER_DYNAMICOBJ | WORLDFILTER::FILTER_MONSTERBODY | WORLDFILTER::FILTER_PLAYERBODY;
		}

		pActorCom->Set_ShapeFlag(true, false, true);
		pActorCom->Set_SimulationFilterData(fd);
		pActorCom->Set_QueryFilterData(fd);
		pActorCom->Set_Owner(this);
		pActorCom->Set_ColliderType(COLLIDERTYPE::BREAKABLE_OBJECT);
	}

	return S_OK;
}


HRESULT CBreakableMesh::Ready_Components(void* pArg)
{
	BREAKABLEMESH_DESC* pDesc = static_cast<BREAKABLEMESH_DESC*>(pArg);

	//��Ʈ �޽��� ���� �޾Ƽ� �÷�����
	m_iPartModelCount = pDesc->iPartModelCount;
	m_pPartModelComs.resize(m_iPartModelCount);
	m_pPartPhysXActorComs.resize(m_iPartModelCount);
	m_pPartTransformComs.resize(m_iPartModelCount);

	wstring BaseTag = TEXT("Prototype_Component_Model_");

	wstring MainTag = BaseTag + pDesc->ModelName;

	/* Com_Model */ //�� ��
	if (FAILED(__super::Add_Component(m_iLevelID, MainTag,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPBRMesh"),
		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	//��Ʈ ������
	for (_uint i = 0; i < m_iPartModelCount; ++i)
	{
#pragma region ��Ʈ ��
		//Com_PartModel0...1...2...
		wstring ComTag = TEXT("Com_PartModel") + to_wstring(i);

		wstring PartTag = BaseTag + pDesc->PartModelNames[i];

		if (FAILED(__super::Add_Component(m_iLevelID, PartTag,
			ComTag.c_str(), reinterpret_cast<CComponent**>(&m_pPartModelComs[i]))))
			return E_FAIL;
#pragma endregion
#pragma region ��Ʈ ������
		////Com_PhysX_Part0...1...2...
		wstring PartPhysXTag = TEXT("Com_PhysX_Part") + to_wstring(i);

		/* For.Com_PhysX */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
			TEXT("Prototype_Component_PhysX_Dynamic"), PartPhysXTag, reinterpret_cast<CComponent**>(&m_pPartPhysXActorComs[i]))))
			return E_FAIL;
#pragma endregion
#pragma region ��Ʈ Ʈ������
		////Com_Transform_Part0...1...2...
		wstring PartTransformTag = TEXT("Com_Transform_Part") + to_wstring(i);

		/* For.Com_Transform */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
			TEXT("Prototype_Component_Transform"), PartTransformTag, reinterpret_cast<CComponent**>(&m_pPartTransformComs[i]))))
			return E_FAIL;
#pragma endregion
	}

	//Ǫ���ڸ� �׺� �ʿ��ϴ�. ���� ��Ȱ��ȭ/Ȱ��ȭ �ϱ����ؼ�
	if (m_bFireEaterBossPipe)
	{
		if (pDesc->wsNavName.empty() == false)
		{
			//������ �� �׺���̼�
			wstring wsPrototypeTag = TEXT("Prototype_Component_Navigation_") + pDesc->wsNavName; //� �׺� STAION, HOTEL...
			if (FAILED(__super::Add_Component(pDesc->iLevelID, wsPrototypeTag.c_str(),
				TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNaviCom))))
				return E_FAIL;

			//�׽�Ʈ�ʿ� �����Ǵ� �μ����ִ� �ֵ���, Ǫ���� �׺� ã�ƹ����� �������� ()
		}
	}

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

	for (_uint i = 0; i < m_iPartModelCount; ++i)
	{
		Safe_Release(m_pPartModelComs[i]);
		Safe_Release(m_pPartPhysXActorComs[i]);
		Safe_Release(m_pPartTransformComs[i]);
	}

	Safe_Release(m_pModelCom);
	Safe_Release(m_pPhysXActorCom);

	Safe_Release(m_pShaderCom);

	Safe_Release(m_pPlayer);

	Safe_Release(m_pNaviCom);
}
