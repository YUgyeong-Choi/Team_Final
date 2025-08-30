#include "BossUnit.h"
#include "Player.h"
#include "GameInstance.h"
#include <PlayerFrontCollider.h>

CBossUnit::CBossUnit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEliteUnit(pDevice, pContext)
{
}

CBossUnit::CBossUnit(const CBossUnit& Prototype)
	: CEliteUnit(Prototype)
{
	m_eUnitType = EUnitType::BOSS;
}

HRESULT CBossUnit::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	// 컷씬 시작 전 대기
	m_pAnimator->Update(0.f);
	m_pModelCom->Update_Bones();
	m_pAnimator->SetPlaying(false);
	Ready_AttackPatternWeightForPhase1();
	return S_OK;
}

void CBossUnit::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
	if (CalculateCurrentHpRatio() <= m_fPhase2HPThreshold && m_bIsPhase2 == false)
	{
		Ready_AttackPatternWeightForPhase2();
	}
}

void CBossUnit::EnterCutScene()
{
	SwitchEmissive(true, 1.f);
	m_pAnimator->SetPlaying(true);
	m_bCutSceneOn = true;
}

void CBossUnit::Reset()
{
	__super::Reset();
	m_eCurrentState = EEliteState::CUTSCENE;
	m_bIsPhase2 = false;
	m_bStartPhase2 = false;
	m_bCutSceneOn = false;
	m_pAnimator->Update(0.f);
	m_pModelCom->Update_Bones();
	m_pAnimator->SetPlaying(false);
	Ready_AttackPatternWeightForPhase1();
	m_ActiveEffect.clear();
	m_pAnimator->SetPlayRate(1.f);
}

void CBossUnit::Ready_AttackPatternWeightForPhase1()
{
}

void CBossUnit::Ready_AttackPatternWeightForPhase2()
{
}

void CBossUnit::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (eColliderType == COLLIDERTYPE::TRIGGER) // 이게 아마 플레이어의 프론트 콜라이더
	{
		if (auto pFrontTrigger = dynamic_cast<CPlayerFrontCollider*>(pOther))
		{
			if (m_bStartPhase2 == false && m_eCurrentState == EEliteState::GROGGY && m_pAnimator)
			{
				if (auto pPlayer = pFrontTrigger->Get_Owner())
				{
					if (pPlayer->Get_PlayerState() == EPlayerState::WEAKATTACKA ||
						pPlayer->Get_PlayerState() == EPlayerState::WEAKATTACKB)
					{
						m_pAnimator->SetTrigger("Fatal");
						m_fGroggyEndTimer = 0.f;
						m_bGroggyActive = false;
						m_fGroggyGauge = 0.f;
						cout << "Elite Fatal Attack" << endl;
					}
				}
			}
		}
	}
}

CBossUnit* CBossUnit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBossUnit* pInstance = new CBossUnit(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBossUnit");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CBossUnit::Clone(void* pArg)
{
	CBossUnit* pInstance = new CBossUnit(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CBossUnit");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBossUnit::Free()
{
	__super::Free();
}
