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
	// 디폴트 컨트롤러 생성해서 설정
	m_pCurAnimController->SetAnimator(this);
//	m_pCurAnimController->Initialize_Prototype();
	m_pCurAnimController->SetName(m_pModel->Get_ModelName() + "_Default");
	// 컨트롤러 이름

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
	// 디폴트 컨트롤러 생성해서 설정
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

	vector<string> triggeredEvents; // 이벤트를 한 번에 수집
	size_t iBoneCount = m_Bones.size();

	if (m_Blend.active) //  블렌딩 중
	{
		UpdateBlend(fTimeScale, iBoneCount, triggeredEvents);
	}
	else // 블렌드가 아닐 때
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

	// 블렌딩에 사용될 클립
	m_Blend.fromLowerAnim = transitionResult.pFromLowerAnim;
	m_Blend.toLowerAnim = transitionResult.pToLowerAnim;
	m_Blend.fromUpperAnim = transitionResult.pFromUpperAnim;
	m_Blend.toUpperAnim = transitionResult.pToUpperAnim;
	m_eCurrentTransitionType = transitionResult.eType; // 현재 전환 타입 저장
	m_Blend.blendWeight = transitionResult.fBlendWeight; // 상하체 블렌드 가중치 
	m_Blend.canSameAnimReset = transitionResult.bCanSameAnimReset; // 같은 애니메이션일 때 초기화 가능한지 여부


	_bool bFromLowerAnimSame = (transitionResult.pFromLowerAnim == transitionResult.pToLowerAnim);
	_bool bToUpperLowerSame = (transitionResult.pFromUpperAnim == transitionResult.pToUpperAnim);
	_bool bFromAnimSameToUpper = (transitionResult.pFromLowerAnim == transitionResult.pToUpperAnim);
	_bool bFromAnimSameToLower = (transitionResult.pFromUpperAnim == transitionResult.pToLowerAnim);
	
	if (bFromLowerAnimSame == false && bFromAnimSameToLower == false ) // 이전 하체랑도 다르고 이전 상체랑도 다른 하체라면
	{
		_float fLowerStartTime = transitionResult.fLowerStartTime * m_Blend.toLowerAnim->GetDuration();
		if (m_Blend.toLowerAnim->IsReverse())
		{
			// 역재생인 경우
			_float fDuration = m_Blend.toLowerAnim->GetDuration();
			fLowerStartTime = fDuration - fLowerStartTime; // 역재생이므로 시작 시간을 Duration에서 빼줌
			fLowerStartTime = max(0.f, fLowerStartTime); // 0보다 작으면 0으로 설정
		}
		m_Blend.toLowerAnim->SetCurrentTrackPosition(fLowerStartTime);
	}
	if (bToUpperLowerSame == false && bFromAnimSameToUpper == false&& m_eCurrentTransitionType != ET::FullbodyToFullbody)
	{
		_float fUpperStartTime = transitionResult.fUpperStartTime * m_Blend.toUpperAnim->GetDuration();
		if (m_Blend.toUpperAnim->IsReverse())
		{
			// 역재생인 경우
			_float fDuration = m_Blend.toUpperAnim->GetDuration();
			fUpperStartTime = fDuration - fUpperStartTime; // 역재생이므로 시작 시간을 Duration에서 빼줌
			fUpperStartTime = max(0.f, fUpperStartTime); // 0보다 작으면 0으로 설정
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

	// 회전 델타 계산
	_vector prevRotInverse = XMQuaternionInverse(XMLoadFloat4(&m_PrevRootRotation));
	_vector currentRot = XMLoadFloat4(&m_CurrentRootRotation);
	_vector deltaRot = XMQuaternionMultiply(prevRotInverse, currentRot);

	XMStoreFloat4(&m_RootRotationDelta, deltaRot);
}

void CAnimator::SetCurrentRootPosition(const _float3& pos)
{	// 이전 위치/회전 저장
	m_PrevRootPosition = m_CurrentRootPosition;

	// 현재 위치/회전 업데이트
	m_CurrentRootPosition = pos;

	// 위치 델타 계산 (Y축 포함 여부는 설정에 따라)
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
			// 디버그용 출력
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
				m_Blend.blendWeight = transitionResult.fBlendWeight; // 블렌드 가중치 설정
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

	// 매트릭스 미리 가져오기
	vector<_matrix> fromL(iBoneCount), toL(iBoneCount), fromU(iBoneCount), toU(iBoneCount);

	CollectBoneMatrices(m_Blend.fromLowerAnim, fromL, iBoneCount);
	CollectBoneMatrices(m_Blend.toLowerAnim, toL, iBoneCount);
	CollectBoneMatrices(m_Blend.fromUpperAnim, fromU, iBoneCount);
	CollectBoneMatrices(m_Blend.toUpperAnim, toU, iBoneCount);

	//블렌드
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
				// 상체 애니메이션이 실제로 같은지 확인
				if (m_Blend.fromUpperAnim == m_Blend.toUpperAnim)
				{
					finalM = toU[i]; // 같으면 블렌딩 없이
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
				// 상체 애니메이션 → 통짜 애니메이션
				if (m_Blend.fromUpperAnim == m_Blend.toLowerAnim)
				{
					finalM = toL[i]; // 같으면 블렌딩 없이
				}
				else
				{
					finalM = LerpMatrix(fromU[i], toL[i], fBlendFactor);
				}
			}
			else
			{
				// 하체 애니메이션 → 통짜 애니메이션
				if (m_Blend.fromLowerAnim == m_Blend.toLowerAnim)
				{
					finalM = toL[i]; // 같으면 블렌딩 없이
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
					// 상체 애니메이션이 같으면 블렌딩 없이 그대로 사용
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
					// 하체 애니메이션이 같으면 블렌딩 없이 그대로 사용
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

	//  블렌드 끝났으면 정리
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
		// 하체/상체 업데이트
		vector<_float4x4> lowerLocalMatrices(iBoneCount);
		m_pLowerClip->Update_Bones(fDeltaTime, m_Bones, m_pLowerClip->Get_isLoop(), &triggeredEvents, &lowerLocalMatrices);
		vector<_float4x4> upperLocalMatrices(iBoneCount);
		m_bIsFinished = m_pUpperClip->Update_Bones(fDeltaTime, m_Bones, m_pUpperClip->Get_isLoop(), &triggeredEvents, &upperLocalMatrices);

		// 매트릭스 미리 가져오기
		vector<_matrix> lowerM(iBoneCount), upperM(iBoneCount);
		for (size_t i = 0; i < iBoneCount; ++i)
		{
			lowerM[i] = m_pLowerClip->GetBoneMatrix((unsigned)i);
			upperM[i] = m_pUpperClip->GetBoneMatrix((unsigned)i);
		}

		// 상태에 설정된 가중치로 블렌드
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
		// 단일 전체 애니메이션
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
		//	//	int p = m_Bones[i]->Get_ParentBoneIndex();               // 원본 부모
		//	//	if (m_bApplyRootMotion && p == 1) p = -1; // CPU와 동일 규칙
		//	//	parentsForCS[i] = p;
		//	//}

		//	//// 이 parentsForCS로 레벨 계산 + GPU 업로드
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
		//	//// 로컬 행렬 업로드
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
	// 비어있으면 처리 안하기
	if (maskBoneName.empty())
	{
		return;
	}
	if (m_UpperMaskSetMap.count(maskBoneName) > 0)
	{
		// 이미 마스크 세트가 있다면 재사용
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

	// 이 본의 모든 자식 이름을 가져와서 재귀
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
	// 유효 인덱스인지, 이미 추가된 본인지 확인
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
	// 중복 검사
	for (_int i = 0; i < clipCount; ++i)
	{
		if (pArray[i] == pClip) // 이미 존재하는 클립이거나 루프 애니메이션이면
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

	// 꺼내온 조합 행렬
	_matrix rootMat = XMLoadFloat4x4(rootBone->Get_CombinedTransformationMatrix());
	_matrix pelvisMat = XMLoadFloat4x4(pelvisBone->Get_TransformationMatrix());

	// 위치·회전·스케일 분해
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
		m_pCurAnimController = pController; // 첫 번째 등록된 컨트롤러를 현재 컨트롤러로 설정
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
			// 이미 현재 컨트롤러와 같으면 아무것도 안함
			return;
		}
		// 현재 애니메이션의 컨트롤러와 바꿀 컨트롤러의 애니메이션 클립을 블렌딩
		CAnimController* pCurrentController = m_pCurAnimController;
		CAnimController* pTargetController = it->second;
		if (stateName.empty() == false && pTargetController->GetStateByName(stateName) != nullptr)
		{
			auto pFromAnimState = pCurrentController->GetCurrentState(); // 현재 애니메이터가 재생 중인 AnimState
			auto pToAnimState = pTargetController->GetStateByName(stateName); // 목표 AnimState

			if (pFromAnimState == nullptr || pToAnimState == nullptr)
			{
				// 현재 상태나 목표 상태를 찾을 수 없는 경우 처리 (에러 또는 기본 전환)
				m_pCurAnimController = pTargetController;
				m_pCurAnimController->SetState(stateName);
				return;
			}

			//현재 및 목표 상태가 Fullbody인지 Masked인지 판단
			_bool bFromIsMasked = pFromAnimState->maskBoneName.empty() == false;
			_bool bToIsMasked = pToAnimState->maskBoneName.empty() == false;

			//  m_eCurrentTransitionType 결정
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

			// From 애니메이션 설정
			if (bFromIsMasked)
			{
				m_Blend.fromLowerAnim = m_pModel->GetAnimationClipByName(pFromAnimState->lowerClipName);
				m_Blend.fromUpperAnim = m_pModel->GetAnimationClipByName(pFromAnimState->upperClipName);
			}
			else // Fullbody
			{
				m_Blend.fromLowerAnim = m_pCurrentAnim; // 현재 재생 중인 Fullbody 애니
				m_Blend.fromUpperAnim = m_pCurrentAnim; // Fullbody의 상체도 동일 애니
			}

			// To 애니메이션 설정
			if (bToIsMasked)
			{
				m_Blend.toLowerAnim = m_pModel->GetAnimationClipByName(pToAnimState->lowerClipName);
				m_Blend.toUpperAnim = m_pModel->GetAnimationClipByName(pToAnimState->upperClipName);
			}
			else // Fullbody
			{
				m_Blend.toLowerAnim = pToAnimState->clip; // Fullbody는 clip 사용
				m_Blend.toUpperAnim = pToAnimState->clip; // Fullbody의 상체도 동일 애니
			}

			CAnimController::TransitionResult transitionResult{};
			transitionResult.pFromLowerAnim = m_Blend.fromLowerAnim;
			transitionResult.pToLowerAnim = m_Blend.toLowerAnim;
			transitionResult.pFromUpperAnim = m_Blend.fromUpperAnim;
			transitionResult.pToUpperAnim = m_Blend.toUpperAnim;

			transitionResult.fDuration = 0.2f;

			m_Blend.active = true; // 블렌딩 활성화
			m_pCurAnimController = pTargetController; // 현재 컨트롤러 변경
			m_pCurAnimController->SetState(stateName); // 컨트롤러의 현재 상태 설정
			StartTransition(transitionResult); // 실제 블렌딩 시작
		}
		else
		{
			// 없으면 그냥 컨트롤러가 변경되면 됨
			m_pCurAnimController = it->second;
			m_pCurAnimController->SetState(""); // 상태를 비워서 초기화
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
		return false; // 기본값 반환
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
		return 0.f; // 기본값 반환
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
		return 0; // 기본값 반환
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
		return false; // 기본값 반환
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
		return false; // 기본값 반환
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
		static unordered_map<string, Parameter> emptyParams; // 빈 맵 반환
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
		static unordered_map<string, Parameter> emptyParams; // 빈 맵 반환
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

	// EXE가 있는 폴더를 기준으로 경로 계산
	filesystem::path exeDir = filesystem::path(exeFullPath).parent_path();
	filesystem::path saveDir = exeDir
		/ "Save"
		/ "AnimationStates"
		/ "AnimControllers";

	// 경로 없으면 생성
	filesystem::create_directories(saveDir);

	json j;
	j["Controllers"] = json::array();

	// 컨트롤러별 파일로 내보내기
	for (auto& [name, pCtrl] : m_AnimControllers)
	{
		if (!pCtrl)
			continue;

		json ctrlJ = pCtrl->Serialize();

		// 파일명: <exeDir>/Bin/Save/AnimationStates/<ControllerName>.json
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

	// 컨트롤러에 연결 되는 오버라이트 컨트롤러 저장

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
	m_pCurrentAnim = nullptr; // 현재 애니메이션 초기화
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

				// 첫 번째 컨트롤러를 current로
				if (!m_pCurAnimController)
					m_pCurAnimController = pCtrl;

				// 현재 애니메이션 컨트롤러의 스테이트들을 불러와서 MaskBone을 사용하면 다 미리 불러두기
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

	// 오버라이드 컨트롤러 컨트롤러에 등록
	for (auto& [name, ctrl] : m_OverrideControllerMap)
	{
		auto it = m_AnimControllers.find(ctrl.controllerName);
		if (it != m_AnimControllers.end())
		{
			it->second->Add_OverrideAnimController(name, ctrl);
		}
	}
}
