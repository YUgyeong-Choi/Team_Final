#include "Sound_Device.h"
#include "Sound_Core.h"
#include "Transform.h"
#include <filesystem>

namespace fs = std::filesystem;

CSound_Device::CSound_Device()
{
}

HRESULT CSound_Device::Initialize()
{
    if (FMOD::System_Create(&m_pCoreSystem) != FMOD_OK)
        return E_FAIL;

    // 기본 출력: 자동 감지
    m_pCoreSystem->setOutput(FMOD_OUTPUTTYPE_AUTODETECT);

    // 드라이버 설정
    int numDrivers = 0;
    m_pCoreSystem->getNumDrivers(&numDrivers);

    for (int i = 0; i < numDrivers; ++i)
    {
        char name[256] = {};
        FMOD_GUID guid = {};
        int systemRate = 0;
        FMOD_SPEAKERMODE speakerMode = FMOD_SPEAKERMODE_DEFAULT;
        int speakerModeChannels = 0;

        m_pCoreSystem->getDriverInfo(i, name, sizeof(name), &guid, &systemRate, &speakerMode, &speakerModeChannels);

        if (strstr(name, "ASIO"))
        {
            m_pCoreSystem->setOutput(FMOD_OUTPUTTYPE_ASIO);
            m_pCoreSystem->setDriver(i);
            m_pCoreSystem->setDSPBufferSize(64, 2);
            m_pCoreSystem->setSoftwareFormat(systemRate, speakerMode, 0);
            break;
        }
    }

    FMOD_OUTPUTTYPE outputType = FMOD_OUTPUTTYPE_AUTODETECT;
    m_pCoreSystem->getOutput(&outputType);

    if (outputType != FMOD_OUTPUTTYPE_ASIO)
    {
        m_pCoreSystem->setDSPBufferSize(256, 4);
        m_pCoreSystem->setSoftwareFormat(48000, FMOD_SPEAKERMODE_STEREO, 0);
    }

    FMOD_ADVANCEDSETTINGS adv = {};
    adv.cbSize = sizeof(FMOD_ADVANCEDSETTINGS);
    adv.DSPBufferPoolSize = 512;
    m_pCoreSystem->setAdvancedSettings(&adv);

    m_pCoreSystem->set3DSettings(1.0f, 1.0f, 1.0f);

    if (m_pCoreSystem->init(512, FMOD_INIT_NORMAL, nullptr) != FMOD_OK)
        return E_FAIL;

    m_pCoreSystem->setSoftwareChannels(128);

    Set_Master_Limiter();

    return S_OK;
}

void CSound_Device::Update()
{
    m_pCoreSystem->update();
}

void CSound_Device::Set_Master_Limiter()
{
    m_pCoreSystem->createDSPByType(FMOD_DSP_TYPE_LIMITER, &m_pMasterLimiter);

    m_pMasterLimiter->setParameterFloat(FMOD_DSP_LIMITER_RELEASETIME, 10.0f);      // 빠르게 회복
    m_pMasterLimiter->setParameterFloat(FMOD_DSP_LIMITER_CEILING, -1.0f);          // 0보다 살짝 낮게 출력 제한
    m_pMasterLimiter->setParameterFloat(FMOD_DSP_LIMITER_MAXIMIZERGAIN, 0);     // 볼륨 살짝 증폭
    m_pMasterLimiter->setParameterInt(FMOD_DSP_LIMITER_MODE, 0);                   // 디지털 클리핑 방지

    FMOD::ChannelGroup* pMasterGroup = nullptr;
    m_pCoreSystem->getMasterChannelGroup(&pMasterGroup);
    pMasterGroup->addDSP(0, m_pMasterLimiter);
    m_pMasterLimiter->setActive(true);
}

HRESULT CSound_Device::LoadSound(const string& Path, _bool is3D, _bool loop, _bool stream, unordered_map<string, class CSound_Core*>* _Out_ pOut)
{
    if (pOut == nullptr)
        pOut = &m_SingleSounds;

    if (!m_pCoreSystem)
        return E_FAIL;

    fs::path fsPath(Path);
    if (!fs::exists(fsPath))
        return E_FAIL;

    FMOD_MODE mode = FMOD_DEFAULT;

    if (is3D) mode |= FMOD_3D_LINEARROLLOFF;
    else      mode |= FMOD_2D;

    if (loop) mode |= FMOD_LOOP_NORMAL;
    else      mode |= FMOD_LOOP_OFF;

    if (stream) mode |= FMOD_CREATESTREAM;
    else        mode |= FMOD_CREATECOMPRESSEDSAMPLE;

    if (fs::is_directory(fsPath))
    {
        for (const auto& entry : fs::recursive_directory_iterator(fsPath))
        {
            if (!entry.is_regular_file()) continue;

            fs::path filePath = entry.path();
            string key = filePath.stem().string();

            FMOD::Sound* pSound = nullptr;
            FMOD_RESULT result{};
            if(filePath.extension().string() == ".xm")
                result = m_pCoreSystem->createSound(filePath.string().c_str(), FMOD_DEFAULT, nullptr, &pSound);
            else
                result = m_pCoreSystem->createSound(filePath.string().c_str(), mode, nullptr, &pSound);
            if (result == FMOD_OK)
            {
                auto pSoundCore = CSound_Core::Create(m_pCoreSystem, pSound);
                pOut->emplace(key, pSoundCore);
            }
        }
    }
    else if (fs::is_regular_file(fsPath))
    {
        string key = fsPath.stem().string();

        if (pOut->count(key) == 0)
        {
            FMOD::Sound* pSound = nullptr;
            FMOD_RESULT result{};
            if (fsPath.extension().string() == ".xm")
                result = m_pCoreSystem->createSound(fsPath.string().c_str(), FMOD_DEFAULT, nullptr, &pSound);
            else
                result = m_pCoreSystem->createSound(fsPath.string().c_str(), mode, nullptr, &pSound);
            if (result != FMOD_OK)
                return E_FAIL;

            auto pSoundCore = CSound_Core::Create(m_pCoreSystem, pSound);
            pOut->emplace(key, pSoundCore);
        }
    }

    return S_OK;
}

void CSound_Device::Set_Listener_Position(CTransform* pTransform, const _float3& vel)
{
    _float3 vPos ,vLook , vUp = {};
    XMStoreFloat3(&vPos, pTransform->Get_State(STATE::POSITION));
    XMStoreFloat3(&vLook, pTransform->Get_State(STATE::LOOK));
    XMStoreFloat3(&vUp, pTransform->Get_State(STATE::UP));

    memcpy(&m_ListenerAttr.position, &vPos, sizeof _float3);
    memcpy(&m_ListenerAttr.forward, &vLook, sizeof _float3);
    memcpy(&m_ListenerAttr.up, &vUp, sizeof _float3);
    m_ListenerAttr.velocity = { vel.x, vel.y, vel.z };

    m_pCoreSystem->set3DListenerAttributes(0, &m_ListenerAttr.position, &m_ListenerAttr.velocity,
        &m_ListenerAttr.forward, &m_ListenerAttr.up);
}

CSound_Core* CSound_Device::Get_Single_Sound(const string& strKey)
{
    auto Iter = m_SingleSounds.find(strKey);
    if (Iter == m_SingleSounds.end())
        return nullptr;

    Safe_AddRef(Iter->second);

    return Iter->second;
}

void CSound_Device::Set_Master_Volume(_float volume)
{
    FMOD::ChannelGroup* masterGroup = nullptr;
    m_pCoreSystem->getMasterChannelGroup(&masterGroup);
    if (masterGroup)
        masterGroup->setVolume(volume);
}

CSound_Device* CSound_Device::Create()
{
    CSound_Device* pInstance = new CSound_Device();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create: CSound_Device");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSound_Device::Free()
{
    __super::Free();

    for (auto& [Key, Value] : m_SingleSounds)
        Safe_Release(Value);
    m_SingleSounds.clear();

    if (m_pMasterLimiter)
    {
        m_pMasterLimiter->release();
        m_pMasterLimiter = nullptr;
    }

    if (m_pCoreSystem)
    {
        m_pCoreSystem->release();
        m_pCoreSystem = nullptr;
    }
}
