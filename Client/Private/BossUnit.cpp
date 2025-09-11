#include "BossUnit.h"
#include "Player.h"
#include "GameInstance.h"
#include "SpringBoneSys.h"
#include "LockOn_Manager.h"

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

	m_fMaxHp = 1200.f;
	m_fHp = m_fMaxHp;
	// ÄÆ¾À ½ÃÀÛ Àü ´ë±â
	m_pAnimator->Update(0.f);
	m_pModelCom->Update_Bones();
	m_pAnimator->SetPlaying(false);
	Ready_AttackPatternWeightForPhase1();
	m_fGroggyScale_Weak = 0.08f;
	m_fGroggyScale_Strong = 0.1f;
	m_fGroggyScale_Charge = 0.15f;
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

void CBossUnit::Update(_float fTimeDelta)
{
	if (CalculateCurrentHpRatio() <= 0.f)
	{
		// Á×À½ Ã³¸®
		m_bUseLockon = false;
		if (m_eCurrentState != EEliteState::DEAD)
		{
			m_eCurrentState = EEliteState::DEAD;
			m_pAnimator->SetTrigger("SpecialDie");
			CLockOn_Manager::Get_Instance()->Set_Off(this);
			m_pAnimator->SetPlayRate(1.f);
			SwitchEmissive(false, 1.f);
			SwitchFury(false, 1.f);
		}
		Safe_Release(m_pHPBar);
	}
	__super::Update(fTimeDelta);
	if (m_pSpringBoneSys)
	{
		m_pSpringBoneSys->Update(fTimeDelta);
	}
}

void CBossUnit::EnterCutScene()
{
	m_pAnimator->Get_CurrentAnimController()->SetStateToEntry();
	m_pAnimator->SetPlaying(true);
	m_bCutSceneOn = true;
	SwitchEmissive(true, 0.9f);
}

void CBossUnit::Reset()
{
	__super::Reset();
	m_eCurrentState = EEliteState::CUTSCENE;
	m_ePrevState = EEliteState::CUTSCENE;
	m_bIsPhase2 = false;
	m_bStartPhase2 = false;
	m_bPlayerCollided = false;
	m_bCutSceneOn = false;
	m_pAnimator->Update(0.f);
	m_pModelCom->Update_Bones();
	m_pAnimator->SetPlaying(false);
	Ready_AttackPatternWeightForPhase1();
	m_ActiveEffect.clear();
	m_pAnimator->SetPlayRate(1.f);
	m_fFirstChaseBeforeAttack = 2.f;
	m_pAnimator->ResetTrigger("SpecialDie");
}

void CBossUnit::Ready_AttackPatternWeightForPhase1()
{
}

void CBossUnit::Ready_AttackPatternWeightForPhase2()
{
}

void CBossUnit::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	__super::On_CollisionStay(pOther, eColliderType, HitPos, HitNormal);
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
	Safe_Release(m_pSpringBoneSys);
}
