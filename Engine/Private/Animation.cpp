#include "Animation.h"
#include "Channel.h"
#include "Bone.h"

CAnimation::CAnimation()
{

}

CAnimation::CAnimation(const CAnimation& Prototype)
	: m_fDuration { Prototype.m_fDuration }
	, m_fTickPerSecond { Prototype.m_fTickPerSecond }
	, m_fCurrentTrackPosition { Prototype.m_fCurrentTrackPosition }
	, m_CurrentKeyFrameIndices { Prototype.m_CurrentKeyFrameIndices }
	, m_iNumChannels { Prototype.m_iNumChannels }
	, m_Channels { Prototype.m_Channels }
	, m_TransformMatrices{ Prototype.m_TransformMatrices }
{
	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);
}


HRESULT CAnimation::Initialize(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones)
{
	/* 이 애니메이션이 컨트롤해야하는 뼈의 갯수 */
	m_iNumChannels = pAIAnimation->mNumChannels;
	m_fTickPerSecond = (_float)pAIAnimation->mTicksPerSecond;
	m_fDuration = (_float)pAIAnimation->mDuration;
	string AnimationName = pAIAnimation->mName.data;
	m_CurrentKeyFrameIndices.resize(m_iNumChannels);

	/* 각 뼈의 정보를 새엇ㅇ한다. */
	for (size_t i = 0; i < m_iNumChannels; i++)
	{		
		CChannel* pChannel = CChannel::Create(pAIAnimation->mChannels[i], Bones);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.push_back(pChannel);
	}

    return S_OK;
}

HRESULT CAnimation::Initialize(ifstream& ifs, const vector<class CBone*>& Bones, _uint iRootBoneIdx)
{
	ifs.read(reinterpret_cast<_char*>(&m_fTickPerSecond), sizeof(_float));  // 애니메이션 안에 채널 몇개읨 
	ifs.read(reinterpret_cast<_char*>(&m_fDuration), sizeof(_float));  // 애니메이션 안에 채널 몇개읨 
	/* 이 애니메이션이 컨트롤해야하는 뼈의 갯수 */
	ifs.read(reinterpret_cast<_char*>(&m_iNumChannels), sizeof(_uint));  // 애니메이션 안에 채널 몇개읨 

	m_CurrentKeyFrameIndices.resize(m_iNumChannels);

	/* 각 뼈의 정보를 생성한다. */
	for (size_t i = 0; i < m_iNumChannels; i++)
	{
		CChannel* pChannel = CChannel::Create(ifs, Bones, iRootBoneIdx);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.push_back(pChannel);
	}
 	m_TransformMatrices.resize(Bones.size());
	for (auto& mat : m_TransformMatrices)
		mat = XMMatrixIdentity();
	for (size_t i = 0; i < Bones.size(); ++i)
	{
		m_TransformMatrices[i] =
			XMLoadFloat4x4(Bones[i]->Get_TransformationMatrix());
	}

	return S_OK;
}

_bool CAnimation::Update_Bones(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop)
{
	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;

	if (m_fCurrentTrackPosition >= m_fDuration)
	{
		if (false == isLoop)
		{
			Reset();
			return true;
		}		
		
		m_fCurrentTrackPosition = 0.f;			
	}

	/*for (auto& pChannel : m_Channels)*/
	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Update_TransformationMatrix(&m_CurrentKeyFrameIndices[i], m_fCurrentTrackPosition, Bones);

		_uint idx = m_Channels[i]->Get_BoneIndex();

		m_TransformMatrices[idx] =
			XMLoadFloat4x4(Bones[idx]->Get_TransformationMatrix());
	}


	return false;

}

void CAnimation::Reset()
{
	// 1) 시간 위치를 0으로
	m_fCurrentTrackPosition = 0.f;

	// 2) 각 채널의 키프레임 인덱스도 모두 0으로
	fill(m_CurrentKeyFrameIndices.begin(),
		m_CurrentKeyFrameIndices.end(),
		0);
}

CAnimation* CAnimation::Create(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones)
{
	CAnimation* pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize(pAIAnimation, Bones)))
	{
		MSG_BOX("Failed to Created : CAnimation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimation* CAnimation::Create(ifstream& ifs, const vector<class CBone*>& Bones, _uint iRootBoneIdx)
{
	CAnimation* pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize(ifs, Bones, iRootBoneIdx)))
	{
		MSG_BOX("Failed to Created : CAnimation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimation* CAnimation::Clone()
{
	return new CAnimation(*this);
}

void CAnimation::Free()
{
	__super::Free();

		for (auto& pChannel : m_Channels)
			Safe_Release(pChannel);

	m_Channels.clear();


}
