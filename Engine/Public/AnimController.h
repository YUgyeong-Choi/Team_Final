#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CAnimController final :  public CBase
{
public:
	enum class EOp { IsTrue, IsFalse, Greater, Less, Trigger , Finished};
	struct Condition
	{
		string paramName;
		EOp         op;
		_float       threshold = 0.f;
		_float  minTime = 0.f; // ���� ��Ȳ
		_float  maxTime = 1.f; // �ִ� ���� ��Ȳ
		// Evaluate ������ CPP����
		_bool Evaluate(class CAnimator* animator) const;
	};


	struct AnimState
	{
		string stateName;
		class CAnimation* clip = nullptr; // ���� �ִϸ��̼�
		_uint clipIndex{};
		_int iNodeId; // ���󿡼��� ��� ID
		_float2 fNodePos; // ���󿡼��� ��� ��ġ
	};

	struct Transition 
	{
		size_t       fromIdx;    // ���� ���ͻ��� �ε���
		size_t       toIdx;
		function<bool()> condition;
		_float   duration;
		_int iLinkNodeId; // ���󿡼��� ��ũ ��� ID
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

	size_t  AddState(const string& name, class CAnimation* clip, _uint iAnimIndex)
	{
		m_States.push_back({ name, clip, iAnimIndex });
		// ù ���¶�� currentIdx 0����
		if (m_States.size() == 1)
			m_CurrentStateIdx = 0;
		return m_States.size() - 1;
    }

	AnimState* GetCurrentState() {
		return &m_States[m_CurrentStateIdx];
	}

	void AddTransition(size_t fromIdx, size_t toIdx, const Condition& cond, _float duration = 0.2f);

	void SetState(const string& name);
	vector<AnimState>& GetStates() { return m_States; }
	const vector<Transition>& GetTransitions() const { return m_Transitions; }
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
	size_t                 m_CurrentStateIdx = 0;
	class CAnimator* m_pAnimator = nullptr;  // �ִϸ����� ����
	unordered_map<size_t, size_t> m_SubClipIndex; //���� �� ��° Ŭ���� ��� ������ ����

public:
	static CAnimController* Create();
	CAnimController* Clone();
	virtual void Free() override;
};
NS_END
