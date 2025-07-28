#pragma once
#include "Component.h"
#include "Animation.h"

#include "Serializable.h"

NS_BEGIN(Engine)
using AnimEventCallback = function<void(const string&)>;
class ENGINE_DLL CAnimator final : public CComponent, public ISerializable
{
public:
    struct BlendState
    {
        CAnimation* srcAnim = nullptr;   // 이전 애니메이션
        CAnimation* dstAnim = nullptr;   // 다음 애니메이션
        _float        elapsed = 0.f;       // 경과 시간
        _float        duration = 0.2f;      // 블렌드 총 시간
        _bool         isLoop = false;
        _bool         active = false;
        _bool         hasExitTime = false; // 이전 애니메이션 종료했을 때 블렌드 시작
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

    void StartTransition(CAnimation* from, CAnimation* to, _float duration = 0.2f);
    void Set_Animation(_uint iIndex, _float fadeDuration = 0.2f, _bool isLoop = false);
	void Set_BlendState(BlendState blend) { m_Blend = blend; }
	void Set_Model(class CModel* pModel) { m_pModel = pModel; }
	void SetBlendDuration(_float duration) { m_Blend.duration = duration; }
	void SetPlaying(_bool bPlaying) { m_bPlaying = bPlaying; }

    const string GetCurrentAnimName() const;
	class CAnimation* GetCurrentAnim() const { return m_pCurrentAnim; }

    // 애니메이션 컨트롤러 관련
    class CAnimController* Get_CurrentAnimController() const { return m_pCurAnimController; }
	class CAnimController* GetAnimController(const string& name) const {
		auto it = m_AnimControllers.find(name);
		if (it != m_AnimControllers.end())
			return it->second;
		return nullptr;
	}
    void SetCurrentAnimController(const string& name, const string& stateName = "");
    unordered_map<string, Parameter>& GetParameters();

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


    _float GetCurrentAnimProgress() const // 애니메이션 진행도
    {
        if (!m_pCurrentAnim) return 0.f;
        _float elapsed = m_pCurrentAnim->GetCurrentTrackPosition();
        _float duration = m_pCurrentAnim->GetDuration();
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

    // 파라미터 설정
    void SetInt(const string& name, _int v);
    void SetBool(const string& name, _bool v);
    void SetFloat(const string& name, _float v);
    void SetTrigger(const string& name);

    void ResetTrigger(const string& name);
    void DeleteParameter(const string& name);


    void SetParamName(Parameter& param, const string& name);


    // 조건 검사용 ( 현재 애니메이션 컨트롤러에서 가져오기)
    _float GetFloat(const string& name) const;
    _int GetInt(const string& name) const;
    _bool CheckBool(const string& name) const;
    _bool CheckTrigger(const string& name);


	unordered_map<string, class CAnimController*>& GetAnimControllers() { return m_AnimControllers; }

private:
    void UpdateBlend(_float fTimeDelta);
    void MakeMaskBones(const string& maskBoneName);
	void CollectBoneChildren(const _char* boneName);


private:
    _bool                       m_bPlaying = true;
	_bool                       m_bIsFinished = false; // 애니메이션 재생 완료 여부
    _uint						m_iCurrentAnimIndex = { };
    _uint						m_iPrevAnimIndex = { };
	class CAnimController*      m_pCurAnimController = nullptr; // 현재 애니메이션 컨트롤러
    class CModel*               m_pModel{ nullptr };          // 본과 메시 데이터 참조
	CAnimation*                 m_pCurrentAnim = nullptr; // 현재 애니메이션
	unordered_map<string, class CAnimController*> m_AnimControllers; // 컨트롤러 관리
    BlendState                  m_Blend{};

    unordered_map<string,unordered_set<_int>>         m_UpperMaskSetMap; // 한번씩만 매핑 해두기 이름 별로
	unordered_set<_int>         m_UpperMaskSet; // 하위 본들만 모아둔 집합 (마스크용)
	vector<class CBone*>        m_Bones; // 전체 본의 개수
    unordered_map<string, vector<AnimEventCallback>> m_eventListeners;

public:
	static CAnimator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;

    json Serialize() override;
    void Deserialize(const json& j) override;
};
NS_END
