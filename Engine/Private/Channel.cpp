#include "Channel.h"
#include "Animator.h"
#include "Bone.h"

CChannel::CChannel()
{
}

HRESULT CChannel::Initialize(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones)
{
	m_iNumKeyFrames = max(pAIChannel->mNumPositionKeys, pAIChannel->mNumScalingKeys);
	m_iNumKeyFrames = max(pAIChannel->mNumRotationKeys, m_iNumKeyFrames);

	_float3		vScale = {};
	_float4		vRotation = {};
	_float3		vTranslation = {};

	for (size_t i = 0; i < m_iNumKeyFrames; i++)
	{
		KEYFRAME		KeyFrame{};

		if(i < pAIChannel->mNumScalingKeys)
		{
			memcpy(&vScale, &pAIChannel->mScalingKeys[i].mValue, sizeof(_float3));
			KeyFrame.fTrackPosition = (float)pAIChannel->mScalingKeys[i].mTime;
		}

		if (i < pAIChannel->mNumRotationKeys)
		{
			vRotation.x = pAIChannel->mRotationKeys[i].mValue.x;
			vRotation.y = pAIChannel->mRotationKeys[i].mValue.y;
			vRotation.z = pAIChannel->mRotationKeys[i].mValue.z;
			vRotation.w = pAIChannel->mRotationKeys[i].mValue.w;			
			KeyFrame.fTrackPosition = (float)pAIChannel->mRotationKeys[i].mTime;
		}

		if (i < pAIChannel->mNumPositionKeys)
		{
			memcpy(&vTranslation, &pAIChannel->mPositionKeys[i].mValue, sizeof(_float3));
			KeyFrame.fTrackPosition = (float)pAIChannel->mPositionKeys[i].mTime;
		}

		KeyFrame.vScale = vScale;
		KeyFrame.vRotation = vRotation;
		KeyFrame.vTranslation = vTranslation;

		m_KeyFrames.push_back(KeyFrame);
	}

	auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool
		{
			if (true == pBone->Compare_Name(pAIChannel->mNodeName.data))
				return true;

			++m_iBoneIndex;

			return false;
		});


	return S_OK;
}

HRESULT CChannel::Initialize(ifstream& ifs, const vector<class CBone*>& Bones, _uint iRootBoneIdx)
{
	ifs.read(reinterpret_cast<_char*>(&m_iNumKeyFrames), sizeof(_uint));  // 키프레임 몇개읨 

	m_KeyFrames.resize(m_iNumKeyFrames);
	ifs.read(reinterpret_cast<_char*>(m_KeyFrames.data()), sizeof(KEYFRAME) * m_iNumKeyFrames);  // 총 키프레임 (데이터)
	ifs.read(reinterpret_cast<_char*>(&m_iBoneIndex), sizeof(_uint));		// 내가 몇번 뼈임
	m_iRootBoneIndex = iRootBoneIdx;
	return S_OK;
}


void CChannel::Update_TransformationMatrix(_uint& currentKeyFrameIndex, _float fCurrentTrackPosition, const vector<class CBone*>& Bones, _bool bIsReverse)
{
	if (0.0f == fCurrentTrackPosition)
		currentKeyFrameIndex = 0;

	_matrix			TransformationMatrix{};

	KEYFRAME		LastKeyFrame = m_KeyFrames.back();

	_vector			vScale, vRotation, vPosition;

	if (fCurrentTrackPosition >= LastKeyFrame.fTrackPosition)
	{
		vScale = XMLoadFloat3(&LastKeyFrame.vScale);
		vRotation = XMLoadFloat4(&LastKeyFrame.vRotation);
		vRotation = XMQuaternionNormalize(vRotation);
		vPosition = XMVectorSetW(XMLoadFloat3(&LastKeyFrame.vTranslation), 1.f);
	}
	else
	{
		//// 현재 트랙포지션이 현재 키프레임의 트랙포지션보다 크거나 같으면 다음 키프레임으로 이동
		//if (fCurrentTrackPosition >= m_KeyFrames[currentKeyFrameIndex + 1].fTrackPosition)
		//	++currentKeyFrameIndex;

		if (bIsReverse)
		{
			// 역방향
			while (currentKeyFrameIndex > 0 &&
				m_KeyFrames[currentKeyFrameIndex].fTrackPosition > fCurrentTrackPosition)
			{
				--currentKeyFrameIndex;
			}
		}
		else
		{
			// 정방향
			while (currentKeyFrameIndex < m_KeyFrames.size() - 2 &&
				fCurrentTrackPosition >= m_KeyFrames[currentKeyFrameIndex + 1].fTrackPosition)
			{
				++currentKeyFrameIndex;
			}
		}

		_float			fRatio = (fCurrentTrackPosition - m_KeyFrames[currentKeyFrameIndex].fTrackPosition) /
			(m_KeyFrames[currentKeyFrameIndex + 1].fTrackPosition - m_KeyFrames[currentKeyFrameIndex].fTrackPosition);

		_vector			vSourScale, vDestScale;
		_vector			vSourRotation, vDestRotation;
		_vector			vSourTranslation, vDestTranslation;

		vSourScale = XMLoadFloat3(&m_KeyFrames[currentKeyFrameIndex].vScale);
		vDestScale = XMLoadFloat3(&m_KeyFrames[currentKeyFrameIndex + 1].vScale);

		vSourRotation = XMLoadFloat4(&m_KeyFrames[currentKeyFrameIndex].vRotation);
		vDestRotation = XMLoadFloat4(&m_KeyFrames[currentKeyFrameIndex + 1].vRotation);

		vSourTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[currentKeyFrameIndex].vTranslation), 1.f);
		vDestTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[currentKeyFrameIndex + 1].vTranslation), 1.f);

		vScale = XMVectorLerp(vSourScale, vDestScale, fRatio);
		vRotation = XMQuaternionSlerp(vSourRotation, vDestRotation, fRatio);
		vRotation = XMQuaternionNormalize(vRotation);
		vPosition = XMVectorLerp(vSourTranslation, vDestTranslation, fRatio);
	}

	// TransformationMatrix = XMMatrixScaling() * XMMatrixRotationQuaternion() * XMMatrixTranslation();
	TransformationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vPosition);

	if (m_iBoneIndex >= Bones.size())
	{
		return;
	}

	// 나머지 뼈들은 기존 로컬 트랜스폼 그대로
	_matrix localMatrix =
		XMMatrixAffineTransformation(vScale,
			XMVectorZero(),
			vRotation,
			vPosition);

	Bones[m_iBoneIndex]->Set_TransformationMatrix(localMatrix);
	XMStoreFloat4x4(&m_LocalTransformationMatrix, localMatrix);
}

_bool CChannel::IsRootBone(const CBone* pBone) const
{
	return pBone->Is_RootBone();
}

CChannel* CChannel::Create(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones)
{
	CChannel* pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(pAIChannel, Bones)))
	{
		MSG_BOX("Failed to Created : CChannel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CChannel* CChannel::Create(ifstream& ifs, const vector<class CBone*>& Bones, _uint iRootBoneIdx)
{
	CChannel* pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(ifs, Bones, iRootBoneIdx)))
	{
		MSG_BOX("Failed to Created : CChannel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CChannel::Free()
{
	__super::Free();

	m_KeyFrames.clear();
}
