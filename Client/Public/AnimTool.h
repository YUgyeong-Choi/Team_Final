#pragma once
#include "GameObject.h"
#include "Animator.h"
#include "Animation.h"
#include "AnimController.h"
#include "Client_Defines.h"
#include <MySequence.h>

NS_BEGIN(Engine)
class CModel;
class CTransform;
class CShader;
NS_END

NS_BEGIN(Client)

class CAnimTool final : public CGameObject
{
	enum class Operation { TRANSLATE, ROTATE, SCALE };
private:
	CAnimTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimTool(const CAnimTool& Prototype);
	virtual ~CAnimTool() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	HRESULT Render_TransitionConditions();
	HRESULT Render_AnimationSequence();
	HRESULT Render_AnimStatesByNode();
	HRESULT Render_AnimControllers();
	HRESULT Render_SaveLoadButtons();
	HRESULT Render_AddStatePopup(CAnimController* pCtrl, _int& iSpecificNodeId, CAnimation* pCurAnimation);
	HRESULT Render_NodesAndLinks(CAnimController* pCtrl);
	HRESULT Render_SelectedNodeInfo(CAnimController* pCtrl, _int& iSpecificNodeId, const vector<CAnimation*>& anims, const vector<string>& animNames);
	HRESULT Render_Loaded_Models();
	HRESULT Render_Load_Model();
	HRESULT Render_AnimEvents();
	HRESULT Render_Parameters();
	HRESULT Bind_Shader();

	void UpdateCurrentModel(_float fTimeDelta);
	void CreateModel(const string& fileName,const string& filePath);
	
	void Setting_Sequence();

	void SelectAnimation();
	void Setting_AnimationProperties();


	void SaveLoadEvents(_bool isSave = true);
	void SaveLoadAnimStates(_bool isSave = true);
	void Test_AnimEvents();

	void Manipulate(
		Operation op,
		const _float snapT[3] = nullptr,   // �̵��� ���� (unit)
		const _float snapR[3] = nullptr,   // ȸ���� ���� (deg)
		const _float snapS[3] = nullptr    // �����Ͽ� ���� (factor)
	);

	vector<string> GetAnimNames();

	HRESULT Handle_LinkCreation(CAnimController* pCtrl, _int& iSpecificNodeId);
	HRESULT Handle_LinkDeletion(CAnimController* pCtrl);
	HRESULT Handle_NodeDeletion(CAnimController* pCtrl);
	HRESULT Modify_Transition(CAnimController::Transition& transition);

private:
	CAnimator* m_pCurAnimator = nullptr; // ���� ���� �ִϸ�����
	CAnimation* m_pCurAnimation = nullptr; // ���� �ִϸ��̼�
	CModel* m_pCurModel = nullptr; // ���� ��
	string m_ModelFilePath;
	class CGameInstance* m_pGameInstance = nullptr;


	CShader* m_pAnimShader = nullptr; // �ִϸ��̼� ���̴�
	unordered_map<string, class CModel*> m_LoadedModels; // �ε�� �𵨵�
	unordered_map<string, class CAnimator*> m_LoadedAnimators; // ���� ���� �ִϸ�����
	unordered_map<string, vector<class CAnimation*>> m_LoadedAnimations; // ���� ���� �ִϸ��̼ǵ�
	string m_stSelectedModelName; // ���� ���õ� �� �̸�
	
	// �� �ҷ����� ��
	_bool m_bActiveLoadModel = false;
	// �𵨿� ���� ��Ʈ����
	_float4x4 m_ModelWorldMatrix{};
	CTransform* m_pTransformComForModel = nullptr; // ���� Ʈ������ ������Ʈ


	// ���� ������ ���������� ���� ���п�
	_bool m_bRenerLevel = false;



	// �ִϸ��̼� ��������
	CMySequence* m_pMySequence = nullptr;
	_bool m_bUseAnimSequence = true; // �ִϸ��̼� ������ ��� Ȱ��ȭ
	_bool m_bIsPlaying = false;
	_bool  m_bUseSequence = false;   // ������ ��� Ȱ��ȭ �÷���
	_int   m_iSequenceFrame = 0;       // �������� ������ ���� ������
	_int   m_iFirstFrame = 0;       // ������ ���� ù ������ (�ʿ��ϴٸ�)
	_int   m_iPlaySpeed = 1;       // ��� �ӵ�
	_float m_fTimeAcc = 0.0f;
	class CEventMag* m_pEventMag = nullptr; // �̺�Ʈ �Ŵ���

	// �ִϸ��̼� ������Ʈ �ӽ� ������
	_int m_iSpeicificNodeId = 1;
	_bool m_bShowParameters = false; // �Ķ���� UI ǥ�� ����


	static constexpr const _char* BoolOpNames[] = { "IsTrue", "IsFalse"};
	static constexpr const _char* TriggerOpNames[] = { "Trigger" };
	static constexpr const _char* CmpIntOpNames[] = { "Greater", "Less", "Equal", "NotEqual"};
	static constexpr const _char* CmpFloatOpNames[] = { "Greater", "Less" };


	static constexpr CAnimController::EOp BoolOps[] = { CAnimController::EOp::IsTrue,    CAnimController::EOp::IsFalse };
	static constexpr CAnimController::EOp TriggerOps[] = { CAnimController::EOp::Trigger };
	static constexpr CAnimController::EOp CmpIntOps[] = { CAnimController::EOp::Greater,   CAnimController::EOp::Less,     CAnimController::EOp::Equal,
														   CAnimController::EOp::NotEqual };
	static constexpr CAnimController::EOp CmpFloatOps[] = { CAnimController::EOp::Greater,   CAnimController::EOp::Less};
public:
	static CAnimTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END