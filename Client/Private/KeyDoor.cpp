#include "KeyDoor.h"
#include "GameInstance.h"
#include "Player.h"
#include "UI_Manager.h"
#include "Camera_Manager.h"

CKeyDoor::CKeyDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CDynamicMesh{ pDevice, pContext }
{

}

CKeyDoor::CKeyDoor(const CKeyDoor& Prototype)
	: CDynamicMesh(Prototype)
{

}

HRESULT CKeyDoor::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CKeyDoor::Initialize(void* pArg)
{
	CKeyDoor::KEYDOORMESH_DESC* pDoorMeshDESC = static_cast<KEYDOORMESH_DESC*>(pArg);

	m_eInteractType = pDoorMeshDESC->eInteractType;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	Ready_Trigger(pDoorMeshDESC);
	m_pPlayer = GET_PLAYER(m_pGameInstance->GetCurrentLevelIndex());
	if (FAILED(LoadFromJson()))
		return E_FAIL;
	return S_OK;
}

void CKeyDoor::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
	if (m_pPlayer == nullptr)
		return;
	if (m_bCanActive && !m_bFinish)
	{
		if (KEY_DOWN(DIK_E))
		{
			switch (m_eInteractType)
			{
			case Client::OUTDOOR:
				m_bMoveStart = true;
				m_bFinish = m_pPlayer->Get_HaveKey();

				CCamera_Manager::Get_Instance()->SetbMoveable(false);
				CUI_Manager::Get_Instance()->Activate_Popup(false);
				break;
			case INNERDOOR:
				m_bFinish = true;
				m_bMoveStart = true;

				CCamera_Manager::Get_Instance()->SetbMoveable(false);
				CUI_Manager::Get_Instance()->Activate_Popup(false);
				break;
			}
			if (m_bFinish)
			{
				m_pPhysXActorCom->Init_SimulationFilterData();
				m_pPhysXActorCom->Set_ShapeFlag(false, false, false);
			}

		}
	}

	if (KEY_PRESSING(DIK_LCONTROL) && KEY_DOWN(DIK_Z))
	{
		m_pPlayer->Set_GetKey();
	}

#ifdef _DEBUG
	if (KEY_DOWN(DIK_X))
	{
		m_pAnimator->Get_CurrentAnimController()->SetState("Idle");
		m_bFinish = false;
		m_bCanActive = false;
	}
#endif // _DEBUG

	if (m_bStartSound)
		Play_Sound(fTimeDelta);
}

void CKeyDoor::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_pAnimator)
		m_pAnimator->Update(fTimeDelta);

	if (m_pModelCom)
		m_pModelCom->Update_Bones();

	Move_Player(fTimeDelta);
}

void CKeyDoor::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CKeyDoor::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
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


		_float m_fEmissive = 0.f;
		m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &m_fEmissive, sizeof(_float));
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_Emissive", i, aiTextureType_EMISSIVE, 0)))
		{
			_float fEmissive = 0.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
				return E_FAIL;
		}

		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

#ifdef _DEBUG

	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom))
	{
		if (m_pGameInstance->Get_RenderCollider())
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
				return E_FAIL;
		}
	}

#endif

#ifdef _DEBUG

	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXTriggerCom))
	{
		if (m_pGameInstance->Get_RenderCollider())
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXTriggerCom)))
				return E_FAIL;
		}
	}

#endif

	return S_OK;
}

void CKeyDoor::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bFinish)
	{
		m_bCanActive = true;
		CUI_Manager::Get_Instance()->Activate_Popup(true);
		CUI_Manager::Get_Instance()->Set_Popup_Caption(2);
	}
}

void CKeyDoor::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bFinish)
		m_bCanActive = false;


	CUI_Manager::Get_Instance()->Activate_Popup(false);
}

void CKeyDoor::Play_Sound(_float fTimeDelta)
{
	m_fSoundDelta += fTimeDelta;
	switch (m_eInteractType)
	{
	case Client::OUTDOOR:
	{
		if (m_fSoundDelta > 5.f)
		{
			m_bStartSound = false;
			m_pSoundCom->SetVolume("AMB_OJ_DR_Exhibition_Door", 0.7f * g_fInteractSoundVolume);
			m_pSoundCom->Play("AMB_OJ_DR_Exhibition_Door");
			m_fSoundDelta = 0.f;
		}
		break;
	}
	case Client::INNERDOOR:
	{
		if (m_fSoundDelta > 0.2f)
		{
			m_bStartSound = false;
			m_pSoundCom->SetVolume("AMB_OJ_DR_Train_Slide_M", 0.9f* g_fInteractSoundVolume);
			m_pSoundCom->Play("AMB_OJ_DR_Train_Slide_M");
			m_fSoundDelta = 0.f;
		}
		break;
	}
	default:
		break;
	}
}

void CKeyDoor::OpenDoor()
{
	if (m_pAnimator)
	{
		m_pAnimator->SetTrigger("Open");
	}
}

void CKeyDoor::Move_Player(_float fTimeDelta)
{
	/* [ 이거 그냥 야외로 가는 문 전용 클래스로 씁시다ㅏ ] */

	if (m_bMoveStart)
	{
		_vector vTargetPos;
		switch (m_eInteractType)
		{
		case Client::OUTDOOR:
			if(m_bFinish)
				vTargetPos = _vector({ 182.8f, 8.85f, -7.95f, 1.f });
			else
				vTargetPos = _vector({ 183.05f, 8.85f, -7.95f, 1.f });
			break;
		case INNERDOOR:
			vTargetPos = _vector({ 33.75f,0.059f,0.58f, 1.f });
			break;
		default:
			break;
		}

		if (m_pPlayer->MoveToDoor(fTimeDelta, vTargetPos))
		{
			m_bMoveStart = false;
			m_bRotationStart = true;
			if (m_bRotationStart == false)
			{
				m_bStartSound = true;
			}
			m_pPlayer->Get_TransfomCom()->Set_State(STATE::POSITION, vTargetPos);
		}
	}


	if (m_bRotationStart)
	{
		_vector vTargetRotation;
		switch (m_eInteractType)
		{
		case Client::OUTDOOR:
		case Client::INNERDOOR:
			vTargetRotation = _vector({ 1.f, 0.f, 0.f, 0.f });
			break;
		default:
			break;
		}

		if (m_pPlayer->RotateToDoor(fTimeDelta, vTargetRotation))
		{
			m_bRotationStart = false;
			m_bStartCutScene = true;
		}
	}

	if (m_bStartCutScene)
	{
		m_bStartCutScene = false;
		m_bStartSound = true;
		// 문 여는 거 활성화
		m_pPlayer->Interaction_Door(m_eInteractType, this);
		CCamera_Manager::Get_Instance()->SetbMoveable(true);

		if (m_pPlayer->Get_HaveKey()&& m_eInteractType == INTERACT_TYPE::OUTDOOR)
		{
			CCamera_Manager::Get_Instance()->Play_CutScene(CUTSCENE_TYPE::OUTDOOOR);
			CUI_Manager::Get_Instance()->Off_Panel();
		}

	}
}



HRESULT CKeyDoor::Ready_Components(void* pArg)
{
	// 애니메이션 있는거 이걸로 해야 함
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysXTrigger"), reinterpret_cast<CComponent**>(&m_pPhysXTriggerCom))))
		return E_FAIL;

	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_CutSceneDoor"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
		return E_FAIL;

	m_pAnimator = CAnimator::Create(m_pDevice, m_pContext);
	if (nullptr == m_pAnimator)
		return E_FAIL;
	if (FAILED(m_pAnimator->Initialize(m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CKeyDoor::Ready_Trigger(KEYDOORMESH_DESC* pDesc)
{
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
	positionVec += VectorToPxVec3(pDesc->vTriggerOffset);

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	//PxVec3 halfExtents = { 1.f,0.2f,1.f };
	PxVec3 halfExtents = VectorToPxVec3(pDesc->vTriggerSize);
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXTriggerCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXTriggerCom->Set_ShapeFlag(false, true, false);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_INTERACT;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY; // 일단 보류
	m_pPhysXTriggerCom->Set_SimulationFilterData(filterData);
	m_pPhysXTriggerCom->Set_QueryFilterData(filterData);
	m_pPhysXTriggerCom->Set_Owner(this);
	m_pPhysXTriggerCom->Set_ColliderType(COLLIDERTYPE::TRIGGER);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXTriggerCom->Get_Actor());

	return S_OK;
}

HRESULT CKeyDoor::LoadFromJson()
{
	string modelName = m_pModelCom->Get_ModelName();
	if (FAILED(LoadAnimationEventsFromJson(modelName, m_pModelCom)))
		return E_FAIL;
	if (FAILED(LoadAnimationStatesFromJson(modelName, m_pAnimator)))
		return E_FAIL;
	return S_OK;
}

HRESULT CKeyDoor::LoadAnimationEventsFromJson(const string& modelName, CModel* pModelCom)
{
	string path = "../Bin/Save/AnimationEvents/" + modelName + "_events.json";
	ifstream ifs(path);
	if (ifs.is_open())
	{
		json root;
		ifs >> root;
		if (root.contains("animations"))
		{
			auto& animationsJson = root["animations"];
			auto& clonedAnims = pModelCom->GetAnimations();

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
	else
	{
		MSG_BOX("Failed to open animation events file.");
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CKeyDoor::LoadAnimationStatesFromJson(const string& modelName, CAnimator* pAnimator)
{
	string path = "../Bin/Save/AnimationStates/" + modelName + "_States.json";
	ifstream ifsStates(path);
	if (ifsStates.is_open())
	{
		json rootStates;
		ifsStates >> rootStates;
		pAnimator->Deserialize(rootStates);
	}
	else
	{
		MSG_BOX("Failed to open animation states file.");
		return E_FAIL;
	}
	return S_OK;
}


CKeyDoor* CKeyDoor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKeyDoor* pGameInstance = new CKeyDoor(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CKeyDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CKeyDoor::Clone(void* pArg)
{
	CKeyDoor* pGameInstance = new CKeyDoor(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CKeyDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CKeyDoor::Free()
{
	__super::Free();

	Safe_Release(m_pPhysXTriggerCom);
	Safe_Release(m_pSoundCom);
	Safe_Release(m_pAnimator);
}
