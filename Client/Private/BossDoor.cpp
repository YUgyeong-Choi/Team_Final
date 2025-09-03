#include "BossDoor.h"
#include "GameInstance.h"
#include "Player.h"
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

	m_pAnimator->SetPlaying(true);

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
			switch (m_eInteractType)
			{
			case Client::FESTIVALDOOR:
				// 플레이어 움직임
				// 컷씬 플레이
				break;
			case Client::FUOCO:
				// 플레이어 움직임
				// 컷씬 플레이
				break;
			default:
				break;
			}
			m_bFinish = true;
			CUI_Manager::Get_Instance()->Activate_Popup(false);
		}
	}
}

void CBossDoor::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (m_pAnimator)
		m_pAnimator->Update(fTimeDelta);

	if (m_pModelCom)
		m_pModelCom->Update_Bones();
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

void CBossDoor::Play_Sound()
{

	//switch (m_eInteractType)
	//{
	//case Client::TUTORIALDOOR:
	//	m_pSoundCom->SetVolume("AMB_OJ_DR_BossGate_SlidingDoor_Open", 0.5f * g_fInteractSoundVolume);
	//	m_pSoundCom->Play("AMB_OJ_DR_BossGate_SlidingDoor_Open");
	//	break;
	//default:
	//	break;
	//}

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
