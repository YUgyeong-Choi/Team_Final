#include "BossDoor.h"
#include "GameInstance.h"
#include "Player.h"
#include "UI_Manager.h"
#include "Camera_Manager.h"
#include "Effect_Manager.h"
#include "EffectContainer.h"

CBossDoor::CBossDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CDefaultDoor{ pDevice, pContext }
{

}

CBossDoor::CBossDoor(const CBossDoor& Prototype)
	: CDefaultDoor(Prototype)
{

}

HRESULT CBossDoor::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CBossDoor::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	if (FAILED(LoadFromJson()))
		return E_FAIL;

	Register_Events();

	return S_OK;
}

void CBossDoor::Priority_Update(_float fTimeDelta)
{

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

			CCamera_Manager::Get_Instance()->GetCutScene()->Set_BossDoor(this);
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

	if (KEY_PRESSING(DIK_LALT) && KEY_PRESSING(DIK_E))
	{
		if (m_eInteractType == FESTIVALDOOR)
			Create_CrashDoorEffect();
	}
#endif // _DEBUG
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

		m_pAnimator->RegisterEventListener("CrashDoorEffect", [this]()
			{
				Create_CrashDoorEffect();

			});
	}
}

void CBossDoor::Create_CrashDoorEffect()
{
	CEffectContainer::DESC Lightdesc = {};
	auto a = XMLoadFloat4x4(m_pSecondModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("joint17"))) * m_pTransformCom->Get_WorldMatrix();
	_vector vPos = a.r[3];
	XMStoreFloat4x4(&Lightdesc.PresetMatrix, XMMatrixTranslationFromVector(vPos));

	if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Fes_Cutscene_DoorDistortion"), &Lightdesc))
		MSG_BOX("이펙트 생성 실패함");

	vPos.m128_f32[0] -= 1.f;
	vPos.m128_f32[1] -= 1.f;
	XMStoreFloat4x4(&Lightdesc.PresetMatrix, XMMatrixTranslationFromVector(vPos));

	if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_GL_Smoke_Circle"), &Lightdesc))
		MSG_BOX("이펙트 생성 실패함");
}

void CBossDoor::Create_RetryDoor()
{
	switch (m_eInteractType)
	{
	case Client::FESTIVALDOOR:
	{
		/* [ 축제의 인도자 문 ] */
		CDefaultDoor::DEFAULTDOOR_DESC DefaultDoorDesc{};
		DefaultDoorDesc.m_eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
		DefaultDoorDesc.szMeshID = TEXT("FestivalDoor");
		lstrcpy(DefaultDoorDesc.szName, TEXT("FestivalDoor"));

		wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_FestivalRetryDoor");
		lstrcpy(DefaultDoorDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

		_float3 vPosition = _float3(376.f, 15.00f, -48.67f);
		XMMATRIX trans = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
		XMMATRIX world = trans;
		_float4x4 matWorldFloat;
		XMStoreFloat4x4(&matWorldFloat, world);
		DefaultDoorDesc.WorldMatrix = matWorldFloat;
		DefaultDoorDesc.vColliderOffSet = { 0.f, 1.5f, 0.f, 0.f };
		DefaultDoorDesc.vColliderSize = { 0.2f, 2.f, 2.f, 0.f };

		DefaultDoorDesc.eInteractType = INTERACT_TYPE::RESTARTFESTIVAL;
		DefaultDoorDesc.vTriggerOffset = { 0.f, 0.f, 0.f, 0.f };
		DefaultDoorDesc.vTriggerSize = { 0.5f, 0.2f, 1.0f, 0.f };

		if (FAILED(m_pGameInstance->Add_GameObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Prototype_GameObject_BossRetryDoor"),
			m_pGameInstance->GetCurrentLevelIndex(), TEXT("BossRetryDoor"), &DefaultDoorDesc)))
			return;
	}
		break;
	case Client::FUOCO:
	{
		/* [ 푸쿠오 보스 문 ] */
		CDefaultDoor::DEFAULTDOOR_DESC DefaultDoorDesc{};
		DefaultDoorDesc.m_eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
		DefaultDoorDesc.szMeshID = TEXT("PuocoRetryDoor");
		lstrcpy(DefaultDoorDesc.szName, TEXT("PuocoRetryDoor"));

		wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_PuocoRetryDoor");
		lstrcpy(DefaultDoorDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

		_float3 vPosition = _float3(-1.4f, 0.31f, -235.f);
		XMMATRIX trans = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
		XMMATRIX rotY = XMMatrixRotationY(XM_PIDIV2); // = 90도
		XMMATRIX world = rotY * trans;
		_float4x4 matWorldFloat;
		XMStoreFloat4x4(&matWorldFloat, world);
		DefaultDoorDesc.WorldMatrix = matWorldFloat;
		DefaultDoorDesc.vColliderOffSet = { 0.f, 1.5f, 0.f, 0.f };
		DefaultDoorDesc.vColliderSize = { 0.2f, 2.f, 1.5f, 0.f };

		DefaultDoorDesc.eInteractType = INTERACT_TYPE::RESTARTFUOCO;
		DefaultDoorDesc.vTriggerOffset = { 0.f, 0.5f, 0.f, 0.f };
		DefaultDoorDesc.vTriggerSize = { 0.5f, 0.2f, 1.5f, 0.f };

		if (FAILED(m_pGameInstance->Add_GameObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Prototype_GameObject_BossRetryDoor"),
			m_pGameInstance->GetCurrentLevelIndex(), TEXT("BossRetryDoor"), &DefaultDoorDesc)))
			return;
	}
		break;
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


void CBossDoor::Move_Player(_float fTimeDelta)
{
	if (m_bMoveStart)
	{
		m_bFinish = true;
		_vector vTargetPos;
		switch (m_eInteractType)
		{
		case Client::FESTIVALDOOR:
			//X:375.136841, Y:14.995386, Z:-48.836079
			vTargetPos = _vector({ 375.13f, 14.9f, -48.83f, 1.f});
			break;
		case Client::FUOCO:
			vTargetPos = _vector({ -3.2f, 0.3f, -235.87f, 1.f});
			break;
		default:
			break;
		}

		if (m_pPlayer->MoveToDoor(fTimeDelta, vTargetPos))
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

	Safe_Release(m_pSecondModelCom);
	Safe_Release(m_pSecondAnimator);
}
