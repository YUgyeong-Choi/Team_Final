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

	m_CurrentKeyFrameIndices.resize(m_iNumChannels);
	m_Bones = Bones; // 뼈대 정보 저장

	return S_OK;
}

_bool CAnimation::Update_Bones(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop, vector<string>* outEvents)
{
	constexpr _float fTargetTimeDelta = 1.f / 60.f;
	_float fTimeScale = fTimeDelta / fTargetTimeDelta;
	fTimeScale = min(fTimeScale, 3.f); // 너무 큰 값이 들어오는걸 방지
	m_isLoop = isLoop;
	_float prevPos = m_fCurrentTrackPosition;
	m_fPrevTrackPosition = prevPos;

	if (m_bReverse)
		m_fCurrentTrackPosition -= m_fTickPerSecond * fTargetTimeDelta * fTimeScale; // 역 재생
	else
		m_fCurrentTrackPosition += m_fTickPerSecond * fTargetTimeDelta * fTimeScale;
	_bool bIsReverse = (m_fPrevTrackPosition > m_fCurrentTrackPosition); // 이전이 더 크다면 지금 역 재생중

	if (m_bReverse)
	{
		bIsReverse = true;
		if (m_fCurrentTrackPosition <= 0.f)
		{
			m_fCurrentTrackPosition+= m_fDuration;
			if (!isLoop)
			{
				m_fCurrentTrackPosition = 0.f;
				return true; // 역 재생이 끝났음
			}
			ResetTrack();
		}
	}
	else
	{

	if (m_fCurrentTrackPosition >= m_fDuration)
	{
		m_fCurrentTrackPosition = 0.f;
		if (!isLoop)
		{
			m_fCurrentTrackPosition = m_fDuration;
			return true;
		}
	}

	}

	// 이벤트 처리 (prevPos < ev.time <= currentPos)
	if (outEvents)
	{
		for (auto& ev : m_events)
		{
			// prevPos == curPos (deltaTime=0) 인 경우 현재 프레임 이벤트도 잡아줌
			if (fabs(prevPos - m_fCurrentTrackPosition) < 0.0001f)
			{
				if (fabs(ev.fTime - m_fCurrentTrackPosition) < 0.0001f)
					outEvents->push_back(ev.name);
				continue; // 이미 처리했으니 다음 이벤트로
			}

			// 역재생인 경우
			if (m_bReverse)
			{
				if (ev.fTime > m_fCurrentTrackPosition && ev.fTime <= prevPos)
					outEvents->push_back(ev.name);
				continue;
			}

			// 정방향
			if (m_fCurrentTrackPosition >= prevPos) // 일반적인 경우
			{
				if (ev.fTime > prevPos && ev.fTime <= m_fCurrentTrackPosition)
					outEvents->push_back(ev.name);
			}
			else // 루프 발생 (curPos < prevPos)
			{
				if ((ev.fTime > prevPos && ev.fTime <= m_fDuration) ||
					(ev.fTime >= 0.f && ev.fTime <= m_fCurrentTrackPosition))
				{
					outEvents->push_back(ev.name);
				}
			}
		}
	}

	// 본 업데이트 
	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Update_TransformationMatrix(
			m_CurrentKeyFrameIndices[i],
			m_fCurrentTrackPosition,
			Bones, bIsReverse);
	}

	// 논루프 애니 종료 반환
	if (m_bReverse)
	{
		if (!isLoop && m_fCurrentTrackPosition <= 0.f)
		{
			return true; // 역 재생이 끝났음
		}
	}
	else
	{

	if (!isLoop && m_fCurrentTrackPosition >= m_fDuration)
		return true;
	}
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

void CAnimation::ResetTrack()
{
	if (m_bReverse)
	{
		m_fCurrentTrackPosition = m_fDuration; // 역 재생시 처음 위치는 Duration으로 설정
		for (_uint i = 0; i < m_iNumChannels; ++i)
		{
			_uint keyFrameCount = m_Channels[i]->GetNumKeyFrames();
			if (keyFrameCount > 1)
			{
				m_CurrentKeyFrameIndices[i] = keyFrameCount - 2; // 마지막에서 두 번째 키프레임
			}
			else
			{
				m_CurrentKeyFrameIndices[i] = 0;
			}
		}
	}
	else
	{

		m_CurrentKeyFrameIndices.assign(m_iNumChannels, 0u);
		m_fCurrentTrackPosition = 0.f; // 정방향 재생시 처음 위치는 0
	}

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
	sort(m_events.begin(), m_events.end(), [](const AnimationEvent& a, const AnimationEvent& b) {
		return a.fTime < b.fTime;
		});
	for (const auto& event : m_events)
	{
		j["Events"].push_back({
			{"EventName", event.name},
			{"Time", event.fTime}
			});
	}
	j["RootMotion"] = m_bUseRootMotion;
	if (m_bReverse)
		j["Reverse"] = m_bReverse;
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
		m_events.clear(); // 기존 이벤트 초기화
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
	if (j.contains("Reverse") && j["Reverse"].is_boolean())
	{
		m_bReverse = j["Reverse"].get<_bool>();
	}

	sort(m_events.begin(), m_events.end(), [](const AnimationEvent& a, const AnimationEvent& b) {
		return a.fTime < b.fTime;
		});
}
