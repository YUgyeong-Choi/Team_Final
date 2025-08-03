#include "AnimController.h"
#include "Animation.h"
#include "Animator.h"
#include "Model.h"
#include "Bone.h"

_bool CAnimController::Condition::Evaluate(class CAnimController* pAnimController) const
{
	if (paramName.empty())
		return true;
	if (op == EOp::None)
		return true;

	switch (type)
	{
	case ParamType::Bool:
		switch (op)
		{
		case EOp::IsTrue:
			return pAnimController->CheckBool(paramName) == true;
		case EOp::IsFalse:
			return pAnimController->CheckBool(paramName) == false;
		default:
			return false; // Bool 타입에서 지원하지 않는 연산
		}
	case ParamType::Int:
		switch (op)
		{
		case EOp::Greater:
			return (pAnimController->GetInt(paramName) > iThreshold);
		case EOp::Less:
			return (pAnimController->GetInt(paramName) < iThreshold);
		case EOp::NotEqual:
			return (pAnimController->GetInt(paramName) != iThreshold);
		case EOp::Equal:
			return (pAnimController->GetInt(paramName) == iThreshold);
		default:
			return false; // Int 타입에서 지원하지 않는 연산
		}
	case ParamType::Float:
		switch (op)
		{
		case EOp::Greater:
			return (pAnimController->GetFloat(paramName) > fThreshold);
		case EOp::Less:
			return (pAnimController->GetFloat(paramName) < fThreshold);
		default:
			return false; // Float 타입에서 지원하지 않는 연산
		}
	case ParamType::Trigger:
		switch (op)
		{
		case EOp::Trigger:
		case EOp::None:  // op가 None인 경우도 처리
			return pAnimController->CheckTrigger(paramName);
		default:
			return false; // Trigger에서 지원하지 않는 연산
		}
	default:
		return false; // 지원하지 않는 타입
	}
	return false;
}

_bool CAnimController::Transition::Evaluates(CAnimController* pAnimController, CAnimator* pAnimator) const
{
	_float t = pAnimator->GetCurrentAnimProgress();
	if (t < minTime || t > maxTime)
		return false;

	if (conditions.empty())
		return true; // 조건이 없으면 항상 true

	//for (const auto& condition : conditions)
	//{
	//	if (!condition.Evaluate(pAnimController))
	//	{
	//		return false; // 하나라도 false면 전체 false
	//	}
	//}

	for (const auto& condition : conditions)
	{
		if (condition.type != ParamType::Trigger)
		{
			if (!condition.Evaluate(pAnimController))
			{
				return false; // Trigger가 아닌 조건 실패 시 바로 종료
			}
		}
	}

	// 모든 일반 조건이 통과했을 때만 Trigger 확인 
	for (const auto& condition : conditions)
	{
		if (condition.type == ParamType::Trigger)
		{
			if (!condition.Evaluate(pAnimController))
			{
				return false;
			}
		}
	}

	// 모든 조건이 true면
	// 디버그용 출력

	for (const auto& condition : conditions)
	{
		switch (condition.type)
		{
		case ParamType::Bool:
			cout << "Condition passed: " << condition.paramName << " is true." << endl; // 디버그용 출력
			break;
		case ParamType::Int:
			cout << "Condition passed: " << condition.paramName << " is in range ("
				<< condition.iThreshold << ")." << endl; // 디버그용 출력
			break;
		case ParamType::Float:
			cout << "Condition passed: " << condition.paramName << " is in range ("
				<< condition.fThreshold << ")." << endl; // 디버그용 출력
			break;
		case ParamType::Trigger:
			cout << "Condition passed: " << condition.paramName << " trigger activated." << endl; // 디버그용 출력
			break;
		}
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
				
				AnimState* currentAnimState = FindStateByNodeId(m_CurrentStateNodeId);
				if (currentAnimState && currentAnimState->maskBoneName.empty() == false) // 현재 상태가 상하체 분리 상태라면
				{
					CAnimation* upperAnim = m_pAnimator->GetUpperClip();
					if (upperAnim && upperAnim->Get_isLoop())
						continue; // 현재 상체 애니메이션이 루프면 뛰어넘게(보통은 그런 경우가 없을듯함.)
					_float progress = m_pAnimator->GetCurrentUpperAnimProgress();
					if (progress < 1.f)
						continue; // 현재 상체 애니메이션이 끝나지 않았으면 뛰어넘게
				}
				else
				{
					CAnimation* currentAnim = m_pAnimator->GetCurrentAnim();
					if (!currentAnim || currentAnim->Get_isLoop())
						continue; // 현재 애니메이션이 없거나 루프면 뛰어넘게
					_float progress = m_pAnimator->GetCurrentAnimProgress();
					if (progress < 1.f)
						continue; // 현재 애니메이션이 끝나지 않았으면 뛰어넘게
				}
			
			}
			if (!tr.Evaluates(this,m_pAnimator))
				continue;
			AnimState* fromState = FindStateByNodeId(tr.iFromNodeId);
			AnimState* toState = FindStateByNodeId(tr.iToNodeId); 

			if (!fromState || !toState) // 애니메이션 상태가 없으면
				continue;
			m_TransitionResult = TransitionResult{}; // 초기화

			_bool bFromMasked = fromState->maskBoneName.empty() == false; // 분리 상태인지
			_bool bToMasked = toState->maskBoneName.empty() == false; // 분리 상태인지
		
		
			// 통짜-> 분리
			auto* fromClip = GetStateAnimationByNodeId(tr.iFromNodeId);
			auto* toClip = GetStateAnimationByNodeId(tr.iToNodeId);

			if (!bFromMasked && !bToMasked) //  통짜 -> 통짜
			{
				m_TransitionResult.eType = ETransitionType::FullbodyToFullbody;
				m_TransitionResult.pFromLowerAnim = fromState->clip;
				m_TransitionResult.pToLowerAnim = toState->clip;
				m_TransitionResult.bBlendFullbody = true;
			}
			else if (!bFromMasked && bToMasked) //  통짜 -> 상하체 분리
			{
				m_TransitionResult.eType = ETransitionType::FullbodyToMasked;
				m_TransitionResult.pFromLowerAnim = fromState->clip; // 이전 통짜 애니메이션
				m_TransitionResult.pToLowerAnim = m_pAnimator->GetModel()->GetAnimationClipByName(toState->lowerClipName); // 목표 하체
				m_TransitionResult.pToUpperAnim = m_pAnimator->GetModel()->GetAnimationClipByName(toState->upperClipName); // 목표 상체
				//if (m_TransitionResult.pFromLowerAnim == m_TransitionResult.pToUpperAnim)
				//	m_TransitionResult.pToUpperAnim = nullptr; // 상체가 하체와 동일한 경우 상체 애니메이션은 없음
				m_TransitionResult.fBlendWeight = toState->fBlendWeight;
				m_TransitionResult.bBlendFullbody = false;
			}
			else if (bFromMasked && !bToMasked) //  상하체 분리 -> 통짜
			{
				m_TransitionResult.eType = ETransitionType::MaskedToFullbody;
				m_TransitionResult.pFromLowerAnim = m_pAnimator->GetLowerClip(); // 현재 재생 중인 하체 클립
				m_TransitionResult.pFromUpperAnim = m_pAnimator->GetUpperClip(); // 현재 재생 중인 상체 클립
				m_TransitionResult.pToLowerAnim = toState->clip; // 목표 통짜 애니메이션
				m_TransitionResult.fBlendWeight = fromState->fBlendWeight; // 이전 상태의 블렌드 가중치
				m_TransitionResult.bBlendFullbody = false;
			}
			else  // 상하체 분리 -> 상하체 분리
			{
				m_TransitionResult.eType = ETransitionType::MaskedToMasked;
				m_TransitionResult.pFromLowerAnim = m_pAnimator->GetLowerClip();
				m_TransitionResult.pFromUpperAnim = m_pAnimator->GetUpperClip();
				m_TransitionResult.pToLowerAnim = m_pAnimator->GetModel()->GetAnimationClipByName(toState->lowerClipName);
				m_TransitionResult.pToUpperAnim = m_pAnimator->GetModel()->GetAnimationClipByName(toState->upperClipName);
				m_TransitionResult.fBlendWeight = toState->fBlendWeight;
				m_TransitionResult.bBlendFullbody = false;
			}

			//  유효 검사
			if (!m_TransitionResult.pFromLowerAnim || !m_TransitionResult.pToLowerAnim) 
				continue; // 최소한 하체/통짜 클립은 있어야 함
			if (m_TransitionResult.eType == ETransitionType::FullbodyToMasked && !m_TransitionResult.pToUpperAnim)
				continue;
			if (m_TransitionResult.eType == ETransitionType::MaskedToFullbody && !m_TransitionResult.pFromUpperAnim) 
				continue;
			if (m_TransitionResult.eType == ETransitionType::MaskedToMasked && (!m_TransitionResult.pFromUpperAnim || !m_TransitionResult.pToUpperAnim)) 
				continue;

			m_TransitionResult.bTransition = true;
			m_TransitionResult.fDuration = tr.duration;
			m_CurrentStateNodeId = tr.iToNodeId; // 다음 프레임부터 이 상태로 간주
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

void CAnimController::AddTransition(_int fromNode, _int toNode, const Link& link, const Condition& cond, _float duration,_bool bHasExitTime, _bool bBlendFullBody)
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

void CAnimController::AddTransition(_int fromNode, _int toNode, const Link& link, _float duration, _bool bHasExitTime, _bool bBlendFullBody)
{
	Condition noCond{};
	noCond.op = EOp::None;
	AddTransition(fromNode, toNode, link,
		noCond,
		duration,
		bHasExitTime,
		bBlendFullBody);
}

void CAnimController::AddTransitionMultiCondition(_int fromNode, _int toNode, const Link& link, const vector<Condition>& conditions, _float duration, _bool bHasExitTime, _bool bBlendFullBody)
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

CAnimController::TransitionResult & CAnimController::CheckTransition()
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
	else
	{
		m_CurrentStateNodeId = m_EntryStateNodeId; // Entry 상태로 되돌리기
		auto entryState = FindStateByNodeId(m_EntryStateNodeId);
		if (entryState)
		{
			if (entryState->clip)
				m_pAnimator->PlayClip(entryState->clip);
		}
		else
		{
			MSG_BOX("Entry state not found");
		}
	}
}

void CAnimController::SetState(_int iNodeId)
{
	auto state = FindStateByNodeId(iNodeId);
	if (state)
	{
		m_CurrentStateNodeId = state->iNodeId;
		if(state->clip)
			m_pAnimator->PlayClip(state->clip);
		else
		{
			m_pAnimator->UpdateMaskState();
		}
	}
}

void CAnimController::Add_OverrideAnimController(const string& name, const OverrideAnimController& overrideController)
{
	if (m_OverrideAnimControllers.find(name) != m_OverrideAnimControllers.end())
	{
		// 이미 존재하는 컨트롤러 이름이면 덮어쓰기
		m_OverrideAnimControllers[name] = overrideController;
		return;
	}
	m_OverrideAnimControllers.emplace(name, overrideController);
	auto it = m_OverrideAnimControllers.find(name);
	if (it != m_OverrideAnimControllers.end())
	{
		it->second.controllerName = m_Name; // 이름 설정
	}
}

void CAnimController::Applay_OverrideAnimController(const string& ctrlName, const OverrideAnimController& overrideController)
{
	if (overrideController.states.empty())
		return;

	if (m_OriginalAnimStates.empty())
	{
		// 적용 전에 기존 애니메이션 상태들을 저장
		m_OriginalAnimStates["Default"] = m_States; 
	}

	m_OverrideAnimControllers[ctrlName] = overrideController;

	ChangeStates(ctrlName); // 오버라이드 애니메이션 컨트롤러의 상태로 변경
	m_bOverrideAnimController = true; // 오버라이드 애니메이션 컨트롤러 사용 중으로 설
}

void CAnimController::Cancel_OverrideAnimController()
{
	if (m_OriginalAnimStates.empty())
		return;
	m_States = m_OriginalAnimStates["Default"]; // 원래 상태로 되돌리기
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

void CAnimController::ChangeStates(const string& overrideCtrlName)
{
	auto it = m_OverrideAnimControllers.find(overrideCtrlName);

	if (it != m_OverrideAnimControllers.end())
	{
		auto& overrideCtrl = it->second;
		auto& overrideStateMap = overrideCtrl.states;
		for (auto& state : m_States)
		{
			auto it = overrideStateMap.find(state.stateName);
			if (it != overrideStateMap.end())
			{
				OverrideAnimController::OverrideState& overrideState = it->second;
				if (overrideState.clipName.empty() == false)
				{
					state.clip = m_pAnimator->GetModel()->GetAnimationClipByName(overrideState.clipName);
				}
				else
				{
					state.clip = nullptr;
				}

				state.upperClipName = overrideState.upperClipName;
				state.lowerClipName = overrideState.lowerClipName;
				state.maskBoneName = overrideState.maskBoneName;
				state.fBlendWeight = overrideState.fBlendWeight;
			}
		}
		// 오버라이드한 컨트롤러 이름 설정
		overrideCtrl.controllerName = m_Name;
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
	// 컨트롤러 이름 직렬화
	
	j["ControllerName"] = m_Name;

	// 상태 직렬화(오버라이드 컨트롤러 사용하고 있었으면 기본으로 바꿔서 내보내기)
	if (m_bOverrideAnimController)
	{
		m_States = m_OriginalAnimStates["Default"]; // 오버라이드 컨트롤러 사용 중이면 원래 상태로 되돌리기
	}

	for (const auto& state : m_States)
	{
		j["Anim States"].push_back({
			{"NodeId", state.iNodeId},
			{"Name", state.stateName},
			{"Clip", state.clip ? state.clip->Get_Name() : ""},
			{"NodePos", { state.fNodePos.x, state.fNodePos.y }},
			{"MaskBone",state.maskBoneName},
			{"LowerClip", state.lowerClipName},
			{"UpperClip", state.upperClipName},
			{"BlendWeight", state.fBlendWeight}
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

			switch (cond.type)
			{
			case ParamType::Bool:
				cJ["Type"] = "Bool";
				break;
			case ParamType::Int:
				cJ["Type"] = "Int";
				break;
			case ParamType::Float:
				cJ["Type"] = "Float";
				break;
			case ParamType::Trigger:
				cJ["Type"] = "Trigger";
				break;
			default:
				cJ["Type"] = "Unknown"; // 알 수 없는 타입 처리
				break;
			}
			cJ["ParamName"] = cond.paramName;

			switch (cond.op)
			{
			case EOp::IsTrue:
				cJ["Op"] = "IsTrue";
				break;
			case EOp::IsFalse:
				cJ["Op"] = "IsFalse";
				break;
			case EOp::Greater:
				cJ["Op"] = "Greater";
				break;
			case EOp::Less:
				cJ["Op"] = "Less";
				break;
			case EOp::NotEqual:
				cJ["Op"] = "NotEqual";
				break;
			case EOp::Equal:
				cJ["Op"] = "Equal";
				break;
			case EOp::Trigger:
				cJ["Op"] = "Trigger";
				break;
			case EOp::None:
				cJ["Op"] = "None"; // 조건이 없는 경우
				break;
			default:
				cJ["Op"] = "Unknown"; // 알 수 없는 연산자 처리
				break;
			}

			cJ["iThreshold"] = cond.iThreshold;
			cJ["fThreshold"] = cond.fThreshold;
			trJ["Conditions"].push_back(cJ);
		}

		// Link 객체
		trJ["Link"] = {
			{"LinkId",      tr.link.iLinkId},
			{"FromNodeId",  tr.link.iLinkStartID},
			{"ToNodeId",    tr.link.iLinkEndID}
		};

		j["Transitions"].push_back(trJ);
	}

	// 파라미터 직렬화

	for (const auto& [name, param] : m_Params)
	{
		j["Parameters"][name] = {
			{"bValue", param.bValue},
			{"fValue", param.fValue},
			{"iValue", param.iValue},
			{"bTriggered", param.bTriggered},
			{"Type", static_cast<int>(param.type)} // ParamType을 정수로 저장
		};
	}

	// Entry와 Exit State 저장해두기
	if (m_EntryStateName.empty() == false)
	{
		j["EntryState"] = m_EntryStateName;
	}
	if (m_ExitStateName.empty() == false)
	{
		j["ExitState"] = m_ExitStateName;
	}

	return j;
}

void CAnimController::Deserialize(const json& j)
{
	ResetTransAndStates();

	// 컨트롤러 이름 역직렬화
	if (j.contains("ControllerName") && j["ControllerName"].is_string())
	{
		m_Name = j["ControllerName"];
	}

	if (j.contains("Anim States") && j["Anim States"].is_array())
	{

		for (const auto& state : j["Anim States"])
		{
			if (state.contains("NodeId") && state.contains("Name") && state.contains("Clip"))
			{
				_int nodeId = state["NodeId"];
				string name = state["Name"];
				string clipName = state["Clip"];
				string maskBoneName = "";
				string lowerClipName = "";
				string upperClipName = "";
				if (state.contains("MaskBone") && state["MaskBone"].is_string())
				{
					maskBoneName = state["MaskBone"];
				}
				if (state.contains("UpperClip") && state["UpperClip"].is_string())
				{
					upperClipName = state["UpperClip"];
				}
				if (state.contains("LowerClip") && state["LowerClip"].is_string())
				{
					lowerClipName = state["LowerClip"];
				}
				_float fBlendWeight = 1.f; // 기본값
				if (state.contains("BlendWeight") && state["BlendWeight"].is_number())
				{
					fBlendWeight = state["BlendWeight"];
				}
		
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
		
				AddState(name, clip, nodeId, maskBoneName.empty() == false, maskBoneName, upperClipName, lowerClipName);

				//m_States.push_back({ name, clip, nodeId, pos ,lowerClipName, upperClipName,maskBoneName,fBlendWeight });
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
							if (cond["Type"] == "Bool")
								condition.type = ParamType::Bool;
							else if (cond["Type"] == "Int")
								condition.type = ParamType::Int;
							else if (cond["Type"] == "Float")
								condition.type = ParamType::Float;
							else if (cond["Type"] == "Trigger")
								condition.type = ParamType::Trigger;
							condition.paramName = cond["ParamName"];
							if (cond["Op"] == "IsTrue")
								condition.op = EOp::IsTrue;
							else if (cond["Op"] == "IsFalse")
								condition.op = EOp::IsFalse;
							else if (cond["Op"] == "Greater")
								condition.op = EOp::Greater;
							else if (cond["Op"] == "Less")
								condition.op = EOp::Less;
							else if (cond["Op"] == "NotEqual")
								condition.op = EOp::NotEqual;
							else if (cond["Op"] == "Equal")
								condition.op = EOp::Equal;
							else if (cond["Op"] == "Trigger")
								condition.op = EOp::Trigger;
							else
								condition.op = EOp::None; // 알 수 없는 연산자 처리
							condition.iThreshold = cond["iThreshold"];
							condition.fThreshold = cond["fThreshold"];
							conditions.push_back(condition);
						}
					}
				}
				_bool bBlendFullBody = true; // 기본값
				if (tr.contains("BlendFullbody"))
				{
					bBlendFullBody = tr["BlendFullbody"];
				}
				AddTransitionMultiCondition(fromNode, toNode, link, conditions, duration, hasExitTime, bBlendFullBody);
				m_Transitions.back().minTime = minTime; // 최소 시간
				m_Transitions.back().maxTime = maxTime; // 최대 시간
			}
		}
	}

	if (j.contains("Parameters"))
	{
		for (const auto& [name, param] : j["Parameters"].items())

		{
			Parameter p;
			p.bValue = param["bValue"].get<_bool>();
			p.fValue = param["fValue"].get<_float>();
			p.iValue = param["iValue"].get<_int>();
			p.bTriggered = param["bTriggered"].get<_bool>();
			p.type = static_cast<ParamType>(param["Type"].get<int>());
			m_Params[name] = p;
			SetParamName(m_Params[name], name);
		}
	}

	// Entry와 Exit State 역직렬화
	if (j.contains("EntryState") && j["EntryState"].is_string())
	{
		m_EntryStateName = j["EntryState"];
		SetState(m_EntryStateName);
	}
	if (j.contains("ExitState") && j["ExitState"].is_string())
	{
		m_ExitStateName = j["ExitState"];
	}
	SetEntry(m_EntryStateName);
	SetExit(m_ExitStateName);

	m_OriginalAnimStates["Default"] = m_States; // 기본 애니메이션 상태들 저장
}

