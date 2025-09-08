#pragma once
#include "Component.h"
#include "Animation.h"
#include "AnimController.h"
#include "AnimComputeShader.h"

#include "Serializable.h"

NS_BEGIN(Engine)
using AnimEventCallback = function<void()>;
class ENGINE_DLL CAnimator final : public CComponent, public ISerializable
{
	friend class CAnimController;
public:
    typedef struct tagAnimatorDesc
    {
		const _wstring wstrCSOFilePath = L"";
		class CModel* pModel = nullptr; // 애니메이션이 적용될 모델
	}ANIMATOR_DESC;
public:
    struct BlendState
    {
        CAnimation* fromLowerAnim = nullptr;   // 이전 하체 애니메이션
        CAnimation* toLowerAnim = nullptr;   // 다음 하체 애니메이션
		CAnimation* fromUpperAnim = nullptr;   // 이전 상체 애니메이션
		CAnimation* toUpperAnim = nullptr;   // 다음 상체 애니메이션
        _float        elapsed = 0.f;       // 경과 시간
        _float        duration = 0.2f;      // 블렌드 총 시간
        _bool         isLoop = false;
        _bool         active = false;
        _bool         hasExitTime = false; // 이전 애니메이션 종료했을 때 블렌드 시작
		_bool         belendFullBody = true; // 전체 바디 블렌드 여부
		_bool         canSameAnimReset = false; // 같은 애니메이션일 때 초기화 가능한지
        _float 	      blendWeight = 0.f; // 블렌드 가중치 (0~1 사이)
		_float        lowerStartTime = 0.f; // 하체 애니메이션 시작 시간
		_float        upperStartTime = 0.f; // 상체 애니메이션 시작 시간
    };

private:
    CAnimator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CAnimator(const CAnimator& Prototype);
    virtual ~CAnimator() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
    HRESULT Initialize_Test(void* pArg);
    void Update(_float fDeltaTime);

    void Reset();

	void PlayClip(class CAnimation* pAnim, _bool isLoop = true);
    void StopAnimation() { m_bPlaying = false; }

    void StartTransition(const CAnimController::TransitionResult& transitionResult);
	void Set_BlendState(BlendState blend) { m_Blend = blend; }
	void Set_Model(class CModel* pModel) { m_pModel = pModel; }
	void SetBlendDuration(_float duration) { m_Blend.duration = duration; }
	void SetPlaying(_bool bPlaying) { m_bPlaying = bPlaying; }

    const string GetCurrentAnimName() const;
	class CAnimation* GetCurrentAnim() const { return m_pCurrentAnim; }
	class CAnimation* GetUpperClip() const { return m_pUpperClip; } // 상체 애니메이션 클립
	class CAnimation* GetLowerClip() const { return m_pLowerClip; } // 하체 애니메이션 클립

    // 애니메이션 컨트롤러 관련
    class CAnimController* Get_CurrentAnimController() const { return m_pCurAnimController; }
	class CAnimController* GetAnimController(const string& name) const {
		auto it = m_AnimControllers.find(name);
		if (it != m_AnimControllers.end())
			return it->second;
		return nullptr;
	}
    void SetCurrentAnimController(const string& name, const string& stateName = "");
#ifdef USE_IMGUI
    void SetCurrentAnimControllerForEditor(const string& name, const string& stateName = "")
    {
        // 에디터에서 애니메이션 컨트롤러 설정
        if (m_AnimControllers.find(name) != m_AnimControllers.end())
        {
            m_pCurAnimController = m_AnimControllers[name];
            m_pCurAnimController->SetState(stateName);
        }
    }

     unordered_map<string, Parameter>& GetParametersForEditor();
#endif


   const  unordered_map<string, Parameter>& GetParameters() const;

    void AddParameter(const string& name, Parameter& parm);
    void ResetParameters();

    void RegisterEventListener(const string& eventName, AnimEventCallback&& cb);

    void RegisterAnimController(const string& name, class CAnimController* pController);
    void UnregisterAnimController(const string& name);
    void RenameAnimController(const string& oldName, const string& newName);
	const unordered_map<string, vector<AnimEventCallback>>& GetEventListeners() const { return m_eventListeners; }

	_bool IsPlaying() const { return m_bPlaying; }
	_bool IsBlending() const { return m_Blend.active; }

	_float GetCurrentAnimDuration() const {
		if (m_pCurrentAnim == nullptr)
			return 0.f;
		return m_pCurrentAnim->GetDuration();
	}


    _float GetCurrentAnimProgress() const // 애니메이션 진행도
    {
		if (!m_pCurrentAnim) return 0.f;
		_float elapsed = m_pCurrentAnim->GetCurrentTrackPosition();
		_float duration = m_pCurrentAnim->GetDuration();
		return duration > 0.f
			? (elapsed / duration)
			: 0.f;
    }

	_float GetCurrentUpperAnimProgress() const // 상체 애니메이션 진행도
	{
		if (!m_pUpperClip) return 0.f;
		_float elapsed = m_pUpperClip->GetCurrentTrackPosition();
		_float duration = m_pUpperClip->GetDuration();
		return duration > 0.f
			? (elapsed / duration)
			: 0.f;
	}

	_float GetCurrentLowerAnimProgress() const // 하체 애니메이션 진행도
	{
		if (!m_pLowerClip) return 0.f;
		_float elapsed = m_pLowerClip->GetCurrentTrackPosition();
		_float duration = m_pLowerClip->GetDuration();
		return duration > 0.f
			? (elapsed / duration)
			: 0.f;
	}

    _float GetStateClipLengthByName(const string& name) const;
	_bool IsFinished() const { return m_bIsFinished; }
    class CModel* GetModel() const { return m_pModel; }
public:

    _bool ExisitsParameter(const string& name) const;
    void AddBool(const string& name);
    void AddFloat(const string& name);
    void AddTrigger(const string& name);
    void AddInt(const string& name);

    // 파라미터 설정
    void SetInt(const string& name, _int v);
    void SetBool(const string& name, _bool v);
    void SetFloat(const string& name, _float v);
    void SetTrigger(const string& name);

    void ResetTrigger(const string& name);
    void DeleteParameter(const string& name);


    void SetParamName(Parameter& param, const string& name);

    void SetApplyRootMotion(_bool bApply);
        
    _bool IsApplyRootMotion() const { return m_bApplyRootMotion; }

    // 조건 검사용 ( 현재 애니메이션 컨트롤러에서 가져오기)
    _float GetFloat(const string& name) const;
    _int GetInt(const string& name) const;
    _bool CheckBool(const string& name) const;
    _bool CheckTrigger(const string& name);


	unordered_map<string, class CAnimController*>& GetAnimControllers() { return m_AnimControllers; }
    void Add_OverrideAnimController(const string& name, const OverrideAnimController& overrideController)
    {
        if (m_OverrideControllerMap.find(name) != m_OverrideControllerMap.end())
        {
            // 이미 존재하는 컨트롤러 이름이면 덮어쓰기
            m_OverrideControllerMap[name] = overrideController;
            return;
        }
		m_OverrideControllerMap.emplace(name, overrideController);
		auto it = m_OverrideControllerMap.find(name);
        if (it != m_OverrideControllerMap.end())
        {
			it->second.name = name; // 이름 설정
        }
    }

    auto& GetOverrideAnimControllersMap() { return m_OverrideControllerMap; }

	void ApplyOverrideAnimController(const string& ctrlName)
    {
		auto it = m_OverrideControllerMap.find(ctrlName);
        if (it == m_OverrideControllerMap.end())
            return;
        if (m_pCurAnimController)
        {
			m_pCurAnimController->Applay_OverrideAnimController(it->first, it->second);
        }
	}

	void RemoveOverrideAnimController(const string& ctrlName)
	{
		auto it = m_OverrideControllerMap.find(ctrlName);
		if (it != m_OverrideControllerMap.end())
		{
			m_OverrideControllerMap.erase(it);
		}
	}
	void CancelOverrideAnimController()
	{
		if (m_pCurAnimController)
		{
			m_pCurAnimController->Cancel_OverrideAnimController();
		}
	}

	void RenameOverrideAnimController(const string& oldName, const string& newName)
	{
		auto it = m_OverrideControllerMap.find(oldName);
		if (it != m_OverrideControllerMap.end())
		{
			OverrideAnimController overrideCtrl = it->second;
			m_OverrideControllerMap.erase(it);
			overrideCtrl.name = newName; // 이름 변경
			m_OverrideControllerMap[newName] = overrideCtrl;
		}
	}



    _float3& GetRootMotionDelta()  { return m_RootMotionDelta; }
    _float4& GetRootRotationDelta()  { return m_RootRotationDelta; }
    _float GetYAngleFromQuaternion(const _vector& quat);
    ID3D11ShaderResourceView* GetFinalBoneMatricesSRV() const {
        if (m_pAnimComputeShader == nullptr)
            return nullptr;
        return m_pAnimComputeShader->GetOutputBoneSRV();
    }
	ID3D11UnorderedAccessView* GetFinalBoneMatricesUAV() const {
		if (m_pAnimComputeShader == nullptr)
			return nullptr;
		return m_pAnimComputeShader->GetOutputBoneUAV();
	}
#ifdef _DEBUG
    void DebugComputeShader();
	vector<_float4x4> DebugGetFinalBoneMatrices() const {
		if (m_pAnimComputeShader == nullptr)
			return {};
		vector<_float4x4> matrices(m_Bones.size());
        m_pAnimComputeShader->DownloadBoneMatrices(matrices.data(),static_cast<_uint>(m_Bones.size()));
		return matrices;
	}
#endif


    void SetPlayRate(_float fSpeed) { m_fPlaybackSpeed = fSpeed; }
private:
    // 애니메이션 재생관련
    void RefreshAndProcessTransition(_float fDeltaTime);
    void UpdateBlend(_float fDeltaTime, size_t iBoneCount, vector<string>& triggeredEvents);
	void UpdateAnimation(_float fDeltaTime, size_t iBoneCount, vector<string>& triggeredEvents);
    void AddUniqueClip(CAnimation* pClip, array<CAnimation*, 4>& pArray, _int& clipCount);

    // 마스크 본 관련
    void UpdateMaskState();
    void MakeMaskBones(const string& maskBoneName);
	void CollectBoneChildren(const _char* boneName);
    void CollectBoneChildren(const _char* boneName, const _char* stopBoneName);

	

    // 이벤트 처리
    void DispatchAnimEvents(const vector<string>& triggeredEvents);

    //루트모션
    void RootMotionDecomposition();
    void ResetRootMotion();
    void SetCurrentRootRotation(const _float4& rot);
    void SetCurrentRootPosition(const _float3& pos);

    // 행렬 계산
    _matrix LerpMatrix(const _matrix& src, const _matrix& dst, _float t);
	void CollectBoneMatrices(CAnimation* pAnim, vector<_matrix>& boneMatrices, size_t iBoneCount);


private:
    _bool                       m_bPlaying = true;
	_bool                       m_bIsFinished = false; // 애니메이션 재생 완료 여부
    _uint						m_iCurrentAnimIndex = { };
    _uint						m_iPrevAnimIndex = { };
	_float                      m_fPlaybackSpeed = 1.f; // 재생 속도 배율
	class CAnimController*      m_pCurAnimController = nullptr; // 현재 애니메이션 컨트롤러
    class CModel*               m_pModel{ nullptr };          // 본과 메시 데이터 참조
	CAnimation*                 m_pCurrentAnim = nullptr; // 현재 애니메이션
	unordered_map<string, class CAnimController*> m_AnimControllers; // 컨트롤러 관리
    BlendState                  m_Blend{};

	CAnimation* m_pUpperClip = nullptr; // 상체 애니메이션 클립
	CAnimation* m_pLowerClip = nullptr; // 하체 애니메이션 클립
    CAnimController::ETransitionType m_eCurrentTransitionType = CAnimController::ETransitionType::FullbodyToFullbody; // 현재 진행 중인 전환 타입

    unordered_map<string,unordered_set<_int>>         m_UpperMaskSetMap; // 한번씩만 매핑 해두기 이름 별로
	unordered_set<_int>         m_UpperMaskSet; // 하위 본들만 모아둔 집합 (마스크용)
	_bool                       m_bPlayMask = false; // 마스크 애니메이션 재생 여부 (반복 재생으로 Mask로 하는지)
    _bool                       m_bSeparateUpperLowerBlend = false; // 상태 전환 시에 상,하체 분리 플레그
	vector<class CBone*>        m_Bones; // 전체 본의 개수
    unordered_map<string, vector<AnimEventCallback>> m_eventListeners;
    unordered_map<string, OverrideAnimController> m_OverrideControllerMap; // 오버라이드하는 컨트롤러 이름과 컨트롤러 매핑

    // 루트 모션
    _float3 m_PrevRootPosition = { 0.f, 0.f, 0.f };
    _float3 m_CurrentRootPosition = { 0.f, 0.f, 0.f };
    _float4 m_PrevRootRotation = { 0.f, 0.f, 0.f, 1.f };
    _float4 m_CurrentRootRotation = { 0.f, 0.f, 0.f, 1.f };

    _float3 m_RootMotionDelta = { 0.f, 0.f, 0.f };
    _float4 m_RootRotationDelta = { 0.f, 0.f, 0.f, 1.f };
    _bool m_bFirstFrameAfterReset = false;
	_bool m_bApplyRootMotion = true; // 루트 모션 적용 여부

    array<CAnimation*, 4> m_pBlendAnimArray{nullptr,};
	_int m_iBlendAnimCount = 0; // 현재 애니메이션 개수

    // 컴퓨트 셰이더
	CAnimComputeShader* m_pAnimComputeShader = nullptr; // 애니메이션 컴퓨트 셰이더
    vector<_matrix> m_vLocalBoneMatrices;
	vector<_matrix> m_vFinalBoneMatrices; // 최종 본 행렬들 (GPU에서 받아온 행렬)

public:
	static CAnimator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;

    json Serialize() override;
    void Deserialize(const json& j) override;
};
NS_END
