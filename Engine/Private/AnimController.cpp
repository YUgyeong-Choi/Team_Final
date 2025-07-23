#include "AnimController.h"
#include "Animation.h"
#include "Animator.h"

_bool CAnimController::Condition::Evaluate(CAnimator* animator) const
{
	_float t = animator->GetCurrentAnimProgress();
	if (t < minTime || t > maxTime)
		return false;
	switch (op)
	{
	case EOp::IsTrue:   
		return animator->CheckBool(paramName);
	case EOp::IsFalse: 
		return !animator->CheckBool(paramName);
	case EOp::Greater:  
		return animator->GetFloat(paramName) > threshold;
	case EOp::Less:     
		return animator->GetFloat(paramName) < threshold;
	case EOp::Trigger:  
		return animator->CheckTrigger(paramName);
	case EOp::Finished:
		CAnimation* cur = animator->GetCurrentAnim();
		if (!cur)
			return false;
		// 논루프 애니만 검사
		if (cur->Get_isLoop())
			return false;
		// 0~1 정규화 진행도를 체크
		return animator->GetCurrentAnimProgress() >= 1.0f;
	}
	return false;
}

CAnimController::CAnimController()
	:m_CurrentStateIdx{ 0 }
{
}

CAnimController::CAnimController(const CAnimController& Prototype)
	: CBase(Prototype)
	, m_States(Prototype.m_States)
	, m_Transitions(Prototype.m_Transitions)
	, m_CurrentStateIdx(Prototype.m_CurrentStateIdx)
{
}

HRESULT CAnimController::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAnimController::Initialize(void* pArg)
{
	return S_OK;
}

void CAnimController::Update(_float fTimeDelta)
{
	if (!m_pAnimator->IsBlending())
	{
		for (auto& tr : m_Transitions)
		{
			if (tr.fromIdx != m_CurrentStateIdx)
				continue;
			if (!tr.condition())
				continue;

			auto* fromClip = m_States[tr.fromIdx].clip;
			auto* toClip = m_States[tr.toIdx].clip;

			m_pAnimator->StartTransition(fromClip, toClip, tr.duration);
			m_CurrentStateIdx = tr.toIdx;
			break;
		}
	}

	m_pAnimator->Update(fTimeDelta);
}

_float CAnimController::GetStateLength(const string& name)
{
	auto state = FindState(name);
	if (state)
		return state->clip->GetClipLength();
	return 0.f;
}

void CAnimController::AddTransition(size_t fromIdx, size_t toIdx, const Condition& cond, _float duration)
{	// Transition 저장 시 condition.Evaluate(animator)
	if (fromIdx >= m_States.size() || toIdx >= m_States.size())
		return;
	m_Transitions.push_back({
		fromIdx, toIdx,
		[&]() { return cond.Evaluate(m_pAnimator); },
		duration
		});
}

void CAnimController::SetState(const string& name)
{
	auto state = FindState(name);
	if (state)
	{
		m_CurrentStateIdx = state->clipIndex;
		m_pAnimator->PlayClip(state->clip);
	}
}

CAnimController* CAnimController::Create()
{
	CAnimController* pInstance = new CAnimController();
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created CAnimController");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CAnimController* CAnimController::Clone()
{
	CAnimController* pInstance = new CAnimController(*this);
	if (FAILED(pInstance->Initialize(nullptr)))
	{
		MSG_BOX("Failed to Created CAnimController");
		Safe_Release(pInstance);
	}
	return pInstance;
}


void CAnimController::Free()
{
	__super::Free();
	m_States.clear();
	m_Transitions.clear();
}
