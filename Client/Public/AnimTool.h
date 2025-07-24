#pragma once
#include "GameObject.h"

#define USE_IMGUI
#include "Client_Defines.h"
#include <MySequence.h>
#undef USE_IMGUI

NS_BEGIN(Engine)
class CModel;
class CAnimation;
class CAnimator;
class CTransform;
class CAnimController;
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
	HRESULT Render_AnimationSequence();
	HRESULT Render_AnimStatesByNode();
	HRESULT Render_Loaded_Models();
	HRESULT Render_Load_Model();

	HRESULT Bind_Shader();

	void UpdateCurrentModel(_float fTimeDelta);
	void SelectAnimation();
	void CreateModel(const string& fileName,const string& filePath);

	void Manipulate(
		Operation op,
		const _float snapT[3] = nullptr,   // 이동용 스냅 (unit)
		const _float snapR[3] = nullptr,   // 회전용 스냅 (deg)
		const _float snapS[3] = nullptr    // 스케일용 스냅 (factor)
	);

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

	_bool m_bRenerLevel = false;

	_bool m_bUseAnimSequence = false; // 애니메이션 시퀀스 모드 활성화

	// 애니메이션 시퀀스용

	CMySequence* m_pMySequence = nullptr;
	_bool m_bIsPlaying = false;
	_bool  m_bUseSequence = false;   // 시퀀서 모드 활성화 플래그
	_int   m_iSequenceFrame = 0;       // 시퀀서로 제어할 현재 프레임
	_int   m_iFirstFrame = 0;       // 시퀀서 뷰의 첫 프레임 (필요하다면)
	_int   m_playSpeed = 1;       // 재생 속도
	_float m_fTimeAcc = 0.0f;
	class CEventMag* m_pEventMag = nullptr; // 이벤트 매니저

public:
	static CAnimTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END