#pragma once
#include "Base.h"

#include "fmod.hpp"
#include "fmod_studio.hpp"

NS_BEGIN(Engine)

class CSound_Device : public CBase
{
private:
	CSound_Device();
	virtual ~CSound_Device() = default;

public:
    HRESULT Initialize();
    void Update();

    // 리미터란? 소리가 한계치를 넘어간 소리를 좀 눌러서 정리해주는 이펙트
    void Set_Master_Limiter();
    // 사운드 컨트롤러용 로드 사운드 함수
    HRESULT LoadSound(const string& Path, _bool is3D, _bool loop, _bool stream, unordered_map<string, class CSound_Core*>* _Out_ pOut = nullptr);
    void Set_Listener_Position(class CTransform* pTransform, const _float3& vel);
    class CSound_Core* Get_Single_Sound(const string& strKey);
    void Set_Master_Volume(_float volume);
private:
    FMOD::DSP* m_pMasterLimiter = nullptr;
    FMOD::System* m_pCoreSystem = nullptr;
    FMOD_3D_ATTRIBUTES m_ListenerAttr{};
    unordered_map<string, class CSound_Core*> m_SingleSounds;

public:
    static CSound_Device* Create();
    virtual void Free();
};
NS_END