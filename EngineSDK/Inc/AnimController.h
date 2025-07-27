#pragma once
#include "Base.h"
#include "Serializable.h"

NS_BEGIN(Engine)

class ENGINE_DLL CAnimController final : public CBase, public ISerializable
{
public:
	enum class EOp { IsTrue, IsFalse, Greater, Less,NotEqual,Equal,  Trigger, None};
	struct Condition
	{
		string			paramName;
		ParamType type	= ParamType::Bool; // 파라미터 타입
		EOp          op;
		_int         iThreshold = 0; // int에 값 비교할 때
		_float       fThreshold = 0.f; // float이나 int에 값 비교할 때
		// Evaluate 구현은 CPP에서
		_bool Evaluate(class CAnimator* animator) const;
	};


	struct AnimState
	{
		string stateName;
		class CAnimation* clip = nullptr; // 현재 애니메이션
		_int iNodeId; // 툴상에서의 노드 ID
		_float2 fNodePos; // 툴상에서의 노드 위치
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

		_bool Evaluates(class CAnimator* animator) const;
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

	void AddTransition(_int fromNode, _int toNode,  const Link& link, const Condition& cond, _float duration = 0.2f,_bool bHasExitTime = false);
	void AddTransition(_int fromNode, _int toNode, const Link& link, _float duration = 0.2f, _bool bHasExitTime = false);
	void AddTransitionMultiCondition(
		_int fromNode, _int toNode, const Link& link,
		const vector<Condition>& conditions, _float duration = 0.2f, _bool bHasExitTime = false);
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
	vector<AnimState>      m_States;
	vector<Transition>     m_Transitions;
	vector<Condition>	   m_Conditions; // 아직 쓰는 곳 없음

	class CAnimator*	   m_pAnimator = nullptr;  // 애니메이터 참조
	unordered_map<size_t, size_t> m_SubClipIndex; //지금 몇 번째 클립을 재생 중인지 저장

	TransitionResult       m_TransitionResult{}; // 트래지션을 한 결과 (애니메이터에서 요청)

public:
	static CAnimController* Create();
	CAnimController* Clone();
	virtual void Free() override;

	// ISerializable을(를) 통해 상속됨
	json Serialize() override;
	void Deserialize(const json& j) override;
};
NS_END
