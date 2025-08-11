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
		FullbodyToFullbody,         // ��¥ -> ��¥
		FullbodyToMasked,           // ��¥ -> ����ü �и�
		MaskedToFullbody,           // ����ü �и� -> ��¥
		MaskedToMasked              // ����ü �и� -> ����ü �и�
	};


	struct Condition
	{
		string			paramName;
		ParamType type	= ParamType::Bool; // �Ķ���� Ÿ��
		EOp          op = EOp::None; // ������
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
		_float2 fNodePos{ 0.f, 0.f };

		// ����ü �и� �ִϸ��̼��� ���
		string lowerClipName; // ��ü �ִϸ��̼� �̸�
		string upperClipName; // ��ü �ִϸ��̼� �̸�
		string maskBoneName; // ����ũ�� �� �̸� (������ �� ���ڿ�)
		_float fBlendWeight = 1.f; // ���� ����ġ (0~1 ����)
		_float fLowerStartTime = 0.f; // ��ü �ִϸ��̼� ���� �ð�
		_float fUpperStartTime = 0.f; // ��ü �ִϸ��̼� ���� �ð�
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
		_float fLowerStartTime = 0.f; // ��ü �ִϸ��̼� ���� �ð�
		_float fUpperStartTime = 0.f; // ��ü �ִϸ��̼� ���� �ð�
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
		return nullptr; // ã�� ������ �� nullptr ��ȯ
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
		return nullptr; // ã�� ������ �� nullptr ��ȯ
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
		return empty; // ã�� ������ �� �� ���ڿ� ��ȯ
	}




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
			cout << "Entry state not found: " << entryStateName << endl; // ����׿� ���
			return;
		}
		else
		{
			m_EntryStateNodeId = m_EntryState->iNodeId; // Entry ���� ��� ID ����
			m_CurrentStateNodeId = m_EntryStateNodeId; // ���� ���¸� Entry ���·� ����
		}
	}
	void SetExit(const string& exitStateName)
	{
		m_ExitStateName = exitStateName;
		m_ExitState = FindState(exitStateName);
		if (m_ExitState == nullptr)
		{
			cout << "Exit state not found: " << exitStateName << endl; // ����׿� ���
			return;
		}
		else
		{
			m_ExitStateNodeId = m_ExitState->iNodeId; // Exit ���� ��� ID ����
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
	void ChangeStatesForOverride(const string& overrideCtrlName);// �������̵� �ִϸ��̼� ��Ʈ�ѷ��� ������ �� ���µ��� ����
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
				cout << "Exit state not found: " << m_ExitStateName << endl; // ����׿� ���
#endif // _DEBUG

				return -1; // Exit ���°� ������ -1 ��ȯ
			}
		}
		else
			return iNodeId;
	}

	_int ConvertAnyStateNodeIdToAnyState(_int iNodeId) const
	{
		if (iNodeId == m_iCAnyStateNodeID)
		{
			// AnyState���� �Ѿ�°Ŷ�� ���� ������� �ִϸ��̼��� ��� ID �ѱ��
			return m_CurrentStateNodeId;
		}
		return iNodeId; // AnyState�� �ƴ� ��� �״�� ��ȯ
	}
private:
	
	_bool m_bOverrideAnimController = false; // �������̵� �ִϸ��̼� ��Ʈ�ѷ� ��� ������
	// Override �ִϸ��̼� ��Ʈ�ѷ���
	unordered_map<string, OverrideAnimController> m_OverrideAnimControllers;
	// ���¡���ȯ ����
    _int                   m_CurrentStateNodeId= 0;
	_float                 m_fLayerWeight{ 0.f };
	unordered_map<string, vector<AnimState>> m_Layers;
	_int m_EntryStateNodeId = -1; // 
	_int m_ExitStateNodeId = -1;
	AnimState*			   m_EntryState{ nullptr };
	AnimState*			   m_ExitState{nullptr};
	AnimState*			   m_AnyState{ nullptr }; // Any ���� (��� ���¸� �����ϴ� ����)
	string				   m_EntryStateName{};
	string				   m_ExitStateName{};
	vector<AnimState>      m_States;
	vector<Transition>     m_Transitions;
	vector<Condition>	   m_Conditions; // ���� ���� �� ����

	class CAnimator*	   m_pAnimator = nullptr;  // �ִϸ����� ����
	unordered_map<string, Parameter> m_Params; // �Ķ���� ���� (��Ʈ�ѷ� �� ������ ����)
	unordered_map<string, vector<AnimState>> m_OriginalAnimStates; // ���� �ִϸ��̼� ���µ� (���纻�� ���� �� ���)
	TransitionResult       m_TransitionResult{}; // Ʈ�������� �� ��� (�ִϸ����Ϳ��� ��û)
	string 				   m_Name; // ��Ʈ�ѷ� �̸�

	const _int m_iCExitStateNodeID = 100000;
	const _int m_iCAnyStateNodeID = 100001;

	_bool m_bChangeDefaultController = false; // �⺻ ��Ʈ�ѷ��� ���� ����
	CAnimation* m_pTmpUpperAnimForChagned = nullptr; // ����Ʈ�� ����� ���� �ִϸ��̼� ��� ä���� �ֱ�
	CAnimation* m_pTmpLowerAnimForChagned = nullptr; // ����Ʈ�� ����� ���� �ִϸ��̼� ��� ä���� �ֱ�
public:
	static CAnimController* Create();
	CAnimController* Clone();
	virtual void Free() override;

	// ISerializable��(��) ���� ��ӵ�
	json Serialize() override;
	void Deserialize(const json& j) override;
};
NS_END
