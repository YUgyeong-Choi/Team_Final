#include "TriggerTalk.h"
#include "GameInstance.h"
#include "Camera_Manager.h"
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
			m_pSoundCom->Play(m_vecSoundData[m_iSoundIndex].strSoundTag);
			// ���⿡ �����Ѵ� UI ��Ȱ��ȭ
			// ���ϴ� UIȰ��ȭ
		}
	}

	if (m_bDoOnce)
	{
		if (KEY_DOWN(DIK_F))
			m_bAutoTalk = !m_bAutoTalk;

		// ��ȭ �� �� �� �ִٸ�
		if (m_bCanCancel && KEY_DOWN(DIK_ESCAPE))
		{
			m_bDoOnce = false;
			m_bTalkActive = false;
			m_iSoundIndex = -1;
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true);
			CCamera_Manager::Get_Instance()->SetbMoveable(true);
			m_pSoundCom->StopAll();
			// UI ��Ȱ��ȭ
		}
	}

	if (m_bDoOnce)
	{
		// Auto Talk Active True
		if (m_bAutoTalk)
		{
			_bool bIsPlaying = false;
			bIsPlaying = m_pSoundCom->IsPlaying(m_vecSoundData[m_iSoundIndex].strSoundTag);

			if (!bIsPlaying)
			{
				m_iSoundIndex++;
				if (m_iSoundIndex >= m_vecSoundData.size())
				{
					m_bDead = true;
					m_pPhysXTriggerCom->RemovePhysX();
					CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true);
					CCamera_Manager::Get_Instance()->SetbMoveable(true);
				}
				else
				{
					m_pSoundCom->Play(m_vecSoundData[m_iSoundIndex].strSoundTag);
					// ���ϴ� UI ��� ����
				}
			}
		}
		else
		{
			if (KEY_DOWN(DIK_SPACE))
			{
				m_pSoundCom->StopAll();
				m_iSoundIndex++;
				if (m_iSoundIndex >= m_vecSoundData.size())
				{
					m_bDead = true;
					m_pPhysXTriggerCom->RemovePhysX();
					CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true);
					CCamera_Manager::Get_Instance()->SetbMoveable(true);
				}
				else
				{
					m_pSoundCom->Play(m_vecSoundData[m_iSoundIndex].strSoundTag);
					// ���ϴ� UI ��� ����
				}
			}
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
		m_bTalkActive = true;
	}
}

void CTriggerTalk::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bDoOnce)
	{
		// ���⿡ �����Ѵ� UI ��Ȱ��ȭ �صθ� ��
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
