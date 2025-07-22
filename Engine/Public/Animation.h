#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CAnimation final : public CBase
{
private:
	CAnimation();
	CAnimation(const CAnimation& Prototype);
	virtual ~CAnimation() = default;

public:
	HRESULT Initialize(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones);
	HRESULT Initialize(ifstream& ifs,  const vector<class CBone*>& Bones, _uint iRootBoneIdx);
	_bool Update_Bones(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop);
	void Reset();
	const _matrix& Get_TransformMatrix(_uint iBoneIndex) const {
		return m_TransformMatrices[iBoneIndex];
	}
	void Set_TickPerSecond(_float fTickPerSecond) {
		m_fTickPerSecond = fTickPerSecond;
	}
	const _float Get_CurrentTrackPosition() { return m_fCurrentTrackPosition; }
	const _float Get_Duration() { return m_fDuration; }

private:
	string					m_strName = {};
	/* 전체 재생 거리. */
	_float					m_fDuration = {};
	_float					m_fTickPerSecond = {};
	_float					m_fCurrentTrackPosition = {};
	vector<_uint>			m_CurrentKeyFrameIndices;

	/* 이 애니메이션을 표현하기위해서 사용하는 뼈의 갯수 */
	_uint					m_iNumChannels = {};
	vector<class CChannel*>	m_Channels;
	vector<_matrix>			m_TransformMatrices;

public:
	static CAnimation* Create(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones);
	static CAnimation* Create(ifstream& ifs, const vector<class CBone*>& Bones, _uint iRootBoneIdx);
	CAnimation* Clone();
	virtual void Free() override;
};

NS_END