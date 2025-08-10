#include "Animation.h"
#include "Bone.h"
#include "Channel.h"

CAnimation::CAnimation()
	: m_fDuration{ 0.f }
	, m_fTickPerSecond{ 0.f }
	, m_fCurrentTrackPosition{ 0.f }
	, m_iNumChannels{ 0 }
	, m_CurrentKeyFrameIndices{}
	, m_Channels{}
{
}

CAnimation::CAnimation(const CAnimation& Prototype)
	: m_fDuration{ Prototype.m_fDuration }
	, m_fTickPerSecond{ Prototype.m_fTickPerSecond }
	, m_fCurrentTrackPosition{ Prototype.m_fCurrentTrackPosition }
	, m_CurrentKeyFrameIndices{ Prototype.m_CurrentKeyFrameIndices }
	, m_iNumChannels{ Prototype.m_iNumChannels }
	, m_Channels{ Prototype.m_Channels }
	, m_AnimationName{ Prototype.m_AnimationName }
	, m_events{ Prototype.m_events }

{
	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);
	strcpy_s(m_Name, Prototype.m_Name);
}



HRESULT CAnimation::Initialize(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones)
{
	/* 애니메이션 이름을 복사한다. */

	strcpy_s(m_Name, pAIAnimation->mName.data);

	/* 이 애니메이션이 컨트롤해야하는 뼈의 갯수 */
	m_iNumChannels = pAIAnimation->mNumChannels;

	m_fTickPerSecond = static_cast<_float>(pAIAnimation->mTicksPerSecond);
	m_fDuration = static_cast<_float>(pAIAnimation->mDuration);


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

HRESULT CAnimation::InitializeByBinary(ifstream& ifs, const vector<class CBone*>& Bones)
{
	_uint nameLength = 0;
	ifs.read(reinterpret_cast<char*>(&nameLength), sizeof(_uint));  // 길이 먼저 읽음
	string AnimationName(nameLength, '\0');  
	ifs.read(&AnimationName[0], nameLength);      
	m_AnimationName = AnimationName;  // 애니메이션 이름을 복사

	ifs.read(reinterpret_cast<_char*>(&m_fTickPerSecond), sizeof(_float));  
	ifs.read(reinterpret_cast<_char*>(&m_fDuration), sizeof(_float));  
	ifs.read(reinterpret_cast<_char*>(&m_iNumChannels), sizeof(_uint)); 

	_int iRootBoneIdx = -1; // 루트 본 인덱스 초기화

	for (size_t i = 0; i < Bones.size(); ++i)
	{
		if (Bones[i]->Is_RootBone())
		{
			iRootBoneIdx = static_cast<_int>(i);
			break;
		}
	}
	
	m_Channels.reserve(m_iNumChannels);
	for (size_t i = 0; i < m_iNumChannels; i++)
	{
		CChannel* pChannel = CChannel::Create(ifs, Bones, iRootBoneIdx);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.push_back(pChannel);
	}
	//m_TransformMatrices.resize(Bones.size());
	//for (auto& mat : m_TransformMatrices)
	//	mat = XMMatrixIdentity();
	//for (size_t i = 0; i < Bones.size(); ++i)
	//{
	//	m_TransformMatrices[i] =
	//		XMLoadFloat4x4(Bones[i]->Get_TransformationMatrix());
	//}
	m_CurrentKeyFrameIndices.resize(m_iNumChannels);
	m_Bones = Bones; // 뼈대 정보 저장
	m_fTickPerSecond = 55.f;
	return S_OK;
}

//_bool CAnimation::Update_Bones(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop)
//{
//	m_isLoop = isLoop;
//	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;
//
//	if (m_fCurrentTrackPosition >= m_fDuration)
//	{
//		m_fCurrentTrackPosition = 0.f;
//		if (false == isLoop)
//		{
//			m_fCurrentTrackPosition = m_fDuration;
//			return true;
//		}
//	}
//
//	// 채널이 각 뼈들의 정보 (예: 오른쪽 팔, 손목, 손가락등)
//	for (_uint i = 0; i < m_iNumChannels; ++i)
//	{
//		m_Channels[i]->Update_TransformationMatrix(m_CurrentKeyFrameIndices[i], m_fCurrentTrackPosition, Bones);
//	}
//	return false;
//}

_bool CAnimation::Update_Bones(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop, vector<string>* outEvents,vector<_float4x4>* outLocalMatrices)
{
	m_isLoop = isLoop;
	_float prevPos = m_fCurrentTrackPosition;
	m_fPrevTrackPosition = prevPos;

	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;
	_bool bIsReverse = (m_fPrevTrackPosition > m_fCurrentTrackPosition); // 이전이 더 크다면 지금 역 재생중


	if (m_fCurrentTrackPosition >= m_fDuration)
	{
		m_fCurrentTrackPosition = 0.f;
		if (false == isLoop)
		{
			m_fCurrentTrackPosition = m_fDuration;
			return true;
		}
	}


	// 이벤트 처리 (prevPos < ev.time <= currentPos)
	if (outEvents)
	{
		for (auto& ev : m_events)
		{
			if (ev.fTime > prevPos && ev.fTime <= m_fCurrentTrackPosition)
			{
				outEvents->push_back(ev.name);
			}
		}
	}

	// 본 업데이트 
	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Update_TransformationMatrix(
			m_CurrentKeyFrameIndices[i],
			m_fCurrentTrackPosition,
			Bones, bIsReverse, outLocalMatrices
		);
	}

	// 논루프 애니 종료 반환
	if (!isLoop && m_fCurrentTrackPosition >= m_fDuration)
		return true;
	return false;
}

_matrix CAnimation::GetBoneMatrix(_uint iIndex)
{
	for (auto& pChannel : m_Channels)
	{
		if (pChannel->Get_BoneIndex() == iIndex)
		{
			return XMLoadFloat4x4(&pChannel->GetLocalMatrix());
		}
	}
	//채널이 없으면 해당 뼈의 로컬 바인드 포즈를 반환
	return XMLoadFloat4x4(&m_Bones[iIndex]->Get_LocalBindPose());
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

CAnimation* CAnimation::CreateByBinary(ifstream& ifs, const vector<class CBone*>& Bones)
{
	CAnimation* pInstance = new CAnimation();
	if (FAILED(pInstance->InitializeByBinary(ifs, Bones)))
	{
		MSG_BOX("Failed to Created : CAnimation");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CAnimation* CAnimation::Clone(const vector<class CBone*>& Bones)
{
	CAnimation* pInstance = new CAnimation(*this);
	pInstance->m_Bones = Bones;
	return pInstance;
}

void CAnimation::Free()
{
	__super::Free();

	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);

	m_Channels.clear();
} 

json CAnimation::Serialize()
{
	json j;
	// 이벤트 직렬화
	j["ClipName"] = m_AnimationName;
	j["Duration"] = m_fDuration;
	j["TickPerSecond"] = m_fTickPerSecond;
	j["IsLoop"] = m_isLoop;
	for (const auto& event : m_events)
	{
		j["Events"].push_back({
			{"EventName", event.name},
			{"Time", event.fTime}
			});
	}
	j["RootMotion"] = m_bUseRootMotion;
	return j;
}

void CAnimation::Deserialize(const json& j)
{
	if (j.contains("ClipName") && j["ClipName"].is_string())
	{
		m_AnimationName = j["ClipName"].get<string>();
	}

	if (j.contains("TickPerSecond") && j["TickPerSecond"].is_number())
	{
		m_fTickPerSecond = j["TickPerSecond"].get<_float>();
	}

	if (j.contains("IsLoop") && j["IsLoop"].is_boolean())
	{
		m_isLoop = j["IsLoop"].get<_bool>();
	}

	if (j.contains("Events") && j["Events"].is_array())
	{
		for (const auto& event : j["Events"])
		{
			if (event.contains("Time") && event.contains("EventName"))
			{
				m_events.push_back({
					event["Time"].get<_float>(),
					event["EventName"].get<string>()
					});
			}
		}
	}
	if (j.contains("RootMotion") && j["RootMotion"].is_boolean())
	{
		m_bUseRootMotion = j["RootMotion"].get<_bool>();
	}

	m_fTickPerSecond = max(m_fTickPerSecond, 55.f); // 최소값 설정
}
