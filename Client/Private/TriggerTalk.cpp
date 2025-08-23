#include "TriggerTalk.h"
#include "GameInstance.h"
#include "Camera_Manager.h"
#include "UI_Manager.h"
CTriggerTalk::CTriggerTalk(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CTriggerBox{ pDevice, pContext }
{

}

CTriggerTalk::CTriggerTalk(const CTriggerTalk& Prototype)
	: CTriggerBox(Prototype)
{

}

HRESULT CTriggerTalk::Initialize_Prototype()
{
	/* �ܺ� �����ͺ��̽��� ���ؼ� ���� ä���. */

	return S_OK;
}

HRESULT CTriggerTalk::Initialize(void* pArg)
{
	CTriggerTalk::TRIGGERTALK_DESC* TriggerTalkDESC = static_cast<TRIGGERTALK_DESC*>(pArg);
	m_bCanCancel = TriggerTalkDESC->bCanCancel;

	if (FAILED(__super::Initialize(TriggerTalkDESC)))
		return E_FAIL;

	if (TriggerTalkDESC->gameObjectTag != "")
		if (FAILED(Ready_TriggerObject(TriggerTalkDESC)))
			return E_FAIL;

	return S_OK;
}

void CTriggerTalk::Priority_Update(_float fTimeDelta)
{
	if (m_bDead)
		return;

	if (!m_bActive)
	{
		if (m_eTriggerBoxType == TRIGGERBOX_TYPE::SELECTWEAPON)
		{
			// ���⼭ ���� ����
			// ���� ���� ui ���� ���� ���� ���� �Ϸ��ϸ�
			// �� Ʈ���� �ּҸ� ������ �ű⼭ setbdead�� ������
		}
	}

	if (!m_bActive)
		return;

	// ��ȭ �� �� �� �ִٸ�
	if (KEY_DOWN(DIK_ESCAPE) || BUTTON_FINISH == CUI_Manager::Get_Instance()->Check_Script_Click_Button())
	{
		m_bDoOnce = false;
		m_bTalkActive = false;
		m_iSoundIndex = -1;
		CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true);
		CCamera_Manager::Get_Instance()->SetbMoveable(true);
		m_pSoundCom->StopAll();
		// UI ��Ȱ��ȭ
		CUI_Manager::Get_Instance()->Activate_TalkScript(false);
		CUI_Manager::Get_Instance()->On_Panel();
	}

	if (m_bTalkActive && !m_bDoOnce)
	{
		// ������
		if (KEY_DOWN(DIK_E))
		{
			m_bDoOnce = true;
			m_bTalkActive = false;
			m_iSoundIndex = 0;
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(true, this, false);
			CCamera_Manager::Get_Instance()->SetbMoveable(false);

			m_pSoundCom->SetVolume(m_vecSoundData[m_iSoundIndex].strSoundTag, 0.5f * g_fInteractSoundVolume);
			m_pSoundCom->Play(m_vecSoundData[m_iSoundIndex].strSoundTag);
			// ���⿡ �����Ѵ� UI ��Ȱ��ȭ
			CUI_Manager::Get_Instance()->Off_Panel();
			CUI_Manager::Get_Instance()->Activate_Popup(false);
			// ���ϴ� UIȰ��ȭ
			CUI_Manager::Get_Instance()->Activate_TalkScript(true);
			CUI_Manager::Get_Instance()->Update_TalkScript(m_vecSoundData[m_iSoundIndex].strSpeaker, m_vecSoundData[m_iSoundIndex].strSoundText, m_bAutoTalk);
		}
	}

	if (m_bDoOnce)
	{
		if (m_eTriggerBoxType == TRIGGERBOX_TYPE::SELECTWEAPON)
		{
			CTransform* pPlayerTransform = m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player"))->Get_TransfomCom();
			pPlayerTransform->RotateToDirectionSmoothly(m_pTransformCom->Get_State(STATE::POSITION), 0.005f);
		}

		

		if (m_bAutoTalk)
		{
			_bool bIsPlaying = false;
			bIsPlaying = m_pSoundCom->IsPlaying(m_vecSoundData[m_iSoundIndex].strSoundTag);

			if (!bIsPlaying)
			{
				Next_Talk();
			}

		}

		if (KEY_DOWN(DIK_SPACE) || BUTTON_NEXT == CUI_Manager::Get_Instance()->Check_Script_Click_Button())
		{
			m_pSoundCom->StopAll();
			Next_Talk();
		}


		if (KEY_DOWN(DIK_F) || BUTTON_AUTO == CUI_Manager::Get_Instance()->Check_Script_Click_Button())
		{
			m_bAutoTalk = !m_bAutoTalk;
			CUI_Manager::Get_Instance()->Update_TalkScript(m_vecSoundData[m_iSoundIndex].strSpeaker, m_vecSoundData[m_iSoundIndex].strSoundText, m_bAutoTalk);
		}
			

	}

	
}

void CTriggerTalk::Update(_float fTimeDelta)
{
	
}

void CTriggerTalk::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTriggerTalk::Render()
{
	__super::Render();
	return S_OK;
}

void CTriggerTalk::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bDoOnce)
	{
		// ���⿡ �����Ѵ� UI Ȱ��ȭ �صθ� ��
		CUI_Manager::Get_Instance()->Activate_Popup(true);
		CUI_Manager::Get_Instance()->Set_Popup_Caption(0);

		CUI_Manager::Get_Instance()->Activate_TextScript(false);
		m_bTalkActive = true;
	}
}

void CTriggerTalk::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bDoOnce)
	{
		// ���⿡ �����Ѵ� UI ��Ȱ��ȭ �صθ� ��
		CUI_Manager::Get_Instance()->Activate_Popup(false);
		m_bTalkActive = false;
	}

}

void CTriggerTalk::Play_Sound()
{

}

HRESULT CTriggerTalk::Ready_TriggerObject(void* pArg)
{
	return S_OK;
}

void CTriggerTalk::Next_Talk()
{
	m_iSoundIndex++;
	if (m_iSoundIndex >= m_vecSoundData.size())
	{
		if (m_eTriggerBoxType == TRIGGERBOX_TYPE::SELECTWEAPON)
		{
			// ���� ���� UI Ȱ��ȭ
			m_bActive = false;
			CUI_Manager::Get_Instance()->Activate_TalkScript(false);

			//m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_SelectWeapon"), 
							 //m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_SelectWeapon"), nullptr);

			
			
		}
		else
		{
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true);
			CCamera_Manager::Get_Instance()->SetbMoveable(true);
			m_bDead = true;
			m_pPhysXTriggerCom->RemovePhysX();
			CUI_Manager::Get_Instance()->Activate_TalkScript(false);
			CUI_Manager::Get_Instance()->On_Panel();
		}
	}
	else
	{
		m_pSoundCom->SetVolume(m_vecSoundData[m_iSoundIndex].strSoundTag, 0.5f * g_fInteractSoundVolume);
		m_pSoundCom->Play(m_vecSoundData[m_iSoundIndex].strSoundTag);
		// ���ϴ� UI ��� ����
		CUI_Manager::Get_Instance()->Update_TalkScript(m_vecSoundData[m_iSoundIndex].strSpeaker, m_vecSoundData[m_iSoundIndex].strSoundText, m_bAutoTalk);
	}
}



CTriggerTalk* CTriggerTalk::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTriggerTalk* pGameInstance = new CTriggerTalk(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTriggerTalk");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CTriggerTalk::Clone(void* pArg)
{
	CTriggerTalk* pGameInstance = new CTriggerTalk(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTriggerTalk");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CTriggerTalk::Free()
{
	__super::Free();
}
