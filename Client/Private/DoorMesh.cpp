#include "DoorMesh.h"
#include "GameInstance.h"
#include "Player.h"
#include "Client_Calculation.h"
CDoorMesh::CDoorMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CDynamicMesh{ pDevice, pContext }
{

}

CDoorMesh::CDoorMesh(const CDoorMesh& Prototype)
	: CDynamicMesh(Prototype)
{

}

HRESULT CDoorMesh::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CDoorMesh::Initialize(void* pArg)
{
	CDoorMesh::DOORMESH_DESC* pDoorMeshDESC = static_cast<DOORMESH_DESC*>(pArg);

	m_eInteractType = pDoorMeshDESC->eInteractType;
 	m_pBGM = pDoorMeshDESC->pBGM;
	Safe_AddRef(m_pBGM);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	Ready_Trigger(pDoorMeshDESC);

	return S_OK;
}

void CDoorMesh::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_bCanActive && !m_bFinish)
	{
		if (KEY_DOWN(DIK_E))
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(m_eMeshLevelID), TEXT("Layer_Player")));
			pPlayer->Interaction_Door(m_eInteractType, this);
			m_bFinish = true;
		}
	}

	Play_BGM(fTimeDelta);
}

void CDoorMesh::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CDoorMesh::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CDoorMesh::Render()
{
	__super::Render();

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

void CDoorMesh::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if(!m_bFinish)
		m_bCanActive = true;
}

void CDoorMesh::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bFinish)
		m_bCanActive = false;


	m_bInSound = !m_bInSound;
	m_bBGMToZero = true;
}

void CDoorMesh::Play_Sound()
{
	switch (m_eInteractType)
	{
	case Client::TUTORIALDOOR:
		m_pSoundCom->SetVolume("AMB_OJ_DR_BossGate_SlidingDoor_Open", 0.5f * g_fInteractSoundVolume);
		m_pSoundCom->Play("AMB_OJ_DR_BossGate_SlidingDoor_Open");
		break;
	default:
		break;
	}

}

void CDoorMesh::Play_BGM(_float fTimeDelta)
{	
	/*
	// BGM
	if (m_bBGMToZero)
	{
		if (m_bInSound)
		{
			//안에서 밖으로 나감
			// m_fBGMVolume 이 1일텐데 0으로 lerp할거임
			m_fBGMVolume = LerpFloat(m_fBGMVolume, 0.f, fTimeDelta * 2.f);
			m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);

			if (m_fBGMVolume < 0.01f)
			{
				switch (m_eInteractType)
				{
				case Client::TUTORIALDOOR:
					m_pBGM->Stop();
					m_pGameInstance->Release_Single_Sound("AMB_SS_CentralstationB_Inside");
					m_pBGM = m_pGameInstance->Get_Single_Sound("AMB_SS_Train_Out_Rain");
					m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);
					m_pBGM->Play();
					break;
				default:
					break;
				}


				m_bBGMToZero = false;
				m_bBGMToVolume = true;
			}
		}
		else
		{
			////밖에서 안으로
			//m_pBGM->Stop();
			//switch (m_eInteractType)
			//{
			//case Client::TUTORIALDOOR:
			//	m_pGameInstance->Release_Single_Sound("AMB_SS_Train_Out_Rain");
			//	m_pBGM = m_pGameInstance->Get_Single_Sound("AMB_SS_CentralstationB_Inside");
			//	m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);
			//	break;
			//default:
			//	break;
			//}
			//m_pBGM->Play();
		}
	}



	if (m_bBGMToVolume)
	{
		if (m_bInSound)
		{
			// m_fBGMVolume 이 0일텐데 1로 lerp할거임
			m_fBGMVolume = LerpFloat(m_fBGMVolume, 1.f, fTimeDelta*2.f);
			m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);

			// 만약에 m_fBGMVolume가 1이 되면
			if(m_fBGMVolume > 0.99f)
				m_bBGMToVolume = false;
		}
		else
		{
			////밖에서 안으로
			//m_pBGM->Stop();
			//switch (m_eInteractType)
			//{
			//case Client::TUTORIALDOOR:
			//	m_pBGM = m_pGameInstance->Get_Single_Sound("AMB_SS_CentralstationB_Inside");
			//	m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);
			//	break;
			//default:
			//	break;
			//}
			//m_pBGM->Play();
		}
	}

	*/
}

HRESULT CDoorMesh::Ready_Components(void* pArg)
{
	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysXTrigger"), reinterpret_cast<CComponent**>(&m_pPhysXTriggerCom))))
		return E_FAIL;

	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_CutSceneDoor"),TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CDoorMesh::Ready_Trigger(DOORMESH_DESC* pDesc)
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


CDoorMesh* CDoorMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDoorMesh* pGameInstance = new CDoorMesh(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CDoorMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CDoorMesh::Clone(void* pArg)
{
	CDoorMesh* pGameInstance = new CDoorMesh(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CDoorMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CDoorMesh::Free()
{
	__super::Free();

	Safe_Release(m_pPhysXTriggerCom);
	Safe_Release(m_pSoundCom);
	Safe_Release(m_pBGM);
}
