#include "Trigger_Effect.h"
#include "GameInstance.h"
#include "Effect_Manager.h"
#include "EffectContainer.h"

CTrigger_Effect::CTrigger_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CTriggerBox{ pDevice, pContext }
{
}

CTrigger_Effect::CTrigger_Effect(const CTrigger_Effect& Prototype)
	: CTriggerBox{ Prototype }
{
}

HRESULT CTrigger_Effect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTrigger_Effect::Initialize(void* pArg)
{
	TRIGGER_EFFECT_DESC* eDesc = static_cast<TRIGGER_EFFECT_DESC*>(pArg);

	if (FAILED(__super::Initialize(eDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_strEffectTag = eDesc->strEffectTag;
	m_vMakePos = eDesc->vMakePos;

	return S_OK;
}

void CTrigger_Effect::Priority_Update(_float fTimeDelta)
{
	if (m_bDead)
	{
		m_pPhysXTriggerCom->RemovePhysX();

		size_t iSize = m_strEffectTag.size();
		// Á×±â Àü¿¡ ÀÌÆåÆ® »ý¼ºÇÏ°í Á×ÀÌ±â
		for (size_t i = 0; i < iSize; ++i)
		{
			//CEffectContainer::DESC eDesc = {};

			//XMStoreFloat4x4(&eDesc.PresetMatrix, XMMatrixIdentity());

			//eDesc.PresetMatrix._41 = m_vMakePos[i].x;
			//eDesc.PresetMatrix._42 = m_vMakePos[i].y;
			//eDesc.PresetMatrix._43 = m_vMakePos[i].z;

			//CEffectContainer* pEffect = static_cast<CEffectContainer*>(MAKE_EFFECT(m_pGameInstance->GetCurrentLevelIndex(), m_strEffectTag[i], &eDesc));

			//if (pEffect == nullptr)
			//	MSG_BOX("asdsadsadasd");
		}
	}
}

void CTrigger_Effect::Update(_float fTimeDelta)
{
}

void CTrigger_Effect::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTrigger_Effect::Render()
{
	__super::Render();
	return S_OK;
}

void CTrigger_Effect::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	Set_bDead();
}

void CTrigger_Effect::On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CTrigger_Effect::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT CTrigger_Effect::Ready_Components()
{
	return S_OK;
}

CTrigger_Effect* CTrigger_Effect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTrigger_Effect* pGameInstance = new CTrigger_Effect(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTrigger_Effect");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CTrigger_Effect::Clone(void* pArg)
{
	CTrigger_Effect* pGameInstance = new CTrigger_Effect(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTrigger_Effect");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CTrigger_Effect::Free()
{
	__super::Free();
}
