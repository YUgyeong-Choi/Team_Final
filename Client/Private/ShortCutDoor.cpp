#include "ShortCutDoor.h"
#include "GameInstance.h"
#include "Player.h"
#include "UI_Manager.h"
#include "Camera_Manager.h"
#include "Effect_Manager.h"
#include "EffectContainer.h"

CShortCutDoor::CShortCutDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CDefaultDoor{ pDevice, pContext }
{

}

CShortCutDoor::CShortCutDoor(const CShortCutDoor& Prototype)
	: CDefaultDoor(Prototype)
{

}

HRESULT CShortCutDoor::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CShortCutDoor::Initialize(void* pArg)
{
	m_bIsDissolve = true;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	if (FAILED(LoadFromJson()))
		return E_FAIL;

	Register_Events();

	return S_OK;
}

void CShortCutDoor::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_bCanActive && !m_bFinish)
	{
		if (KEY_DOWN(DIK_E))
		{
			m_bMoveStart = true;

			// 플레이어랑 문 위치랑 비교해서
			_float fPlayerZ = XMVectorGetZ(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION));
			_float fDoorZ = XMVectorGetZ(m_pTransformCom->Get_State(STATE::POSITION));
			if (fPlayerZ < fDoorZ) // 플레이어z가 문보다 작으면.. 왼쪽이면?
			{
				m_bCanOpen = true;
				m_bFinish = true;
				m_pPhysXActorCom->Init_SimulationFilterData();
				m_pPhysXActorCom->Set_ShapeFlag(false, false, false);
			}
			else
			{
				m_bCanOpen = false;
			}

			CCamera_Manager::Get_Instance()->SetbMoveable(false);
			CUI_Manager::Get_Instance()->Activate_Popup(false);
		}
	}

#ifdef _DEBUG
	if (KEY_DOWN(DIK_X))
	{
		//m_pAnimator->Get_CurrentAnimController()->SetState("Idle");
		m_bFinish = false;
		m_bCanActive = false;
		m_bCanOpen = true;
		m_fEscapeTime = 0.f;

		m_bEffectActive = false;
		m_fEffectTime = 0.f;
	}
#endif // _DEBUG
}

void CShortCutDoor::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_pAnimator)
		m_pAnimator->Update(fTimeDelta);

	if (m_pAnimatorFrontKey)
		m_pAnimatorFrontKey->Update(fTimeDelta);

	if (m_pAnimatorBackKey)
		m_pAnimatorBackKey->Update(fTimeDelta);

	if (m_pModelCom)
		m_pModelCom->Update_Bones();

	if (m_pModelComFrontKey)
		m_pModelComFrontKey->Update_Bones();

	if (m_pModelComBackKey)
		m_pModelComBackKey->Update_Bones();

	Move_Player(fTimeDelta);

	if(m_bEffectActive)
		Start_Effect(fTimeDelta);

}

void CShortCutDoor::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CShortCutDoor::Render()
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

	Render_Key();

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

void CShortCutDoor::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bFinish)
	{
		m_bCanActive = true;
		CUI_Manager::Get_Instance()->Activate_Popup(true);
		CUI_Manager::Get_Instance()->Set_Popup_Caption(2);
	}
}

void CShortCutDoor::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bFinish)
		m_bCanActive = false;


	CUI_Manager::Get_Instance()->Activate_Popup(false);
}

void CShortCutDoor::OpenDoor()
{
	if (m_pAnimator)
	{
		m_pAnimator->SetTrigger("Open");
	}
}

void CShortCutDoor::ActivateUnlock()
{
	if (m_bCanOpen)
	{
		m_pAnimatorFrontKey->SetTrigger("Open");
		m_pAnimatorBackKey->SetTrigger("Open");
	}
}

void CShortCutDoor::Move_Player(_float fTimeDelta)
{
	if (m_bMoveStart)
	{
		_vector vTargetPos;
		switch (m_eInteractType)
		{
		case Client::SHORTCUT:
			if (m_bCanOpen)
				vTargetPos = _vector({ 147.75f, 1.f, -25.9f, 1.f });
			else
				vTargetPos = _vector({ 147.15f, 2.66f, -24.52f, 1.f });
			break;
		default:
			break;
		}

		if (m_pPlayer->MoveToDoor(fTimeDelta, vTargetPos))
		{
			m_bMoveStart = false;
			m_bRotationStart = true;
			m_pPlayer->Get_TransfomCom()->Set_State(STATE::POSITION, vTargetPos);
		}
	}


	if (m_bRotationStart)
	{
		_vector vTargetRotation;
		switch (m_eInteractType)
		{
		case Client::SHORTCUT:
			if (m_bCanOpen)
				vTargetRotation = _vector({ 0.f, 0.f, 1.f, 0.f });
			else
				vTargetRotation = _vector({ 0.f, 0.f, -1.f, 0.f });
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
		m_bCanMovePlayer = true;
		m_bEffectActive = true;
		m_bStartSound = true;
		// 문 여는 거 활성화
		m_pPlayer->Interaction_Door(m_eInteractType, this, m_bCanOpen);

		m_fSoundDelta = 0.f;
		m_fEffectTime = 0.f;
	}

	if (m_bCanMovePlayer)
	{
		m_fEscapeTime += fTimeDelta;
		if (m_bCanOpen && m_fEscapeTime > 8.5f)
		{
			CCamera_Manager::Get_Instance()->SetbMoveable(true);
			m_bCanMovePlayer = false;
		}

		if(!m_bCanOpen)
		{
			CCamera_Manager::Get_Instance()->SetbMoveable(true);
			m_bCanMovePlayer = false;
		}
	}
}

void CShortCutDoor::Register_Events()
{
	m_pAnimatorBackKey->RegisterEventListener("UnlockDoor", [this]()
		{
			OpenDoor();
		});
}



HRESULT CShortCutDoor::Ready_Components(void* pArg)
{
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	m_pAnimator = CAnimator::Create(m_pDevice, m_pContext);
	if (nullptr == m_pAnimator)
		return E_FAIL;
	if (FAILED(m_pAnimator->Initialize(m_pModelCom)))
		return E_FAIL;


	/* FrontKey */
	if (FAILED(__super::Add_Component(ENUM_CLASS(m_eMeshLevelID), TEXT("Prototype_Component_Model_HeavyLockSmall"),
		TEXT("Com_ModelFrontKey"), reinterpret_cast<CComponent**>(&m_pModelComFrontKey))))
		return E_FAIL;

	m_pAnimatorFrontKey = CAnimator::Create(m_pDevice, m_pContext);
	if (nullptr == m_pAnimatorFrontKey)
		return E_FAIL;
	if (FAILED(m_pAnimatorFrontKey->Initialize(m_pModelComFrontKey)))
		return E_FAIL;

	/* BackKey */
	if (FAILED(__super::Add_Component(ENUM_CLASS(m_eMeshLevelID), TEXT("Prototype_Component_Model_HeavyLock"),
		TEXT("Com_ModelBackKey"), reinterpret_cast<CComponent**>(&m_pModelComBackKey))))
		return E_FAIL;

	m_pAnimatorBackKey = CAnimator::Create(m_pDevice, m_pContext);
	if (nullptr == m_pAnimatorBackKey)
		return E_FAIL;
	if (FAILED(m_pAnimatorBackKey->Initialize(m_pModelComBackKey)))
		return E_FAIL;

	/* For.Com_Sound */
	//if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_ShortCutDoor"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
	//	return E_FAIL;

	return S_OK;
}


HRESULT CShortCutDoor::LoadFromJson()
{
	string modelName = "Lock";
	if (FAILED(LoadAnimationEventsFromJson(modelName, m_pModelComBackKey)))
		return E_FAIL;

	if (FAILED(LoadAnimationEventsFromJson(modelName, m_pModelComFrontKey)))
		return E_FAIL;
	if (FAILED(LoadAnimationStatesFromJson(modelName, m_pAnimatorFrontKey)))
		return E_FAIL;
	if (FAILED(LoadAnimationStatesFromJson(modelName, m_pAnimatorBackKey)))
		return E_FAIL;

	modelName = m_pModelCom->Get_ModelName();
	if (FAILED(LoadAnimationEventsFromJson(modelName, m_pModelCom)))
		return E_FAIL;
	if (FAILED(LoadAnimationStatesFromJson(modelName, m_pAnimator)))
		return E_FAIL;
	return S_OK;
}


HRESULT CShortCutDoor::Render_Key()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
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

	_uint		iNumMesh = m_pModelComFrontKey->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelComFrontKey->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelComFrontKey->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

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

		if (FAILED(m_pModelComFrontKey->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
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
		if (FAILED(m_pModelComFrontKey->Bind_Material(m_pShaderCom, "g_Emissive", i, aiTextureType_EMISSIVE, 0)))
		{
			_float fEmissive = 0.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
				return E_FAIL;
		}

		m_pModelComFrontKey->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelComFrontKey->Render(i)))
			return E_FAIL;
	}

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	iNumMesh = m_pModelComBackKey->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelComBackKey->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelComBackKey->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

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

		if (FAILED(m_pModelComBackKey->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
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
		if (FAILED(m_pModelComBackKey->Bind_Material(m_pShaderCom, "g_Emissive", i, aiTextureType_EMISSIVE, 0)))
		{
			_float fEmissive = 0.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
				return E_FAIL;
		}

		m_pModelComBackKey->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelComBackKey->Render(i)))
			return E_FAIL;
	}

	return  S_OK;
}

void CShortCutDoor::Start_Effect(_float fTimeDelta)
{
	//m_fEffectTime += fTimeDelta;

	//CEffectContainer::DESC ECDesc = {};
	//ECDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();

	//if (m_bCanOpen) // 문 염
	//{
	//	if (m_fEffectTime > 1.15f)
	//	{
	//		m_bEffectActive = false;
	//		XMStoreFloat4x4(&ECDesc.PresetMatrix, XMMatrixTranslation(0.f, 1.6f, -0.42f));
	//		if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_Shortcut_UnLocked_Door"), &ECDesc))
	//			MSG_BOX("이펙트 생성 실패");
	//	}
	//}
	//else 	
	//{
	//	if (m_fEffectTime > 1.4f)
	//	{
	//		m_bEffectActive = false;
	//		XMStoreFloat4x4(&ECDesc.PresetMatrix, XMMatrixTranslation(0.f, 1.5f, 0.2f));
	//		if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_Shortcut_Locked_LightningDoor_alt_real"), &ECDesc))
	//			MSG_BOX("이펙트 생성 실패");
	//		m_pPlayer->Create_LeftArm_Lightning_Hand(TEXT("EC_Shortcut_Locked_LightningHand"));
	//	}
	//}
}

void CShortCutDoor::Play_Sound(_float fTimeDelta)
{
	m_fSoundDelta += fTimeDelta;

	if (m_bCanOpen) // 문 염주 렌고쿠 쿄쥬로 ㅋㅋ
	{
		if (m_fSoundDelta > 2.f && !m_bSoundPlay[0])
		{
			m_bSoundPlay[0] = true;
			m_pSoundCom->Play_Random("AMB_OJ_DR_HeavyLocker_Open_Gear_", 3);
		}
		if (m_fSoundDelta > 4.f && !m_bSoundPlay[1])
		{
			m_bSoundPlay[1] = true;
			m_pSoundCom->Play_Random("AMB_OJ_DR_HeavyLocker_Open_Drop_", 3);
		}
		if (m_fSoundDelta > 5.f && !m_bSoundPlay[2])
		{
			m_pSoundCom->Play_Random("AMB_OJ_DR_HeavyLocker_Open_Back_Drop_", 3);
			m_bSoundPlay[2] = true;
		}

		if (m_fSoundDelta > 6.3f && !m_bSoundPlay[3])
		{
			SwitchDissolve(false, 0.35f, _float3{ 0.f, 0.f, 0.f }, {});

			m_pSoundCom->Play("AMB_OJ_DR_Metal_Squeak_Open2");
			m_bSoundPlay[3] = true;
			m_bStartSound = false;
		}
	}
	else
	{
		if (m_fSoundDelta > 1.4f)
		{
			m_pSoundCom->Play_Random("AMB_OJ_FX_Crane_Spark_S_",3);
			m_bStartSound = false;
		}
	}

}


CShortCutDoor* CShortCutDoor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CShortCutDoor* pGameInstance = new CShortCutDoor(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CShortCutDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CShortCutDoor::Clone(void* pArg)
{
	CShortCutDoor* pGameInstance = new CShortCutDoor(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CShortCutDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CShortCutDoor::Free()
{
	__super::Free();

	Safe_Release(m_pModelComFrontKey);
	Safe_Release(m_pAnimatorFrontKey);
	Safe_Release(m_pModelComBackKey);
	Safe_Release(m_pAnimatorBackKey);
}
