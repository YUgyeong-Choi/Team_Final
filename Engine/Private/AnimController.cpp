#include "AnimController.h"
#include "Animation.h"
#include "Animator.h"
#include "Model.h"
#include "Bone.h"

_bool CAnimController::Condition::Evaluate(CAnimator* animator) const
{
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
			return false; // Bool 타입에서 지원하지 않는 연산
		}
	case ParamType::Int:
		switch (op)
		{
		case EOp::Greater:
			return animator->GetInt(paramName) > iThreshold;
		case EOp::Less:
			return animator->GetInt(paramName) < iThreshold;
		case EOp::NotEqual:
			return animator->GetInt(paramName) != iThreshold;
		case EOp::Equal:
			return animator->GetInt(paramName) == iThreshold;
		default:
			return false; // Int 타입에서 지원하지 않는 연산
		}
	case ParamType::Float:
		switch (op)
		{
		case EOp::Greater:
			return animator->GetFloat(paramName) > fThreshold;
		case EOp::Less:
			return animator->GetFloat(paramName) < fThreshold;
		default:
			return false; // Float 타입에서 지원하지 않는 연산
		}
	case ParamType::Trigger:
		return animator->CheckTrigger(paramName);
	default:
		return false; // 지원하지 않는 타입
	}
	return false;
}

_bool CAnimController::Transition::Evaluates(CAnimator* animator) const
{
	_float t = animator->GetCurrentAnimProgress();
	if (t < minTime || t > maxTime)
		return false;

	if (conditions.empty())
		return true; // 조건이 없으면 항상 true

	for (const auto& condition : conditions)
	{
		if (!condition.Evaluate(animator))
			return false; // 하나라도 false면 전체 false
	}
	return true; // 모든 조건이 true
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
					continue; // 현재 애니메이션이 없거나 루프면 뛰어넘게
				_float progress = m_pAnimator->GetCurrentAnimProgress();
				if (progress < 1.f)
					continue; // 현재 애니메이션이 끝나지 않았으면 뛰어넘게
			}
			if (!tr.Evaluates(m_pAnimator))
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
	tr.conditions.push_back(cond); // 조건 추가
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

void CAnimController::AddTransitionMultiCondition(_int fromNode, _int toNode, const Link& link, const vector<Condition>& conditions, _float duration, _bool bHasExitTime)
{
	m_Transitions.emplace_back();
	auto& tr = m_Transitions.back();
	tr.conditions = conditions; 
	tr.duration = duration;
	tr.iFromNodeId = fromNode;
	tr.iToNodeId = toNode;
	tr.link = link;
	tr.hasExitTime = bHasExitTime;
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

void CAnimController::ResetTransAndStates()
{
	//for ( auto& state : m_States)
	//{
	//	if (state.clip)
	//		Safe_Release(state.clip);
	//}
	m_States.clear();
	m_Transitions.clear();
	m_CurrentStateNodeId = 0;
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
	//for (auto& state : m_States)
	//{
	//	if (state.clip)
	//		Safe_Release(state.clip);
	//}
	m_States.clear();
	m_Transitions.clear();
}

json CAnimController::Serialize()
{
	json j;
	// 상태 직렬화
	for (const auto& state : m_States)
	{
		j["Anim States"].push_back({
			{"NodeId", state.iNodeId},
			{"Name", state.stateName},
			{"Clip", state.clip ? state.clip->Get_Name() : ""},
			{"NodePos", { state.fNodePos.x, state.fNodePos.y }},
			});
	}

	// 트랜지션 직렬화
	for (const auto& tr : m_Transitions)
	{
		json trJ;
		trJ["FromNodeId"] = tr.iFromNodeId;
		trJ["ToNodeId"] = tr.iToNodeId;
		trJ["Duration"] = tr.duration;
		trJ["MinTime"] = tr.minTime;
		trJ["MaxTime"] = tr.maxTime;
		trJ["HasExitTime"] = tr.hasExitTime;


		// Conditions 배열 생성
		trJ["Conditions"] = json::array();
		for (const auto& cond : tr.conditions)
		{
			json cJ;
			cJ["Type"] = static_cast<int>(cond.type);
			cJ["ParamName"] = cond.paramName;
			cJ["Op"] = static_cast<int>(cond.op);

			cJ["iThreshold"] = cond.iThreshold;
			cJ["fThreshold"] = cond.fThreshold;
			trJ["Conditions"].push_back(cJ);
		}

		// 2) Link 객체
		trJ["Link"] = {
			{"LinkId",      tr.link.iLinkId},
			{"FromNodeId",  tr.link.iLinkStartID},
			{"ToNodeId",    tr.link.iLinkEndID}
		};

		j["Transitions"].push_back(trJ);
	}


	return j;
}

void CAnimController::Deserialize(const json& j)
{
	ResetTransAndStates();
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
					clip = pModel ? pModel->GetAnimationClipByName(clipName): nullptr;
					clip->Set_Bones(m_pAnimator->GetModel()->Get_Bones()); // 애니메이션에 모델의 본 정보 설정
				}
				m_States.push_back({ name, clip, nodeId, pos });
			}
		}
	}

	// 트랜지션 역직렬화
	if (j.contains("Transitions") && j["Transitions"].is_array())
	{
		for (const auto& tr : j["Transitions"])
		{
			if (tr.contains("FromNodeId") && tr.contains("ToNodeId") &&
				tr.contains("Duration") && tr.contains("HasExitTime"))
			{
				_int fromNode = tr["FromNodeId"];
				_int toNode = tr["ToNodeId"];
				_float duration = tr["Duration"];
				_float minTime = tr["MinTime"];
				_float maxTime = tr["MaxTime"];
				_bool hasExitTime = tr["HasExitTime"];
				Link link;
				if (tr.contains("Link"))
				{
					const auto& linkData = tr["Link"];
					link.iLinkId = linkData["LinkId"];
					link.iLinkStartID = linkData["FromNodeId"];
					link.iLinkEndID = linkData["ToNodeId"];
				}
				vector<Condition> conditions;
				if (tr.contains("Conditions") && tr["Conditions"].is_array())
				{
					for (const auto& cond : tr["Conditions"])
					{
						if (cond.contains("Type") && cond.contains("ParamName") &&
							cond.contains("Op") &&
							cond.contains("iThreshold") &&
							cond.contains("fThreshold"))
						{
							Condition condition;
							condition.type = static_cast<ParamType>(cond["Type"]);
							condition.paramName = cond["ParamName"];
							condition.op = static_cast<EOp>(cond["Op"]);
							condition.iThreshold = cond["iThreshold"];
							condition.fThreshold = cond["fThreshold"];
							conditions.push_back(condition);
						}
					}
				}
				AddTransitionMultiCondition(fromNode, toNode, link, conditions, duration, hasExitTime);
			}
		}
	}
}
