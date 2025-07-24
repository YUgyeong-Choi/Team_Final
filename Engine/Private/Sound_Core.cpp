#include "Sound_Core.h"
#include "fmod.hpp"
#include "Transform.h"

// 채널 중복접근 방지를 위한 포인터 초기화용 콜백
FMOD_RESULT F_CALLBACK OnChannelEnd(
    FMOD_CHANNELCONTROL* chanCtrl,
    FMOD_CHANNELCONTROL_TYPE type,
    FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType,
    void* commandData1, void* commandData2)
{
    if (callbackType == FMOD_CHANNELCONTROL_CALLBACK_END && type == FMOD_CHANNELCONTROL_CHANNEL)
    {
        FMOD::Channel* pChannel = reinterpret_cast<FMOD::Channel*>(chanCtrl);

        void* userData = nullptr;
        pChannel->getUserData(&userData);

        if (userData)
        {
            FMOD::Channel** ppChannel = static_cast<FMOD::Channel**>(userData);
            *ppChannel = nullptr;  // 진짜 채널 포인터를 nullptr로 초기화
        }
    }
    return FMOD_OK;
}

CSound_Core::CSound_Core(FMOD::System* pCoreSystem, FMOD::Sound* pSound)
    :m_pCoreSystem(pCoreSystem)
    , m_pSound(pSound, [](FMOD::Sound* Sound) {Sound->release(); })
{
}

CSound_Core::CSound_Core(CSound_Core& Prototype)
    :m_pCoreSystem(Prototype.m_pCoreSystem)
    , m_pSound(Prototype.m_pSound)
{
}

void CSound_Core::Play()
{
    // 만약 채널포인터를 잃어버린다면 콜백을 삭제
    if(m_pChannel)
        m_pChannel->setCallback(nullptr);

    if (m_pCoreSystem->playSound(m_pSound.get(), nullptr, true, &m_pChannel))
    {
        m_pChannel = nullptr;
        return;
    }

    m_pChannel->setVolume(m_fVolume);
    m_pChannel->set3DMinMaxDistance(m_fMinDis, m_fMaxDis);
    m_pChannel->set3DAttributes(reinterpret_cast<FMOD_VECTOR*>(&m_vSoundPos), {});

    // 채널 더블포인터와 콜백 등록
    m_pChannel->setUserData(&m_pChannel);
    m_pChannel->setCallback(OnChannelEnd);

    SetPaused(false);
}

void CSound_Core::Stop()
{
    if (m_pChannel)
    {
        m_pChannel->stop();
        m_pChannel = nullptr;
    }
}

void CSound_Core::Set_Volume(_float Volume)
{
    m_fVolume = Volume;
    if (m_pChannel)
        m_pChannel->setVolume(Volume);
}

void CSound_Core::Set_Pitch(_float Pitch)
{
    if (m_pChannel)
        m_pChannel->setPitch(Pitch);
}

void CSound_Core::Set_Loop(_int LoopCount)
{
    if (m_pChannel)
    {
        m_pChannel->setMode(FMOD_LOOP_NORMAL);
        m_pChannel->setLoopCount(LoopCount);
    }
}

void CSound_Core::Update3DPosition(_float3& vPos)
{
    _float3 vSoundVel = _float3{ vPos.x - m_vSoundPos.x, vPos.y - m_vSoundPos.y,vPos.z - m_vSoundPos.z };
    m_vSoundPos = vPos;
    if (m_pChannel)
        m_pChannel->set3DAttributes(reinterpret_cast<FMOD_VECTOR*>(&m_vSoundPos), reinterpret_cast<FMOD_VECTOR*>(&vSoundVel));

}

void CSound_Core::Set3DState(_float fMin, _float fMax)
{
    m_fMinDis = fMin;
    m_fMaxDis = fMax;
}

_bool CSound_Core::IsPlaying() const
{
    if (!m_pChannel) 
        return m_pChannel;

    _bool playing = false;
    m_pChannel->isPlaying(&playing);
    return playing;
}

void CSound_Core::SetPaused(_bool paused)
{
    if (m_pChannel)
        m_pChannel->setPaused(paused);
}

CSound_Core* CSound_Core::Create(FMOD::System* pCoreSystem, FMOD::Sound* pSound)
{
	return new CSound_Core(pCoreSystem, pSound);
}

CSound_Core* CSound_Core::Clone()
{
    return new CSound_Core(*this);
}

void CSound_Core::Free()
{
    __super::Free();
    Stop();
}

