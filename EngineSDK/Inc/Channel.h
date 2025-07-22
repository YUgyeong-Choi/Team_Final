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
	HRESULT Initialize(ifstream& ifs, const vector<class CBone*>& Bones, _uint iRootBoneIdx);
	void Update_TransformationMatrix(_uint* pCurrentKeyFrameIndex, _float fCurrentTrackPosition, const vector<class CBone*>& Bones);
	_uint Get_BoneIndex() { return m_iBoneIndex; }
private:
	_char				m_szName[MAX_PATH] = {};
	_uint				m_iNumKeyFrames = {}; 
	vector<KEYFRAME>	m_KeyFrames;
	_uint				m_iBoneIndex = {};
	//_uint				m_iCurrentKeyFrameIndex = {};

	_uint				m_iRootBoneIndex = {};
	

public:
	static CChannel* Create(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	static CChannel* Create(ifstream& ifs, const vector<class CBone*>& Bones, _uint iRootBoneIdx);
	virtual void Free() override;

};

NS_END