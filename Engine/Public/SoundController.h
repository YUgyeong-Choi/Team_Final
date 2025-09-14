#pragma once
#include "Component.h"

NS_BEGIN(FMOD)
class ChannelGroup;
class DSP;
class Sound;
NS_END

NS_BEGIN(Engine)

class ENGINE_DLL CSoundController : public CComponent
{
private:
	CSoundController();
    CSoundController(const CSoundController& Prototype);
	virtual	~CSoundController() = default;

public:
    virtual HRESULT Initialize_Prototype(const string& strFolderPath, _bool is3D, _bool loop, _bool stream);
    virtual HRESULT Initialize(void* pArg) override;

    void Play(const string& strTag);

    //네이밍이 0부터 인것 ex) Walk_0, Walk_1 ...
    void Play_Random(const string& strTag, _int randCount, _int iStartNum = 0);

    void Stop(const string& strTag);
    // -1 = 무한루프
    void Set_Loop(const string& strTag, _int _LoopCount = -1);
    void Set_Pitch(const string& strTag, _float Pitch);

    void SetVolume(const string& strTag,_float Volume);
    void Set_AllVolume(_float Volume);
    // 사운드 포지션 업데이트
    void Update3DPosition(const string& strTag, _float3& vPos);
    //트랜스폼과 소리의 감쇠곡선을 결정할 죄소,최대 거리
    void Set3DState(const string& strTag, _float fMin, _float fMax);
    _bool IsPlaying(const string& strTag) const;

    void StopAll();
	void StopAllSpecific(const string& tags);
    void SetVolume(_float Volume);
    // 사운드 포지션 업데이트
    void Update3DPosition(_float3& vPos);
    //트랜스폼과 소리의 감쇠곡선을 결정할 죄소,최대 거리
    void Set3DState(_float fMin, _float fMax);

private:
    unordered_map<string, class CSound_Core*> m_Sounds;
    class CSound_Core* Find_Sound(const string& strTag) const;

public:
    static CSoundController* Create(const string& strFolderPath, _bool is3D = FALSE, _bool loop = FALSE, _bool stream = FALSE);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END