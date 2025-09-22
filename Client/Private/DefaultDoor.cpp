#include "DefaultDoor.h"
#include "GameInstance.h"
#include "Player.h"

CDefaultDoor::CDefaultDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CDefaultDoor::CDefaultDoor(const CDefaultDoor& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CDefaultDoor::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CDefaultDoor::Initialize(void* pArg)
{
	CDefaultDoor::DEFAULTDOOR_DESC* DefaultDoorDESC = static_cast<DEFAULTDOOR_DESC*>(pArg);

	m_eMeshLevelID = DefaultDoorDESC->m_eMeshLevelID;
	m_szMeshID = DefaultDoorDESC->szMeshID;

	
	m_vColliderOffSet = DefaultDoorDESC->vColliderOffSet;
	m_eInteractType = DefaultDoorDESC->eInteractType;

	if (FAILED(__super::Initialize(DefaultDoorDESC)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(DefaultDoorDESC->WorldMatrix);

	if (FAILED(Ready_Collider(DefaultDoorDESC)))
		return E_FAIL;

	if (FAILED(Ready_Trigger(DefaultDoorDESC)))
		return E_FAIL;

	return S_OK;
}

void CDefaultDoor::Priority_Update(_float fTimeDelta)
{
	if (!m_pPlayer)
		m_pPlayer = dynamic_cast<CPlayer*>(GET_PLAYER(m_pGameInstance->GetCurrentLevelIndex()));
	Update_ColliderPos();

	if (m_bStartSound)
		Play_Sound(fTimeDelta);
}

void CDefaultDoor::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CDefaultDoor::Late_Update(_float fTimeDelta)
{
	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom))
	{
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
	}
}

HRESULT CDefaultDoor::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		_float Emissive = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &Emissive, sizeof(_float))))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
		{
			return E_FAIL;
		}

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
		

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom))
	{
		if (m_pGameInstance->Get_RenderCollider())
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
				return E_FAIL;

			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXTriggerCom)))
				return E_FAIL;
		}
	}
#endif


	return S_OK;
}

void CDefaultDoor::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CDefaultDoor::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CDefaultDoor::Register_Events()
{
}

AABBBOX CDefaultDoor::GetWorldAABB() const
{
	PxBounds3 wb = m_pPhysXActorCom->Get_Actor()->getWorldBounds();
	AABBBOX worldBox{ {wb.minimum.x, wb.minimum.y, wb.minimum.z},
					  {wb.maximum.x, wb.maximum.y, wb.maximum.z} };

	return worldBox;
}

void CDefaultDoor::Update_ColliderPos()
{
	_matrix WorldMatrix = m_pTransformCom->Get_WorldMatrix(); //월드행렬

	// 행렬 → 스케일, 회전, 위치 분해
	_vector vScale, vRotationQuat, vTranslation;
	XMMatrixDecompose(&vScale, &vRotationQuat, &vTranslation, WorldMatrix);

	// 위치 추출
	_float3 vPos;
	XMStoreFloat3(&vPos, vTranslation);
	vPos.x += XMVectorGetX(m_vColliderOffSet);
	vPos.y += XMVectorGetY(m_vColliderOffSet);
	vPos.z += XMVectorGetZ(m_vColliderOffSet);

	// 회전 추출
	_float4 vRot;
	XMStoreFloat4(&vRot, vRotationQuat);

	// PxTransform으로 생성
	PxTransform physxTransform(PxVec3(vPos.x, vPos.y, vPos.z), PxQuat(vRot.x, vRot.y, vRot.z, vRot.w));
	m_pPhysXActorCom->Set_Transform(physxTransform);
}

HRESULT CDefaultDoor::Ready_Components(void* pArg)
{
	CDefaultDoor::DEFAULTDOOR_DESC* DefaultDoorDESC = static_cast<DEFAULTDOOR_DESC*>(pArg);

	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(m_eMeshLevelID), DefaultDoorDESC->szModelPrototypeTag/*_wstring(TEXT("Prototype_Component_Model_")) + m_szMeshID*/,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysXTrigger"), reinterpret_cast<CComponent**>(&m_pPhysXTriggerCom))))
		return E_FAIL;
	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_CutSceneDoor"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CDefaultDoor::Ready_Trigger(DEFAULTDOOR_DESC* pDesc)
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


HRESULT CDefaultDoor::LoadAnimationEventsFromJson(const string& modelName, CModel* pModelCom)
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

HRESULT CDefaultDoor::LoadAnimationStatesFromJson(const string& modelName, CAnimator* pAnimator)
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

void CDefaultDoor::Move_Player(_float fTimeDelta)
{
}

void CDefaultDoor::Play_Sound(_float fTimeDelta)
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
			m_pSoundCom->SetVolume("AMB_OJ_DR_Train_Slide_M", 0.9f * g_fInteractSoundVolume);
			m_pSoundCom->Play("AMB_OJ_DR_Train_Slide_M");
			m_fSoundDelta = 0.f;
		}
		break;
	}
	default:
		break;
	}
}

HRESULT CDefaultDoor::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

HRESULT CDefaultDoor::Ready_Collider(void* pArg)
{
	CDefaultDoor::DEFAULTDOOR_DESC* DefaultDoorDESC = static_cast<DEFAULTDOOR_DESC*>(pArg);

	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
	positionVec += VectorToPxVec3(DefaultDoorDESC->vColliderOffSet);

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxVec3 halfExtents = VectorToPxVec3(DefaultDoorDESC->vColliderSize);
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_MAP;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_Kinematic(true);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::C);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

CDefaultDoor* CDefaultDoor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDefaultDoor* pGameInstance = new CDefaultDoor(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CDefaultDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CDefaultDoor::Clone(void* pArg)
{
	CDefaultDoor* pGameInstance = new CDefaultDoor(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CDefaultDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CDefaultDoor::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAnimator);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pEmissiveCom);

	Safe_Release(m_pPhysXActorCom);
	Safe_Release(m_pPhysXTriggerCom);

	if (m_pSoundCom)
	{
		m_pSoundCom->StopAll();
		Safe_Release(m_pSoundCom);
	}
}
