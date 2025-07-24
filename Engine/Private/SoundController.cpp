#include "SoundController.h"
#include "Sound_Core.h"
#include "GameInstance.h"


CSoundController::CSoundController()
	:CComponent{ nullptr, nullptr }
{
}

CSoundController::CSoundController(const CSoundController& Prototype)
	:CComponent(Prototype)
{
	for (auto& pair : Prototype.m_Sounds)
		m_Sounds.emplace(pair.first, pair.second->Clone());
}

HRESULT CSoundController::Initialize_Prototype(const string& strFolderPath, _bool is3D, _bool loop, _bool stream)
{
	m_pGameInstance->LoadSound(strFolderPath, is3D, loop, stream, &m_Sounds);

	if (m_Sounds.empty())
		return E_FAIL;

	return S_OK;
}

HRESULT CSoundController::Initialize(void* pArg)
{
	return S_OK;
}

void CSoundController::Play(const string& strTag)
{
	auto pSoundCore = Find_Sound(strTag);
	if(pSoundCore)
		pSoundCore->Play();
}

void CSoundController::Stop(const string& strTag)
{
	auto pSoundCore = Find_Sound(strTag);
	if (pSoundCore)
		pSoundCore->Stop();
}

void CSoundController::Set_Loop(const string& strTag, _int _LoopCount)
{
	auto pSoundCore = Find_Sound(strTag);
	if (pSoundCore)
		pSoundCore->Set_Loop(_LoopCount);
}

void CSoundController::Set_Pitch(const string& strTag, _float Pitch)
{
	auto pSoundCore = Find_Sound(strTag);
	if (pSoundCore)
		pSoundCore->Set_Pitch(Pitch);
}

void CSoundController::SetVolume(const string& strTag, _float Volume)
{
	auto pSoundCore = Find_Sound(strTag);
	if (pSoundCore)
		pSoundCore->Set_Volume(Volume);
}

void CSoundController::Update3DPosition(const string& strTag, _float3& vPos)
{
	auto pSoundCore = Find_Sound(strTag);
	if (pSoundCore)
	{
		pSoundCore->Update3DPosition(vPos);
	}
}

void CSoundController::Set3DState(const string& strTag, _float fMin, _float fMax)
{
	auto pSoundCore = Find_Sound(strTag);
	if (pSoundCore)
		pSoundCore->Set3DState(fMin, fMax);
}

_bool CSoundController::IsPlaying(const string& strTag) const
{
	auto pSoundCore = Find_Sound(strTag);
	if (pSoundCore)
		return pSoundCore->IsPlaying();
	return FALSE;
}

void CSoundController::StopAll()
{
	for (auto& [key, value] : m_Sounds)
		value->Stop();
}

void CSoundController::SetVolume(_float Volume)
{
	for (auto& [key, value] : m_Sounds)
		value->Set_Volume(Volume);
}

void CSoundController::Update3DPosition(_float3& vPos)
{
	for (auto& [key, value] : m_Sounds)
	{
		value->Update3DPosition(vPos);
	}
}

void CSoundController::Set3DState(_float fMin, _float fMax)
{
	for (auto& [key,value] : m_Sounds)
		value->Set3DState(fMin, fMax);
}

CSound_Core* CSoundController::Find_Sound(const string& strTag) const
{
	auto Iter = m_Sounds.find(strTag);
	if (Iter == m_Sounds.end())
		return nullptr;

	return Iter->second;
}

CSoundController* CSoundController::Create(const string& strFolderPath, _bool is3D, _bool loop, _bool stream)
{
	CSoundController* pInstance = new CSoundController();

	if (FAILED(pInstance->Initialize_Prototype(strFolderPath, is3D, loop, stream)))
	{
		MSG_BOX("Failed to Created : CSoundController");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CSoundController::Clone(void* pArg)
{
	CSoundController* pInstance = new CSoundController(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CSoundController");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSoundController::Free()
{
	__super::Free();

	for (auto pair : m_Sounds)
		Safe_Release(pair.second);
	m_Sounds.clear();
}
