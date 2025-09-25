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

	/* �ִϸ��̼��� �̸��� ��ȯ. */
	const string& Get_Name() { return m_AnimationName; }
	/* �ִϸ��̼��� �̸��� ����. */
	void Set_Name(const string& name) { m_AnimationName = name; }

	void SetLoop(_bool isLoop) { m_isLoop = isLoop; }
	_bool Get_isLoop() { return m_isLoop; }
	_matrix GetBoneMatrix(_uint iIndex); // �� �ִϸ��̼� ä���� ���� �� ��Ʈ���� ��ȯ�ϱ�

	void SetDuration(_float fDuration) { m_fDuration = fDuration; }
	void SetTickPerSecond(_float fTickPerSecond) { m_fTickPerSecond = fTickPerSecond; }
	void SetCurrentTrackPosition(_float fCurrentTrackPosition) {
		ResetTrack(); // ������ ���� ���ϸ� �ش� �������� ����
		// ���� ��ġ ����
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
			return; // ��ȿ���� ���� �ð� ������ �̺�Ʈ�� �߰����� ����
		auto it = find_if(m_events.begin(), m_events.end(), [&,this](const AnimationEvent& event)
			{
				if (event.name == vEvent.name)
				{
					if (abs(event.fTime- vEvent.fTime) <= 1e-4f)
					{
						return true; // �ߺ� �̺�Ʈ
					}
				}
				return false;
			});
		if (it != m_events.end())
			return; // �ߺ� �̺�Ʈ�� �߰����� ����
		m_events.push_back(vEvent); }
	vector<AnimationEvent>& GetEvents() { return m_events; }

	const _bool IsReverse() const { return m_bReverse; }
	void SetReverse(_bool bReverse) { m_bReverse = bReverse; }

public:
	void ResetTrack();
private:
	/* ��ü ��� �Ÿ�. */
	_float					m_fDuration = {};
	_float					m_fTickPerSecond = {};
	_float					m_fCurrentTrackPosition = {};
	_float                  m_fPrevTrackPosition = 0.f;
	vector<_uint>			m_CurrentKeyFrameIndices;
	// �ִϸ��̼� ���� ����
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

	// ISerializable��(��) ���� ��ӵ�
	json Serialize() override;
	void Deserialize(const json& j) override;
};

NS_END