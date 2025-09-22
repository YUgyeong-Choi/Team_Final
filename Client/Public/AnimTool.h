#pragma once
#ifdef USE_IMGUI
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
	HRESULT Render_OverrideAnimControllers();
	HRESULT Render_AnimationStateMachine();
	HRESULT Render_AnimationSequence();
	HRESULT Render_AnimControllers();
	HRESULT Render_Loaded_Models();
	HRESULT Render_SpawnedObject();
	HRESULT Render_Spawn_Object();
	HRESULT Render_Transitions(class CAnimController* pCtrl,_bool bIsAnyLinkSelected, _bool bIsAnyNodeSelected);
	HRESULT Render_Load_Model();
	HRESULT Render_AnimEvents();
	HRESULT Render_Parameters();
	HRESULT Bind_Shader();


	HRESULT Handle_Links(CAnimController* pCtrl);

	HRESULT Delete_AnimState(CAnimController* pCtrl);

	void InitImNodesStyle();

	void UpdateCurrentModel(_float fTimeDelta);
	void CreateModel(const string& fileName,const string& filePath);
	
	void Setting_Sequence();

	void SelectAnimation();
	void Setting_AnimationProperties();
	void ApplyCategoryLayout(class CAnimController* pCtrl);

	void SaveOrLoadEvents(_bool isSave = true);
	void SaveOrLoadAnimStates(_bool isSave = true);

	string GetStateCategory(const string& stateName);

	HRESULT Modify_Transition(CAnimController::Transition& transition);

	HRESULT Register_Objects();

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
	_bool m_bRenderLevel = false;

	_bool m_bIsObject{ false };


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
	_int m_iSpecificNodeId = 1;
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


	// �ε��� ���� ������ 
	_int m_iSelectedListenerIdx = -1; // ���õ� �ִϸ��̼� �̺�Ʈ ������ �ε���
	_int m_iControllerIndex = 0;	// �ִϸ��̼� ��Ʈ�ѷ� �ε���
	_int m_iSelectEntry = -1; // ������ ���� �ε���
	_int m_iSelectedNodeID = -1; // ���õ� ��� ID
	_int m_iDefaultSelectedAnimIndex = -1; // ���õ� State Default Anim �ε���
	_int m_iSelectedUpperAnimIndex = -1; // ��ü �ִϸ��̼� �ε���
	_int m_iSelectedLowerAnimIndex = -1; // ��ü �ִϸ��̼� �ε���
	_int m_iSelectedModelIndex = -1; // ���õ� �� �ε���
	_int m_iSelectedAnimIndex = -1; // ���õ� �ִϸ��̼� �ε���
	_int m_iSelectedMaskBoneIndex = -1; // ���õ� ����ũ ���� �ε���
	_int m_iSelectedOverrideStateIndex = -1; // ���õ� �������̵� ���� �ε���
	_int m_iSelectedOverrideControllerIndex = -1; // ���õ� �������̵� ��Ʈ�ѷ� �ε���
	
	// Bool ���� ������
	_bool m_bExpanded = true; // Ʈ�� Ȯ�� ����

	// �Ķ���� �˾� ���� ������
	_char m_NewParameterName[64] = ""; // �� �Ķ���� �̸� �Է¿�
	_int  m_iNewType = 0;

	// ���ο� ��Ʈ�ѷ� ���� ����
	_char m_NewControllerName[64] = ""; // �� ��Ʈ�ѷ� �̸� �Է¿�
	_char m_RenameControllerName[64] = ""; // ��Ʈ�ѷ� �̸� �����
	_char m_OverrideControllerRename[64] = ""; // �̸� ���� �������̵� ��Ʈ�ѷ� �̸� �����
	_char m_OverrideControllerName[64] = ""; // ��Ʈ�ѷ� �̸� �����
	_char m_NewStateName[64] = ""; // �� ���� �̸� �Է¿�
	vector<string> m_vecMaskBoneNames; // ����ũ�� ����� ���� �̸���

	// �������̵� �ִϸ��̼� ��Ʈ�ѷ� �̸� 
	_char m_NewOverrideControllerName[64] = ""; // �� �������̵� ��Ʈ�ѷ� �̸� �Է¿�
	_char m_RenameOverrideControllerName[64] = ""; // �������̵� ��Ʈ�ѷ� �̸� �����
	OverrideAnimController m_NewOverrideAnimController; // �� �������̵� �ִϸ��̼� ��Ʈ�ѷ�
	vector<OverrideAnimController> m_vecOverrideAnimControllers; // �������̵� �ִϸ��̼� ��Ʈ�ѷ���
	_bool m_bUseOverrideController = false; // �������̵� ��Ʈ�ѷ� ��� ����
	_bool m_bIsUseMaskBoneState = false;
	_int  m_iOverrideMaskBoneIndex = -1; // �������̵� ��Ʈ�ѷ��� ����ũ ���� �ε���
	_int  m_iOverrideAnimIndex = -1; // �������̵� �ִϸ��̼� �ε���
	_int  m_iOverrideUpperAnimIndex = -1; // �������̵� ��ü �ִϸ��̼� �ε���
	_int  m_iOverrideLowerAnimIndex = -1; // �������̵� ��ü �ִϸ��̼� �ε���
	_int  m_iOverrideControllerIndex = -1;// �ִϸ����Ϳ� ��ϵ� �������̵� ��Ʈ�ѷ�
	_float m_fOverrideBlendWeight = 1.f; // �������̵� ���� ����ġ

	// ���̾ƿ�
	unordered_map<string, _bool> m_CategoryVisibility; // ī�װ��� ���̱� ����
	unordered_map<string, vector<string>> m_CategoryStates; // ī�װ��� ���� �̸���
	_bool m_bShowAll = true;
	_bool m_bShowAllLink = false;

	// Ư�� ��� ���̵��
	const _int EXIT_NODE_ID = 100000;
	const _int ANY_NODE_ID = 100001; 

	unordered_set<_int> m_DrawnInPins;
	unordered_set<_int> m_DrawnOutPins;


	// ������Ʈ�� 
	_bool m_bIsObjectToolActive = false; // ������Ʈ �� Ȱ��ȭ ����
	_bool m_bIsObjectSelected = false; // ������Ʈ�� ���õǾ����� ����
	_int m_iSelectedObjectIndex = -1; // ���õ� ������Ʈ �ε���
	_int m_iSelectedSpawnIndex = -1;
	CGameObject* m_pSelectedObject = nullptr; // ���õ� ������Ʈ
	string m_stSelectedObjectName; // ���õ� ������Ʈ �̸�
	vector<CGameObject*> m_vecObjects; // �ε�� ������Ʈ��
	vector<string> m_vecObjectNames; // ������Ʈ �̸���
	class CEditorObjectFactory* m_pEditorObjectFactory = nullptr; // ������ ������Ʈ ���丮
	unordered_map<string, void*> m_SpawnObjectDesc;


	// ��� �ִϸ��̼� �ӵ� �ϰ� ó���� ���� ����
	_float m_fAllAnimTickperSec = 55.f;


public:
	static CAnimTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END
#endif