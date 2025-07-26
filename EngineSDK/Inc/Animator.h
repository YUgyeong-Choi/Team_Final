#pragma once
#include "Component.h"
#include "Animation.h"
#include <iostream>
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

  //  struct Parameter
  //  {
		//ParamType     eType;
  //      _int          id;       // UI/식별용
  //      _bool         bValue = false;   // Bool/Trigger
  //      _float        fValue = 0.f;      // Float
  //      _bool         bTriggered = false;   // Trigger 전용 플래그
  //      _int          iValue = 0;      // Int
  //  };
    
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
    class CAnimController* GetAnimController() const { return m_pAnimController; }
	unordered_map<string, Parameter>& GetParameters() { return m_Params; }

	void AddParameter(const string& name, Parameter& parm) {
        m_Params[name] = parm;
        SetParamName(m_Params[name], name); // 파라미터 이름 설정
	}

    void RegisterEventListener(const string& eventName, AnimEventCallback cb);
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
public:
    void AddBool(const string& name) {
        m_Params[name].type = { ParamType::Bool }; 
        SetParamName(m_Params[name], name); // 파라미터 이름 설정
    }
    void AddFloat(const string& name) {
        m_Params[name].type = { ParamType::Float };
        SetParamName(m_Params[name], name); // 파라미터 이름 설정}
    }
    void AddTrigger(const string& name) {
        m_Params[name].type = { ParamType::Trigger };
        SetParamName(m_Params[name], name); // 파라미터 이름 설정}
    }
	void AddInt(const string& name) { 
        m_Params[name].type = { ParamType::Int }; 
		SetParamName(m_Params[name], name); // 파라미터 이름 설정
    }
    // 파라미터 설정
    void SetBool(const string& name, _bool v) {
        auto& p = m_Params[name];
        p.bValue = v;
    }
    void SetFloat(const string& name, _float v) {
        auto& p = m_Params[name];
        p.fValue = v;
    }
    void SetTrigger(const string& name) {
        auto& p = m_Params[name];
        p.bTriggered = true;
    }

	void ResetTrigger(const string& name) {
		auto& p = m_Params[name];
		p.bTriggered = false;
	}

	void SetInt(const string& name, _int v) {
		auto& p = m_Params[name];
		p.iValue = v;
	}

	void DeleteParameter(const string& name) {
		m_Params.erase(name);
	}

    // 조건 검사용
    _bool CheckBool(const string& name) const {
        _bool test = m_Params.at(name).bValue;
        return test;
    }
    _float GetFloat(const string& name) const { return m_Params.at(name).fValue; }
    _bool CheckTrigger(const string& name) {
        auto& p = m_Params[name];
        if (p.bTriggered) 
        { 
			cout << "Trigger: " << name << endl; // 디버그용 출력
            p.bTriggered = false; 
            return true; 
        }
        return false;
    }
	_int GetInt(const string& name) const { return m_Params.at(name).iValue; }

	_bool IsFinished() const { return m_bIsFinished; }

	_bool ExisitsParameter(const string& name) const {
		return m_Params.find(name) != m_Params.end();
	}
	class CModel* GetModel() const { return m_pModel; } 
private:
    void UpdateBlend(_float fTimeDelta);
	void SetParamName(Parameter& param, const string& name) {
		param.name = name; // 파라미터 이름 설정
	}
private:
    _bool                       m_bPlaying = true;
	_bool                       m_bIsFinished = false; // 애니메이션 재생 완료 여부
    _uint						m_iCurrentAnimIndex = { };
    _uint						m_iPrevAnimIndex = { };
	class CAnimController*      m_pAnimController = nullptr; // 애니메이션 컨트롤러
    class CModel*               m_pModel{ nullptr };          // 본과 메시 데이터 참조
	CAnimation*                 m_pCurrentAnim = nullptr; // 현재 애니메이션
    BlendState                  m_Blend{};
	vector<class CBone*>        m_Bones; // 전체 본의 개수
    unordered_map<string, Parameter> m_Params;
    unordered_map<string, vector<AnimEventCallback>> m_eventListeners;

public:
	static CAnimator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;

    // ISerializable을(를) 통해 상속됨
    json Serialize() override;
    void Deserialize(const json& j) override;
};
NS_END
