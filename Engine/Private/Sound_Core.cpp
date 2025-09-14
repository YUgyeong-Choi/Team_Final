#include "Sound_Core.h"
#include "fmod.hpp"
#include "Transform.h"
#include <algorithm>

// 채널 종료 콜백 - 채널이 끝나면 벡터에서 자동 제거
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
            // CSound_Core의 ActiveChannels 벡터에서 해당 채널 포인터를 nullptr로 설정
            FMOD::Channel** ppChannel = static_cast<FMOD::Channel**>(userData);
            *ppChannel = nullptr;
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
    , m_fDefaultVolume(Prototype.m_fDefaultVolume)
    , m_fDefaultPitch(Prototype.m_fDefaultPitch)
    , m_fMinDis(Prototype.m_fMinDis)
    , m_fMaxDis(Prototype.m_fMaxDis)
    , m_vSoundPos(Prototype.m_vSoundPos)
    , m_iDefaultLoopCount(Prototype.m_iDefaultLoopCount)
{
}

void CSound_Core::Play()
{
    // 비활성 채널들 정리
    RemoveInactiveChannels();

    FMOD::Channel* pNewChannel = nullptr;

    // 새로운 채널로 사운드 재생
    FMOD_RESULT result = m_pCoreSystem->playSound(m_pSound.get(), nullptr, true, &pNewChannel);
    if (result != FMOD_OK || !pNewChannel)
    {
        return;
    }

    // 채널을 벡터에 추가
    m_ActiveChannels.push_back(pNewChannel);

    // 기본 설정들 적용
    pNewChannel->setVolume(m_fDefaultVolume);
    pNewChannel->setPitch(m_fDefaultPitch);
    pNewChannel->set3DMinMaxDistance(m_fMinDis, m_fMaxDis);
    pNewChannel->set3DAttributes(reinterpret_cast<FMOD_VECTOR*>(&m_vSoundPos), nullptr);

    // 루프 설정
    if (m_iDefaultLoopCount != 0)
    {
        pNewChannel->setMode(FMOD_LOOP_NORMAL);
        pNewChannel->setLoopCount(m_iDefaultLoopCount);
    }
    else
    {
        pNewChannel->setMode(FMOD_LOOP_OFF);
    }

    // 콜백 설정 - 채널 포인터의 주소를 userData로 전달
    auto& channelRef = m_ActiveChannels.back();
    pNewChannel->setUserData(&channelRef);
    pNewChannel->setCallback(OnChannelEnd);

    // 재생 시작
    pNewChannel->setPaused(false);
}

void CSound_Core::Stop()
{
    RemoveInactiveChannels();

    // 가장 최근에 재생된 채널 하나만 정지
    if (!m_ActiveChannels.empty())
    {
        auto it = m_ActiveChannels.rbegin();
        if (*it)
        {
            (*it)->stop(); // 콜백에서 자동으로 nullptr 처리됨
        }
    }
}

void CSound_Core::StopAll()
{
    // 모든 활성 채널 정지
    for (auto& channel : m_ActiveChannels)
    {
        if (channel)
        {
            channel->stop(); // 콜백에서 자동으로 nullptr 처리됨
        }
    }
    m_ActiveChannels.clear();
}

void CSound_Core::Set_Volume(_float Volume)
{
    m_fDefaultVolume = Volume;

    RemoveInactiveChannels();

    // 현재 재생 중인 모든 채널에 볼륨 적용
    for (auto& channel : m_ActiveChannels)
    {
        if (channel)
            channel->setVolume(Volume);
    }
}

void CSound_Core::Set_Pitch(_float Pitch)
{
    m_fDefaultPitch = Pitch;

    RemoveInactiveChannels();

    // 현재 재생 중인 모든 채널에 피치 적용
    for (auto& channel : m_ActiveChannels)
    {
        if (channel)
            channel->setPitch(Pitch);
    }
}

void CSound_Core::Set_Loop(_int LoopCount)
{
    // 기본값 저장 (다음 재생부터 적용)
    m_iDefaultLoopCount = LoopCount;

    // 현재 재생 중인 채널들에는 적용하지 않음 (끊김 방지)
    // 새로 재생되는 사운드부터 루프 설정이 적용됨
}

void CSound_Core::Update3DPosition(_float3& vPos)
{
    _float3 vSoundVel = _float3{ vPos.x - m_vSoundPos.x, vPos.y - m_vSoundPos.y, vPos.z - m_vSoundPos.z };
    m_vSoundPos = vPos;

    RemoveInactiveChannels();

    // 모든 활성 채널에 3D 위치 업데이트
    for (auto& channel : m_ActiveChannels)
    {
        if (channel)
            channel->set3DAttributes(reinterpret_cast<FMOD_VECTOR*>(&m_vSoundPos),
                reinterpret_cast<FMOD_VECTOR*>(&vSoundVel));
    }
}

void CSound_Core::Set3DState(_float fMin, _float fMax)
{
    m_fMinDis = fMin;
    m_fMaxDis = fMax;

    RemoveInactiveChannels();

    // 모든 활성 채널에 3D 거리 설정 적용  
    for (auto& channel : m_ActiveChannels)
    {
        if (channel)
            channel->set3DMinMaxDistance(fMin, fMax);
    }
}

_bool CSound_Core::IsPlaying() const
{
    // 하나라도 재생 중이면 true 반환
    for (const auto& channel : m_ActiveChannels)
    {
        if (channel)
        {
            bool playing = false;
            if (channel->isPlaying(&playing) == FMOD_OK && playing)
                return true;
        }
    }
    return false;
}

void CSound_Core::SetPaused(_bool paused)
{
    RemoveInactiveChannels();

    // 모든 활성 채널에 일시정지 적용
    for (auto& channel : m_ActiveChannels)
    {
        if (channel)
            channel->setPaused(paused);
    }
}

void CSound_Core::RemoveInactiveChannels()
{
    // nullptr이거나 재생이 끝난 채널들을 벡터에서 제거
    m_ActiveChannels.erase(
        std::remove_if(m_ActiveChannels.begin(), m_ActiveChannels.end(),
            [](FMOD::Channel* channel) {
                if (!channel) return true;

                bool playing = false;
                return (channel->isPlaying(&playing) != FMOD_OK || !playing);
            }),
        m_ActiveChannels.end()
    );
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
    StopAll();
}