#include "TriggerBGM.h"
#include "GameInstance.h"
#include "Client_Calculation.h"

CTriggerBGM::CTriggerBGM(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CTriggerBox{ pDevice, pContext }
{

}

CTriggerBGM::CTriggerBGM(const CTriggerBGM& Prototype)
	: CTriggerBox(Prototype)
{

}

HRESULT CTriggerBGM::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CTriggerBGM::Initialize(void* pArg)
{
	CTriggerBGM::TRIGGERBGM_DESC* TriggerBGMDESC = static_cast<TRIGGERBGM_DESC*>(pArg);
	m_pBGM = TriggerBGMDESC->pBGM;
	m_strInBGM = TriggerBGMDESC->strInBGM;
	m_strOutBGM = TriggerBGMDESC->strOutBGM;


	Safe_AddRef(m_pBGM);

	if (FAILED(__super::Initialize(TriggerBGMDESC)))
		return E_FAIL;


	return S_OK;
}

void CTriggerBGM::Priority_Update(_float fTimeDelta)
{
	Play_BGM(fTimeDelta);
}

void CTriggerBGM::Update(_float fTimeDelta)
{
}

void CTriggerBGM::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTriggerBGM::Render()
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

void CTriggerBGM::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{

}

void CTriggerBGM::On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
 	_int a = 10;
}

void CTriggerBGM::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	m_bInSound = !m_bInSound;
	m_bBGMToZero = true;
	m_bBGMToVolume = false;
	m_fBGMVolume = 1.f;
}

void CTriggerBGM::Play_BGM(_float fTimeDelta)
{

	// BGM
	if (m_bBGMToZero)
	{
		if (m_bInSound)
		{
			//안에서 밖으로 나감
			// m_fBGMVolume 이 1일텐데 0으로 lerp할거임
			m_fBGMVolume = LerpFloat(m_fBGMVolume, 0.f, fTimeDelta * 4.f);
			m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);

			if (m_fBGMVolume < 0.01f)
			{
				m_pBGM->Stop();
				Safe_Release(m_pBGM);
				m_pBGM = m_pGameInstance->Get_Single_Sound(m_strOutBGM);
				m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);
				m_pBGM->Play();


				m_bBGMToZero = false;
				m_bBGMToVolume = true;
			}
		}
		else
		{
			//안에서 밖으로 나감
			m_fBGMVolume = LerpFloat(m_fBGMVolume, 0.f, fTimeDelta * 4.f);
			m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);

			if (m_fBGMVolume < 0.01f)
			{
				m_pBGM->Stop();
				Safe_Release(m_pBGM);
				m_pBGM = m_pGameInstance->Get_Single_Sound(m_strInBGM);
				m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);
				m_pBGM->Play();

				m_bBGMToZero = false;
				m_bBGMToVolume = true;
			}
		}
	}



	if (m_bBGMToVolume)
	{
		// m_fBGMVolume 이 0일텐데 1로 lerp할거임
		m_fBGMVolume = LerpFloat(m_fBGMVolume, 2.f, fTimeDelta * 3.f);
		m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);

		// 만약에 m_fBGMVolume가 1이 되면
		if (m_fBGMVolume > 0.99f)
			m_bBGMToVolume = false;
	}


}

HRESULT CTriggerBGM::Ready_Components()
{	

	return S_OK;
}


CTriggerBGM* CTriggerBGM::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTriggerBGM* pGameInstance = new CTriggerBGM(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTriggerBGM");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CTriggerBGM::Clone(void* pArg)
{
	CTriggerBGM* pGameInstance = new CTriggerBGM(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTriggerBGM");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CTriggerBGM::Free()
{
	__super::Free();

	Safe_Release(m_pBGM);
}
