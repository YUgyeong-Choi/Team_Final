#include "Sound_Core.h"
#include "fmod.hpp"
#include "Transform.h"
#include <algorithm>

// ä�� ���� �ݹ� - ä���� ������ ���Ϳ��� �ڵ� ����
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
            // CSound_Core�� ActiveChannels ���Ϳ��� �ش� ä�� �����͸� nullptr�� ����
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
    // ��Ȱ�� ä�ε� ����
    RemoveInactiveChannels();

    FMOD::Channel* pNewChannel = nullptr;

    // ���ο� ä�η� ���� ���
    FMOD_RESULT result = m_pCoreSystem->playSound(m_pSound.get(), nullptr, true, &pNewChannel);
    if (result != FMOD_OK || !pNewChannel)
    {
        return;
    }

    // ä���� ���Ϳ� �߰�
    m_ActiveChannels.push_back(pNewChannel);

    // �⺻ ������ ����
    pNewChannel->setVolume(m_fDefaultVolume);
    pNewChannel->setPitch(m_fDefaultPitch);
    pNewChannel->set3DMinMaxDistance(m_fMinDis, m_fMaxDis);
    pNewChannel->set3DAttributes(reinterpret_cast<FMOD_VECTOR*>(&m_vSoundPos), nullptr);

    // ���� ����
    if (m_iDefaultLoopCount != 0)
    {
        pNewChannel->setMode(FMOD_LOOP_NORMAL);
        pNewChannel->setLoopCount(m_iDefaultLoopCount);
    }
    else
    {
        pNewChannel->setMode(FMOD_LOOP_OFF);
    }

    // �ݹ� ���� - ä�� �������� �ּҸ� userData�� ����
    auto& channelRef = m_ActiveChannels.back();
    pNewChannel->setUserData(&channelRef);
    pNewChannel->setCallback(OnChannelEnd);

    // ��� ����
    pNewChannel->setPaused(false);
}

void CSound_Core::Stop()
{
    RemoveInactiveChannels();

    // ���� �ֱٿ� ����� ä�� �ϳ��� ����
    if (!m_ActiveChannels.empty())
    {
        auto it = m_ActiveChannels.rbegin();
        if (*it)
        {
            (*it)->stop(); // �ݹ鿡�� �ڵ����� nullptr ó����
        }
    }
}

void CSound_Core::StopAll()
{
    // ��� Ȱ�� ä�� ����
    for (auto& channel : m_ActiveChannels)
    {
        if (channel)
        {
            channel->stop(); // �ݹ鿡�� �ڵ����� nullptr ó����
        }
    }
    m_ActiveChannels.clear();
}

void CSound_Core::Set_Volume(_float Volume)
{
    m_fDefaultVolume = Volume;

    RemoveInactiveChannels();

    // ���� ��� ���� ��� ä�ο� ���� ����
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

    // ���� ��� ���� ��� ä�ο� ��ġ ����
    for (auto& channel : m_ActiveChannels)
    {
        if (channel)
            channel->setPitch(Pitch);
    }
}

void CSound_Core::Set_Loop(_int LoopCount)
{
    // �⺻�� ���� (���� ������� ����)
    m_iDefaultLoopCount = LoopCount;

    // ���� ��� ���� ä�ε鿡�� �������� ���� (���� ����)
    // ���� ����Ǵ� ������� ���� ������ �����
}

void CSound_Core::Update3DPosition(_float3& vPos)
{
    _float3 vSoundVel = _float3{ vPos.x - m_vSoundPos.x, vPos.y - m_vSoundPos.y, vPos.z - m_vSoundPos.z };
    m_vSoundPos = vPos;

    RemoveInactiveChannels();

    // ��� Ȱ�� ä�ο� 3D ��ġ ������Ʈ
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

    // ��� Ȱ�� ä�ο� 3D �Ÿ� ���� ����  
    for (auto& channel : m_ActiveChannels)
    {
        if (channel)
            channel->set3DMinMaxDistance(fMin, fMax);
    }
}

_bool CSound_Core::IsPlaying() const
{
    // �ϳ��� ��� ���̸� true ��ȯ
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

    // ��� Ȱ�� ä�ο� �Ͻ����� ����
    for (auto& channel : m_ActiveChannels)
    {
        if (channel)
            channel->setPaused(paused);
    }
}

void CSound_Core::RemoveInactiveChannels()
{
    // nullptr�̰ų� ����� ���� ä�ε��� ���Ϳ��� ����
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