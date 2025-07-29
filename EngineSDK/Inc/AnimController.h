#pragma once
#include "Base.h"
#include <iostream>
#include "Serializable.h"

NS_BEGIN(Engine)

class ENGINE_DLL CAnimController final : public CBase, public ISerializable
{
public:
	enum class EOp { IsTrue, IsFalse, Greater, Less,NotEqual,Equal,  Trigger, None};

	enum class ETransitionType {
		FullbodyToFullbody,         // 통짜 -> 통짜
		FullbodyToMasked,           // 통짜 -> 상하체 분리
		MaskedToFullbody,           // 상하체 분리 -> 통짜
		MaskedToMasked              // 상하체 분리 -> 상하체 분리
	};
	struct Condition
	{
		string			paramName;
		ParamType type	= ParamType::Bool; // 파라미터 타입
		EOp          op;
		_int         iThreshold = 0; // int에 값 비교할 때
		_float       fThreshold = 0.f; // float이나 int에 값 비교할 때
		// Evaluate 구현은 CPP에서
		_bool Evaluate(class CAnimController* pAnimController) const;
	};


	struct AnimState
	{
		string stateName;
		class CAnimation* clip = nullptr; // 현재 애니메이션
		_int iNodeId; // 툴상에서의 노드 ID
		_float2 fNodePos; // 툴상에서의 노드 위치

		// 상하체 분리 애니메이션인 경우
		string lowerClipName; // 하체 애니메이션 이름
		string upperClipName; // 상체 애니메이션 이름
		string maskBoneName; // 마스크용 뼈 이름 (없으면 빈 문자열)
		_float fBlendWeight = 1.f; // 블렌드 가중치 (0~1 사이)
	};

	struct Transition 
	{
		_float		 duration = 0.2f; // 전환 시간
		_float 	     minTime = 0.f; // 최소 시간 (애니메이션 진행 시간)
		_float       maxTime = 1.f; // 최대 시간 (애니메이션 진행 시간)
		_int		iFromNodeId; // 시작 노드 ID
		_int		iToNodeId; // 끝 노드 ID

		Link link; // 링크 정보 (툴상에서 사용)
		vector<Condition> conditions; // 전환 조건
		_bool hasExitTime = false; // 애니메이션이 다 끝난 경우에
		_bool Evaluates(class CAnimController* pAnimController, class CAnimator* pAnimator) const;
	};

	struct TransitionResult
	{
		_bool bTransition = false; // 전환 여부
		_bool bBlendFullbody = true; // 블렌드 여부
		ETransitionType eType = ETransitionType::FullbodyToFullbody;
		CAnimation* pFromLowerAnim = nullptr; // 전환 시작 하체/통짜 클립
		CAnimation* pToLowerAnim = nullptr;   // 전환 목표 하체/통짜 클립
		CAnimation* pFromUpperAnim = nullptr; // 전환 시작 상체 클립 
		CAnimation* pToUpperAnim = nullptr;   // 전환 목표 상체 클립 
		_float fDuration = 0.f; // 전환 시간
		_float fBlendWeight = 1.f; // 마스크에 사용함
	};


private:
	CAnimController();
	CAnimController(const CAnimController& Prototype);
	virtual ~CAnimController() = default;

public:
	HRESULT Initialize_Prototype();
	HRESULT Initialize(void* pArg);
	void Update(_float fTimeDelta);

public:

	void SetAnimator(class CAnimator* animator) { m_pAnimator = animator; }
	_float GetStateLength(const string& name);

	size_t  AddState(const string& name, class CAnimation* clip,_int iNodeId)
	{
		m_States.push_back({ name, clip, iNodeId });
		if (m_States.size() == 1)
		{
			m_CurrentStateNodeId = iNodeId;
		}
		return m_States.size() - 1;
    }

	AnimState* GetCurrentState() {
		return FindStateByNodeId(m_CurrentStateNodeId);
	}

	void AddTransition(_int fromNode, _int toNode,  const Link& link, const Condition& cond, _float duration = 0.2f,_bool bHasExitTime = false,_bool bBlendFullBody = true);
	void AddTransition(_int fromNode, _int toNode, const Link& link, _float duration = 0.2f, _bool bHasExitTime = false, _bool bBlendFullBody = true);
	void AddTransitionMultiCondition(
		_int fromNode, _int toNode, const Link& link,
		const vector<Condition>& conditions, _float duration = 0.2f, _bool bHasExitTime = false, _bool bBlendFullBody = true);
	TransitionResult& CheckTransition();
	void ResetTransitionResult() { m_TransitionResult = TransitionResult{}; }

	void SetState(const string& name);
	void SetState(_int iNodeId);
	vector<AnimState>& GetStates() { return m_States; }
	AnimState* GetStateByName(const string& name)
	{
		return FindState(name);
	}

	const string& GetStateNameByNodeId(_int nodeId) const
	{
		for (const auto& state : m_States)
		{
			if (state.iNodeId == nodeId)
				return state.stateName;
		}
		static string empty = "";
		return empty; // 찾지 못했을 때 빈 문자열 반환
	}
	CAnimation* GetStateAnimationByNodeId(_int nodeId) const
	{
		for (const auto& state : m_States)
		{
			if (state.iNodeId == nodeId)
				return state.clip;
		}
		return nullptr; // 찾지 못했을 때 nullptr 반환
	}

	vector<Transition>& GetTransitions()  { return m_Transitions; }

	void SetName(const string& name) { m_Name = name; }
	const string& GetName() const { return m_Name; }

	// 파라미터 관련
	_bool ExisitsParameter(const string& name) const {
		return m_Params.find(name) != m_Params.end();
	}

	void AddParameter(const string& name, Parameter& parm) {
		m_Params[name] = parm;
		SetParamName(m_Params[name], name); // 파라미터 이름 설정
	}

	void AddBool(const string& name) {
		m_Params[name].type = { ParamType::Bool };
		SetParamName(m_Params[name], name); // 파라미터 이름 설정
	}
	void AddFloat(const string& name) {
		m_Params[name].type = { ParamType::Float };
		SetParamName(m_Params[name], name); // 파라미터 이름 설정}
	}
	void AddTrigger(const string& name) {
		m_Params[name].type = { ParamType::Trigger };
		SetParamName(m_Params[name], name); // 파라미터 이름 설정}
	}
	void AddInt(const string& name) {
		m_Params[name].type = { ParamType::Int };
		SetParamName(m_Params[name], name); // 파라미터 이름 설정
	}
	// 파라미터 설정
	void SetBool(const string& name, _bool v) {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // 디버그용 출력
			return; // 파라미터가 없으면 아무것도 하지 않음
		}
		auto& p = m_Params[name];
		p.bValue = v;
	}
	void SetFloat(const string& name, _float v) {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // 디버그용 출력
			return; // 파라미터가 없으면 아무것도 하지 않음
		}
		auto& p = m_Params[name];
		p.fValue = v;
	}
	void SetTrigger(const string& name) {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // 디버그용 출력
			return; // 파라미터가 없으면 아무것도 하지 않음
		}
		auto& p = m_Params[name];
		p.bTriggered = true;
	}

	void ResetTrigger(const string& name) {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // 디버그용 출력
			return; // 파라미터가 없으면 아무것도 하지 않음
		}
		auto& p = m_Params[name];
		p.bTriggered = false;
	}

	void SetInt(const string& name, _int v) {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // 디버그용 출력
			return; // 파라미터가 없으면 아무것도 하지 않음
		}
		auto& p = m_Params[name];
		p.iValue = v;
	}

	void DeleteParameter(const string& name) {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // 디버그용 출력
			return; // 파라미터가 없으면 아무것도 하지 않음
		}
		m_Params.erase(name);
	}

	// 조건 검사용
	_bool CheckBool(const string& name) const {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // 디버그용 출력
			return false; // 기본값 반환
		}
		return m_Params.at(name).bValue;
	}
	_float GetFloat(const string& name) const {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // 디버그용 출력
			return 0.f; // 기본값 반환
		}
		return m_Params.at(name).fValue;
	}
	_bool CheckTrigger(const string& name) {
		auto& p = m_Params[name];
		if (p.bTriggered)
		{
			cout << "Trigger: " << name << endl; // 디버그용 출력
			p.bTriggered = false;
			return true;
		}
		return false;
	}
	_int GetInt(const string& name) const {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // 디버그용 출력
			return 0; // 기본값 반환
		}
		return m_Params.at(name).iValue;
	}

	void SetParamName(Parameter& param, const string& name) {
		param.name = name; // 파라미터 이름 설정
	}
	unordered_map<string, Parameter>& GetParameters() {
		return m_Params;
	}

	void SetEntry(const AnimState& entryState)
	{
		m_EntryState = entryState;
	}
	void SetExit(const AnimState& exitState)
	{
		m_ExitState = exitState;
	}
private:
	AnimState* FindState(const string& name) 
	{
		for (auto& s : m_States)
			if (s.stateName == name) return &s;
		return nullptr;
	}
	AnimState* FindStateByNodeId(_int iNodeId)
	{
		for (auto& s : m_States)
			if (s.iNodeId == iNodeId) return &s;
		return nullptr;
	}
	void ResetTransAndStates();
private:
	// 상태·전환 저장
    _int                   m_CurrentStateNodeId= 0;
	_float                 m_fLayerWeight{ 0.f };
	unordered_map<string, vector<AnimState>> m_Layers;

	AnimState              m_EntryState;
	AnimState              m_ExitState;
	vector<AnimState>      m_States;
	vector<Transition>     m_Transitions;
	vector<Condition>	   m_Conditions; // 아직 쓰는 곳 없음

	class CAnimator*	   m_pAnimator = nullptr;  // 애니메이터 참조
	unordered_map<string, Parameter> m_Params; // 파라미터 관리 (컨트롤러 별 개별로 관리)

	TransitionResult       m_TransitionResult{}; // 트래지션을 한 결과 (애니메이터에서 요청)
	string 				   m_Name; // 컨트롤러 이름
public:
	static CAnimController* Create();
	CAnimController* Clone();
	virtual void Free() override;

	// ISerializable을(를) 통해 상속됨
	json Serialize() override;
	void Deserialize(const json& j) override;
};
NS_END
