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
	HRESULT Render_TransitionConditions();
	HRESULT Render_AnimationSequence();
	HRESULT Render_AnimStatesByNode();
	HRESULT Render_AnimControllers();
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
	void ApplyHierarchicalLayout(class CAnimController* pCtrl);
	void ApplyCategoryLayout(class CAnimController* pCtrl);

	void SaveLoadEvents(_bool isSave = true);
	void SaveLoadAnimStates(_bool isSave = true);
	void Test_AnimEvents();
	void Add_OverrideAnimController(const string& name, const OverrideAnimController& overrideController);

	void Manipulate(
		Operation op,
		const _float snapT[3] = nullptr,   // 이동용 스냅 (unit)
		const _float snapR[3] = nullptr,   // 회전용 스냅 (deg)
		const _float snapS[3] = nullptr    // 스케일용 스냅 (factor)
	);

	string GetStateCategory(const string& stateName);

	HRESULT Modify_Transition(CAnimController::Transition& transition);

private:
	CAnimator* m_pCurAnimator = nullptr; // 현재 모델의 애니메이터
	CAnimation* m_pCurAnimation = nullptr; // 현재 애니메이션
	CModel* m_pCurModel = nullptr; // 현재 모델
	string m_ModelFilePath;
	class CGameInstance* m_pGameInstance = nullptr;


	CShader* m_pAnimShader = nullptr; // 애니메이션 셰이더
	unordered_map<string, class CModel*> m_LoadedModels; // 로드된 모델들
	unordered_map<string, class CAnimator*> m_LoadedAnimators; // 각각 모델의 애니메이터
	unordered_map<string, vector<class CAnimation*>> m_LoadedAnimations; // 각각 모델의 애니메이션들
	string m_stSelectedModelName; // 현재 선택된 모델 이름
	
	// 모델 불러오기 툴
	_bool m_bActiveLoadModel = false;
	// 모델용 월드 매트릭스
	_float4x4 m_ModelWorldMatrix{};
	CTransform* m_pTransformComForModel = nullptr; // 모델의 트랜스폼 컴포넌트


	// 레벨 렌더와 렌더러와의 렌더 구분용
	_bool m_bRenerLevel = false;



	// 애니메이션 시퀀스용
	CMySequence* m_pMySequence = nullptr;
	_bool m_bUseAnimSequence = true; // 애니메이션 시퀀스 모드 활성화
	_bool m_bIsPlaying = false;
	_bool  m_bUseSequence = false;   // 시퀀서 모드 활성화 플래그
	_int   m_iSequenceFrame = 0;       // 시퀀서로 제어할 현재 프레임
	_int   m_iFirstFrame = 0;       // 시퀀서 뷰의 첫 프레임 (필요하다면)
	_int   m_iPlaySpeed = 1;       // 재생 속도
	_float m_fTimeAcc = 0.0f;
	class CEventMag* m_pEventMag = nullptr; // 이벤트 매니저

	// 애니메이션 스테이트 머신 관리용
	_int m_iSpeicificNodeId = 1;
	_bool m_bShowParameters = false; // 파라미터 UI 표시 여부


	static constexpr const _char* BoolOpNames[] = { "IsTrue", "IsFalse"};
	static constexpr const _char* TriggerOpNames[] = { "Trigger" };
	static constexpr const _char* CmpIntOpNames[] = { "Greater", "Less", "Equal", "NotEqual"};
	static constexpr const _char* CmpFloatOpNames[] = { "Greater", "Less" };


	static constexpr CAnimController::EOp BoolOps[] = { CAnimController::EOp::IsTrue,    CAnimController::EOp::IsFalse };
	static constexpr CAnimController::EOp TriggerOps[] = { CAnimController::EOp::Trigger };
	static constexpr CAnimController::EOp CmpIntOps[] = { CAnimController::EOp::Greater,   CAnimController::EOp::Less,     CAnimController::EOp::Equal,
														   CAnimController::EOp::NotEqual };
	static constexpr CAnimController::EOp CmpFloatOps[] = { CAnimController::EOp::Greater,   CAnimController::EOp::Less};


	// 인덱스 선택 변수들 
	_int m_iSelectedListenerIdx = -1; // 선택된 애니메이션 이벤트 리스너 인덱스
	_int m_iControllerIndex = 0;	// 애니메이션 컨트롤러 인덱스
	_int m_iSelectEntry = -1; // 시퀀스 선택 인덱스
	_int m_iSelectedNodeID = -1; // 선택된 노드 ID
	_int m_iDefualtSeletedAnimIndex = -1; // 선택된 State Default Anim 인덱스
	_int m_iSelectedUpperAnimIndex = -1; // 상체 애니메이션 인덱스
	_int m_iSelectedLowerAnimIndex = -1; // 하체 애니메이션 인덱스
	_int m_iSelectedModelIndex = -1; // 선택된 모델 인덱스
	_int m_iSelectedAnimIndex = -1; // 선택된 애니메이션 인덱스
	_int m_iSelectedMaskBoneIndex = -1; // 선택된 마스크 뼈대 인덱스
	_int m_iSelectedOverrideStateIndex = -1; // 선택된 오버라이드 상태 인덱스
	_int m_iSelectedOverrideControllerIndex = -1; // 선택된 오버라이드 컨트롤러 인덱스
	
	// Bool 관련 변수들
	_bool m_bExpanded = true; // 트랙 확장 여부

	// 파라미터 팝업 관련 변수들
	_char m_NewParameterName[64] = ""; // 새 파라미터 이름 입력용
	_int  m_iNewType = 0;

	// 새로운 컨트롤러 생성 변수
	_char m_NewControllerName[64] = ""; // 새 컨트롤러 이름 입력용
	_char m_RenameControllerName[64] = ""; // 컨트롤러 이름 변경용
	_char m_OverrideControllerRename[64] = ""; // 이름 변경 오버라이드 컨트롤러 이름 변경용
	_char m_OverrideControllerName[64] = ""; // 컨트롤러 이름 변경용
	_char m_NewStateName[64] = ""; // 새 상태 이름 입력용
	vector<string> m_vecMaskBoneNames; // 마스크에 사용할 뼈대 이름들

	// 오버라이드 애니메이션 컨트롤러 이름 
	_char m_NewOverrideControllerName[64] = ""; // 새 오버라이드 컨트롤러 이름 입력용
	_char m_RenameOverrideControllerName[64] = ""; // 오버라이드 컨트롤러 이름 변경용
	OverrideAnimController m_NewOverrideAnimController; // 새 오버라이드 애니메이션 컨트롤러
	vector<OverrideAnimController> m_vecOverrideAnimControllers; // 오버라이드 애니메이션 컨트롤러들
	_bool m_bUseOverrideController = false; // 오버라이드 컨트롤러 사용 여부
	_bool m_bIsUesMaskBoneState = false;
	_int  m_iOverrideMaskBoneIndex = -1; // 오버라이드 컨트롤러의 마스크 뼈대 인덱스
	_int  m_iOverrideAnimIndex = -1; // 오버라이드 애니메이션 인덱스
	_int  m_iOverrideUpperAnimIndex = -1; // 오버라이드 상체 애니메이션 인덱스
	_int  m_iOverrideLowerAnimIndex = -1; // 오버라이드 하체 애니메이션 인덱스
	_int  m_iOverrideControllerIndex = -1;// 애니메이터에 등록된 오버라이드 컨트롤러
	_float m_fOverrideBlendWeight = 1.f; // 오버라이드 블렌드 가중치

	// 레이아웃
	unordered_map<string, _bool> m_CategoryVisibility; // 카테고리별 보이기 상태
	unordered_map<string, vector<string>> m_CategoryStates; // 카테고리별 상태 이름들
	_bool m_bShowAll = true;
	_bool m_bShowAllLink = false;

public:
	static CAnimTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END
#endif