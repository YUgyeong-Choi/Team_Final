#include "Animator.h"
#include "AnimController.h"
#include "Model.h"
#include "Bone.h"

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
	m_AnimControllers["Default"] = m_pCurAnimController;
	m_pCurAnimController->SetName(m_pModel->Get_ModelName() + "_Default");
	return S_OK;
}

void CAnimator::Update(_float fDeltaTime)
{
	if (m_bPlaying == false)
		return;


	if (m_pCurAnimController)
	{
		m_pCurAnimController->Update(fDeltaTime);
		auto& transitionResult = m_pCurAnimController->CheckTransition();
		if (transitionResult.bTransition)
		{
			UpdateMaskState(); // ����ŷ ���� ������Ʈ
			m_Blend.belendFullBody = transitionResult.bBlendFullbody;
			if (m_Blend.belendFullBody == false)
			{
				m_Blend.blendWeight = transitionResult.fBlendWeight; // ���� ����ġ ����
			}
			StartTransition(transitionResult);
		//	StartTransition(transitionResult.pFromAnim, transitionResult.pToAnim, transitionResult.fDuration);
			m_pCurAnimController->ResetTransitionResult();
		}
	}

	//if (m_Blend.active)
	//{
	//	UpdateBlend(fDeltaTime);
	//}
	//else
	//{
	//	if (m_bPlayMask)
	//	{	
	//			
	//		if (m_pUpperClip && m_pLowerClip)
	//		{
	//			vector<string> triggeredEvents;
	//			// ��ü �������� �������� �Ǵ��ϱ� 
	//			 m_pLowerClip->Update_Bones(fDeltaTime, m_Bones, m_pLowerClip->Get_isLoop(), &triggeredEvents);

	//			 m_bIsFinished= m_pUpperClip->Update_Bones(fDeltaTime, m_Bones, m_pUpperClip->Get_isLoop(), &triggeredEvents);
	//			_float t =1.f; // ��ü ���� ����

	//			for (size_t i = 0; i < m_Bones.size(); ++i)
	//			{
	//				_matrix srcM = m_pLowerClip->GetBoneMatrix(static_cast<_uint>(i));
	//				_matrix dstM = m_pUpperClip->GetBoneMatrix(static_cast<_uint>(i));

	//				_vector sS, sR, sT, dS, dR, dT;
	//				XMMatrixDecompose(&sS, &sR, &sT, srcM);
	//				XMMatrixDecompose(&dS, &dR, &dT, dstM);

	//				_vector bS = XMVectorLerp(sS, dS, t);
	//				_vector bR = XMQuaternionSlerp(sR, dR, t);
	//				_vector bT = XMVectorLerp(sT, dT, t);


	//				if (m_UpperMaskSet.count(static_cast<_int>(i)))
	//				{
	//					_matrix M = XMMatrixScalingFromVector(bS)
	//						* XMMatrixRotationQuaternion(bR)
	//						* XMMatrixTranslationFromVector(bT);
	//					m_Bones[i]->Set_TransformationMatrix(M);
	//				}
	//				else
	//				{
	//					m_Bones[i]->Set_TransformationMatrix(srcM);
	//				}
	//			}
	//			for (auto& name : triggeredEvents)
	//				for (auto& cb : m_eventListeners[name])
	//					cb(name);
	//		}
	//	}
	//	else
	//	{
	//		if (m_pCurrentAnim)
	//		{
	//			if (m_pUpperClip)
	//			{
	//				// ��ü���� ��ü��
	//				StartTransition(m_pUpperClip,m_pCurrentAnim, 0.2f);
	//				m_bPlayMask = false;
	//			}

	//			vector<string> triggeredEvents;
	//			// Update_Bones ȣ�� (outEvents ����)
	//			m_bIsFinished = m_pCurrentAnim->Update_Bones(
	//				fDeltaTime,
	//				m_Bones,
	//				m_pCurrentAnim->Get_isLoop(),
	//				&triggeredEvents
	//			);

	//			// �ݹ� ����
	//			for (auto& name : triggeredEvents)
	//			{
	//				// �̺�Ʈ �����ʿ� ��� �صаſ� �ִϸ��̼� �̺�Ʈ�� �ִ��� ã��
	//				auto it = m_eventListeners.find(name);
	//				if (it != m_eventListeners.end())
	//				{
	//					for (auto& cb : it->second)
	//						cb(name);
	//				}
	//			}
	//		}
	//	}
	//}


	vector<string> triggeredEvents; // �̺�Ʈ�� �� ���� ����
	size_t boneCount = m_Bones.size();

	if (m_Blend.active) //  ���� ��
	{
	/*	m_pBlendFromLowerAnim->Update_Bones(fDeltaTime, m_Bones, m_pBlendFromLowerAnim->Get_isLoop(), &triggeredEvents);
		m_pBlendToLowerAnim->Update_Bones(fDeltaTime, m_Bones, m_pBlendToLowerAnim->Get_isLoop(), &triggeredEvents);
		if (m_pBlendFromUpperAnim) m_pBlendFromUpperAnim->Update_Bones(fDeltaTime, m_Bones, m_pBlendFromUpperAnim->Get_isLoop(), &triggeredEvents);
		if (m_pBlendToUpperAnim)   m_pBlendToUpperAnim->Update_Bones(fDeltaTime, m_Bones, m_pBlendToUpperAnim->Get_isLoop(), &triggeredEvents);*/

		if (m_pBlendFromLowerAnim && m_pBlendFromLowerAnim != m_pBlendToLowerAnim) // ��ü �ִϸ��̼��� �ٸ���
			m_pBlendFromLowerAnim->Update_Bones(fDeltaTime, m_Bones, m_pBlendFromLowerAnim->Get_isLoop(), &triggeredEvents);

		if (m_pBlendToLowerAnim) // Ÿ�� ��ü �ִϸ��̼�
			m_pBlendToLowerAnim->Update_Bones(fDeltaTime, m_Bones, m_pBlendToLowerAnim->Get_isLoop(), &triggeredEvents);

		if (m_pBlendFromUpperAnim && m_pBlendFromUpperAnim != m_pBlendToUpperAnim) // ��ü �ִϸ��̼��� �ٸ���
			m_pBlendFromUpperAnim->Update_Bones(fDeltaTime, m_Bones, m_pBlendFromUpperAnim->Get_isLoop(), &triggeredEvents);

		if (m_pBlendToUpperAnim) // Ÿ�� ��ü �ִϸ��̼�
			m_pBlendToUpperAnim->Update_Bones(fDeltaTime, m_Bones, m_pBlendToUpperAnim->Get_isLoop(), &triggeredEvents);
		// ��Ʈ���� �̸� ��������
		vector<_matrix> fromL(boneCount), toL(boneCount), fromU(boneCount), toU(boneCount);
		for (size_t i = 0; i < boneCount; ++i)
		{
			// nullptr �˻�
			if(m_pBlendFromLowerAnim)
				fromL[i] = m_pBlendFromLowerAnim->GetBoneMatrix((unsigned)i);
			if (m_pBlendToLowerAnim)
				toL[i] = m_pBlendToLowerAnim->GetBoneMatrix((unsigned)i);
			if (m_pBlendFromUpperAnim)
				fromU[i] = m_pBlendFromUpperAnim ? m_pBlendFromUpperAnim->GetBoneMatrix((unsigned)i) : _matrix{};
			if (m_pBlendToUpperAnim)
				toU[i] = m_pBlendToUpperAnim ? m_pBlendToUpperAnim->GetBoneMatrix((unsigned)i) : _matrix{};
		}

		//����
		m_Blend.elapsed += fDeltaTime;
		_float fBlendFactor = min(m_Blend.elapsed / m_Blend.duration, 1.f);
		// �ٵ� ���� ����ü �и� ����� ���� ����Ʈ��
		if (m_Blend.belendFullBody == false)
		{
			fBlendFactor = m_Blend.blendWeight;
		}

		for (size_t i = 0; i < boneCount; ++i)
		{
			bool isUpperBone = (m_UpperMaskSet.count((int)i) != 0);
			_matrix finalM;

			using ET = CAnimController::ETransitionType;
			switch (m_eCurrentTransitionType)
			{
			case ET::FullbodyToFullbody:
				finalM = LerpMatrix(fromL[i], toL[i], fBlendFactor);
				break;

			case ET::FullbodyToMasked:
				finalM = isUpperBone
					? LerpMatrix(fromL[i], toU[i], fBlendFactor)
					: LerpMatrix(fromL[i], toL[i], fBlendFactor);
				break;

			case ET::MaskedToFullbody:
				finalM = isUpperBone
					? LerpMatrix(fromU[i], toL[i], fBlendFactor)
					: LerpMatrix(fromL[i], toL[i], fBlendFactor);
				break;

			case ET::MaskedToMasked:
				finalM = isUpperBone
					? LerpMatrix(fromU[i], toU[i], fBlendFactor)
					: LerpMatrix(fromL[i], toL[i], fBlendFactor);
				break;
			}

			m_Bones[i]->Set_TransformationMatrix(finalM);
		}

		//  ���� �������� ����
		if (fBlendFactor >= 1.f)
		{
			m_Blend.active = false;
			//if (m_pBlendFromLowerAnim) m_pBlendFromLowerAnim->ResetTrack();
		//	if (m_pBlendToLowerAnim)   m_pBlendToLowerAnim->ResetTrack();
		////	if (m_pBlendFromUpperAnim) m_pBlendFromUpperAnim->ResetTrack();
		//	if (m_pBlendToUpperAnim&&m_pBlendToUpperAnim!= m_pBlendToLowerAnim)   m_pBlendToUpperAnim->ResetTrack();
			if (m_pBlendFromLowerAnim && m_pBlendFromLowerAnim != m_pBlendToLowerAnim)
				m_pBlendFromLowerAnim->ResetTrack();
			if (m_pBlendFromUpperAnim && m_pBlendFromUpperAnim != m_pBlendToUpperAnim)
				m_pBlendFromUpperAnim->ResetTrack();
			m_pCurrentAnim = m_pBlendToLowerAnim;
			m_pLowerClip = m_pBlendToLowerAnim;
			m_pUpperClip = m_pBlendToUpperAnim;
			UpdateMaskState();
		}
	}
	else // ���尡 �ƴ� ��
	{
		if (m_bPlayMask && m_pLowerClip && m_pUpperClip)
		{
			// ��ü/��ü ������Ʈ
			m_bIsFinished = m_pLowerClip->Update_Bones(fDeltaTime, m_Bones, m_pLowerClip->Get_isLoop(), &triggeredEvents);
			m_pUpperClip->Update_Bones(fDeltaTime, m_Bones, m_pUpperClip->Get_isLoop(), &triggeredEvents);

			// ��Ʈ���� �̸� ��������
			vector<_matrix> lowerM(boneCount), upperM(boneCount);
			for (size_t i = 0; i < boneCount; ++i)
			{
				lowerM[i] = m_pLowerClip->GetBoneMatrix((unsigned)i);
				upperM[i] = m_pUpperClip->GetBoneMatrix((unsigned)i);
			}

			// ���¿� ������ ����ġ�� ����
			_float fWeight = m_pCurAnimController->GetCurrentState()->fBlendWeight;
			for (size_t i = 0; i < boneCount; ++i)
			{
				if (m_UpperMaskSet.count((int)i))
					m_Bones[i]->Set_TransformationMatrix(LerpMatrix(lowerM[i], upperM[i], fWeight));
				else
					m_Bones[i]->Set_TransformationMatrix(lowerM[i]);
			}
		}
		else if (m_pCurrentAnim)
		{
			// ���� ��ü �ִϸ��̼�
			m_bIsFinished = m_pCurrentAnim->Update_Bones(fDeltaTime, m_Bones, m_pCurrentAnim->Get_isLoop(), &triggeredEvents);
		}
	}


	for (auto& name : triggeredEvents)
	{
		auto it = m_eventListeners.find(name);
		if (it != m_eventListeners.end())
		{
			for (auto& cb : it->second)
				cb(name);
		}
	}


		//auto curAnimState = m_pCurAnimController->GetCurrentState();
		//
		//if (m_Blend.active && false == curAnimState->maskBoneName.empty())
		//{
		//	vector<string> triggeredEvents;
		//	// Update_Bones ȣ�� (outEvents ����)
		//	m_bIsFinished = m_pCurrentAnim->Update_Bones(
		//		fDeltaTime,
		//		m_Bones,
		//		m_pCurrentAnim->Get_isLoop(),
		//		&triggeredEvents
		//	);

		//	CAnimation* pUpper = m_pModel->GetAnimationClipByName(curAnimState->upperClipName);

		//	vector<CBone*> tmpBones = m_Bones;
		//	pUpper->Update_Bones(fDeltaTime, tmpBones, pUpper->Get_isLoop(), &triggeredEvents);

		//	for (auto i : m_UpperMaskSet)
		//		m_Bones[i]->Set_TransformationMatrix(XMLoadFloat4x4(tmpBones[i]->Get_TransformationMatrix()));

		//	for (auto& name : triggeredEvents)
		//		for (auto& cb : m_eventListeners[name])
		//			cb(name);
		//}
		//else if (!m_Blend.active && true  == curAnimState->maskBoneName.empty())
		//{
		//	// �ִϸ��̼� ��Ʈ�ѷ� ������Ʈ 
		//	// ��Ʈ�ѷ� ������Ʈ �ϸ鼭 Ʈ������ Ȯ���ؼ� ���� ��ȯ
		//	// ���� �� �ִϸ��̼� Ŭ�� 2���� ��ȯ �ð� �޾ƿ���

		//	if (m_pCurAnimController)
		//	{
		//		m_pCurAnimController->Update(fDeltaTime);
		//		auto& transitionResult = m_pCurAnimController->CheckTransition();
		//		if (transitionResult.bTransition)
		//		{
		//			StartTransition(transitionResult.pFromAnim,transitionResult.pToAnim,transitionResult.fDuration);
		//			m_pCurAnimController->ResetTransitionResult();
		//		}
		//	}
	 //
		//	// ���� ���� �ƴϸ� �׳� ���� �ִϸ��̼Ǹ� ������Ʈ
		//	if (m_pCurrentAnim)
		//	{


		//	vector<string> triggeredEvents;
		//	// Update_Bones ȣ�� (outEvents ����)
		//	m_bIsFinished = m_pCurrentAnim->Update_Bones(
		//		fDeltaTime,
		//		m_Bones,
		//		m_pCurrentAnim->Get_isLoop(),
		//		&triggeredEvents
		//	);

		//	// �ݹ� ����
		//	for (auto& name : triggeredEvents)
		//	{
		//		// �̺�Ʈ �����ʿ� ��� �صаſ� �ִϸ��̼� �̺�Ʈ�� �ִ��� ã��
		//		auto it = m_eventListeners.find(name);
		//		if (it != m_eventListeners.end()) 
		//		{
		//			for (auto& cb : it->second)
		//				cb(name);
		//		}
		//	}
		//	}
		//}
		//else
		//{
		//	UpdateBlend(fDeltaTime);
		//}
	}


	void CAnimator::PlayClip(CAnimation * pAnim, _bool isLoop)
	{
		if (pAnim == nullptr)
			return;
		m_pCurrentAnim = pAnim;
		m_Blend.active = false;
		m_bPlaying = true;
		UpdateMaskState();
	}

	void CAnimator::StartTransition(CAnimation * from, CAnimation * to, _float duration)
	{
		if (from == nullptr || to == nullptr)
			return;

		m_Blend.active = true;
		m_Blend.srcAnim = from;
		m_Blend.dstAnim = to;
		m_Blend.elapsed = 0.f;
		m_Blend.duration = duration;
		m_Blend.isLoop = to->Get_isLoop();
		m_bPlaying = true;
	}

	void CAnimator::StartTransition(CAnimController::TransitionResult& transitionResult)
	{
		if (m_Blend.active) {
			if (m_Blend.srcAnim) m_Blend.srcAnim->ResetTrack();
			if (m_Blend.dstAnim) m_Blend.dstAnim->ResetTrack();
		}
		if (m_pBlendFromUpperAnim) m_pBlendFromUpperAnim->ResetTrack();
		if (m_pBlendToUpperAnim) m_pBlendToUpperAnim->ResetTrack();

		m_Blend.active = true;
		m_Blend.elapsed = 0.f;
		m_Blend.duration = transitionResult.fDuration;

		// ������ ���� Ŭ��
		m_pBlendFromLowerAnim = transitionResult.pFromLowerAnim;
		m_pBlendToLowerAnim = transitionResult.pToLowerAnim;
		m_pBlendFromUpperAnim = transitionResult.pFromUpperAnim;
		m_pBlendToUpperAnim = transitionResult.pToUpperAnim;
		m_eCurrentTransitionType = transitionResult.eType; // ���� ��ȯ Ÿ�� ����
		m_Blend.blendWeight = transitionResult.fBlendWeight; // ����ü ���� ����ġ 

		m_bPlaying = true;

		// �� �������� CAnimController�� m_CurrentStateNodeId�� �̹� ����.
		UpdateMaskState();
	}

	void CAnimator::Set_Animation(_uint iIndex, _float fadeDuration, _bool isLoop)
	{
		if (m_pModel && iIndex >= m_pModel->Get_NumAnimations())
			return;
		m_iPrevAnimIndex = m_iCurrentAnimIndex;

		if (m_iPrevAnimIndex != iIndex)
		{
			StartTransition(
				m_pModel->GetAnimationClip(m_iPrevAnimIndex),
				m_pModel->GetAnimationClip(iIndex),
				fadeDuration);
			m_iCurrentAnimIndex = iIndex;
			return;
		}

		m_pCurrentAnim = m_pModel->GetAnimationClip(iIndex);
		m_Blend.active = false;
	}

	void CAnimator::UpdateBlend(_float fDeltaTime)
	{

		vector<string> triggeredEvents;

		m_Blend.srcAnim->Update_Bones(
			fDeltaTime,
			m_Bones,
			m_Blend.srcAnim->Get_isLoop(),
			&triggeredEvents       // �̺�Ʈ ����
		);
		m_Blend.dstAnim->Update_Bones(
			fDeltaTime,
			m_Bones,
			m_Blend.dstAnim->Get_isLoop(),
			&triggeredEvents
		);



		m_Blend.elapsed += fDeltaTime;
		_float t = min(m_Blend.elapsed / m_Blend.duration, 1.f);


		if (m_bSeparateUpperLowerBlend)
		{
			t = m_Blend.blendWeight; // ���� ����ġ ��� (����Ƽ�� ���̾�ó��)
			for (size_t i = 0; i < m_Bones.size(); ++i)
			{
				_matrix srcM = m_Blend.srcAnim->GetBoneMatrix(static_cast<_uint>(i));
				_matrix dstM = m_Blend.dstAnim->GetBoneMatrix(static_cast<_uint>(i));

				_vector sS, sR, sT, dS, dR, dT;
				XMMatrixDecompose(&sS, &sR, &sT, srcM);
				XMMatrixDecompose(&dS, &dR, &dT, dstM);

				_vector bS = XMVectorLerp(sS, dS, t);
				_vector bR = XMQuaternionSlerp(sR, dR, t);
				_vector bT = XMVectorLerp(sT, dT, t);


				if (m_UpperMaskSet.count(static_cast<_int>(i)))
				{
					_matrix M = XMMatrixScalingFromVector(bS)
						* XMMatrixRotationQuaternion(bR)
						* XMMatrixTranslationFromVector(bT);
					m_Bones[i]->Set_TransformationMatrix(M);
				}
				else
				{
					m_Bones[i]->Set_TransformationMatrix(srcM);
				}
			}
		}
		else
		{
			for (size_t i = 0; i < m_Bones.size(); ++i)
			{
				_matrix srcM = m_Blend.srcAnim->GetBoneMatrix(static_cast<_uint>(i));
				_matrix dstM = m_Blend.dstAnim->GetBoneMatrix(static_cast<_uint>(i));

				_vector sS, sR, sT, dS, dR, dT;
				XMMatrixDecompose(&sS, &sR, &sT, srcM);
				XMMatrixDecompose(&dS, &dR, &dT, dstM);

				_vector bS = XMVectorLerp(sS, dS, t);
				_vector bR = XMQuaternionSlerp(sR, dR, t);
				_vector bT = XMVectorLerp(sT, dT, t);

				_matrix M = XMMatrixScalingFromVector(bS)
					* XMMatrixRotationQuaternion(bR)
					* XMMatrixTranslationFromVector(bT);

				m_Bones[i]->Set_TransformationMatrix(M);
			}
		}
	

		//  ������ �̺�Ʈ�� ���� ������ �����ϰ� �ݹ� ����
		for (auto& name : triggeredEvents)
		{
			auto it = m_eventListeners.find(name);
			if (it != m_eventListeners.end())
			{
				for (auto& cb : it->second)
					cb(name);
			}
		}



		// ���� �Ϸ� ��
		if (t >= 1.f)
		{
			if (m_pUpperClip)
			{
				m_pUpperClip->ResetTrack();
				m_pUpperClip = nullptr;
			}
			m_bSeparateUpperLowerBlend = false;
			m_Blend.active = false;
			m_pCurrentAnim = m_Blend.dstAnim;
			m_Blend.srcAnim->ResetTrack();
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
			if (m_pLowerClip&&m_pLowerClip->GetCurrentTrackPosition() >= m_pLowerClip->GetDuration())
			{
				m_pLowerClip->ResetTrack();
			}
			if (m_pUpperClip&&m_pUpperClip->GetCurrentTrackPosition() >= m_pUpperClip->GetDuration())
			{
				m_pUpperClip->ResetTrack();
			}

			MakeMaskBones(curState -> maskBoneName);
		}
		else
		{
			m_bPlayMask = false;
			m_pLowerClip = nullptr;
			m_pUpperClip = nullptr;
			m_UpperMaskSet.clear();
		}
	}

	void CAnimator::MakeMaskBones(const string & maskBoneName)
	{
		//m_UpperMaskSet.clear();

		//auto it = find_if(
		//		m_Bones.begin(), m_Bones.end(),
		//	[&](CBone* pBone) { 
		//		return pBone->Compare_Name(maskBoneName.c_str()); });

		//if (it != m_Bones.end())
		//{
		//	CBone* pMaskBone = *it;
		//	m_UpperMaskSet.insert(pMaskBone->Get_BoneIndex());
		//	// ���� ������� ��� �߰�
		//	while (pMaskBone->Get_ParentBoneIndex() != -1)
		//	{
		//		pMaskBone = m_Bones[pMaskBone->Get_ParentBoneIndex()];
		//		m_UpperMaskSet.insert(pMaskBone->Get_BoneIndex());
		//	}
		//}

		m_UpperMaskSet.clear();

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
		// ��� �� �̸� ����ؼ� ��Ȯ�� �̸� Ȯ��
	}

	void CAnimator::CollectBoneChildren(const _char * boneName)
	{
		_int idx = m_pModel->Find_BoneIndex(boneName);
		if (idx < 0 || m_UpperMaskSet.count(idx) || m_Bones.size() <= idx)
			return;

		cout << "Collecting bone: " << boneName << endl;
		m_UpperMaskSet.insert(idx);

		// �� ���� ��� �ڽ� �̸��� �����ͼ� ���
		for (int childIdx : m_pModel->GetBoneChildren(boneName))
		{
			const char* childName = m_pModel->Get_Bones()[childIdx]->Get_Name();
			cout << "Child bone: " << childName << endl;
			CollectBoneChildren(childName);
		}
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



	const string CAnimator::GetCurrentAnimName() const
	{
		return m_pCurrentAnim->Get_Name();
	}

	void CAnimator::RegisterEventListener(const string & eventName, AnimEventCallback cb)
	{
		m_eventListeners[eventName].push_back(move(cb));
	}

	void CAnimator::RegisterAnimController(const string & name, CAnimController * pController)
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

	void CAnimator::UnregisterAnimController(const string & name)
	{
		auto it = m_AnimControllers.find(name);
		if (it != m_AnimControllers.end())
		{
			Safe_Release(it->second);
			m_AnimControllers.erase(it);
		}
	}

	void CAnimator::RenameAnimController(const string & oldName, const string & newName)
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

	void CAnimator::SetCurrentAnimController(const string & name, const string & stateName)
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
					m_pBlendFromLowerAnim = m_pModel->GetAnimationClipByName(pFromAnimState->lowerClipName);
					m_pBlendFromUpperAnim = m_pModel->GetAnimationClipByName(pFromAnimState->upperClipName);
				}
				else // Fullbody
				{
					m_pBlendFromLowerAnim = m_pCurrentAnim; // ���� ��� ���� Fullbody �ִ�
					m_pBlendFromUpperAnim = m_pCurrentAnim; // Fullbody�� ��ü�� ���� �ִ�
				}

				// To �ִϸ��̼� ����
				if (bToIsMasked)
				{
					m_pBlendToLowerAnim = m_pModel->GetAnimationClipByName(pToAnimState->lowerClipName);
					m_pBlendToUpperAnim = m_pModel->GetAnimationClipByName(pToAnimState->upperClipName);
				}
				else // Fullbody
				{
					m_pBlendToLowerAnim = pToAnimState->clip; // Fullbody�� clip ���
					m_pBlendToUpperAnim = pToAnimState->clip; // Fullbody�� ��ü�� ���� �ִ�
				}

				CAnimController::TransitionResult transitionResult{};
				transitionResult.pFromLowerAnim = m_pBlendFromLowerAnim;
				transitionResult.pToLowerAnim = m_pBlendToLowerAnim;
				transitionResult.pFromUpperAnim = m_pBlendFromUpperAnim;
				transitionResult.pToUpperAnim = m_pBlendToUpperAnim;

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

	_float CAnimator::GetStateLengthByName(const string & name) const
	{
		return m_pCurAnimController->GetStateLength(name);
	}

	_bool CAnimator::ExisitsParameter(const string & name) const
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

	void CAnimator::AddBool(const string & name)
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

	void CAnimator::AddFloat(const string & name)
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

	void CAnimator::AddTrigger(const string & name)
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

	void CAnimator::AddInt(const string & name)
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

	void CAnimator::SetInt(const string & name, _int v)
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

	void CAnimator::SetBool(const string & name, _bool v)
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

	void CAnimator::SetFloat(const string & name, _float v)
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

	void CAnimator::SetTrigger(const string & name)
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

	void CAnimator::ResetTrigger(const string & name)
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

	void CAnimator::DeleteParameter(const string & name)
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

	void CAnimator::SetParamName(Parameter & param, const string & name)
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

	_float CAnimator::GetFloat(const string & name) const
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

	_int CAnimator::GetInt(const string & name) const
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

	_bool CAnimator::CheckBool(const string & name) const
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

	_bool CAnimator::CheckTrigger(const string & name)
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

	void CAnimator::AddParameter(const string & name, Parameter & parm)
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

	unordered_map<string, Parameter>& CAnimator::GetParameters()
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

	CAnimator* CAnimator::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
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
	}

	json CAnimator::Serialize()
	{
		//json j = m_pAnimController->Serialize();
		//// �ִϸ��̼� ��Ʈ�ѷ� ������ �̸����� 
		//// �ִϸ������� �Ķ���͵� ����
		//for (const auto& [name, param] : m_Params)
		//{
		//	j["Parameters"][name] = {
		//		{"bValue", param.bValue},
		//		{"fValue", param.fValue},
		//		{"iValue", param.iValue},
		//		{"bTriggered", param.bTriggered},
		//		{"Type", static_cast<int>(param.type)} // ParamType�� ������ ����
		//	};
		//}
		//return j;

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
		return j;
	}

	void CAnimator::Deserialize(const json & j)
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
				pCtrl->SetAnimator(this);
				pCtrl->Deserialize(ctrlJ);
				m_AnimControllers[name] = pCtrl;

				// ù ��° ��Ʈ�ѷ��� current��
				if (!m_pCurAnimController)
					m_pCurAnimController = pCtrl;

				// ���� �ִϸ��̼� ��Ʈ�ѷ��� ������Ʈ���� �ҷ��ͼ� MaskBone�� ����ϸ� �� �̸� �ҷ��α�
				auto& states = pCtrl->GetStates();

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
