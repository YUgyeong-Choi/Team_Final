#pragma once

#include "Base.h"

/* 특정 애니메이션을 구동하기위해서 필요한, 뼈의 시간당 상태정보*/

NS_BEGIN(Engine)

class CChannel final : public CBase
{
private:
	CChannel();
	virtual ~CChannel() = default;

public:
	HRESULT Initialize(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	HRESULT Initialize(ifstream& ifs, const vector<class CBone*>& Bones, _uint iRootBoneIdx);
	void Update_TransformationMatrix(_uint& currentKeyFrameIndex, _float fCurrentTrackPosition, const vector<class CBone*>& Bones,_bool bIsReverse = false,class CAnimator* pAnimator = nullptr);
	_uint Get_BoneIndex() { return m_iBoneIndex; }

	const _float4x4& GetLocalMatrix() const
	{
		return m_LocalTransformationMatrix;
	}
private:
	_bool IsRootBone(const CBone* pBone) const;
private:
	_char				m_szName[MAX_PATH] = {};
	_uint				m_iNumKeyFrames = {}; 
	vector<KEYFRAME>	m_KeyFrames;
	_uint				m_iBoneIndex = {};
	//_uint				m_iCurrentKeyFrameIndex = {};

	_uint				m_iRootBoneIndex = {};
	
	_float4x4 		    m_LocalTransformationMatrix = {};
public:
	static CChannel* Create(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	static CChannel* Create(ifstream& ifs, const vector<class CBone*>& Bones, _uint iRootBoneIdx);
	virtual void Free() override;

};

NS_END