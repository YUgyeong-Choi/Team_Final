#include "AnimController.h"
#include "Animation.h"
#include "Animator.h"

_bool CAnimController::Condition::Evaluate(CAnimator* animator) const
{

	_float t = animator->GetCurrentAnimProgress();
	if (t < minTime || t > maxTime)
		return false;

	if (op == EOp::None)
		return true;

	switch (type)
	{
	case ParamType::Bool:
		switch (op)
		{
		case EOp::IsTrue:
			return animator->CheckBool(paramName);
		case EOp::IsFalse:
			return !animator->CheckBool(paramName);
		default:
			return false; // Bool Ÿ�Կ��� �������� �ʴ� ����
		}
	case ParamType::Int:
		switch (op)
		{
		case EOp::Greater:
			return animator->GetInt(paramName) < iThreshold;
		case EOp::Less:
			return animator->GetInt(paramName) > iThreshold;
		case EOp::NotEqual:
			return animator->GetInt(paramName) != iThreshold;
		case EOp::Equal:
			return animator->GetInt(paramName) == iThreshold;
		default:
			return false; // Int Ÿ�Կ��� �������� �ʴ� ����
		}
	case ParamType::Float:
		switch (op)
		{
		case EOp::Greater:
			return animator->GetFloat(paramName) < fThreshold;
		case EOp::Less:
			return animator->GetFloat(paramName) > fThreshold;
		default:
			return false; // Float Ÿ�Կ��� �������� �ʴ� ����
		}
	case ParamType::Trigger:
		return animator->CheckTrigger(paramName);
	default:
		return false; // �������� �ʴ� Ÿ��
	}


	//switch (op)
	//{
	//case EOp::IsTrue:   
	//	return animator->CheckBool(paramName);
	//case EOp::IsFalse: 
	//	return !animator->CheckBool(paramName);
	//case EOp::Greater:  
	//	return animator->GetFloat(paramName) > fThreshold;
	//case EOp::Less:     
	//	return animator->GetFloat(paramName) < fThreshold;
	//case EOp::Trigger:  
	//	return animator->CheckTrigger(paramName);
	////case EOp::Finished:
	////	CAnimation* cur = animator->GetCurrentAnim();
	////	if (!cur)
	////		return false;
	////	// ����� �ִϸ� �˻�
	////	if (cur->Get_isLoop())
	////		return false;
	////	// 0~1 ����ȭ ���൵�� üũ
	////	return animator->GetCurrentAnimProgress() >= 1.0f;
	//}
	return false;
}

CAnimController::CAnimController()
	:m_CurrentStateNodeId{ 0 }
{
}

CAnimController::CAnimController(const CAnimController& Prototype)
	: CBase(Prototype)
	, m_States(Prototype.m_States)
	, m_Transitions(Prototype.m_Transitions)
	, m_CurrentStateNodeId(Prototype.m_CurrentStateNodeId)
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
			if (tr.iFromNodeId != m_CurrentStateNodeId)
				continue;
			if (tr.hasExitTime)
			{
				CAnimation* currentAnim = m_pAnimator->GetCurrentAnim();
				if (!currentAnim || currentAnim->Get_isLoop())
					continue; // ���� �ִϸ��̼��� ���ų� ������ �پ�Ѱ�
				_float progress = m_pAnimator->GetCurrentAnimProgress();
				if (progress < 1.f)
					continue; // ���� �ִϸ��̼��� ������ �ʾ����� �پ�Ѱ�
			}
			if (!tr.condition.Evaluate(m_pAnimator))
				continue;

		/*	auto* fromClip = m_States[tr.fromIdx].clip;
			auto* toClip = m_States[tr.toIdx].clip;*/

			auto* fromClip = GetStateAnimationByNodeId(tr.iFromNodeId);
			auto* toClip = GetStateAnimationByNodeId(tr.iToNodeId);

			if (!fromClip || !toClip)
				continue;
			m_pAnimator->StartTransition(fromClip, toClip, tr.duration);
			m_CurrentStateNodeId = tr.iToNodeId;
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

void CAnimController::AddTransition(_int fromNode, _int toNode, const Link& link, const Condition& cond, _float duration,_bool bHasExitTime)
{	
	m_Transitions.emplace_back();
	auto& tr = m_Transitions.back();
	tr.condition = cond;
	tr.duration = duration;
	tr.iFromNodeId = fromNode;
	tr.iToNodeId = toNode;
	tr.link = link;
	tr.hasExitTime = bHasExitTime;

	//m_Transitions.push_back({
	//	[&cond, this]() { return cond.Evaluate(m_pAnimator); },
	//	duration,
	//	fromNode, // ���� ��� ID
	//	toNode,   // �� ��� ID
	//	link // ��ũ ���� 
	//	,cond // ����� ����
	//	,bHasExtiTime // �ִϸ��̼��� ���� ��쿡
	//	});
}

void CAnimController::AddTransition(_int fromNode, _int toNode, const Link& link, _float duration, _bool bHasExitTime)
{
	Condition noCond{};
	noCond.op = EOp::None;
	AddTransition(fromNode, toNode, link,
		noCond,
		duration,
		bHasExitTime);
}


void CAnimController::SetState(const string& name)
{
	auto state = FindState(name);
	if (state)
	{
		m_CurrentStateNodeId = state->iNodeId;
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
