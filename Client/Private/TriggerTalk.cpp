#include "TriggerTalk.h"
#include "GameInstance.h"
#include "Camera_Manager.h"
#include "UI_Manager.h"
#include "UI_SelectWeapon.h"
#include "PlayerLamp.h"
#include "TriggerItem.h"
#include "Player.h"
#include "UI_Container.h"
#include "UI_Guide.h"
#include "EffectContainer.h"
#include "Effect_Manager.h"

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
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CTriggerTalk::Initialize(void* pArg)
{
	CTriggerTalk::TRIGGERTALK_DESC* TriggerTalkDESC = static_cast<TRIGGERTALK_DESC*>(pArg);
	m_eTriggerSoundType = TriggerTalkDESC->eTriggerBoxType;
	m_bCanCancel = TriggerTalkDESC->bCanCancel;

	if (FAILED(__super::Initialize(TriggerTalkDESC)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (TriggerTalkDESC->gameObjectTag != "")
	{
		if (FAILED(Ready_TriggerObject(TriggerTalkDESC)))
			return E_FAIL;
	}

	if (m_eTriggerSoundType == TRIGGERSOUND_TYPE::SELECTWEAPON)
	{
		if (FAILED(Ready_Effect())) 
			return E_FAIL;
	}

	return S_OK;
}

void CTriggerTalk::Priority_Update(_float fTimeDelta)
{
	if (m_bDead)
	{
		m_pPhysXTriggerCom->RemovePhysX();
		CCamera_Manager::Get_Instance()->SetbMoveable(true);

		if (m_eTriggerSoundType == TRIGGERSOUND_TYPE::MONADLIGHT)
		{
			//add_item..
			CUI_Manager::Get_Instance()->Activate_UI(TEXT("Pickup_Item"), true);

			CUI_Container::UI_CONTAINER_DESC eDesc{};
			eDesc.fDelay = 0.5f;
			eDesc.useLifeTime = true;
			eDesc.fLifeTime = 8.f;
			eDesc.strFilePath = TEXT("../Bin/Save/UI/Popup/Lamp_Description.json");

			m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Lamp_Desc"), &eDesc);

			CUI_Guide::UI_GUIDE_DESC eGuideDesc{};
			eGuideDesc.fDelay = 1.25f;
			eGuideDesc.partPaths = { TEXT("../Bin/Save/UI/Guide/Guide_Belt.json") };
 			CCamera_Manager::Get_Instance()->SetbMoveable(false);
			m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Guide"), m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player_UI_Guide"), &eGuideDesc);

			m_pPlayer->Add_Icon(TEXT("Prototype_GameObject_Lamp"));
			m_pPlayer->Get_PlayerLamp()->SetbLampVisible(true);

			eDesc.fDelay = 3.f;
			eDesc.useLifeTime = true;
			eDesc.fLifeTime = 8.f;
			eDesc.strFilePath = TEXT("../Bin/Save/UI/Info/Info_Lamp.json");
			m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Lamp_Desc"), &eDesc);
			//CUI_Manager::Get_Instance()->On_Panel();

			CGameObject* pLampLight = m_pGameInstance->Find_CustomLight(TEXT("Lamp_Light"))->back();
			dynamic_cast<CDH_ToolMesh*>(pLampLight)->SetIntensity(0.f);
		}
			

		if (m_pTriggerObject)
			m_pTriggerObject->Set_bDead();

		if (m_pEffectSet)
			m_pEffectSet->End_Effect();

		return;
	}

	if (!m_pPlayer)
		m_pPlayer = GET_PLAYER(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION));

}

void CTriggerTalk::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;
	
	
	

	if (m_bTalkActive && !m_bDoOnce)
	{
		

		/* [ 대화 시작 ] */
   		if (KEY_DOWN(DIK_E))
		{
			m_bDoOnce = true;
			m_bTalkActive = false;
			m_iSoundIndex = 0;

			_float offSet = 1.7f;
			if (m_eTriggerSoundType == TRIGGERSOUND_TYPE::MONADLIGHT)
				offSet = 0.f;



			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(true, this, false, offSet);
			CCamera_Manager::Get_Instance()->SetbMoveable(false);

			m_pSoundCom->SetVolume(m_vecSoundData[m_iSoundIndex].strSoundTag, 0.5f * g_fInteractSoundVolume);
			m_pSoundCom->Play(m_vecSoundData[m_iSoundIndex].strSoundTag);

			// 여기에 조사한다 UI 비활성화
			CUI_Manager::Get_Instance()->Off_Panel();
			CUI_Manager::Get_Instance()->Activate_Popup(false);
			// 말하는 UI활성화
			CUI_Manager::Get_Instance()->Activate_TalkScript(true);
			CUI_Manager::Get_Instance()->Update_TalkScript(m_vecSoundData[m_iSoundIndex].strSpeaker, m_vecSoundData[m_iSoundIndex].strSoundText, m_bAutoTalk);

			// 모나드 관련 
			if (m_eTriggerSoundType == TRIGGERSOUND_TYPE::MONADLIGHT)
			{
				m_pPlayer->OnTriggerEvent(CPlayer::eTriggerEvent::MONAD_GET_START);
			}

		}
	}

	if (m_bDoOnce)
	{
		// 대화 중 끌 수 있다면
		if (KEY_DOWN(DIK_ESCAPE) || BUTTON_FINISH == CUI_Manager::Get_Instance()->Check_Script_Click_Button())
		{
			m_bDoOnce = false;
			m_bTalkActive = false;
			m_iSoundIndex = -1;
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true, 0.f);
			CCamera_Manager::Get_Instance()->SetbMoveable(true);
			m_pSoundCom->StopAll();
			// UI 비활성화
			CUI_Manager::Get_Instance()->Activate_TalkScript(false);
			CUI_Manager::Get_Instance()->On_Panel();

			if (m_eTriggerSoundType == TRIGGERSOUND_TYPE::MONADLIGHT)
			{
				m_pPlayer->OnTriggerEvent(CPlayer::eTriggerEvent::MONAD_GET_END);
			}
			return;
		}

		_vector vDir = m_pTransformCom->Get_State(STATE::POSITION) - m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
		vDir = XMVector3Normalize(XMVectorSet(XMVectorGetX(vDir), 0, XMVectorGetZ(vDir), 0)); // Y 제거
		_bool bFinish = m_pPlayer->Get_TransfomCom()->RotateToDirectionSmoothly(vDir, 0.005f);

		// 무기 선택 관련
		if (m_eTriggerSoundType == TRIGGERSOUND_TYPE::SELECTWEAPON && bFinish)
		{
			XMVECTOR backDir = XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -1;
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_TargetYawPitch(backDir, 15.f);
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
		// 여기에 조사한다 UI 활성화 해두면 됨
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
		// 여기에 조사한다 UI 비활성화 해두면 됨
		CUI_Manager::Get_Instance()->Activate_Popup(false);
		m_bTalkActive = false;
	}

}

HRESULT CTriggerTalk::Ready_Components()
{
	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_Trigger"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
		return E_FAIL;

	return S_OK;
}


HRESULT CTriggerTalk::Ready_TriggerObject(TRIGGERTALK_DESC* TriggerTalkDESC)
{
	m_iLevelID = m_pGameInstance->GetCurrentLevelIndex();
	wstring strTriggerObjectTag = L"Prototype_GameObject_";
	strTriggerObjectTag += wstring(TriggerTalkDESC->gameObjectTag.begin(), TriggerTalkDESC->gameObjectTag.end());

	CTriggerItem::TRIGGERITEM_DESC Desc{};
	Desc.triggerWorldMatrix = m_pTransformCom->Get_WorldMatrix();
	Desc.vOffSetObj = TriggerTalkDESC->vOffSetObj;
	Desc.vScaleObj = TriggerTalkDESC->vScaleObj;

	if (FAILED(m_pGameInstance->Add_GameObjectReturn(m_iLevelID, strTriggerObjectTag.c_str(),
		m_iLevelID, TEXT("Layer_TriggerItem"), &m_pTriggerObject, &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTriggerTalk::Ready_Effect()
{
	CEffectContainer::DESC Lightdesc = {};
	
	Lightdesc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();

	XMStoreFloat4x4(&Lightdesc.PresetMatrix, XMMatrixScaling(0.25f,0.25f,0.25f));

	Lightdesc.PresetMatrix._41 = -1.25f;
	Lightdesc.PresetMatrix._42 = 1.f;
	Lightdesc.PresetMatrix._43 = -0.1f;
	

	m_pEffectSet = static_cast<CEffectContainer*>(MAKE_EFFECT(m_pGameInstance->GetCurrentLevelIndex(), TEXT("EC_Interaction_S1"), &Lightdesc));

	if(!m_pEffectSet)
	   MSG_BOX("이펙트 생성 실패함");

	return S_OK;
}

void CTriggerTalk::Next_Talk()
{
	m_iSoundIndex++;
	if (m_iSoundIndex >= m_vecSoundData.size())
	{
		if (m_eTriggerSoundType == TRIGGERSOUND_TYPE::SELECTWEAPON)
		{
			// 무기 선택 UI 활성화
			m_bActive = false;
			CUI_Manager::Get_Instance()->Activate_TalkScript(false);

			CGameObject* pObj = m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_SelectWeapon"));

			if (nullptr == pObj)
			{
				CUI_SelectWeapon::SELECT_WEAPON_UI_DESC eDesc{};

				eDesc.pTarget = this;
				eDesc.strFilePath = TEXT("../Bin/Save/UI/SelectWeapon/SelectWeapon_Background.json");

				m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_SelectWeapon"),
					m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_SelectWeapon"), &eDesc);
				return;
			}
			
			
		}
		else
		{
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true, 0.f);
			CCamera_Manager::Get_Instance()->SetbMoveable(true);
			if (m_eTriggerSoundType == TRIGGERSOUND_TYPE::MONADLIGHT)
				m_pPlayer->Get_Animator()->SetTrigger("EndInteraction");
			m_bDead = true;
			m_pPhysXTriggerCom->RemovePhysX();
			CUI_Manager::Get_Instance()->Activate_TalkScript(false);
			
		}
	}
	else
	{
		m_pSoundCom->SetVolume(m_vecSoundData[m_iSoundIndex].strSoundTag, 0.5f * g_fInteractSoundVolume);
		m_pSoundCom->Play(m_vecSoundData[m_iSoundIndex].strSoundTag);
		// 말하는 UI 대사 변경
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
	Safe_Release(m_pSoundCom);

}
