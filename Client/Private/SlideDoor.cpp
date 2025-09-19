#include "SlideDoor.h"
#include "GameInstance.h"
#include "Player.h"
#include "UI_Manager.h"

CSlideDoor::CSlideDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CDefaultDoor{ pDevice, pContext }
{

}

CSlideDoor::CSlideDoor(const CSlideDoor& Prototype)
	: CDefaultDoor(Prototype)
{

}

HRESULT CSlideDoor::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CSlideDoor::Initialize(void* pArg)
{

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	return S_OK;
}

void CSlideDoor::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_bCanActive && !m_bFinish)
	{
		if (KEY_DOWN(DIK_E))
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(m_eMeshLevelID), TEXT("Layer_Player")));
			switch (m_eInteractType)
			{
			case Client::TUTORIALDOOR:
			{
				m_pPhysXActorCom->Init_SimulationFilterData();
			
				pPlayer->Interaction_Door(m_eInteractType, this);
				//CUI_Manager::Get_Instance()->Off_Panel();
				break;
			}
			default:
				break;
			}

			m_bFinish = true;
			CUI_Manager::Get_Instance()->Activate_Popup(false);
		}
	}
}

void CSlideDoor::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSlideDoor::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSlideDoor::Render()
{
	__super::Render();

	return S_OK;
}

void CSlideDoor::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bFinish)
	{
		m_bCanActive = true;
		CUI_Manager::Get_Instance()->Activate_Popup(true);
  		CUI_Manager::Get_Instance()->Set_Popup_Caption(2);
	}
}

void CSlideDoor::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bFinish)
		m_bCanActive = false;


	CUI_Manager::Get_Instance()->Activate_Popup(false);
}

void CSlideDoor::Play_Sound()
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



HRESULT CSlideDoor::Ready_Components(void* pArg)
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	return S_OK;
}

CSlideDoor* CSlideDoor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSlideDoor* pGameInstance = new CSlideDoor(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CSlideDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CSlideDoor::Clone(void* pArg)
{
	CSlideDoor* pGameInstance = new CSlideDoor(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CSlideDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CSlideDoor::Free()
{
	__super::Free();
}
