#include "StargazerEffect.h"
#include "GameInstance.h"
#include "EffectContainer.h"
#include "Effect_Manager.h"
#include "Stargazer.h"

CStargazerEffect::CStargazerEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{ pDevice, pContext }
{
}

CStargazerEffect::CStargazerEffect(const CStargazerEffect& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CStargazerEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CStargazerEffect::Initialize(void* pArg)
{
	if (pArg == nullptr)
	{
		MSG_BOX("별바라기 위치가 필요해용");
		return E_FAIL;
	}
	DESC* pDesc = static_cast<DESC*>(pArg);
	m_pOwner = pDesc->pOwner;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_Effect()))
		return E_FAIL;

	return S_OK;
}

void CStargazerEffect::Priority_Update(_float fTimeDelta)
{
	if (m_eStatus == STARGAZER_STATUS::DEACTIVATE)
	{
		for (auto& pTransform : m_pButterflyTrans_D)
		{
			if (pTransform)
				pTransform->UpdateOrbit(fTimeDelta);
		}
	}
	else if (m_eStatus == STARGAZER_STATUS::ACTIVATE)
	{

	}

	if (KEY_DOWN(DIK_LBRACKET))
	{
		CEffectContainer::DESC desc = {};
		desc.pSocketMatrix = m_pOwner->Get_TransfomCom()->Get_WorldMatrix_Ptr();
		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(0.f, 3.f, 0.f));
		(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Activating_Shrink"), &desc));
	}

}

void CStargazerEffect::Update(_float fTimeDelta)
{

}

void CStargazerEffect::Late_Update(_float fTimeDelta)
{
}

HRESULT CStargazerEffect::Render()
{

	return S_OK;
}

void CStargazerEffect::Activate_Stargazer()
{                                                                                                                                                                                                                                                                               
	for (auto& pEff : m_pButterflyEffect_D)
	{
		pEff->End_Effect();
		pEff = nullptr;
	}
	m_eStatus = STARGAZER_STATUS::ACTIVATE;

}

HRESULT CStargazerEffect::Bind_ShaderResources()
{

	return S_OK;
}

HRESULT CStargazerEffect::Ready_Components()
{

	return S_OK;
}

HRESULT CStargazerEffect::Ready_Effect()
{
	CEffectContainer::DESC desc = {};
	desc.pSocketMatrix = m_pOwner->Get_TransfomCom()->Get_WorldMatrix_Ptr();
	XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(0.f, 1.f, 0.f));
	m_pFloatingEffect = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Deactive_FloatingParticle"), &desc));
	if (m_pFloatingEffect == nullptr)
		return E_FAIL;

	// 나비3마리.
	XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(1.5f, 1.f, 0.f));
	m_pButterflyEffect_D[0] = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Butterfly"), &desc));
	if (m_pButterflyEffect_D[0] == nullptr)
		return E_FAIL;

	XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(0.f, 0.8f, 1.f));
	m_pButterflyEffect_D[1] = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Butterfly"), &desc));
	if (m_pButterflyEffect_D[1] == nullptr)
		return E_FAIL;

	XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(0.9f, 1.2f, 1.4f));
	m_pButterflyEffect_D[2] = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Butterfly"), &desc));
	if (m_pButterflyEffect_D[2] == nullptr)
		return E_FAIL;
	
	m_pButterflyTrans_D[0] = m_pButterflyEffect_D[0]->Get_TransfomCom();
	m_pButterflyTrans_D[1] = m_pButterflyEffect_D[1]->Get_TransfomCom();
	m_pButterflyTrans_D[2] = m_pButterflyEffect_D[2]->Get_TransfomCom();

	for (auto& pTrans : m_pButterflyTrans_D)
	{
		// 나비들 초기 공전 정보를 설정함
		if (pTrans)
			pTrans->Set_Orbit(XMVectorSet(0.f, m_pGameInstance->Compute_Random(0.5f, 3.f), 0.f, 0.f),
				XMVector3Normalize(XMVectorSet(m_pGameInstance->Compute_Random(0.f, 0.3f), m_pGameInstance->Compute_Random(0.f, 1.f), m_pGameInstance->Compute_Random(0.f, 0.3f), 0.f)),
				m_pGameInstance->Compute_Random(0.2f, 2.f),
				m_pGameInstance->Compute_Random(0.1f, 0.8f));
	}
	return S_OK;
}

CStargazerEffect* CStargazerEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStargazerEffect* pInstance = new CStargazerEffect(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CStargazerEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CStargazerEffect::Clone(void* pArg)
{
	CStargazerEffect* pInstance = new CStargazerEffect(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CStargazerEffect");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CStargazerEffect::Free()
{
	__super::Free();

}
