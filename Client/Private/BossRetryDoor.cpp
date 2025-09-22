#include "BossRetryDoor.h"
#include "GameInstance.h"
#include "Player.h"
#include "UI_Manager.h"
#include "Camera_Manager.h"

CBossRetryDoor::CBossRetryDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CDefaultDoor{ pDevice, pContext }
{

}

CBossRetryDoor::CBossRetryDoor(const CBossRetryDoor& Prototype)
	: CDefaultDoor(Prototype)
{

}

HRESULT CBossRetryDoor::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBossRetryDoor::Initialize(void* pArg)
{
	m_fDissolve = 1.f;
	m_bIsLOD = false;
	if (FAILED(Add_Component(0, TEXT("Prototype_Component_Texture_NoiseMap2"),
		TEXT("Dissolve2_Com"), reinterpret_cast<CComponent**>(&m_pDissolveMap))))
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;


	return S_OK;
}

void CBossRetryDoor::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);


	if (m_bCanActive && !m_bFinish)
	{
		if (KEY_DOWN(DIK_E))
		{
			m_bMoveStart = true;
			CUI_Manager::Get_Instance()->Activate_Popup(false);
			CCamera_Manager::Get_Instance()->SetbMoveable(false);
			m_pPhysXActorCom->Init_SimulationFilterData();
			m_pPhysXActorCom->Set_ShapeFlag(false, false, false);
			// [ 리트라이 도어에서 해야할 일 ] 
			// 1. 보스 전투바로 (컷씬 x)
			// 2. 플레이어 애니메이션 재생
			// 3. 디졸브 변수 올리기 (m_vDissolveGlowColor, m_fDissolve)
		}
	}

#ifdef _DEBUG
	if (KEY_DOWN(DIK_X))
	{
		m_pAnimator->Get_CurrentAnimController()->SetState("Idle");
		m_bFinish = false;
		m_bCanActive = false;
	}
#endif
}

void CBossRetryDoor::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	Move_Player(fTimeDelta);
}

void CBossRetryDoor::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CBossRetryDoor::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	//타일링을 사용 하는가? 인스턴스된 애들은 타일링 하기 번거롭겠다.
	_bool bTile = false;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bTile", &bTile, sizeof(_bool))))
		return E_FAIL;

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

void CBossRetryDoor::Reset()
{
	m_bEnd = false;
	m_bFinish = false;
	SwitchDissolve(true, 1.f, _float3{ 0.f, 0.8f, 0.5f }, {});
	m_pPhysXActorCom->Set_SimulationFilterData(m_pPhysXActorCom->Get_FilterData());
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);
}

void CBossRetryDoor::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bFinish)
	{
		m_bCanActive = true;
		CUI_Manager::Get_Instance()->Activate_Popup(true);
		CUI_Manager::Get_Instance()->Set_Popup_Caption(2);
	}
}

void CBossRetryDoor::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bFinish)
		m_bCanActive = false;


	CUI_Manager::Get_Instance()->Activate_Popup(false);
}


void CBossRetryDoor::Move_Player(_float fTimeDelta)
{
	if (m_bMoveStart)
	{
		m_bFinish = true;
		_vector vTargetPos;
		switch (m_eInteractType)
		{
		case Client::FESTIVALDOOR:
			//PlayerPos 374.990265f, 14.957887f, -48.613216f
			vTargetPos = _vector({ 374.99f, 14.95f, -48.61f, 1.f });
			break;
		case Client::FUOCO:
			//PlayerPos - 1.447618f, 0.412294f, -235.871674f
			vTargetPos = _vector({ -1.44f, 0.41f, -235.87f, 1.f });
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
			SwitchDissolve(false, 0.35f, _float3{ 0.f, 0.8f, 0.5f }, {});
		}
	}

	if (m_fDissolve < 0.1f && !m_bEnd)
	{
		m_bWalkFront = true;
	}

	if (m_bWalkFront)
	{
		_vector vTargetPos;
		switch (m_eInteractType)
		{
		case Client::FESTIVALDOOR:
			vTargetPos = _vector({ 374.99f, 14.95f, -48.61f, 1.f });
			break;
		case Client::FUOCO:
			vTargetPos = _vector({ -1.836335f, 0.296629f, -231.623810f, 1.f });
			break;
		default:
			break;
		}

		if (m_pPlayer->MoveToDoor(fTimeDelta, vTargetPos))
		{
			m_bWalkFront = false;
			m_bEnd = true;
			CCamera_Manager::Get_Instance()->SetbMoveable(true);
			switch (m_eInteractType)
			{
			case Client::FESTIVALDOOR:
			{
				CBossUnit* unit = static_cast<CBossUnit*>(m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_FestivalLeader")));
				unit->EnterCutScene();
			}
				break;
			case Client::FUOCO:
			{
				CBossUnit* unit = static_cast<CBossUnit*>(m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_FireEater")));
				unit->EnterCutScene();
			}
				break;
			default:
				break;
			}

			list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), L"BossRetryDoor");
			for (auto& obj : objList)
				m_pGameInstance->Return_PoolObject(L"BossRetryDoor", obj);
			m_pGameInstance->UseAll_PoolObjects(L"BossRetryDoor");
		}
	}
}



HRESULT CBossRetryDoor::Ready_Components(void* pArg)
{
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPBRMesh"),
		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/*m_pAnimator = CAnimator::Create(m_pDevice, m_pContext);
	if (nullptr == m_pAnimator)
		return E_FAIL;
	if (FAILED(m_pAnimator->Initialize(m_pModelCom)))
		return E_FAIL;*/

	return S_OK;
}


CBossRetryDoor* CBossRetryDoor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBossRetryDoor* pGameInstance = new CBossRetryDoor(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CBossRetryDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CBossRetryDoor::Clone(void* pArg)
{
	CBossRetryDoor* pGameInstance = new CBossRetryDoor(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CBossRetryDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CBossRetryDoor::Free()
{
	__super::Free();
}
