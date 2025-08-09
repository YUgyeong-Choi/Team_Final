#pragma once
#include "Component.h"
#include "Animation.h"
#include "AnimController.h"

#include "Serializable.h"

NS_BEGIN(Engine)
using AnimEventCallback = function<void(const string&)>;
class ENGINE_DLL CAnimator final : public CComponent, public ISerializable
{
	friend class CAnimController;
public:
    struct BlendState
    {
        CAnimation* fromLowerAnim = nullptr;   // ���� ��ü �ִϸ��̼�
        CAnimation* toLowerAnim = nullptr;   // ���� ��ü �ִϸ��̼�
		CAnimation* fromUpperAnim = nullptr;   // ���� ��ü �ִϸ��̼�
		CAnimation* toUpperAnim = nullptr;   // ���� ��ü �ִϸ��̼�
        _float        elapsed = 0.f;       // ��� �ð�
        _float        duration = 0.2f;      // ���� �� �ð�
        _bool         isLoop = false;
        _bool         active = false;
        _bool         hasExitTime = false; // ���� �ִϸ��̼� �������� �� ���� ����
		_bool         belendFullBody = true; // ��ü �ٵ� ���� ����
        _float 	      blendWeight = 0.f; // ���� ����ġ (0~1 ����)
    };

private:
    CAnimator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CAnimator(const CAnimator& Prototype);
    virtual ~CAnimator() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
    void Update(_float fDeltaTime);

	void PlayClip(class CAnimation* pAnim, _bool isLoop = true);
    void StopAnimation() { m_bPlaying = false; }

    void StartTransition(const CAnimController::TransitionResult& transitionResult);
	void Set_BlendState(BlendState blend) { m_Blend = blend; }
	void Set_Model(class CModel* pModel) { m_pModel = pModel; }
	void SetBlendDuration(_float duration) { m_Blend.duration = duration; }
	void SetPlaying(_bool bPlaying) { m_bPlaying = bPlaying; }

    const string GetCurrentAnimName() const;
	class CAnimation* GetCurrentAnim() const { return m_pCurrentAnim; }
	class CAnimation* GetUpperClip() const { return m_pUpperClip; } // ��ü �ִϸ��̼� Ŭ��
	class CAnimation* GetLowerClip() const { return m_pLowerClip; } // ��ü �ִϸ��̼� Ŭ��

    // �ִϸ��̼� ��Ʈ�ѷ� ����
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
        // �����Ϳ��� �ִϸ��̼� ��Ʈ�ѷ� ����
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

    void RegisterEventListener(const string& eventName, AnimEventCallback cb);
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


    _float GetCurrentAnimProgress() const // �ִϸ��̼� ���൵
    {
		if (!m_pCurrentAnim) return 0.f;
		_float elapsed = m_pCurrentAnim->GetCurrentTrackPosition();
		_float duration = m_pCurrentAnim->GetDuration();
		return duration > 0.f
			? (elapsed / duration)
			: 0.f;
    }

	_float GetCurrentUpperAnimProgress() const // ��ü �ִϸ��̼� ���൵
	{
		if (!m_pUpperClip) return 0.f;
		_float elapsed = m_pUpperClip->GetCurrentTrackPosition();
		_float duration = m_pUpperClip->GetDuration();
		return duration > 0.f
			? (elapsed / duration)
			: 0.f;
	}

	_float GetCurrentLowerAnimProgress() const // ��ü �ִϸ��̼� ���൵
	{
		if (!m_pLowerClip) return 0.f;
		_float elapsed = m_pLowerClip->GetCurrentTrackPosition();
		_float duration = m_pLowerClip->GetDuration();
		return duration > 0.f
			? (elapsed / duration)
			: 0.f;
	}

    _float GetStateLengthByName(const string& name) const;
	_bool IsFinished() const { return m_bIsFinished; }
    class CModel* GetModel() const { return m_pModel; }
public:

    _bool ExisitsParameter(const string& name) const;
    void AddBool(const string& name);
    void AddFloat(const string& name);
    void AddTrigger(const string& name);
    void AddInt(const string& name);

    // �Ķ���� ����
    void SetInt(const string& name, _int v);
    void SetBool(const string& name, _bool v);
    void SetFloat(const string& name, _float v);
    void SetTrigger(const string& name);

    void ResetTrigger(const string& name);
    void DeleteParameter(const string& name);


    void SetParamName(Parameter& param, const string& name);

    void SetApplyRootMotion(_bool bApply);
        
    _bool IsApplyRootMotion() const { return m_bApplyRootMotion; }

    // ���� �˻�� ( ���� �ִϸ��̼� ��Ʈ�ѷ����� ��������)
    _float GetFloat(const string& name) const;
    _int GetInt(const string& name) const;
    _bool CheckBool(const string& name) const;
    _bool CheckTrigger(const string& name);


	unordered_map<string, class CAnimController*>& GetAnimControllers() { return m_AnimControllers; }
    void Add_OverrideAnimController(const string& name, const OverrideAnimController& overrideController)
    {
        if (m_OverrideControllerMap.find(name) != m_OverrideControllerMap.end())
        {
            // �̹� �����ϴ� ��Ʈ�ѷ� �̸��̸� �����
            m_OverrideControllerMap[name] = overrideController;
            return;
        }
		m_OverrideControllerMap.emplace(name, overrideController);
		auto it = m_OverrideControllerMap.find(name);
        if (it != m_OverrideControllerMap.end())
        {
			it->second.name = name; // �̸� ����
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
			overrideCtrl.name = newName; // �̸� ����
			m_OverrideControllerMap[newName] = overrideCtrl;
		}
	}



    _float3& GetRootMotionDelta()  { return m_RootMotionDelta; }
    _float4& GetRootRotationDelta()  { return m_RootRotationDelta; }
    _float GetYAngleFromQuaternion(const _vector& quat);

private:
    // �ִϸ��̼� �������
    void RefreshAndProcessTransition(_float fDeltaTime);
    void UpdateBlend(_float fDeltaTime, size_t iBoneCount, vector<string>& triggeredEvents);
	void UpdateAnimation(_float fDeltaTime, size_t iBoneCount, vector<string>& triggeredEvents);
    void AddUniqueClip(CAnimation* pClip, array<CAnimation*, 4>& pArray, _int& clipCount);

    // ����ũ �� ����
    void UpdateMaskState();
    void MakeMaskBones(const string& maskBoneName);
	void CollectBoneChildren(const _char* boneName);
    void CollectBoneChildren(const _char* boneName, const _char* stopBoneName);

	

    // �̺�Ʈ ó��
    void DispatchAnimEvents(const vector<string>& triggeredEvents);

    //��Ʈ���
    void RootMotionDecomposition();
    void ResetRootMotion();
    void SetCurrentRootRotation(const _float4& rot);
    void SetCurrentRootPosition(const _float3& pos);

    // ��� ���
    _matrix LerpMatrix(const _matrix& src, const _matrix& dst, _float t);
	void CollectBoneMatrices(CAnimation* pAnim, vector<_matrix>& boneMatrices, size_t iBoneCount);
private:
    _bool                       m_bPlaying = true;
	_bool                       m_bIsFinished = false; // �ִϸ��̼� ��� �Ϸ� ����
    _uint						m_iCurrentAnimIndex = { };
    _uint						m_iPrevAnimIndex = { };
	class CAnimController*      m_pCurAnimController = nullptr; // ���� �ִϸ��̼� ��Ʈ�ѷ�
    class CModel*               m_pModel{ nullptr };          // ���� �޽� ������ ����
	CAnimation*                 m_pCurrentAnim = nullptr; // ���� �ִϸ��̼�
	unordered_map<string, class CAnimController*> m_AnimControllers; // ��Ʈ�ѷ� ����
    BlendState                  m_Blend{};

	CAnimation* m_pUpperClip = nullptr; // ��ü �ִϸ��̼� Ŭ��
	CAnimation* m_pLowerClip = nullptr; // ��ü �ִϸ��̼� Ŭ��

    //CAnimation* m_pBlendFromLowerAnim = nullptr;
    //CAnimation* m_pBlendToLowerAnim = nullptr;
    //CAnimation* m_pBlendFromUpperAnim = nullptr; // ��ȯ ���� ���� ��ü Ŭ��
    //CAnimation* m_pBlendToUpperAnim = nullptr;   // ��ȯ ���� ��ǥ ��ü Ŭ��
    CAnimController::ETransitionType m_eCurrentTransitionType = CAnimController::ETransitionType::FullbodyToFullbody; // ���� ���� ���� ��ȯ Ÿ��

    unordered_map<string,unordered_set<_int>>         m_UpperMaskSetMap; // �ѹ����� ���� �صα� �̸� ����
	unordered_set<_int>         m_UpperMaskSet; // ���� ���鸸 ��Ƶ� ���� (����ũ��)
	_bool                       m_bPlayMask = false; // ����ũ �ִϸ��̼� ��� ���� (�ݺ� ������� Mask�� �ϴ���)
    _bool                       m_bSeparateUpperLowerBlend = false; // ���� ��ȯ �ÿ� ��,��ü �и� �÷���
	vector<class CBone*>        m_Bones; // ��ü ���� ����
    unordered_map<string, vector<AnimEventCallback>> m_eventListeners;
    unordered_map<string, OverrideAnimController> m_OverrideControllerMap; // �������̵��ϴ� ��Ʈ�ѷ� �̸��� ��Ʈ�ѷ� ����

    // ��Ʈ ���
    _float3 m_PrevRootPosition = { 0.f, 0.f, 0.f };
    _float3 m_CurrentRootPosition = { 0.f, 0.f, 0.f };
    _float4 m_PrevRootRotation = { 0.f, 0.f, 0.f, 1.f };
    _float4 m_CurrentRootRotation = { 0.f, 0.f, 0.f, 1.f };

    _float3 m_RootMotionDelta = { 0.f, 0.f, 0.f };
    _float4 m_RootRotationDelta = { 0.f, 0.f, 0.f, 1.f };
    _bool m_bFirstFrameAfterReset = false;
	_bool m_bApplyRootMotion = true; // ��Ʈ ��� ���� ����

    array<CAnimation*, 4> m_pBlendAnimArray{nullptr,};
	_int m_iBlendAnimCount = 0; // ���� �ִϸ��̼� ����

public:
	static CAnimator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;

    json Serialize() override;
    void Deserialize(const json& j) override;
};
NS_END
