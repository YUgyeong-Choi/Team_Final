#pragma once

#include "Base.h"
#include  "Serializable.h"
#include <iostream>
NS_BEGIN(Engine)

class ENGINE_DLL CAnimation final : public CBase, public ISerializable
{
private:
	CAnimation();
	CAnimation(const CAnimation& Prototype);
	virtual ~CAnimation() = default;

public:
	HRESULT Initialize(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones);
	HRESULT InitializeByBinary(ifstream& ifs, const vector<class CBone*>& Bones);

	_bool Update_Bones(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop, vector<string>* outEvents = nullptr);

	/* 애니메이션의 이름을 반환. */
	const string& Get_Name() { return m_AnimationName; }
	/* 애니메이션의 이름을 설정. */
	void Set_Name(const string& name) { m_AnimationName = name; }

	void SetLoop(_bool isLoop) { m_isLoop = isLoop; }
	_bool Get_isLoop() { return m_isLoop; }
	_matrix GetBoneMatrix(_uint iIndex); // 각 애니메이션 채널의 로컬 본 매트릭스 반환하기

	void SetDuration(_float fDuration) { m_fDuration = fDuration; }
	void SetTickPerSecond(_float fTickPerSecond) { m_fTickPerSecond = fTickPerSecond; }
	void SetCurrentTrackPosition(_float fCurrentTrackPosition) {
		ResetTrack(); // 강제로 리셋 안하면 해당 뼈정보가 꼬임
		// 이전 위치 저장
		m_fPrevTrackPosition = m_fCurrentTrackPosition;
		m_fCurrentTrackPosition = fCurrentTrackPosition; }
	void SetCurrentKeyFrameIndices(_uint iIndex, _uint iKeyFrameIndex) {
		if (iIndex >= m_CurrentKeyFrameIndices.size())
			return;
		m_CurrentKeyFrameIndices[iIndex] = iKeyFrameIndex;
	}

	void Set_Bones(const vector<class CBone*>& Bones) {
		if(m_Bones.empty())
			m_Bones = Bones; }	

	_uint GetCurrentKeyFrameIndices(_uint iIndex) const { return m_CurrentKeyFrameIndices[iIndex]; }
	_float GetDuration() const { return m_fDuration; }
	_float GetTickPerSecond() const { return m_fTickPerSecond; }
	_float GetCurrentTrackPosition() const { return m_fCurrentTrackPosition; }
	_float GetClipLength() const
	{
		return m_fTickPerSecond > 0.f
			? (m_fDuration / m_fTickPerSecond)
			: 0.f;
	}
	_bool IsRootMotionEnabled() const { return m_bUseRootMotion; }
	void SetUseRootMotion(_bool bUseRootMotion) { m_bUseRootMotion = bUseRootMotion; }

	void AddEvent(AnimationEvent vEvent) {
		if (vEvent.fTime < 0.f || vEvent.fTime > m_fDuration)
			return; // 유효하지 않은 시간 범위의 이벤트는 추가하지 않음
		auto it = find_if(m_events.begin(), m_events.end(), [&,this](const AnimationEvent& event)
			{
				if (event.name == vEvent.name)
				{
					if (abs(event.fTime- vEvent.fTime) <= 1e-4f)
					{
						return true; // 중복 이벤트
					}
				}
				return false;
			});
		if (it != m_events.end())
			return; // 중복 이벤트는 추가하지 않음
		m_events.push_back(vEvent); }
	vector<AnimationEvent>& GetEvents() { return m_events; }

	const _bool IsReverse() const { return m_bReverse; }
	void SetReverse(_bool bReverse) { m_bReverse = bReverse; }

public:
	void ResetTrack();
private:
	/* 전체 재생 거리. */
	_float					m_fDuration = {};
	_float					m_fTickPerSecond = {};
	_float					m_fCurrentTrackPosition = {};
	_float                  m_fPrevTrackPosition = 0.f;
	vector<_uint>			m_CurrentKeyFrameIndices;
	// 애니메이션 뼈의 개수
	_uint					m_iNumChannels;
	vector<class CChannel*>	m_Channels;
	_char m_Name[MAX_PATH] = {};
	_bool		m_isLoop = false;
	_bool    m_bUseRootMotion = false;
	_bool    m_bReverse = false;
	vector<class CBone*> m_Bones;
	vector<AnimationEvent> m_events;
	vector<_matrix>			m_TransformMatrices;

	string m_AnimationName = {};
public:
	static CAnimation* Create(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones);
	static CAnimation* CreateByBinary(ifstream& ifs, const vector<class CBone*>& Bones);
	CAnimation* Clone(const vector<class CBone*>& Bones);
	virtual void Free() override;

	// ISerializable을(를) 통해 상속됨
	json Serialize() override;
	void Deserialize(const json& j) override;
};

NS_END