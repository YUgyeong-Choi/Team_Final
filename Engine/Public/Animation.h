#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CAnimation final : public CBase
{
private:
	CAnimation();
	CAnimation(const CAnimation& Prototype);
	virtual ~CAnimation() = default;

public:
	HRESULT Initialize(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones);
	HRESULT InitializeByBinary(ifstream& ifs, const vector<class CBone*>& Bones);

	//	_bool Update_Bones(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop);
	_bool Update_Bones(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop, vector<string>* outEvents = nullptr);

	void ExportBinary(ofstream& ofs);

	/* 애니메이션의 이름을 반환. */
	const string& Get_Name() { return m_AnimationName; }

	void SetLoop(_bool isLoop) { m_isLoop = isLoop; }
	_bool Get_isLoop() { return m_isLoop; }
	_matrix GetBoneMatrix(_uint iIndex); // 각 애니메이션 채널의 로컬 본 매트릭스 반환하기

	void SetDuration(_float fDuration) { m_fDuration = fDuration; }
	void SetTickPerSecond(_float fTickPerSecond) { m_fTickPerSecond = fTickPerSecond; }
	void SetCurrentTrackPosition(_float fCurrentTrackPosition) { m_fCurrentTrackPosition = fCurrentTrackPosition; }
	void SetCurrentKeyFrameIndices(_uint iIndex, _uint iKeyFrameIndex) {
		if (iIndex >= m_CurrentKeyFrameIndices.size())
			return;
		m_CurrentKeyFrameIndices[iIndex] = iKeyFrameIndex;
	}

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

	void AddEvent(AnimationEvent vEvent) { m_events.push_back(vEvent); }
	vector<AnimationEvent>& GetEvents() { return m_events; }

public:
	void ResetTrack()
	{
		m_fCurrentTrackPosition = 0.f;
		m_CurrentKeyFrameIndices.assign(m_iNumChannels, 0u);
	}
private:
	/* 전체 재생 거리. */
	_float					m_fDuration = {};
	_float					m_fTickPerSecond = {};
	_float					m_fCurrentTrackPosition = {};
	vector<_uint>			m_CurrentKeyFrameIndices;
	// 애니메이션 뼈의 개수
	_uint					m_iNumChannels;
	vector<class CChannel*>	m_Channels;
	char m_Name[MAX_PATH] = {};
	_bool m_isLoop = false;
	vector<class CBone*> m_Bones;
	vector<AnimationEvent> m_events;
	vector<_matrix>			m_TransformMatrices;

	string m_AnimationName = {};
public:
	static CAnimation* Create(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones);
	static CAnimation* CreateByBinary(ifstream& ifs, const vector<class CBone*>& Bones);
	CAnimation* Clone(const vector<class CBone*>& Bones);
	virtual void Free() override;
};

NS_END