#include "TriggerSound.h"
#include "GameInstance.h"
#include "UI_Manager.h"

CTriggerSound::CTriggerSound(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CTriggerBox{ pDevice, pContext }
{

}

CTriggerSound::CTriggerSound(const CTriggerSound& Prototype)
	: CTriggerBox(Prototype)
{

}

HRESULT CTriggerSound::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CTriggerSound::Initialize(void* pArg)
{
	CTriggerSound::TRIGGERNOMESH_DESC* TriggerNoMeshDESC = static_cast<TRIGGERNOMESH_DESC*>(pArg);


	if (FAILED(__super::Initialize(TriggerNoMeshDESC)))
		return E_FAIL;


	return S_OK;
}

void CTriggerSound::Priority_Update(_float fTimeDelta)
{
	if (m_bDoOnce)
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
				// 소리가 끝나면 스크립트도 끈다.
				CUI_Manager::Get_Instance()->Activate_TextScript(false);
			}
			else
			{
				m_pSoundCom->SetVolume(m_vecSoundData[m_iSoundIndex].strSoundTag, 0.5f * g_fInteractSoundVolume);
				m_pSoundCom->Play(m_vecSoundData[m_iSoundIndex].strSoundTag);
				CUI_Manager::Get_Instance()->Update_TextScript(m_vecSoundData[m_iSoundIndex].strSoundText);
			}
		}
	}
}

void CTriggerSound::Update(_float fTimeDelta)
{
}

void CTriggerSound::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTriggerSound::Render()
{
	__super::Render();
	return S_OK;
}

void CTriggerSound::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bDoOnce)
	{
		m_bDoOnce = true;
		m_iSoundIndex = 0;
		m_pSoundCom->SetVolume(m_vecSoundData[m_iSoundIndex].strSoundTag, 0.5f * g_fInteractSoundVolume);
		m_pSoundCom->Play(m_vecSoundData[m_iSoundIndex].strSoundTag);

		// 스크립트를 킨다.
		CUI_Manager::Get_Instance()->Activate_TextScript(true);
		CUI_Manager::Get_Instance()->Update_TextScript(m_vecSoundData[m_iSoundIndex].strSoundText);
	}
		
}

void CTriggerSound::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CTriggerSound::Play_Sound()
{
}



CTriggerSound* CTriggerSound::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTriggerSound* pGameInstance = new CTriggerSound(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTriggerSound");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CTriggerSound::Clone(void* pArg)
{
	CTriggerSound* pGameInstance = new CTriggerSound(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTriggerSound");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CTriggerSound::Free()
{
	__super::Free();
}
