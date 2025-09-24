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

	if (pDesc->bIsPlayer)
		return S_OK;

	if (FAILED(Ready_Effect()))
		return E_FAIL;

	return S_OK;
}

void CStargazerEffect::Priority_Update(_float fTimeDelta)
{
	for (auto& pTransform : m_pButterflyTrans)
	{
		if (pTransform)
			pTransform->UpdateOrbit(fTimeDelta);
	}
	if (m_pPlayerButterflyTrans)
		m_pPlayerButterflyTrans->UpdateOrbit(fTimeDelta);

	if (m_eStatus == STARGAZER_STATUS::DEACTIVATE)
	{
		//m_fFlyingParticleTicker += fTimeDelta;
		//if (m_fFlyingParticleTicker > m_fFlyingParticleInterval)
		//{
		//	m_fFlyingParticleTicker = 0.f;
		//	m_fFlyingParticleInterval = m_pGameInstance->Compute_Random(0.5f, 1.f);
		//	CEffectContainer::DESC desc = {};
		//	XMStoreFloat4x4(&desc.PresetMatrix,
		//		XMMatrixTranslationFromVector(m_pOwner->Get_TransfomCom()->Get_State(STATE::POSITION))
		//	);
		//	if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Floating"), &desc))
		//		MSG_BOX("별바라기 떠다니는 이펙트 생성 실패");
		//}//아이거어떻게하지,,
	}
	else if (m_eStatus == STARGAZER_STATUS::ACTIVATE)
	{
	}

	if (KEY_DOWN(DIK_LBRACKET))
	{
		//CEffectContainer::DESC desc = {};
		//desc.pSocketMatrix = m_pOwner->Get_TransfomCom()->Get_WorldMatrix_Ptr();
		//XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(0.f, 3.f, 0.f));
		//(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Activating_Shrink"), &desc));
		//Activate_Stargazer_Shrink();
	}

}

void CStargazerEffect::Update(_float fTimeDelta)
{
	if (m_bDelete && m_pPlayerButterflyEffect)
	{
		m_pPlayerButterflyEffect->End_Effect();
		m_pPlayerButterflyEffect = nullptr;

		m_pPlayerButterflyTrans = nullptr;
	}
}

void CStargazerEffect::Late_Update(_float fTimeDelta)
{
}

HRESULT CStargazerEffect::Render()
{

	return S_OK;
}

void CStargazerEffect::Activate_Stargazer_Reassemble()
{
	//CEffectContainer::DESC desc = {};
	//desc.pSocketMatrix = m_pOwner->Get_TransfomCom()->Get_WorldMatrix_Ptr();
	//XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(0.f, -0.5f, 0.f));
	//if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Activate_SpinParticle"), &desc))
	//	MSG_BOX("별바라기 회전 이펙트 생성 실패");
}

void CStargazerEffect::Activate_Stargazer_Spread()
{                                                                                                                                                                                                                                                                               
	//for (auto& pEff : m_pButterflyEffect)
	//{
	//	if (pEff)
	//		pEff->End_Effect();
	//	pEff = nullptr;
	//}
	//for (auto& pTrans : m_pButterflyTrans)
	//{
	//	if (pTrans)
	//		pTrans = nullptr;
	//}
	//m_eStatus = STARGAZER_STATUS::ACTIVATE;

	//CEffectContainer::DESC desc = {};
	//desc.pSocketMatrix = m_pOwner->Get_TransfomCom()->Get_WorldMatrix_Ptr();
	//XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(0.f, 1.f, 0.f));
	//if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Activating"), &desc))
	//	MSG_BOX("별바라기 폭발 이펙트 생성 실패");
	// 
	///**************/

	//// 생성 이후 나비3마리.
	//XMStoreFloat4x4(&desc.PresetMatrix, 
	//	XMMatrixTranslation(
	//		m_pGameInstance->Compute_Random(0.2f, 0.8f), 
	//		m_pGameInstance->Compute_Random(0.6f, 1.3f),
	//		m_pGameInstance->Compute_Random(0.2f, 0.8f)));
	//m_pButterflyEffect[0] = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Butterfly_superfast"), &desc));
	//if (m_pButterflyEffect[0] == nullptr)
	//	return ;

	//XMStoreFloat4x4(&desc.PresetMatrix,
	//	XMMatrixTranslation(
	//		m_pGameInstance->Compute_Random(0.2f, 0.8f),
	//		m_pGameInstance->Compute_Random(0.6f, 1.3f),
	//		m_pGameInstance->Compute_Random(0.2f, 0.8f)));
	//m_pButterflyEffect[1] = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Butterfly_superfast"), &desc));
	//if (m_pButterflyEffect[1] == nullptr)
	//	return ;

	//XMStoreFloat4x4(&desc.PresetMatrix,
	//	XMMatrixTranslation(
	//		m_pGameInstance->Compute_Random(0.2f, 0.8f),
	//		m_pGameInstance->Compute_Random(0.6f, 1.3f),
	//		m_pGameInstance->Compute_Random(0.2f, 0.8f)));
	//m_pButterflyEffect[2] = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Butterfly_superfast"), &desc));
	//if (m_pButterflyEffect[2] == nullptr)
	//	return ;

	//m_pButterflyTrans[0] = m_pButterflyEffect[0]->Get_TransfomCom();
	//m_pButterflyTrans[1] = m_pButterflyEffect[1]->Get_TransfomCom();
	//m_pButterflyTrans[2] = m_pButterflyEffect[2]->Get_TransfomCom();

	//for (auto& pTrans : m_pButterflyTrans)
	//{
	//	// 나비들 초기 공전 정보를 설정함
	//	if (pTrans)
	//		pTrans->Set_Orbit(XMVectorSet(0.f, m_pGameInstance->Compute_Random(1.f, 2.f), 0.f, 0.f),
	//			XMVector3Normalize(XMVectorSet(0.f, 1.f, 0.f, 0.f)),
	//			m_pGameInstance->Compute_Random(0.2f, 1.f),
	//			m_pGameInstance->Compute_Random(0.2f, 0.6f));
	//}

	//m_pFloatingEffect->End_Effect();
	//m_pFloatingEffect = nullptr;
	//desc.pSocketMatrix = m_pOwner->Get_TransfomCom()->Get_WorldMatrix_Ptr();
	//XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(0.f, 1.f, 0.f));
	//m_pFloatingEffect = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Active_FloatingParticle"), &desc));
	//if (m_pFloatingEffect == nullptr)
	//	return ;


	//CEffectContainer::DESC shdesc = {};
	//shdesc.pSocketMatrix = m_pOwner->Get_TransfomCom()->Get_WorldMatrix_Ptr();
	//XMStoreFloat4x4(&shdesc.PresetMatrix, XMMatrixTranslation(0.f, 3.f, 0.f));
	//if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Activating_ShrinkParticle"), &shdesc))
	//	MSG_BOX("별바라기 수렴 이펙트 생성 실패");

}
void CStargazerEffect::Activate_Stargazer_PlayerButterfly()
{
	//if (m_pPlayerButterflyEffect)
	//	return;

	////플레이어의 월드를 가져온다.
	//CEffectContainer::DESC desc = {};
	//desc.pSocketMatrix = m_pOwner->Get_TransfomCom()->Get_WorldMatrix_Ptr();

	////나비 한마리를 소환한다.
	//XMStoreFloat4x4(&desc.PresetMatrix,
	//	XMMatrixTranslation(
	//		m_pGameInstance->Compute_Random(0.2f, 0.8f),
	//		m_pGameInstance->Compute_Random(0.6f, 1.3f),
	//		m_pGameInstance->Compute_Random(0.2f, 0.8f)));
	//m_pPlayerButterflyEffect = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Butterfly_superfastsupermany"), &desc));
	//if (m_pPlayerButterflyEffect == nullptr)
	//	return;

	////나비의 공전을 설정한다.
	//m_pPlayerButterflyTrans = m_pPlayerButterflyEffect->Get_TransfomCom();
	//if (m_pPlayerButterflyTrans)
	//{
	//	m_pPlayerButterflyTrans->Set_Orbit(XMVectorSet(0.f, m_pGameInstance->Compute_Random(1.f, 2.f), 0.f, 0.f),
	//		XMVector3Normalize(XMVectorSet(0.f, 1.f, 0.f, 0.f)),
	//		m_pGameInstance->Compute_Random(0.2f, 1.f),
	//		m_pGameInstance->Compute_Random(1.5f, 2.f));
	//}
}

void CStargazerEffect::Activate_Stargazer_Shrink()
{
	//CEffectContainer::DESC desc = {};
	//desc.pSocketMatrix = m_pOwner->Get_TransfomCom()->Get_WorldMatrix_Ptr();
	//XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(0.f, 3.f, 0.f));
	//if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Activating_ShrinkParticle"), &desc))
	//	MSG_BOX("별바라기 수렴 이펙트 생성 실패");
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
	//CEffectContainer::DESC desc = {};
	//desc.pSocketMatrix = m_pOwner->Get_TransfomCom()->Get_WorldMatrix_Ptr();
	//XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(0.f, 1.f, 0.f));
	//m_pFloatingEffect = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Deactive_FloatingParticle"), &desc));
	//if (m_pFloatingEffect == nullptr)
	//	return E_FAIL;

	//// 나비3마리.
	//XMStoreFloat4x4(&desc.PresetMatrix,
	//	XMMatrixTranslation(
	//		m_pGameInstance->Compute_Random(0.7f, 1.6f),
	//		m_pGameInstance->Compute_Random(0.7f, 1.3f),
	//		m_pGameInstance->Compute_Random(0.7f, 1.6f)));
	//m_pButterflyEffect[0] = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Butterfly"), &desc));
	//if (m_pButterflyEffect[0] == nullptr)
	//	return E_FAIL;

	//XMStoreFloat4x4(&desc.PresetMatrix,
	//	XMMatrixTranslation(
	//		m_pGameInstance->Compute_Random(0.7f, 1.6f),
	//		m_pGameInstance->Compute_Random(0.7f, 1.3f),
	//		m_pGameInstance->Compute_Random(0.7f, 1.6f)));
	//m_pButterflyEffect[1] = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Butterfly"), &desc));
	//if (m_pButterflyEffect[1] == nullptr)
	//	return E_FAIL;

	//XMStoreFloat4x4(&desc.PresetMatrix,
	//	XMMatrixTranslation(
	//		m_pGameInstance->Compute_Random(0.7f, 1.6f),
	//		m_pGameInstance->Compute_Random(0.7f, 1.3f),
	//		m_pGameInstance->Compute_Random(0.7f, 1.6f)));
	//m_pButterflyEffect[2] = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Stargazer_Butterfly"), &desc));
	//if (m_pButterflyEffect[2] == nullptr)
	//	return E_FAIL;
	//
	//m_pButterflyTrans[0] = m_pButterflyEffect[0]->Get_TransfomCom();
	//m_pButterflyTrans[1] = m_pButterflyEffect[1]->Get_TransfomCom();
	//m_pButterflyTrans[2] = m_pButterflyEffect[2]->Get_TransfomCom();

	//for (auto& pTrans : m_pButterflyTrans)
	//{
	//	// 나비들 초기 공전 정보를 설정함
	//	if (pTrans)
	//		pTrans->Set_Orbit(XMVectorSet(0.f, m_pGameInstance->Compute_Random(0.5f, 3.f), 0.f, 0.f),
	//			XMVector3Normalize(XMVectorSet(m_pGameInstance->Compute_Random(0.f, 0.3f), m_pGameInstance->Compute_Random(0.f, 1.f), m_pGameInstance->Compute_Random(0.f, 0.3f), 0.f)),
	//			m_pGameInstance->Compute_Random(0.2f, 2.f),
	//			m_pGameInstance->Compute_Random(0.4f, 0.8f));
	//}
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
