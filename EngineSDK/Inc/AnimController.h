#pragma once
#include "Base.h"
#include <iostream>
#include "Serializable.h"

NS_BEGIN(Engine)

class ENGINE_DLL CAnimController final : public CBase, public ISerializable
{
public:
	enum class EOp { IsTrue, IsFalse, Greater, Less,NotEqual,Equal,  Trigger, None};

	enum class ETransitionType
	{
		FullbodyToFullbody,         // 통짜 -> 통짜
		FullbodyToMasked,           // 통짜 -> 상하체 분리
		MaskedToFullbody,           // 상하체 분리 -> 통짜
		MaskedToMasked              // 상하체 분리 -> 상하체 분리
	};


	struct Condition
	{
		string			paramName;
		ParamType type	= ParamType::Bool; // 파라미터 타입
		EOp          op = EOp::None; // 연산자
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
		_float2 fNodePos{ 0.f, 0.f };

		// 상하체 분리 애니메이션인 경우
		string lowerClipName; // 하체 애니메이션 이름
		string upperClipName; // 상체 애니메이션 이름
		string maskBoneName; // 마스크용 뼈 이름 (없으면 빈 문자열)
		_float fBlendWeight = 1.f; // 블렌드 가중치 (0~1 사이)
		_float fLowerStartTime = 0.f; // 하체 애니메이션 시작 시간
		_float fUpperStartTime = 0.f; // 상체 애니메이션 시작 시간
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
		_float fLowerStartTime = 0.f; // 하체 애니메이션 시작 시간
		_float fUpperStartTime = 0.f; // 상체 애니메이션 시작 시간
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
			SetEntry(name);
		}
		return m_States.size() - 1;
    }
	size_t AddState(const string& stateName, CAnimation* defaultClip, _int nodeId, _bool bIsMaskBone = false, const string& initialMaskBone = "",
		const string& initialUpperClip = "",
		const string& initialLowerClip = "");

	const AnimState* GetCurrentState() const {
		return FindStateByNodeId(m_CurrentStateNodeId);
	}

	void AddTransition(_int fromNode, _int toNode,  const Link& link, const Condition& cond, _float duration = 0.2f,_bool bHasExitTime = false,_bool bBlendFullBody = true);
	void AddTransition(_int fromNode, _int toNode, const Link& link, _float duration = 0.2f, _bool bHasExitTime = false, _bool bBlendFullBody = true);
	void AddTransitionMultiCondition(
		_int fromNode, _int toNode, const Link& link,
		const vector<Condition>& conditions, _float duration = 0.2f, _bool bHasExitTime = false, _bool bBlendFullBody = true);
	const TransitionResult& CheckTransition() const;
	void ResetTransitionResult() { m_TransitionResult = TransitionResult{}; }

	void SetState(const string& name);
	void SetState(_int iNodeId);
	const vector<AnimState>& GetStates() const { return m_States; }
	const AnimState* GetStateByName(const string& name) const
	{
		return FindState(name);
	}

	const AnimState* GetStateByName(const string& name)
	{
		return FindState(name);
	}

	const CAnimation* GetStateAnimationByNodeId(_int nodeId) const
	{
		for (const auto& state : m_States)
		{
			if (state.iNodeId == nodeId)
				return state.clip;
		}
		return nullptr; // 찾지 못했을 때 nullptr 반환
	}
	const vector<Transition>& GetTransitions() const  { return m_Transitions; }
#ifdef USE_IMGUI


	AnimState* GetCurrentStateForEditor() {
		return FindStateByNodeId(m_CurrentStateNodeId);
	}
	AnimState* GetStateByNameForEditor(const string& name)
	{
		return FindState(name);
	}

	CAnimation* GetStateAnimationByNodeIdForEditor(_int nodeId) const
	{
		for (const auto& state : m_States)
		{
			if (state.iNodeId == nodeId)
				return state.clip;
		}
		return nullptr; // 찾지 못했을 때 nullptr 반환
	}
	vector<Transition>& GetTransitionsForEditor() { return m_Transitions; }
	vector<AnimState>& GetStatesForEditor()  { return m_States; }
#endif

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
	const unordered_map<string, Parameter>& GetParameters()const {
		return m_Params;
	}
#ifdef USE_IMGUI
	unordered_map<string, Parameter>& GetParametersForEditor() {
		return m_Params;
	}
#endif

	void SetEntry(const string& entryStateName)
	{
		m_EntryStateName = entryStateName;
		m_EntryState = FindState(entryStateName);
		if (m_EntryState == nullptr)
		{
			cout << "Entry state not found: " << entryStateName << endl; // 디버그용 출력
			return;
		}
		else
		{
			m_EntryStateNodeId = m_EntryState->iNodeId; // Entry 상태 노드 ID 설정
			m_CurrentStateNodeId = m_EntryStateNodeId; // 현재 상태를 Entry 상태로 설정
		}
	}
	void SetExit(const string& exitStateName)
	{
		m_ExitStateName = exitStateName;
		m_ExitState = FindState(exitStateName);
		if (m_ExitState == nullptr)
		{
			cout << "Exit state not found: " << exitStateName << endl; // 디버그용 출력
			return;
		}
		else
		{
			m_ExitStateNodeId = m_ExitState->iNodeId; // Exit 상태 노드 ID 설정
		}
	}
#ifdef USE_IMGUI
	AnimState* GetEntryStateForEditor() const { return m_EntryState; }
	AnimState* GetExitStateForEditor() const { return m_ExitState; }
#endif
	const AnimState* GetEntryState() const { return m_EntryState; }
	const AnimState* GetExitState() const { return m_ExitState; }
	_int GetEntryNodeId() const { return m_EntryStateNodeId; }
	_int GetExitNodeId() const { return m_ExitStateNodeId; }
	void Add_OverrideAnimController(const string& name, const OverrideAnimController& overrideController);
	void Applay_OverrideAnimController(const string& ctrlName, const OverrideAnimController& overrideController);
	void Cancel_OverrideAnimController();
private:
	const AnimState* FindState(const string& name) const
	{
		for (auto& s : m_States)
			if (s.stateName == name) 
				return &s;
		return nullptr;
	}

	AnimState* FindState(const string& name)
	{
		for (auto& s : m_States)
			if (s.stateName == name)
				return &s;
		return nullptr;
	}
	const AnimState* FindStateByNodeId(_int iNodeId) const
	{
		for (auto& s : m_States)
			if (s.iNodeId == iNodeId) return &s;
		return nullptr;
	}

	AnimState* FindStateByNodeId(_int iNodeId)
	{
		for (auto& s : m_States)
			if (s.iNodeId == iNodeId) return &s;
		return nullptr;
	}
	void ResetTransAndStates();
	void ChangeStatesForOverride(const string& overrideCtrlName);// 오버라이드 애니메이션 컨트롤러를 적용할 때 상태들을 변경
	void ChangeStatesForDefault();
	_int ConvertExitNodeToExitStateNodeId(_int iNodeId) const
	{
		if (iNodeId == m_iCExitStateNodeID)
		{

			auto it = FindState(m_ExitStateName);
			if (it != nullptr)
				return it->iNodeId;
			else
			{
#ifdef _DEBUG
				cout << "Exit state not found: " << m_ExitStateName << endl; // 디버그용 출력
#endif // _DEBUG

				return -1; // Exit 상태가 없으면 -1 반환
			}
		}
		else
			return iNodeId;
	}

	_int ConvertAnyStateNodeIdToAnyState(_int iNodeId) const
	{
		if (iNodeId == m_iCAnyStateNodeID)
		{
			// AnyState에서 넘어가는거라면 현재 재생중인 애니메이션의 노드 ID 넘기기
			return m_CurrentStateNodeId;
		}
		return iNodeId; // AnyState가 아닌 경우 그대로 반환
	}
private:
	
	_bool m_bOverrideAnimController = false; // 오버라이드 애니메이션 컨트롤러 사용 중인지
	// Override 애니메이션 컨트롤러들
	unordered_map<string, OverrideAnimController> m_OverrideAnimControllers;
	// 상태·전환 저장
    _int                   m_CurrentStateNodeId= 0;
	_float                 m_fLayerWeight{ 0.f };
	unordered_map<string, vector<AnimState>> m_Layers;
	_int m_EntryStateNodeId = -1; // 
	_int m_ExitStateNodeId = -1;
	AnimState*			   m_EntryState{ nullptr };
	AnimState*			   m_ExitState{nullptr};
	AnimState*			   m_AnyState{ nullptr }; // Any 상태 (모든 상태를 포함하는 상태)
	string				   m_EntryStateName{};
	string				   m_ExitStateName{};
	vector<AnimState>      m_States;
	vector<Transition>     m_Transitions;
	vector<Condition>	   m_Conditions; // 아직 쓰는 곳 없음

	class CAnimator*	   m_pAnimator = nullptr;  // 애니메이터 참조
	unordered_map<string, Parameter> m_Params; // 파라미터 관리 (컨트롤러 별 개별로 관리)
	unordered_map<string, vector<AnimState>> m_OriginalAnimStates; // 원본 애니메이션 상태들 (복사본을 만들 때 사용)
	TransitionResult       m_TransitionResult{}; // 트래지션을 한 결과 (애니메이터에서 요청)
	string 				   m_Name; // 컨트롤러 이름

	const _int m_iCExitStateNodeID = 100000;
	const _int m_iCAnyStateNodeID = 100001;

	_bool m_bChangeDefaultController = false; // 기본 컨트롤러로 변경 여부
	CAnimation* m_pTmpUpperAnimForChagned = nullptr; // 디폴트로 변경시 없는 애니메이션 잠깐 채워서 넣기
	CAnimation* m_pTmpLowerAnimForChagned = nullptr; // 디폴트로 변경시 없는 애니메이션 잠깐 채워서 넣기
public:
	static CAnimController* Create();
	CAnimController* Clone();
	virtual void Free() override;

	// ISerializable을(를) 통해 상속됨
	json Serialize() override;
	void Deserialize(const json& j) override;
};
NS_END
