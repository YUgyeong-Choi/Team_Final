#include "AnimController.h"
#include "Animation.h"
#include "Animator.h"
#include "Model.h"

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
			return animator->CheckBool(paramName) == false;
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
	m_TransitionResult.bTransition = false;
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

			auto* fromClip = GetStateAnimationByNodeId(tr.iFromNodeId);
			auto* toClip = GetStateAnimationByNodeId(tr.iToNodeId);

			if (!fromClip || !toClip)
				continue;

			m_TransitionResult.bTransition = true;
			m_TransitionResult.pFromAnim = fromClip;
			m_TransitionResult.pToAnim = toClip;
			m_TransitionResult.fDuration = tr.duration;
			m_CurrentStateNodeId = tr.iToNodeId;
			break;
		}
	}
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

TransitionResult& CAnimController::CheckTransition()
{
	return m_TransitionResult;
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

void CAnimController::SetState(_int iNodeId)
{
	auto state = FindStateByNodeId(iNodeId);
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

json CAnimController::Serialize()
{
	json j;
	// ���� ����ȭ
	for (const auto& state : m_States)
	{
		j["Anim States"].push_back({
			{"NodeId", state.iNodeId},
			{"Name", state.stateName},
			{"Clip", state.clip ? state.clip->Get_Name() : ""},
			{"NodePos", { state.fNodePos.x, state.fNodePos.y }},
			});
	}

	// Ʈ������ ����ȭ
	for (const auto& tr : m_Transitions)
	{
		j["Transitions"].push_back({
			{"FromNodeId", tr.iFromNodeId},
			{"ToNodeId", tr.iToNodeId},
			{"Duration", tr.duration},
			{"HasExitTime", tr.hasExitTime},
			{"Condition", {
				{"Type", static_cast<int>(tr.condition.type)},
				{"ParamName", tr.condition.paramName},
				{"Op", static_cast<int>(tr.condition.op)},
				{"MinTime", tr.condition.minTime},
				{"MaxTime", tr.condition.maxTime},
				{"iThreshold", tr.condition.iThreshold},
				{"fThreshold", tr.condition.fThreshold}
			}},
			{"Link", {
				{"LinkId", tr.link.iLinkId},
				{"FromNodeId", tr.link.iLinkStartID},
				{"ToNodeId", tr.link.iLinkEndID}
			}}
			});
	}
	return j;
}

void CAnimController::Deserialize(const json& j)
{
	if (j.contains("Anim States") && j["Anim States"].is_array())
	{
		for (const auto& state : j["Anim States"])
		{
			if (state.contains("NodeId") && state.contains("Name") && state.contains("Clip"))
			{
				_int nodeId = state["NodeId"];
				string name = state["Name"];
				string clipName = state["Clip"];
				_float2 pos = { 0.f, 0.f };
				if (state.contains("NodePos"))
				{
					pos.x = state["NodePos"][0];
					pos.y = state["NodePos"][1];
				}
				CAnimation* clip = nullptr;
				if (!clipName.empty())
				{
					auto pModel = m_pAnimator->GetModel();
					clip = pModel ? pModel->GetAnimationClipByName(clipName) : nullptr;
				}
				m_States.push_back({ name, clip, nodeId, pos });
			}
		}
	}

	// Ʈ������ ������ȭ

	if (j.contains("Transitions") && j["Transitions"].is_array())
	{
		for (const auto& tr : j["Transitions"])
		{
			if (tr.contains("FromNodeId") && tr.contains("ToNodeId") && tr.contains("Duration"))
			{
				_int fromNodeId = tr["FromNodeId"];
				_int toNodeId = tr["ToNodeId"];
				_float duration = tr["Duration"];
				_bool hasExitTime = tr.value("HasExitTime", false);
				Link link;
				if (tr.contains("Link"))
				{
					link.iLinkId = tr["Link"]["LinkId"];
					link.iLinkStartID = tr["Link"]["FromNodeId"];
					link.iLinkEndID = tr["Link"]["ToNodeId"];
				}
				Condition cond;
				if (tr.contains("Condition"))
				{
					cond.type = static_cast<ParamType>(tr["Condition"]["Type"]);
					cond.paramName = tr["Condition"]["ParamName"];
					cond.op = static_cast<EOp>(tr["Condition"]["Op"]);
					cond.minTime = tr.value("MinTime", 0.f);
					cond.maxTime = tr.value("MaxTime", 1.f);
					cond.iThreshold = tr.value("iThreshold", 0);
					cond.fThreshold = tr.value("fThreshold", 0.f);
				}
				AddTransition(fromNodeId, toNodeId, link, cond, duration, hasExitTime);
			}
		}
	}
}
