#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CAnimController final :  public CBase
{
public:
	enum class EOp { IsTrue, IsFalse, Greater, Less,NotEqual,Equal,  Trigger, None};
	struct Condition
	{
		string			paramName;
		ParamType type	= ParamType::Bool; // �Ķ���� Ÿ��
		EOp          op;
		_int         iThreshold = 0; // int�� �� ���� ��
		_float       fThreshold = 0.f; // float�̳� int�� �� ���� ��
		_float  minTime = 0.f; // ���� ��Ȳ
		_float  maxTime = 1.f; // �ִ� ���� ��Ȳ
		// Evaluate ������ CPP����
		_bool Evaluate(class CAnimator* animator) const;
	};


	struct AnimState
	{
		string stateName;
		class CAnimation* clip = nullptr; // ���� �ִϸ��̼�
		_int iNodeId; // ���󿡼��� ��� ID
		_float2 fNodePos; // ���󿡼��� ��� ��ġ
	};

	struct Transition 
	{
		_float   duration; // ��ȯ �ð�
		_int     iFromNodeId; // ���� ��� ID
		_int      iToNodeId; // �� ��� ID

		Link link; // ��ũ ���� (���󿡼� ���)
		Condition condition; // ��ȯ ����
		_bool hasExitTime = false; // �ִϸ��̼��� �� ���� ��쿡
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
		// ù ���¶�� currentIdx 0����
		if (m_States.size() == 1)
		{
			m_CurrentStateNodeId = iNodeId;
		}
		return m_States.size() - 1;
    }

	AnimState* GetCurrentState() {
		return &m_States[m_CurrentStateNodeId];
	}

	void AddTransition(_int fromNode, _int toNode,  const Link& link, const Condition& cond, _float duration = 0.2f,_bool bHasExitTime = false);
	void AddTransition(_int fromNode, _int toNode, const Link& link, _float duration = 0.2f, _bool bHasExitTime = false);

	void SetState(const string& name);
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
		return empty; // ã�� ������ �� �� ���ڿ� ��ȯ
	}
	CAnimation* GetStateAnimationByNodeId(_int nodeId) const
	{
		for (const auto& state : m_States)
		{
			if (state.iNodeId == nodeId)
				return state.clip;
		}
		return nullptr; // ã�� ������ �� nullptr ��ȯ
	}

	vector<Transition>& GetTransitions()  { return m_Transitions; }
public:
	//virtual json Serialize() override;
	//virtual void Deserialize(const json& j) override;
private:
	AnimState* FindState(const string& name) 
	{
		for (auto& s : m_States)
			if (s.stateName == name) return &s;
		return nullptr;
	}

private:
	// ���¡���ȯ ����
	vector<AnimState>      m_States;
	vector<Transition>     m_Transitions;
	vector<Condition>   m_Conditions;
    _int                 m_CurrentStateNodeId= 0;
	class CAnimator* m_pAnimator = nullptr;  // �ִϸ����� ����
	unordered_map<size_t, size_t> m_SubClipIndex; //���� �� ��° Ŭ���� ��� ������ ����

public:
	static CAnimController* Create();
	CAnimController* Clone();
	virtual void Free() override;
};
NS_END
