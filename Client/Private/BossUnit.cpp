#include "BossUnit.h"
#include "Player.h"
#include "GameInstance.h"

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
	m_fMaxHp = 700.f;
	m_fHp = m_fMaxHp;
	if (FAILED(__super::Initialize(pArg)))
 		return E_FAIL;

	// �ƾ� ���� �� ���
	m_pAnimator->Update(0.f);
	m_pModelCom->Update_Bones();
	m_pAnimator->SetPlaying(false);
	Ready_AttackPatternWeightForPhase1();
	m_fGroggyScale_Weak = 0.1f;
	m_fGroggyScale_Strong = 0.12f;
	m_fGroggyScale_Charge = 0.14f;
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
}
