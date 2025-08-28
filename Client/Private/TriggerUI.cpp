#include "TriggerUI.h"
#include "GameInstance.h"
#include "UI_Manager.h"
#include "UI_Container.h"
#include "UI_Guide.h"
#include "Camera_Manager.h"

CTriggerUI::CTriggerUI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CTriggerBox{ pDevice, pContext }
{

}

CTriggerUI::CTriggerUI(const CTriggerUI& Prototype)
	: CTriggerBox(Prototype)
{

}

HRESULT CTriggerUI::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CTriggerUI::Initialize(void* pArg)
{
   	CTriggerUI::TRIGGERUI_DESC* TriggerUIDESC = static_cast<TRIGGERUI_DESC*>(pArg);

	m_eTriggerUIType = TriggerUIDESC->eTriggerUIType;

 	if (FAILED(__super::Initialize(TriggerUIDESC)))
		return E_FAIL;


	return S_OK;
}

void CTriggerUI::Priority_Update(_float fTimeDelta)
{
	if (m_bDead)
	{
		m_pPhysXTriggerCom->RemovePhysX();
		CCamera_Manager::Get_Instance()->SetbMoveable(true);
	}

}

void CTriggerUI::Update(_float fTimeDelta)
{
}

void CTriggerUI::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTriggerUI::Render()
{
	__super::Render();

	return S_OK;
}

void CTriggerUI::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bDoOnce)
	{
		m_bDoOnce = true;
		CUI_Container::UI_CONTAINER_DESC eDesc{};
		CUI_Guide::UI_GUIDE_DESC eGuideDesc = {};

		switch (m_eTriggerUIType)
		{
		case Client::TRIGGERUI_TYPE::STATIONTITLE:
			eDesc.useLifeTime = true;
			eDesc.fLifeTime = 8.f;
			eDesc.strFilePath = TEXT("../Bin/Save/UI/Location/Location_BackGround.json");
			m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Location_UI"), &eDesc);
			eDesc.strFilePath = TEXT("../Bin/Save/UI/Location/Location_Text.json");
			m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Location_UI"), &eDesc);
			break;
		case Client::TRIGGERUI_TYPE::GUARD:
			eGuideDesc.partPaths = { TEXT("../Bin/Save/UI/Guide/Guide_Guard.json"),TEXT("../Bin/Save/UI/Guide/Guide_PerfectGuard.json") };
			eGuideDesc.pTrigger = this;
			CCamera_Manager::Get_Instance()->SetbMoveable(false);
			m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Guide"), m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Guide"), &eGuideDesc);
			break;

		case Client::TRIGGERUI_TYPE::LOCK:
			eGuideDesc.partPaths = { TEXT("../Bin/Save/UI/Guide/Guide_Lock.json") };
			eGuideDesc.pTrigger = this;
			CCamera_Manager::Get_Instance()->SetbMoveable(false);
			m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Guide"), m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Guide"), &eGuideDesc);
			break;

		default:
			break;
		}

	}
		
}

void CTriggerUI::On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{

}

void CTriggerUI::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

CTriggerUI* CTriggerUI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTriggerUI* pGameInstance = new CTriggerUI(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTriggerUI");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CTriggerUI::Clone(void* pArg)
{
	CTriggerUI* pGameInstance = new CTriggerUI(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTriggerUI");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CTriggerUI::Free()
{
	__super::Free();
}
