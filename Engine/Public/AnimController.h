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
		FullbodyToFullbody,         // ��¥ -> ��¥
		FullbodyToMasked,           // ��¥ -> ����ü �и�
		MaskedToFullbody,           // ����ü �и� -> ��¥
		MaskedToMasked              // ����ü �и� -> ����ü �и�
	};
	struct Condition
	{
		string			paramName;
		ParamType type	= ParamType::Bool; // �Ķ���� Ÿ��
		EOp          op;
		_int         iThreshold = 0; // int�� �� ���� ��
		_float       fThreshold = 0.f; // float�̳� int�� �� ���� ��
		// Evaluate ������ CPP����
		_bool Evaluate(class CAnimController* pAnimController) const;
	};


	struct AnimState
	{
		string stateName;
		class CAnimation* clip = nullptr; // ���� �ִϸ��̼�
		_int iNodeId; // ���󿡼��� ��� ID
		_float2 fNodePos; // ���󿡼��� ��� ��ġ

		// ����ü �и� �ִϸ��̼��� ���
		string lowerClipName; // ��ü �ִϸ��̼� �̸�
		string upperClipName; // ��ü �ִϸ��̼� �̸�
		string maskBoneName; // ����ũ�� �� �̸� (������ �� ���ڿ�)
		_float fBlendWeight = 1.f; // ���� ����ġ (0~1 ����)
	};

	struct Transition 
	{
		_float		 duration = 0.2f; // ��ȯ �ð�
		_float 	     minTime = 0.f; // �ּ� �ð� (�ִϸ��̼� ���� �ð�)
		_float       maxTime = 1.f; // �ִ� �ð� (�ִϸ��̼� ���� �ð�)
		_int		iFromNodeId; // ���� ��� ID
		_int		iToNodeId; // �� ��� ID

		Link link; // ��ũ ���� (���󿡼� ���)
		vector<Condition> conditions; // ��ȯ ����
		_bool hasExitTime = false; // �ִϸ��̼��� �� ���� ��쿡
		_bool Evaluates(class CAnimController* pAnimController, class CAnimator* pAnimator) const;
	};

	struct TransitionResult
	{
		_bool bTransition = false; // ��ȯ ����
		_bool bBlendFullbody = true; // ���� ����
		ETransitionType eType = ETransitionType::FullbodyToFullbody;
		CAnimation* pFromLowerAnim = nullptr; // ��ȯ ���� ��ü/��¥ Ŭ��
		CAnimation* pToLowerAnim = nullptr;   // ��ȯ ��ǥ ��ü/��¥ Ŭ��
		CAnimation* pFromUpperAnim = nullptr; // ��ȯ ���� ��ü Ŭ�� 
		CAnimation* pToUpperAnim = nullptr;   // ��ȯ ��ǥ ��ü Ŭ�� 
		_float fDuration = 0.f; // ��ȯ �ð�
		_float fBlendWeight = 1.f; // ����ũ�� �����
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

	void SetName(const string& name) { m_Name = name; }
	const string& GetName() const { return m_Name; }

	// �Ķ���� ����
	_bool ExisitsParameter(const string& name) const {
		return m_Params.find(name) != m_Params.end();
	}

	void AddParameter(const string& name, Parameter& parm) {
		m_Params[name] = parm;
		SetParamName(m_Params[name], name); // �Ķ���� �̸� ����
	}

	void AddBool(const string& name) {
		m_Params[name].type = { ParamType::Bool };
		SetParamName(m_Params[name], name); // �Ķ���� �̸� ����
	}
	void AddFloat(const string& name) {
		m_Params[name].type = { ParamType::Float };
		SetParamName(m_Params[name], name); // �Ķ���� �̸� ����}
	}
	void AddTrigger(const string& name) {
		m_Params[name].type = { ParamType::Trigger };
		SetParamName(m_Params[name], name); // �Ķ���� �̸� ����}
	}
	void AddInt(const string& name) {
		m_Params[name].type = { ParamType::Int };
		SetParamName(m_Params[name], name); // �Ķ���� �̸� ����
	}
	// �Ķ���� ����
	void SetBool(const string& name, _bool v) {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // ����׿� ���
			return; // �Ķ���Ͱ� ������ �ƹ��͵� ���� ����
		}
		auto& p = m_Params[name];
		p.bValue = v;
	}
	void SetFloat(const string& name, _float v) {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // ����׿� ���
			return; // �Ķ���Ͱ� ������ �ƹ��͵� ���� ����
		}
		auto& p = m_Params[name];
		p.fValue = v;
	}
	void SetTrigger(const string& name) {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // ����׿� ���
			return; // �Ķ���Ͱ� ������ �ƹ��͵� ���� ����
		}
		auto& p = m_Params[name];
		p.bTriggered = true;
	}

	void ResetTrigger(const string& name) {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // ����׿� ���
			return; // �Ķ���Ͱ� ������ �ƹ��͵� ���� ����
		}
		auto& p = m_Params[name];
		p.bTriggered = false;
	}

	void SetInt(const string& name, _int v) {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // ����׿� ���
			return; // �Ķ���Ͱ� ������ �ƹ��͵� ���� ����
		}
		auto& p = m_Params[name];
		p.iValue = v;
	}

	void DeleteParameter(const string& name) {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // ����׿� ���
			return; // �Ķ���Ͱ� ������ �ƹ��͵� ���� ����
		}
		m_Params.erase(name);
	}

	// ���� �˻��
	_bool CheckBool(const string& name) const {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // ����׿� ���
			return false; // �⺻�� ��ȯ
		}
		return m_Params.at(name).bValue;
	}
	_float GetFloat(const string& name) const {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // ����׿� ���
			return 0.f; // �⺻�� ��ȯ
		}
		return m_Params.at(name).fValue;
	}
	_bool CheckTrigger(const string& name) {
		auto& p = m_Params[name];
		if (p.bTriggered)
		{
			cout << "Trigger: " << name << endl; // ����׿� ���
			p.bTriggered = false;
			return true;
		}
		return false;
	}
	_int GetInt(const string& name) const {
		if (m_Params.find(name) == m_Params.end()) {
			cout << "Parameter not found: " << name << endl; // ����׿� ���
			return 0; // �⺻�� ��ȯ
		}
		return m_Params.at(name).iValue;
	}

	void SetParamName(Parameter& param, const string& name) {
		param.name = name; // �Ķ���� �̸� ����
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
	// ���¡���ȯ ����
    _int                   m_CurrentStateNodeId= 0;
	_float                 m_fLayerWeight{ 0.f };
	unordered_map<string, vector<AnimState>> m_Layers;

	AnimState              m_EntryState;
	AnimState              m_ExitState;
	vector<AnimState>      m_States;
	vector<Transition>     m_Transitions;
	vector<Condition>	   m_Conditions; // ���� ���� �� ����

	class CAnimator*	   m_pAnimator = nullptr;  // �ִϸ����� ����
	unordered_map<string, Parameter> m_Params; // �Ķ���� ���� (��Ʈ�ѷ� �� ������ ����)

	TransitionResult       m_TransitionResult{}; // Ʈ�������� �� ��� (�ִϸ����Ϳ��� ��û)
	string 				   m_Name; // ��Ʈ�ѷ� �̸�
public:
	static CAnimController* Create();
	CAnimController* Clone();
	virtual void Free() override;

	// ISerializable��(��) ���� ��ӵ�
	json Serialize() override;
	void Deserialize(const json& j) override;
};
NS_END
