#pragma once
#include "Base.h"

NS_BEGIN(FMOD)
class System;
class Sound;
class Channel;
class ChannelGroup;
NS_END

NS_BEGIN(Engine)

// FMOD_CORE_API 기반 래핑 클래스
class ENGINE_DLL CSound_Core : public CBase
{
private:
	friend class CSound_Control_Group;
	CSound_Core(FMOD::System* pCoreSystem, FMOD::Sound* pSound);
	CSound_Core(CSound_Core& Prototype);
	virtual ~CSound_Core() = default;

public:
	// 주의점 - 이 클래스는 마지막으로 재생된 채널만 저장하고 있습니다.
	// 즉 여러번 호출시에는 마지막 재생했던 사운드를 제외하고는 제어가 불가능합니다.
	void Play();
	void Stop();
	void Set_Volume(_float Volume);
	void Set_Pitch(_float Pitch);
	void Set_Loop(_int LoopCount);
	// 사운드 포지션 업데이트
	void Update3DPosition(_float3& vPos);
	//트랜스폼과 소리의 감쇠곡선을 결정할 죄소,최대 거리
	void Set3DState(_float fMin, _float fMax);
	_bool IsPlaying() const;
	void SetPaused(_bool paused);
	void StopAll();

private:
	void RemoveInactiveChannels();
private:
	FMOD::System* m_pCoreSystem = { nullptr };
	shared_ptr<FMOD::Sound> m_pSound = {nullptr};
	FMOD::Channel* m_pChannel = { nullptr };

	vector<FMOD::Channel*> m_ActiveChannels;
	_float m_fDefaultVolume = 1.0f;
	_float m_fDefaultPitch = 1.0f;
	_int m_iDefaultLoopCount = 0;

	_float m_fVolume{ 1.f };
	_float3		m_vSoundPos{};
	_float		m_fMinDis;
	_float		m_fMaxDis;

public:
	static CSound_Core* Create(FMOD::System* pCoreSystem, FMOD::Sound* pSound);
	CSound_Core* Clone();
	virtual void Free() override;
};

NS_END