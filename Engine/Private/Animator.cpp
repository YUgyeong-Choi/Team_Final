#include "Animator.h"
#include "AnimController.h"
#include "Model.h"
#include "GameInstance.h"
#include "Bone.h"
using ET = CAnimController::ETransitionType;
CAnimator::CAnimator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
	, m_pModel{ nullptr }
	, m_pCurrentAnim{ nullptr }
	, m_Blend{}
{
}

CAnimator::CAnimator(const CAnimator& Prototype)
	: CComponent(Prototype)
	, m_pModel{ Prototype.m_pModel }
	, m_pCurrentAnim{ Prototype.m_pCurrentAnim }
	, m_Blend{ Prototype.m_Blend }
	, m_Bones{ Prototype.m_Bones }
	, m_pCurAnimController{ Prototype.m_pCurAnimController }
	, m_iCurrentAnimIndex{ Prototype.m_iCurrentAnimIndex }
	, m_iPrevAnimIndex{ Prototype.m_iPrevAnimIndex }
	, m_AnimControllers{ Prototype.m_AnimControllers }
{

}

HRESULT CAnimator::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAnimator::Initialize(void* pArg)
{
	if (pArg == nullptr)
		return E_FAIL;

	m_pModel = static_cast<CModel*>(pArg);

	m_Bones = m_pModel->Get_Bones();

	m_pCurAnimController = CAnimController::Create();
	if (m_pCurAnimController == nullptr)
		return E_FAIL;
	// ����Ʈ ��Ʈ�ѷ� �����ؼ� ����
	m_pCurAnimController->SetAnimator(this);
//	m_pCurAnimController->Initialize_Prototype();
	m_pCurAnimController->SetName(m_pModel->Get_ModelName() + "_Default");
	// ��Ʈ�ѷ� �̸�

	m_AnimControllers[m_pModel->Get_ModelName() + "_Default"] = m_pCurAnimController;
	return S_OK;
}

HRESULT CAnimator::Initialize_Test(void* pArg)
{
	if (pArg == nullptr)
		return E_FAIL;

	ANIMATOR_DESC* pDesc = static_cast<ANIMATOR_DESC*>(pArg);
	m_pModel = pDesc->pModel;
	m_Bones = m_pModel->Get_Bones();

	m_pCurAnimController = CAnimController::Create();
	if (m_pCurAnimController == nullptr)
		return E_FAIL;
	// ����Ʈ ��Ʈ�ѷ� �����ؼ� ����
	m_pCurAnimController->SetAnimator(this);
	m_AnimControllers["Default"] = m_pCurAnimController;
	m_pCurAnimController->SetName(m_pModel->Get_ModelName() + "_Default");
	_wstring csoFilepath = TEXT("../Bin/ShaderFiles/Shader_AnimCS.cso");

	m_pAnimComputeShader = CAnimComputeShader::Create(m_pDevice, m_pContext, csoFilepath,static_cast<_uint>(m_Bones.size()));
	
	if (m_pAnimComputeShader == nullptr)
	{
		MSG_BOX("Failed to create CAnimComputeShader");
		return E_FAIL;
	}

	return S_OK;
}

void CAnimator::Update(_float fDeltaTime)
{
	if (m_bPlaying == false)
		return;
	_float fTimeScale = fDeltaTime * m_fPlaybackSpeed;
	RefreshAndProcessTransition(fDeltaTime);

	vector<string> triggeredEvents; // �̺�Ʈ�� �� ���� ����
	size_t iBoneCount = m_Bones.size();

	if (m_Blend.active) //  ���� ��
	{
		UpdateBlend(fTimeScale, iBoneCount, triggeredEvents);
	}
	else // ���尡 �ƴ� ��
	{
		UpdateAnimation(fTimeScale, iBoneCount, triggeredEvents);
	}

	DispatchAnimEvents(triggeredEvents);
}

void CAnimator::PlayClip(CAnimation* pAnim, _bool isLoop)
{
	if (pAnim == nullptr)
		return;
	m_pCurrentAnim = pAnim;
	m_pCurrentAnim->ResetTrack();
	m_Blend.active = false;
	m_bPlaying = true;
	UpdateMaskState();
}

void CAnimator::StartTransition(const CAnimController::TransitionResult& transitionResult)
{
	ResetRootMotion();

	m_Blend.active = true;
	m_Blend.elapsed = 0.f;
	m_Blend.duration = transitionResult.fDuration;

	// ������ ���� Ŭ��
	m_Blend.fromLowerAnim = transitionResult.pFromLowerAnim;
	m_Blend.toLowerAnim = transitionResult.pToLowerAnim;
	m_Blend.fromUpperAnim = transitionResult.pFromUpperAnim;
	m_Blend.toUpperAnim = transitionResult.pToUpperAnim;
	m_eCurrentTransitionType = transitionResult.eType; // ���� ��ȯ Ÿ�� ����
	m_Blend.blendWeight = transitionResult.fBlendWeight; // ����ü ���� ����ġ 
	m_Blend.canSameAnimReset = transitionResult.bCanSameAnimReset; // ���� �ִϸ��̼��� �� �ʱ�ȭ �������� ����


	_bool bFromLowerAnimSame = (transitionResult.pFromLowerAnim == transitionResult.pToLowerAnim);
	_bool bToUpperLowerSame = (transitionResult.pFromUpperAnim == transitionResult.pToUpperAnim);
	_bool bFromAnimSameToUpper = (transitionResult.pFromLowerAnim == transitionResult.pToUpperAnim);
	_bool bFromAnimSameToLower = (transitionResult.pFromUpperAnim == transitionResult.pToLowerAnim);
	
	if (bFromLowerAnimSame == false && bFromAnimSameToLower == false ) // ���� ��ü���� �ٸ��� ���� ��ü���� �ٸ� ��ü���
	{
		_float fLowerStartTime = transitionResult.fLowerStartTime * m_Blend.toLowerAnim->GetDuration();
		if (m_Blend.toLowerAnim->IsReverse())
		{
			// ������� ���
			_float fDuration = m_Blend.toLowerAnim->GetDuration();
			fLowerStartTime = fDuration - fLowerStartTime; // ������̹Ƿ� ���� �ð��� Duration���� ����
			fLowerStartTime = max(0.f, fLowerStartTime); // 0���� ������ 0���� ����
		}
		m_Blend.toLowerAnim->SetCurrentTrackPosition(fLowerStartTime);
	}
	if (bToUpperLowerSame == false && bFromAnimSameToUpper == false&& m_eCurrentTransitionType != ET::FullbodyToFullbody)
	{
		_float fUpperStartTime = transitionResult.fUpperStartTime * m_Blend.toUpperAnim->GetDuration();
		if (m_Blend.toUpperAnim->IsReverse())
		{
			// ������� ���
			_float fDuration = m_Blend.toUpperAnim->GetDuration();
			fUpperStartTime = fDuration - fUpperStartTime; // ������̹Ƿ� ���� �ð��� Duration���� ����
			fUpperStartTime = max(0.f, fUpperStartTime); // 0���� ������ 0���� ����
		}
		m_Blend.toUpperAnim->SetCurrentTrackPosition(fUpperStartTime);
	}

	m_bPlaying = true;
	UpdateMaskState();
}

void CAnimator::SetCurrentRootRotation(const _float4& rot)
{
	_float4 convertRot = { rot.y, rot.x, rot.z, rot.w };
	m_PrevRootRotation = m_CurrentRootRotation;
	m_CurrentRootRotation = convertRot;

	// ȸ�� ��Ÿ ���
	_vector prevRotInverse = XMQuaternionInverse(XMLoadFloat4(&m_PrevRootRotation));
	_vector currentRot = XMLoadFloat4(&m_CurrentRootRotation);
	_vector deltaRot = XMQuaternionMultiply(prevRotInverse, currentRot);

	XMStoreFloat4(&m_RootRotationDelta, deltaRot);
}

void CAnimator::SetCurrentRootPosition(const _float3& pos)
{	// ���� ��ġ/ȸ�� ����
	m_PrevRootPosition = m_CurrentRootPosition;

	// ���� ��ġ/ȸ�� ������Ʈ
	m_CurrentRootPosition = pos;

	// ��ġ ��Ÿ ��� (Y�� ���� ���δ� ������ ����)
	m_RootMotionDelta = {
		m_CurrentRootPosition.x - m_PrevRootPosition.x,
		m_CurrentRootPosition.y - m_PrevRootPosition.y,
		m_CurrentRootPosition.z - m_PrevRootPosition.z
	};

}
#ifdef _DEBUG
void CAnimator::DebugComputeShader()
{
	_int iBoneCount = static_cast<_int>(m_Bones.size());
	vector<_float4x4> boneMatrices(iBoneCount);
	if (SUCCEEDED(m_pAnimComputeShader->DownloadBoneMatrices(boneMatrices.data(),static_cast<_uint>(m_Bones.size()))))
	{
		for (_int i = 0; i < iBoneCount; ++i)
		{
			_matrix tmpMatrix = XMLoadFloat4x4(&boneMatrices[i]);
			// ����׿� ���
			cout << "Bone " << i << ": "
				<< "Position: (" << tmpMatrix.r[3].m128_f32[0] << ", "
				<< tmpMatrix.r[3].m128_f32[1] << ", "
				<< tmpMatrix.r[3].m128_f32[2] << "), "
				<< "Rotation: (" << tmpMatrix.r[0].m128_f32[0] << ", "
				<< tmpMatrix.r[1].m128_f32[1] << ", "
				<< tmpMatrix.r[2].m128_f32[2] << ")\n";
		}
	}
	else
	{
		MSG_BOX("Failed to dispatch compute shader");
	}
}
#endif

void CAnimator::RefreshAndProcessTransition(_float fDeltaTime)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->Update(fDeltaTime);
		auto& transitionResult = m_pCurAnimController->CheckTransition();
		if (transitionResult.bTransition)
		{
			m_Blend.belendFullBody = transitionResult.bBlendFullbody;
			if (m_Blend.belendFullBody == false)
			{
				m_Blend.blendWeight = transitionResult.fBlendWeight; // ���� ����ġ ����
			}
			StartTransition(transitionResult);
			m_pCurAnimController->ResetTransitionResult();
		}
	}
}

void CAnimator::UpdateBlend(_float fDeltaTime, size_t iBoneCount, vector<string>& triggeredEvents)
{
	m_iBlendAnimCount = 0;
	AddUniqueClip(m_Blend.fromLowerAnim, m_pBlendAnimArray, m_iBlendAnimCount);
	AddUniqueClip(m_Blend.toLowerAnim, m_pBlendAnimArray, m_iBlendAnimCount);
	AddUniqueClip(m_Blend.fromUpperAnim, m_pBlendAnimArray, m_iBlendAnimCount);
	AddUniqueClip(m_Blend.toUpperAnim, m_pBlendAnimArray, m_iBlendAnimCount);

	for (_int i = 0; i < m_iBlendAnimCount; ++i)
	{
		if (m_pBlendAnimArray[i])
		{
			m_pBlendAnimArray[i]->Update_Bones(
				fDeltaTime,
				m_Bones,
				m_pBlendAnimArray[i]->Get_isLoop(),
				&triggeredEvents, nullptr
			);
		}
	}

	// ��Ʈ���� �̸� ��������
	vector<_matrix> fromL(iBoneCount), toL(iBoneCount), fromU(iBoneCount), toU(iBoneCount);

	CollectBoneMatrices(m_Blend.fromLowerAnim, fromL, iBoneCount);
	CollectBoneMatrices(m_Blend.toLowerAnim, toL, iBoneCount);
	CollectBoneMatrices(m_Blend.fromUpperAnim, fromU, iBoneCount);
	CollectBoneMatrices(m_Blend.toUpperAnim, toU, iBoneCount);

	//����
	m_Blend.elapsed += fDeltaTime;
	_float fBlendFactor = min(m_Blend.elapsed / m_Blend.duration, 1.f);

	for (size_t i = 0; i < iBoneCount; ++i)
	{
		_bool isUpperBone = (m_UpperMaskSet.count(static_cast<_int>(i)) != 0);
		_matrix finalM = XMMatrixIdentity();

		switch (m_eCurrentTransitionType)
		{
		case ET::FullbodyToFullbody:
			finalM = LerpMatrix(fromL[i], toL[i], fBlendFactor);
			break;

		case ET::FullbodyToMasked:
			if (isUpperBone)
			{
				// ��ü �ִϸ��̼��� ������ ������ Ȯ��
				if (m_Blend.fromUpperAnim == m_Blend.toUpperAnim)
				{
					finalM = toU[i]; // ������ ���� ����
				}
				else
				{
					finalM = LerpMatrix(fromU[i], toU[i], fBlendFactor);
				}
			}
			else
			{
				finalM = LerpMatrix(fromL[i], toL[i], fBlendFactor);
			}
			break;

		case ET::MaskedToFullbody:

			if (isUpperBone)
			{
				// ��ü �ִϸ��̼� �� ��¥ �ִϸ��̼�
				if (m_Blend.fromUpperAnim == m_Blend.toLowerAnim)
				{
					finalM = toL[i]; // ������ ���� ����
				}
				else
				{
					finalM = LerpMatrix(fromU[i], toL[i], fBlendFactor);
				}
			}
			else
			{
				// ��ü �ִϸ��̼� �� ��¥ �ִϸ��̼�
				if (m_Blend.fromLowerAnim == m_Blend.toLowerAnim)
				{
					finalM = toL[i]; // ������ ���� ����
				}
				else
				{
					finalM = LerpMatrix(fromL[i], toL[i], fBlendFactor);
				}
			}

			break;

		case ET::MaskedToMasked:
			if (isUpperBone) 
			{
				if (m_Blend.fromUpperAnim == m_Blend.toUpperAnim)
				{
					// ��ü �ִϸ��̼��� ������ ���� ���� �״�� ���
					finalM = toU[i];
				}
				else {
					finalM = LerpMatrix(fromU[i], toU[i], fBlendFactor);
				}
			}
			else
			{
				if (m_Blend.fromLowerAnim == m_Blend.toLowerAnim)
				{
					// ��ü �ִϸ��̼��� ������ ���� ���� �״�� ���
					finalM = toL[i];
				}
				else
				{
					finalM = LerpMatrix(fromL[i], toL[i], fBlendFactor);
				}
			}
			break;
		}

		m_Bones[i]->Set_TransformationMatrix(finalM);
	}

	//  ���� �������� ����
	if (fBlendFactor >= 1.f)
	{
		_bool lowerChanged = (m_Blend.fromLowerAnim != m_Blend.toLowerAnim);
		_bool upperChanged = (m_Blend.fromUpperAnim != m_Blend.toUpperAnim);
		_bool lowerUpperSame = (m_Blend.fromLowerAnim != m_Blend.toUpperAnim);
		_bool upperLowerSame = (m_Blend.fromUpperAnim != m_Blend.toLowerAnim);
		unordered_set<CAnimation*> toReset;

		if (m_Blend.canSameAnimReset)
		{
			if (m_Blend.fromLowerAnim == m_Blend.toLowerAnim)
				toReset.insert(m_Blend.fromLowerAnim);

			if (m_Blend.fromUpperAnim == m_Blend.toUpperAnim)
				toReset.insert(m_Blend.fromUpperAnim);
		}

		if (lowerChanged)
		{
			if (lowerUpperSame)
			{
				toReset.insert(m_Blend.fromLowerAnim);
			}
		}
		if (upperChanged)
		{
			if (upperLowerSame)
			{
				toReset.insert(m_Blend.fromUpperAnim);
			}
		}

		for (auto* anim : toReset)
		{
			if (anim && anim->Get_isLoop() == false)
				anim->ResetTrack();

		}


		if (m_eCurrentTransitionType == ET::MaskedToFullbody)
		{
			m_bPlayMask = false;
			m_pLowerClip = nullptr;
			m_pUpperClip = nullptr;
			m_UpperMaskSet.clear();
		}
		m_bIsFinished = false;
		m_Blend.elapsed = 0.f;
		m_Blend.active = false;
		m_pCurrentAnim = m_Blend.toLowerAnim;

		return;
	}
}

void CAnimator::UpdateAnimation(_float fDeltaTime, size_t iBoneCount, vector<string>& triggeredEvents)
{
	if (m_bPlayMask && m_pLowerClip && m_pUpperClip)
	{
		// ��ü/��ü ������Ʈ
		vector<_float4x4> lowerLocalMatrices(iBoneCount);
		m_pLowerClip->Update_Bones(fDeltaTime, m_Bones, m_pLowerClip->Get_isLoop(), &triggeredEvents, &lowerLocalMatrices);
		vector<_float4x4> upperLocalMatrices(iBoneCount);
		m_bIsFinished = m_pUpperClip->Update_Bones(fDeltaTime, m_Bones, m_pUpperClip->Get_isLoop(), &triggeredEvents, &upperLocalMatrices);

		// ��Ʈ���� �̸� ��������
		vector<_matrix> lowerM(iBoneCount), upperM(iBoneCount);
		for (size_t i = 0; i < iBoneCount; ++i)
		{
			lowerM[i] = m_pLowerClip->GetBoneMatrix((unsigned)i);
			upperM[i] = m_pUpperClip->GetBoneMatrix((unsigned)i);
		}

		// ���¿� ������ ����ġ�� ����
		auto state = m_pCurAnimController->GetCurrentState();
		if (state)
		{

			_float fWeight = state->fBlendWeight;
			for (size_t i = 0; i < iBoneCount; ++i)
			{
				if (m_UpperMaskSet.count(static_cast<_int>(i)))
					m_Bones[i]->Set_TransformationMatrix(LerpMatrix(lowerM[i], upperM[i], fWeight));
				else
					m_Bones[i]->Set_TransformationMatrix(lowerM[i]);
			}
		}

		//if (m_pLowerClip->IsRootMotionEnabled() || m_pUpperClip->IsRootMotionEnabled())
		//{
		//	RootMotionDecomposition();
		//}
	}
	else if (m_pCurrentAnim)
	{
		// ���� ��ü �ִϸ��̼�
		vector<_float4x4> vecLocalMat(iBoneCount);

		//m_bIsFinished = m_pCurrentAnim->Update_Bones(fDeltaTime, m_Bones, m_pCurrentAnim->Get_isLoop(), &triggeredEvents,&vecLocalMat);
		m_bIsFinished = m_pCurrentAnim->Update_Bones(fDeltaTime, m_Bones, m_pCurrentAnim->Get_isLoop(), &triggeredEvents, nullptr);
		//for (size_t i = 0; i < iBoneCount; ++i)
		//{
		//	vecLocalMat[i] = *m_Bones[i]->Get_TransformationMatrix();
		//}
		//if (!vecLocalMat.empty())
		//{
		//	//vector<_int> parentsForCS(m_Bones.size());
		//	//for (_uint i = 0; i < m_Bones.size(); ++i)
		//	//{
		//	//	int p = m_Bones[i]->Get_ParentBoneIndex();               // ���� �θ�
		//	//	if (m_bApplyRootMotion && p == 1) p = -1; // CPU�� ���� ��Ģ
		//	//	parentsForCS[i] = p;
		//	//}

		//	//// �� parentsForCS�� ���� ��� + GPU ���ε�
		//	//BuildHierarchyLevelsFrom(parentsForCS);
		//	//UploadParentsBuffer(parentsForCS); // g_Parents (t2)
		//	//vector<_int> boneParents;
		//	//boneParents.reserve(m_Bones.size());
		//	//for (const auto& bone : m_Bones)
		//	//{
		//	//	boneParents.push_back(bone->Get_UseParentIndex());
		//	//}
		//	//m_pAnimComputeShader->SetParentIndices(parentsForCS);
		//	//m_pAnimComputeShader->BuildHierarchyLevels();
		//	//vector<_float> boneMask(m_Bones.size(), 0.f);
		//	//m_pAnimComputeShader->SetBoneMask(boneMask);
		//	//// ���� ��� ���ε�
		//	//m_pAnimComputeShader->UploadBoneMatrices(vecLocalMat.data());
		//	//_float4x4 preTransform = m_pModel->Get_PreTransformMatrix();
		//	//m_pAnimComputeShader->ExecuteHierarchical(preTransform);
		//}
		if (m_pCurrentAnim->IsRootMotionEnabled())
		{
			RootMotionDecomposition();
		}
	}
}

void CAnimator::UpdateMaskState()
{
	if (m_pCurAnimController == nullptr)
		return;
	auto curState = m_pCurAnimController->GetCurrentState();
	if (curState && !curState->maskBoneName.empty())
	{
		m_bPlayMask = true;
		m_pLowerClip = m_pModel->GetAnimationClipByName(curState->lowerClipName);
		m_pUpperClip = m_pModel->GetAnimationClipByName(curState->upperClipName);

		if (m_pLowerClip && m_pLowerClip->GetCurrentTrackPosition() >= m_pLowerClip->GetDuration())
		{
			if (m_Blend.toLowerAnim != m_pLowerClip)
				m_pLowerClip->ResetTrack();
		}
		if (m_pUpperClip && m_pUpperClip->GetCurrentTrackPosition() >= m_pUpperClip->GetDuration())
		{
			if (m_Blend.toUpperAnim != m_pUpperClip)
				m_pUpperClip->ResetTrack();
		}

		MakeMaskBones(curState->maskBoneName);
	}
	else
	{
		if (m_eCurrentTransitionType != ET::MaskedToFullbody &&
			m_eCurrentTransitionType != ET::MaskedToMasked && m_eCurrentTransitionType != ET::FullbodyToMasked)
		{
			m_bPlayMask = false;
			m_pLowerClip = nullptr;
			m_pUpperClip = nullptr;
			m_UpperMaskSet.clear();
		}
	}
}

void CAnimator::MakeMaskBones(const string& maskBoneName)
{
	// ��������� ó�� ���ϱ�
	if (maskBoneName.empty())
	{
		return;
	}
	if (m_UpperMaskSetMap.count(maskBoneName) > 0)
	{
		// �̹� ����ũ ��Ʈ�� �ִٸ� ����
		m_UpperMaskSet = m_UpperMaskSetMap[maskBoneName];
		return;
	}
	else
	{
		CollectBoneChildren(maskBoneName.c_str());
		m_UpperMaskSetMap[maskBoneName] = m_UpperMaskSet;
	}
}

void CAnimator::CollectBoneChildren(const _char* boneName)
{
	_int idx = m_pModel->Find_BoneIndex(boneName);
	if (idx < 0 || m_UpperMaskSet.count(idx) || m_Bones.size() <= idx)
		return;

#ifdef _DEBUG
	cout << "Collecting bone: " << boneName << endl;
#endif // _DEBUG

	m_UpperMaskSet.insert(idx);

	// �� ���� ��� �ڽ� �̸��� �����ͼ� ���
	for (int childIdx : m_pModel->GetBoneChildren(boneName))
	{
		const char* childName = m_pModel->Get_Bones()[childIdx]->Get_Name();
//#ifdef _DEBUG
//		cout << "Child bone: " << childName << endl;
//#endif // _DEBUG
		CollectBoneChildren(childName, "Neck");
	}
}

void CAnimator::CollectBoneChildren(const _char* boneName, const _char* stopBoneName)
{
	_int idx = m_pModel->Find_BoneIndex(boneName);
	// ��ȿ �ε�������, �̹� �߰��� ������ Ȯ��
	if (idx < 0 || m_UpperMaskSet.count(idx))
		return;

	string bone = boneName;
	if (bone.find(stopBoneName) != string::npos)
		return;
	m_UpperMaskSet.insert(idx);

	for (int childIdx : m_pModel->GetBoneChildren(boneName))
	{
		const char* childName = m_pModel->Get_Bones()[childIdx]->Get_Name();
		CollectBoneChildren(childName, stopBoneName);
	}
}

void CAnimator::AddUniqueClip(CAnimation* pClip, array<CAnimation*, 4>& pArray, _int& clipCount)
{
	if (!pClip)
		return;
	// �ߺ� �˻�
	for (_int i = 0; i < clipCount; ++i)
	{
		if (pArray[i] == pClip) // �̹� �����ϴ� Ŭ���̰ų� ���� �ִϸ��̼��̸�
			return;
	}
	pArray[clipCount++] = pClip;
}



_matrix CAnimator::LerpMatrix(const _matrix& src, const _matrix& dst, _float t)
{
	_vector sS, sR, sT, dS, dR, dT;
	XMMatrixDecompose(&sS, &sR, &sT, src);
	XMMatrixDecompose(&dS, &dR, &dT, dst);
	_vector bS = XMVectorLerp(sS, dS, t);
	_vector bR = XMQuaternionSlerp(sR, dR, t);
	_vector bT = XMVectorLerp(sT, dT, t);
	return XMMatrixScalingFromVector(bS) * XMMatrixRotationQuaternion(bR) * XMMatrixTranslationFromVector(bT);
}

void CAnimator::CollectBoneMatrices(CAnimation* pAnim, vector<_matrix>& boneMatrices, size_t iBoneCount)
{
	for (size_t i = 0; i < iBoneCount; ++i)
	{
		boneMatrices[i] = pAnim ? pAnim->GetBoneMatrix(static_cast<_uint>(i)) : XMMatrixIdentity();
	}
}


void CAnimator::RootMotionDecomposition()
{
	CBone* rootBone = m_Bones[1];
	CBone* pelvisBone = m_Bones[1];

	// ������ ���� ���
	_matrix rootMat = XMLoadFloat4x4(rootBone->Get_CombinedTransformationMatrix());
	_matrix pelvisMat = XMLoadFloat4x4(pelvisBone->Get_TransformationMatrix());

	// ��ġ��ȸ���������� ����
	_vector rootScale, rootRotQuat, rootTrans;
	XMMatrixDecompose(&rootScale, &rootRotQuat, &rootTrans, rootMat);

	_vector pelvisScale, pelvisRotQuat, pelvisTrans;
	XMMatrixDecompose(&pelvisScale, &pelvisRotQuat, &pelvisTrans, pelvisMat);

	_float3 rootPos;      XMStoreFloat3(&rootPos, rootTrans);
	_float4 pelvisRot;    XMStoreFloat4(&pelvisRot, pelvisRotQuat);

	this->SetCurrentRootRotation(pelvisRot);
	this->SetCurrentRootPosition(rootPos);
}

void CAnimator::ResetRootMotion()
{
	m_bFirstFrameAfterReset = true;
	m_RootMotionDelta = { 0.f, 0.f, 0.f };

	if (!m_Bones.empty())
	{
		_matrix rootMat = XMLoadFloat4x4(m_Bones[1]->Get_CombinedTransformationMatrix());
		_vector s, r, t;
		XMMatrixDecompose(&s, &r, &t, rootMat);
		XMStoreFloat3(&m_PrevRootPosition, t);
		XMStoreFloat3(&m_CurrentRootPosition, t);
		XMStoreFloat4(&m_PrevRootRotation, r);
		XMStoreFloat4(&m_CurrentRootRotation, r);
	}
	else
	{
		m_PrevRootPosition = { 0.f, 0.f, 0.f };
		m_CurrentRootPosition = { 0.f, 0.f, 0.f };
		m_PrevRootRotation = { 0.f, 0.f, 0.f, 1.f };
		m_CurrentRootRotation = { 0.f, 0.f, 0.f, 1.f };
	}
	//m_PrevRootPosition = { 0.f, 0.f, 0.f };
	//m_CurrentRootPosition = { 0.f, 0.f, 0.f };
	//m_RootRotationDelta = { 0.f, 0.f, 0.f, 1.f };
	//m_PrevRootRotation = { 0.f, 0.f, 0.f, 1.f };
	//m_CurrentRootRotation = { 0.f, 0.f, 0.f, 1.f };
}

void CAnimator::DispatchAnimEvents(const vector<string>& triggeredEvents)
{
	for (auto& name : triggeredEvents)
	{
		auto it = m_eventListeners.find(name);
		if (it != m_eventListeners.end())
		{
			for (auto& cb : it->second)
				cb();
		}
	}
}

_float CAnimator::GetYAngleFromQuaternion(const _vector& quat)
{
	_float4 q;
	XMStoreFloat4(&q, quat);
	return atan2(2.0f * (q.w * q.y + q.x * q.z), 1.0f - 2.0f * (q.y * q.y + q.z * q.z));
}

const string CAnimator::GetCurrentAnimName() const
{
	return m_pCurrentAnim->Get_Name();
}

void CAnimator::RegisterEventListener(const string& eventName, AnimEventCallback&& cb)
{
	m_eventListeners[eventName].push_back(move(cb));
}

void CAnimator::RegisterAnimController(const string& name, CAnimController* pController)
{
	if (pController == nullptr)
	{
		MSG_BOX("Cannot register a null animation controller.");
		return;
	}
	if (m_AnimControllers.find(name) != m_AnimControllers.end())
	{
		MSG_BOX("Animation controller with this name already exists.");
		return;
	}
	pController->SetAnimator(this);
	m_AnimControllers[name] = pController;
	if (m_pCurAnimController == nullptr)
	{
		m_pCurAnimController = pController; // ù ��° ��ϵ� ��Ʈ�ѷ��� ���� ��Ʈ�ѷ��� ����
	}
}

void CAnimator::UnregisterAnimController(const string& name)
{
	auto it = m_AnimControllers.find(name);
	if (it != m_AnimControllers.end())
	{
		Safe_Release(it->second);
		m_AnimControllers.erase(it);
	}
}

void CAnimator::RenameAnimController(const string& oldName, const string& newName)
{
	auto it = m_AnimControllers.find(oldName);
	if (it != m_AnimControllers.end())
	{
		if (m_AnimControllers.find(newName) != m_AnimControllers.end())
		{
			MSG_BOX("Name already exists.");
			return;
		}
		it->second->SetName(newName);
		m_AnimControllers[newName] = it->second;
		m_AnimControllers.erase(it);
	}
	else
	{
		MSG_BOX("Old name does not exist.");
	}
}

void CAnimator::SetCurrentAnimController(const string& name, const string& stateName)
{
	auto it = m_AnimControllers.find(name);

	if (it != m_AnimControllers.end())
	{
		if (it->second == m_pCurAnimController)
		{
			// �̹� ���� ��Ʈ�ѷ��� ������ �ƹ��͵� ����
			return;
		}
		// ���� �ִϸ��̼��� ��Ʈ�ѷ��� �ٲ� ��Ʈ�ѷ��� �ִϸ��̼� Ŭ���� ����
		CAnimController* pCurrentController = m_pCurAnimController;
		CAnimController* pTargetController = it->second;
		if (stateName.empty() == false && pTargetController->GetStateByName(stateName) != nullptr)
		{
			auto pFromAnimState = pCurrentController->GetCurrentState(); // ���� �ִϸ����Ͱ� ��� ���� AnimState
			auto pToAnimState = pTargetController->GetStateByName(stateName); // ��ǥ AnimState

			if (pFromAnimState == nullptr || pToAnimState == nullptr)
			{
				// ���� ���³� ��ǥ ���¸� ã�� �� ���� ��� ó�� (���� �Ǵ� �⺻ ��ȯ)
				m_pCurAnimController = pTargetController;
				m_pCurAnimController->SetState(stateName);
				return;
			}

			//���� �� ��ǥ ���°� Fullbody���� Masked���� �Ǵ�
			_bool bFromIsMasked = pFromAnimState->maskBoneName.empty() == false;
			_bool bToIsMasked = pToAnimState->maskBoneName.empty() == false;

			//  m_eCurrentTransitionType ����
			if (!bFromIsMasked && !bToIsMasked) // Fullbody -> Fullbody
			{
				m_eCurrentTransitionType = CAnimController::ETransitionType::FullbodyToFullbody;
			}
			else if (!bFromIsMasked && bToIsMasked) // Fullbody -> Masked
			{
				m_eCurrentTransitionType = CAnimController::ETransitionType::FullbodyToMasked;
			}
			else if (bFromIsMasked && !bToIsMasked) // Masked -> Fullbody
			{
				m_eCurrentTransitionType = CAnimController::ETransitionType::MaskedToFullbody;
			}
			else // bFromIsMasked && bToIsMasked (Masked -> Masked)
			{
				m_eCurrentTransitionType = CAnimController::ETransitionType::MaskedToMasked;
			}

			// From �ִϸ��̼� ����
			if (bFromIsMasked)
			{
				m_Blend.fromLowerAnim = m_pModel->GetAnimationClipByName(pFromAnimState->lowerClipName);
				m_Blend.fromUpperAnim = m_pModel->GetAnimationClipByName(pFromAnimState->upperClipName);
			}
			else // Fullbody
			{
				m_Blend.fromLowerAnim = m_pCurrentAnim; // ���� ��� ���� Fullbody �ִ�
				m_Blend.fromUpperAnim = m_pCurrentAnim; // Fullbody�� ��ü�� ���� �ִ�
			}

			// To �ִϸ��̼� ����
			if (bToIsMasked)
			{
				m_Blend.toLowerAnim = m_pModel->GetAnimationClipByName(pToAnimState->lowerClipName);
				m_Blend.toUpperAnim = m_pModel->GetAnimationClipByName(pToAnimState->upperClipName);
			}
			else // Fullbody
			{
				m_Blend.toLowerAnim = pToAnimState->clip; // Fullbody�� clip ���
				m_Blend.toUpperAnim = pToAnimState->clip; // Fullbody�� ��ü�� ���� �ִ�
			}

			CAnimController::TransitionResult transitionResult{};
			transitionResult.pFromLowerAnim = m_Blend.fromLowerAnim;
			transitionResult.pToLowerAnim = m_Blend.toLowerAnim;
			transitionResult.pFromUpperAnim = m_Blend.fromUpperAnim;
			transitionResult.pToUpperAnim = m_Blend.toUpperAnim;

			transitionResult.fDuration = 0.2f;

			m_Blend.active = true; // ���� Ȱ��ȭ
			m_pCurAnimController = pTargetController; // ���� ��Ʈ�ѷ� ����
			m_pCurAnimController->SetState(stateName); // ��Ʈ�ѷ��� ���� ���� ����
			StartTransition(transitionResult); // ���� ���� ����
		}
		else
		{
			// ������ �׳� ��Ʈ�ѷ��� ����Ǹ� ��
			m_pCurAnimController = it->second;
			m_pCurAnimController->SetState(""); // ���¸� ����� �ʱ�ȭ
		}
	}
}

_float CAnimator::GetStateClipLengthByName(const string& name) const
{
	return m_pCurAnimController->GetStateClipLength(name);
}

_bool CAnimator::ExisitsParameter(const string& name) const
{
	if (m_pCurAnimController)
	{
		return m_pCurAnimController->ExisitsParameter(name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
		return false; // �⺻�� ��ȯ
	}
}

void CAnimator::AddBool(const string& name)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->AddBool(name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

void CAnimator::AddFloat(const string& name)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->AddFloat(name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

void CAnimator::AddTrigger(const string& name)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->AddTrigger(name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

void CAnimator::AddInt(const string& name)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->AddInt(name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

void CAnimator::SetInt(const string& name, _int v)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->SetInt(name, v);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

void CAnimator::SetBool(const string& name, _bool v)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->SetBool(name, v);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

void CAnimator::SetFloat(const string& name, _float v)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->SetFloat(name, v);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

void CAnimator::SetTrigger(const string& name)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->SetTrigger(name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

void CAnimator::ResetTrigger(const string& name)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->ResetTrigger(name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

void CAnimator::DeleteParameter(const string& name)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->DeleteParameter(name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

void CAnimator::SetParamName(Parameter& param, const string& name)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->SetParamName(param, name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

void CAnimator::SetApplyRootMotion(_bool bApply)
{
	m_bApplyRootMotion = bApply;
	for (auto& pBone : m_Bones)
	{
		pBone->SetApplyRootMotion(bApply);
	}
}

_float CAnimator::GetFloat(const string& name) const
{
	if (m_pCurAnimController)
	{
		return m_pCurAnimController->GetFloat(name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
		return 0.f; // �⺻�� ��ȯ
	}
}

_int CAnimator::GetInt(const string& name) const
{
	if (m_pCurAnimController)
	{
		return m_pCurAnimController->GetInt(name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
		return 0; // �⺻�� ��ȯ
	}
}

_bool CAnimator::CheckBool(const string& name) const
{
	if (m_pCurAnimController)
	{
		return m_pCurAnimController->CheckBool(name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
		return false; // �⺻�� ��ȯ
	}
}

_bool CAnimator::CheckTrigger(const string& name)
{
	if (m_pCurAnimController)
	{
		return m_pCurAnimController->CheckTrigger(name);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
		return false; // �⺻�� ��ȯ
	}
}

void CAnimator::AddParameter(const string& name, Parameter& parm)
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->AddParameter(name, parm);
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

void CAnimator::ResetParameters()
{
	if (m_pCurAnimController)
	{
		m_pCurAnimController->ResetParameters();
	}
	else
	{
		MSG_BOX("No current animation controller set.");
	}
}

#ifdef USE_IMGUI
unordered_map<string, Parameter>& CAnimator::GetParametersForEditor()
{
	if (m_pCurAnimController)
	{
		return m_pCurAnimController->GetParametersForEditor();
	}
	else
	{
		MSG_BOX("No current animation controller set.");
		static unordered_map<string, Parameter> emptyParams; // �� �� ��ȯ
		return emptyParams;
	}
}
#endif
const unordered_map<string, Parameter>& CAnimator::GetParameters() const
{
	if (m_pCurAnimController)
	{
		return m_pCurAnimController->GetParameters();
	}
	else
	{
		MSG_BOX("No current animation controller set.");
		static unordered_map<string, Parameter> emptyParams; // �� �� ��ȯ
		return emptyParams;
	}
}

CAnimator* CAnimator::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAnimator* pInstance = new CAnimator(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAnimator");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent* CAnimator::Clone(void* pArg)
{
	CAnimator* pInstance = new CAnimator(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAnimator");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAnimator::Free()
{
	__super::Free();
	for (auto& Pair : m_AnimControllers)
	{
		Safe_Release(Pair.second);
	}
	Safe_Release(m_pAnimComputeShader);
}

json CAnimator::Serialize()
{
	CHAR exeFullPath[MAX_PATH] = {};
	GetModuleFileNameA(nullptr, exeFullPath, MAX_PATH);

	// EXE�� �ִ� ������ �������� ��� ���
	filesystem::path exeDir = filesystem::path(exeFullPath).parent_path();
	filesystem::path saveDir = exeDir
		/ "Save"
		/ "AnimationStates"
		/ "AnimControllers";

	// ��� ������ ����
	filesystem::create_directories(saveDir);

	json j;
	j["Controllers"] = json::array();

	// ��Ʈ�ѷ��� ���Ϸ� ��������
	for (auto& [name, pCtrl] : m_AnimControllers)
	{
		if (!pCtrl)
			continue;

		json ctrlJ = pCtrl->Serialize();

		// ���ϸ�: <exeDir>/Bin/Save/AnimationStates/<ControllerName>.json
		filesystem::path filePath = saveDir / (pCtrl->GetName() + ".json");

		ofstream ofs(filePath.string());
		if (!ofs)
		{
			MSG_BOX("Failed to write file");
			continue;
		}
		ofs << ctrlJ.dump(4);
		ofs.close();

		j["Controllers"].push_back(name);
	}

	// ��Ʈ�ѷ��� ���� �Ǵ� ��������Ʈ ��Ʈ�ѷ� ����

	j["OverrideControllers"] = json::array();
	for (auto& [name, ctrl] : m_OverrideControllerMap)
	{
		json overrideCtrlJ;
		overrideCtrlJ["Name"] = name;
		overrideCtrlJ["Controller"] = ctrl.controllerName;
		json stateJ;
		stateJ["States"] = json::array();
		for (auto& [stateName, state] : ctrl.states)
		{

			json stateDataJ;
			stateDataJ["Name"] = stateName;
			stateDataJ["ClipName"] = state.clipName;
			stateDataJ["MaskBoneName"] = state.maskBoneName;
			stateDataJ["LowerClipName"] = state.lowerClipName;
			stateDataJ["UpperClipName"] = state.upperClipName;
			stateDataJ["BlendWeight"] = state.fBlendWeight;
			stateJ["States"].push_back(stateDataJ);
		}

		overrideCtrlJ["States"] = stateJ["States"];
		j["OverrideControllers"].push_back(overrideCtrlJ);
	}

	return j;
}

void CAnimator::Deserialize(const json& j)
{
	m_pCurAnimController = nullptr;
	m_pCurrentAnim = nullptr; // ���� �ִϸ��̼� �ʱ�ȭ
	CHAR exeFullPath[MAX_PATH] = {};
	GetModuleFileNameA(nullptr, exeFullPath, MAX_PATH);
	filesystem::path exeDir = filesystem::path(exeFullPath).parent_path();
	auto loadDir = exeDir / "Save" / "AnimationStates" / "AnimControllers";

	for (auto& Pair : m_AnimControllers)
	{
		Safe_Release(Pair.second);
	}
	m_AnimControllers.clear();

	if (j.contains("Controllers") && j["Controllers"].is_array())
	{
		for (auto& nameJ : j["Controllers"])
		{
			if (!nameJ.is_string()) continue;

			string name = nameJ.get<string>();
			auto filePath = loadDir / (name + ".json");
			ifstream ifs(filePath.string());
			if (!ifs) {
				MSG_BOX("Cannot open");
				continue;
			}

			json ctrlJ;
			ifs >> ctrlJ;

			CAnimController* pCtrl = CAnimController::Create();

			if (pCtrl)
			{
				pCtrl->SetAnimator(this);
				pCtrl->Deserialize(ctrlJ);
				m_AnimControllers[name] = pCtrl;

				// ù ��° ��Ʈ�ѷ��� current��
				if (!m_pCurAnimController)
					m_pCurAnimController = pCtrl;

				// ���� �ִϸ��̼� ��Ʈ�ѷ��� ������Ʈ���� �ҷ��ͼ� MaskBone�� ����ϸ� �� �̸� �ҷ��α�
				auto& states = m_pCurAnimController->GetStates();

				for (const auto& state : states)
				{
					if (state.maskBoneName.empty() == false)
					{
						MakeMaskBones(state.maskBoneName);
					}
				}
			}
		}
	}

	if (j.contains("OverrideControllers") && j["OverrideControllers"].is_array())
	{
		for (auto& ctrl : j["OverrideControllers"])
		{
			if (!ctrl.is_object())
				continue;
			OverrideAnimController overrideCtrl;
			overrideCtrl.name = ctrl.value("Name", "");
			overrideCtrl.controllerName = ctrl.value("Controller", "");
			if (overrideCtrl.name.empty() || overrideCtrl.controllerName.empty())
				continue;

			if (ctrl.contains("States") && ctrl["States"].is_array())
			{
				for (auto& sj : ctrl["States"])
				{
					if (!sj.is_object()) continue;
					string stName = sj.value("Name", "");
					if (stName.empty()) continue;

					OverrideAnimController::OverrideState state;
					state.clipName = sj.value("ClipName", "");
					state.maskBoneName = sj.value("MaskBoneName", "");
					state.lowerClipName = sj.value("LowerClipName", "");
					state.upperClipName = sj.value("UpperClipName", "");
					state.fBlendWeight = sj.value("BlendWeight", 1.0f);

					overrideCtrl.states[stName] = state;
				}
			}

			m_OverrideControllerMap[overrideCtrl.name] = overrideCtrl;
		}
	}

	// �������̵� ��Ʈ�ѷ� ��Ʈ�ѷ��� ���
	for (auto& [name, ctrl] : m_OverrideControllerMap)
	{
		auto it = m_AnimControllers.find(ctrl.controllerName);
		if (it != m_AnimControllers.end())
		{
			it->second->Add_OverrideAnimController(name, ctrl);
		}
	}
}
