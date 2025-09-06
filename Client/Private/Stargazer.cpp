#include "Stargazer.h"
#include "GameInstance.h"
#include "Player.h"
#include "Level.h"

CStargazer::CStargazer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
	, m_pShaderCom(nullptr)
{

}

CStargazer::CStargazer(const CStargazer& Prototype)
	:CGameObject(Prototype)
	, m_pShaderCom(Prototype.m_pShaderCom)
{

	Safe_AddRef(m_pShaderCom);
}

HRESULT CStargazer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CStargazer::Initialize(void* pArg)
{
	STARGAZER_DESC* pDesc = static_cast<STARGAZER_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	for (_uint i = 0; i < ENUM_CLASS(STARGAZER_STATE::END); ++i)
	{
		LoadAnimDataFromJson(m_pModelCom[i], m_pAnimator[i]);
	}

	m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);

	m_eStargazerTag = pDesc->eStargazerTag;

	m_eState = STARGAZER_STATE::DESTROYED;

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	//m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

void CStargazer::Priority_Update(_float fTimeDelta)
{
	if(m_pPlayer == nullptr)
		Find_Player();

	//부서진거
	//AS_Stargazer_broken_idle
	//AS_Stargazer_restore 
	
	//부서진거 (Restore)끝나면
	//멀쩡한거 (Open) 실행

	//멀쩡한거
	//AS_Close_Idle 
	//AS_Open
	//AS_Open_Idle

	//상태변경
	if (m_pGameInstance->Key_Down(DIK_C))
	{
		if (m_eState == STARGAZER_STATE::DESTROYED)
			m_eState = STARGAZER_STATE::FUNCTIONAL;
		else
			m_eState = STARGAZER_STATE::DESTROYED;
	}

	if (m_pGameInstance->Key_Down(DIK_R))
	{
		m_pAnimator[ENUM_CLASS(STARGAZER_STATE::DESTROYED)]->SetTrigger("Restore");
	}

	if (m_pGameInstance->Key_Down(DIK_O))
	{
		m_pAnimator[ENUM_CLASS(STARGAZER_STATE::FUNCTIONAL)]->SetTrigger("Open");
	}

	//플레이어와 가깝고 E를 누르면 다른 별바라기로 이동(테스트)
	//플레이어쪽으로 코드 옮기는 작업 필요할지도
	if (m_pGameInstance->Key_Down(DIK_E))
	{
		if (Check_Player_Close())
		{
			//리셋
			m_pGameInstance->Get_CurrentLevel()->Reset();
			m_pPlayer->Reset();
		}
	}

	if (m_pGameInstance->Key_Down(DIK_0))
	{
		if (Check_Player_Close())
		{
			Teleport_Stargazer(STARGAZER_TAG::OUTER);
		}
	}

	if (m_pGameInstance->Key_Down(DIK_9))
	{
		if (Check_Player_Close())
		{
			Teleport_Stargazer(STARGAZER_TAG::FIRE_EATER);
		}
	}

	if (m_pGameInstance->Key_Down(DIK_MINUS))
	{
		if (Check_Player_Close())
		{
			Teleport_Stargazer(STARGAZER_TAG::FESTIVAL_LEADER);
		}
	}
	 
}

void CStargazer::Update(_float fTimeDelta)
{
	/* [ 애니메이션 업데이트 ] */
	if (m_pAnimator[ENUM_CLASS(m_eState)])
		m_pAnimator[ENUM_CLASS(m_eState)]->Update(fTimeDelta);

	if (m_pModelCom[ENUM_CLASS(m_eState)])
		m_pModelCom[ENUM_CLASS(m_eState)]->Update_Bones();
}

void CStargazer::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CStargazer::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom[ENUM_CLASS(m_eState)]->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom[ENUM_CLASS(m_eState)]->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom[ENUM_CLASS(m_eState)]->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);
		m_pModelCom[ENUM_CLASS(m_eState)]->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0);


		m_pModelCom[ENUM_CLASS(m_eState)]->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom[ENUM_CLASS(m_eState)]->Render(i)))
			return E_FAIL;
	}

	if (m_pGameInstance->Get_RenderMapCollider())
	{
		if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
			return E_FAIL;
	}

	return S_OK;
}

void CStargazer::LoadAnimDataFromJson(CModel* pModel, CAnimator* pAnimator)
{
	string path = "../Bin/Save/AnimationEvents/" + pModel->Get_ModelName() + "_events.json";
	ifstream ifs(path);
	if (ifs.is_open())
	{
		json root;
		ifs >> root;
		if (root.contains("animations"))
		{
			auto& animationsJson = root["animations"];
			auto& clonedAnims = pModel->GetAnimations();

			for (const auto& animData : animationsJson)
			{
				const string& clipName = animData["ClipName"];

				for (auto& pAnim : clonedAnims)
				{
					if (pAnim->Get_Name() == clipName)
					{
						pAnim->Deserialize(animData);
						break;
					}
				}
			}
		}
	}

	path = "../Bin/Save/AnimationStates/" + pModel->Get_ModelName() + "_States.json";
	ifstream ifsStates(path);
	if (ifsStates.is_open())
	{
		json rootStates;
		ifsStates >> rootStates;
		pAnimator->Deserialize(rootStates);
	}
}

void CStargazer::Find_Player()
{
	m_pPlayer = static_cast<CPlayer*>(m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player")));
	Safe_AddRef(m_pPlayer);
}

void CStargazer::Teleport_Stargazer(STARGAZER_TAG eTag)
{
	//별바라기 리스트에서 같은 태그를 찾고 그곳으로 플레이어 이동시켜라

	list<CGameObject*>& ObjList = m_pGameInstance->Get_ObjectList(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Stargazer"));

	for (CGameObject* pObj : ObjList)
	{
		if (static_cast<CStargazer*>(pObj)->m_eStargazerTag == eTag)
		{
			_float3 vPos;
			XMStoreFloat3(&vPos, pObj->Get_TransfomCom()->Get_State(STATE::POSITION));

			//살짝 위로
			vPos.y += 1.f;
			//살짝 뒤로
			vPos.x -= 2.f;

			PxVec3 pxPos(vPos.x, vPos.y, vPos.z);

			PxTransform posTrans = PxTransform(pxPos);

			m_pPlayer->Get_Controller()->Set_Transform(posTrans);

			break;
		}
	}


}

_bool CStargazer::Check_Player_Close()
{
	//플레이어가 가까운지 체크
	_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vDiff = vPos - vPlayerPos;
	_float fDist = XMVectorGetX(XMVector3Length(vDiff));

	if (fDist < 2.f)
		return true;
	else 
		return false;
}

HRESULT CStargazer::Bind_ShaderResources()
{
	/* [ 월드 스페이스 넘기기 ] */
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;

	/* [ 뷰 , 투영 스페이스 넘기기 ] */
	_float4x4 ViewMatrix, ProjViewMatrix;
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjViewMatrix)))
		return E_FAIL;

	return S_OK;
}

HRESULT CStargazer::Ready_Collider()
{
	// 3. Transform에서 S, R, T 분리
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 3-1. 스케일, 회전, 위치 변환
	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxVec3 halfExtents = {};

	/*if (pArg != nullptr)
	{
		halfExtents = VectorToPxVec3(XMLoadFloat3(&pDesc->vExtent));
	}
	else
	{
		halfExtents = VectorToPxVec3(XMLoadFloat3(&m_vHalfExtents));
	}*/

	_float3 vHalf = _float3(0.5f, 0.5f, 0.5f);

	halfExtents = VectorToPxVec3(XMLoadFloat3(&vHalf));
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_PLAYERBODY;
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::A);
	//m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

HRESULT CStargazer::Ready_Components(void* pArg)
{
	/* Com_Model */
	if (FAILED(__super::Add_Component(m_iLevelID, _wstring(TEXT("Prototype_Component_Model_Stargazer_Destroyed")),
		TEXT("Com_Model_Destroyed"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(STARGAZER_STATE::DESTROYED)]))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(m_iLevelID, _wstring(TEXT("Prototype_Component_Model_Stargazer")),
		TEXT("Com_Model_Functional"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(STARGAZER_STATE::FUNCTIONAL)]))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	for (_uint i = 0; i < ENUM_CLASS(STARGAZER_STATE::END); ++i)
	{
		m_pAnimator[i] = CAnimator::Create(m_pDevice, m_pContext);
		if (nullptr == m_pAnimator)
			return E_FAIL;

		if (FAILED(m_pAnimator[i]->Initialize(m_pModelCom[i])))
			return E_FAIL;
	}

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;


	return S_OK;
}

CStargazer* CStargazer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStargazer* pGameInstance = new CStargazer(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CStaticMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

CGameObject* CStargazer::Clone(void* pArg)
{
	CStargazer* pInstance = new CStargazer(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CStargazer");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CStargazer::Free()
{
	__super::Free();

	for (_uint i = 0; i < ENUM_CLASS(STARGAZER_STATE::END); ++i)
	{
		Safe_Release(m_pModelCom[i]);
		Safe_Release(m_pAnimator[i]);
	}
	
	Safe_Release(m_pPlayer);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pPhysXActorCom);
}
