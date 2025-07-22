#pragma once

#include "Base.h"

/* Ư�� �ִϸ��̼��� �����ϱ����ؼ� �ʿ���, ���� �ð��� ��������*/

NS_BEGIN(Engine)

class CChannel final : public CBase
{
private:
	CChannel();
	virtual ~CChannel() = default;

public:
	HRESULT Initialize(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	void Update_TransformationMatrix(_uint* pCurrentKeyFrameIndex, _float fCurrentTrackPosition, const vector<class CBone*>& Bones);

private:
	_char				m_szName[MAX_PATH] = {};
	_uint				m_iNumKeyFrames;
	vector<KEYFRAME>	m_KeyFrames;
	_uint				m_iBoneIndex = {};
	//_uint				m_iCurrentKeyFrameIndex = {};
	

public:
	static CChannel* Create(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	virtual void Free() override;

};

NS_END