#include "BossDoor.h"
#include "GameInstance.h"
#include "Player.h"
#include "UI_Manager.h"
#include "Camera_Manager.h"
#include "UI_Manager.h"
CBossDoor::CBossDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CDynamicMesh{ pDevice, pContext }
{

}

CBossDoor::CBossDoor(const CBossDoor& Prototype)
	: CDynamicMesh(Prototype)
{

}

HRESULT CBossDoor::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CBossDoor::Initialize(void* pArg)
{
	CBossDoor::BOSSDOORMESH_DESC* pDoorMeshDESC = static_cast<BOSSDOORMESH_DESC*>(pArg);

	m_eInteractType = pDoorMeshDESC->eInteractType;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	Ready_Trigger(pDoorMeshDESC);

	if (FAILED(LoadFromJson()))
		return E_FAIL;
	Register_Events();
	return S_OK;
}

void CBossDoor::Priority_Update(_float fTimeDelta)
{
	if(!m_pPlayer)
		m_pPlayer = dynamic_cast<CPlayer*>(GET_PLAYER(m_pGameInstance->GetCurrentLevelIndex()));
	__super::Priority_Update(fTimeDelta);

	if (m_bCanActive && !m_bFinish)
	{
		// 상호작용
		if (KEY_DOWN(DIK_E))
		{
			m_bMoveStart = true;
			m_pPhysXActorCom->Init_SimulationFilterData();
			m_pPhysXActorCom->Set_ShapeFlag(false, false, false);
			CUI_Manager::Get_Instance()->Activate_Popup(false);

			CCamera_Manager::Get_Instance()->SetbMoveable(false);
		}
	}

#ifdef _DEBUG
	if (KEY_DOWN(DIK_X))
	{
		m_pAnimator->Get_CurrentAnimController()->SetState("Idle");
		if (m_pSecondAnimator)
			m_pSecondAnimator->Get_CurrentAnimController()->SetState("Idle");
		m_bRenderSecond = false;
		m_bFinish = false;
		m_bCanActive = false;
	}
#endif // _DEBUG

	if (m_bStartSound)
		Play_Sound(fTimeDelta);
}

void CBossDoor::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_pAnimator)
		m_pAnimator->Update(fTimeDelta);

	if (m_pModelCom)
		m_pModelCom->Update_Bones();

	if(m_pSecondAnimator)
		m_pSecondAnimator->Update(fTimeDelta);
	if (m_pSecondModelCom)
		m_pSecondModelCom->Update_Bones();

	Move_Player(fTimeDelta);
}

void CBossDoor::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CBossDoor::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	CModel* pModel = nullptr;
	if (!m_bRenderSecond)
		pModel = m_pModelCom;
	else
		pModel = m_pSecondModelCom;

	_uint		iNumMesh = pModel->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		pModel->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		pModel->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		if (FAILED(pModel->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
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
		if (FAILED(pModel->Bind_Material(m_pShaderCom, "g_Emissive", i, aiTextureType_EMISSIVE, 0)))
		{
			_float fEmissive = 0.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
				return E_FAIL;
		}

		pModel->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(pModel->Render(i)))
			return E_FAIL;
	}

#ifdef _DEBUG

	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom))
	{
		if (m_pGameInstance->Get_RenderCollider() && m_pPhysXActorCom->Get_ReadyForDebugDraw())
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

void CBossDoor::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bFinish)
	{
		m_bCanActive = true;
		CUI_Manager::Get_Instance()->Activate_Popup(true);
  		CUI_Manager::Get_Instance()->Set_Popup_Caption(2);
	}
}

void CBossDoor::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bFinish)
		m_bCanActive = false;


	CUI_Manager::Get_Instance()->Activate_Popup(false);
}

void CBossDoor::Register_Events()
{
	if (m_eInteractType == Client::FESTIVALDOOR)
	{
		m_pAnimator->RegisterEventListener("ChangeDoorModel", [this]() 
			{
				if (m_bRenderSecond)
					m_bRenderSecond = false;
				else
					m_bRenderSecond = true;
			});
	}
}

void CBossDoor::Play_Sound(_float fTimeDelta)
{
	m_fSoundDelta += fTimeDelta;
	switch (m_eInteractType)
	{
	case Client::FUOCO:
	{
		if (m_fSoundDelta > 1.8f)
		{
			m_bStartSound = false;
			m_pSoundCom->SetVolume("AMB_OJ_DR_BossGate_SlidingDoor_Open", 0.7f * g_fInteractSoundVolume);
			m_pSoundCom->Play("AMB_OJ_DR_BossGate_SlidingDoor_Open");
			m_fSoundDelta = 0.f;
		}
		break;
	}
	case Client::FESTIVALDOOR:
	{
		if (m_fSoundDelta > 2.9f)
		{
			m_bStartSound = false;
			m_pSoundCom->SetVolume("AMB_OJ_DR_Metal_Gate_Crash", 0.7f * g_fInteractSoundVolume);
			m_pSoundCom->Play("AMB_OJ_DR_Metal_Gate_Crash");
			m_fSoundDelta = 0.f;
		}
		break;
	}
	default:
		break;
	}

}

HRESULT CBossDoor::Ready_Components(void* pArg)
{
	CBossDoor::BOSSDOORMESH_DESC* BossDoorDESC = static_cast<BOSSDOORMESH_DESC*>(pArg);

	// 애니메이션 있는거 이걸로 해야 함
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	///* Com_Shader */
	//if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
	//	TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
	//	return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysXTrigger"), reinterpret_cast<CComponent**>(&m_pPhysXTriggerCom))))
		return E_FAIL;

	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_CutSceneDoor"),TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
		return E_FAIL;

	m_pAnimator = CAnimator::Create(m_pDevice, m_pContext);
	if (nullptr == m_pAnimator)
		return E_FAIL;
	if (FAILED(m_pAnimator->Initialize(m_pModelCom)))
		return E_FAIL;

	if (BossDoorDESC->bNeedSecondDoor)
	{
		/* Com_Model */
		if (FAILED(__super::Add_Component(ENUM_CLASS(m_eMeshLevelID), BossDoorDESC->szSecondModelPrototypeTag,
			TEXT("Com_Model2"), reinterpret_cast<CComponent**>(&m_pSecondModelCom))))
			return E_FAIL;

		m_pSecondAnimator = CAnimator::Create(m_pDevice, m_pContext);
		if (nullptr == m_pSecondAnimator)
			return E_FAIL;
		if (FAILED(m_pSecondAnimator->Initialize(m_pSecondModelCom)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CBossDoor::Ready_Trigger(BOSSDOORMESH_DESC* pDesc)
{
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
	positionVec += VectorToPxVec3(pDesc->vTriggerOffset);

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxVec3 halfExtents = VectorToPxVec3(pDesc->vTriggerSize);
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXTriggerCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXTriggerCom->Set_ShapeFlag(false, true, false);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_INTERACT;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY; 
	m_pPhysXTriggerCom->Set_SimulationFilterData(filterData);
	m_pPhysXTriggerCom->Set_QueryFilterData(filterData);
	m_pPhysXTriggerCom->Set_Owner(this);
	m_pPhysXTriggerCom->Set_ColliderType(COLLIDERTYPE::TRIGGER);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXTriggerCom->Get_Actor());

	return S_OK;
}

HRESULT CBossDoor::LoadFromJson()
{
	string modelName = m_pModelCom->Get_ModelName();
	if (FAILED(LoadAnimationEventsFromJson(modelName, m_pModelCom)))
		return E_FAIL;
	if (FAILED(LoadAnimationStatesFromJson(modelName,m_pAnimator)))
		return E_FAIL;
	if (m_pSecondModelCom && m_pSecondAnimator)
	{
		string modelName2 = m_pSecondModelCom->Get_ModelName();
		if (FAILED(LoadAnimationEventsFromJson(modelName2, m_pSecondModelCom)))
			return E_FAIL;
		if (FAILED(LoadAnimationStatesFromJson(modelName2, m_pSecondAnimator)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CBossDoor::LoadAnimationEventsFromJson(const string& modelName, CModel* pModelCom)
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

HRESULT CBossDoor::LoadAnimationStatesFromJson(const string& modelName, CAnimator* pAnimator)
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

void CBossDoor::Move_Player(_float fTimeDelta)
{
	if (m_bMoveStart)
	{
		m_bFinish = true;
		switch (m_eInteractType)
		{
		case Client::FESTIVALDOOR:
			//X:375.136841, Y:14.995386, Z:-48.836079
			m_vTargetPos = _vector({ 375.13f, 14.9f, -48.83f, 1.f});
			break;
		case Client::FUOCO:
			m_vTargetPos = _vector({ -3.2f, 0.3f, -235.87f, 1.f});
			break;
		default:
			break;
		}

		if (m_pPlayer->MoveToDoor(fTimeDelta, m_vTargetPos))
		{
			m_bMoveStart = false;
			m_bRotationStart = true;
		}
	}


	if (m_bRotationStart)
	{
		_vector vTargetRotation;
		switch (m_eInteractType)
		{
		case Client::FESTIVALDOOR:
			vTargetRotation = _vector({ 1.f, 0.f, 0.f, 0.f });
			break;
		case Client::FUOCO:
			vTargetRotation = _vector({ 0.f, 0.f, 1.f, 0.f });
			break;
		default:
			break;
		}


		if (m_pPlayer->RotateToDoor(fTimeDelta, vTargetRotation))
		{
			m_bRotationStart = false;
			m_bStartCutScene = true;
			switch (m_eInteractType)
			{
			case Client::FESTIVALDOOR:
				CCamera_Manager::Get_Instance()->Play_CutScene(CUTSCENE_TYPE::FESTIVAL);
				break;
			case Client::FUOCO:
				CCamera_Manager::Get_Instance()->Play_CutScene(CUTSCENE_TYPE::FUOCO);
				break;
			default:
				break;
			}

			// 

			//CUI_Manager::Get_Instance()->Off_Panel();
		}
	}

	if (m_bStartCutScene)
	{
		m_bStartCutScene = false;
		m_pPlayer->Interaction_Door(m_eInteractType, this);
		if (m_pAnimator)
			m_pAnimator->SetTrigger("Open");
		if (m_pSecondAnimator)
			m_pSecondAnimator->SetTrigger("Open");

		m_bStartSound = true;
	}
}


CBossDoor* CBossDoor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBossDoor* pGameInstance = new CBossDoor(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CBossDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CBossDoor::Clone(void* pArg)
{
	CBossDoor* pGameInstance = new CBossDoor(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CBossDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CBossDoor::Free()
{
	__super::Free();

	Safe_Release(m_pPhysXTriggerCom);
	Safe_Release(m_pAnimator);
	Safe_Release(m_pSoundCom);
	Safe_Release(m_pSecondModelCom);
	Safe_Release(m_pSecondAnimator);
}
