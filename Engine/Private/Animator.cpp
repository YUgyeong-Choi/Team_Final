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
	, m_pAnimController{ Prototype.m_pAnimController }
	, m_iCurrentAnimIndex{ Prototype.m_iCurrentAnimIndex }
	, m_iPrevAnimIndex{ Prototype.m_iPrevAnimIndex }
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

	m_pAnimController = CAnimController::Create();
	if (m_pAnimController == nullptr)
		return E_FAIL;
	m_pAnimController->SetAnimator(this);

	return S_OK;
}

void CAnimator::Update(_float fDeltaTime)
{
	if (m_bPlaying == false)
		return;
	if (!m_Blend.active)
	{
		// 블렌드 중이 아니면 그냥 현재 애니메이션만 업데이트
		if (m_pCurrentAnim == nullptr)
			return;
		/*m_bIsFinished =  m_pCurrentAnim->Update_Bones(fDeltaTime, m_Bones, m_Blend.isLoop);*/

		vector<string> triggeredEvents;
		// 수정한 Update_Bones 호출 (outEvents 전달)
		m_bIsFinished = m_pCurrentAnim->Update_Bones(
			fDeltaTime,
			m_Bones,
			m_pCurrentAnim->Get_isLoop(),
			&triggeredEvents
		);

		// 콜백 실행
		for (auto& name : triggeredEvents)
		{
			// 이벤트 리스너에 등록 해둔거에 애니메이션 이벤트가 있는지 찾기
			auto it = m_eventListeners.find(name);
			if (it != m_eventListeners.end()) 
			{
				for (auto& cb : it->second)
					cb(name);
			}
		}
	}
	else
	{
		UpdateBlend(fDeltaTime);
	}
}


void CAnimator::PlayClip(CAnimation* pAnim, _bool isLoop)
{
	if (pAnim == nullptr)
		return;
	m_pCurrentAnim = pAnim;
	m_Blend.active = false;
	m_bPlaying = true;
}

void CAnimator::StartTransition(CAnimation* from, CAnimation* to, _float duration)
{
	m_Blend.active = true;
	m_Blend.srcAnim = from;
	m_Blend.dstAnim = to;
	m_Blend.elapsed = 0.f;
	m_Blend.duration = duration;
	m_Blend.isLoop = to->Get_isLoop();

	// 애니메이션 트랙 초기화
//	m_Blend.srcAnim->ResetTrack();
	m_Blend.dstAnim->ResetTrack();
	m_bPlaying = true;
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

void CAnimator::UpdateBlend(_float fTimeDelta)
{

	vector<string> triggeredEvents;

	//  src / dst 애니메이션을 이벤트 콜백 모드로 업데이트
	m_Blend.srcAnim->Update_Bones(
		fTimeDelta,
		m_Bones,
		m_Blend.srcAnim->Get_isLoop(),
		&triggeredEvents       // 이벤트 수집
	);
	m_Blend.dstAnim->Update_Bones(
		fTimeDelta,
		m_Bones,
		m_Blend.dstAnim->Get_isLoop(),
		&triggeredEvents
	);

	//  수집된 이벤트에 대해 기존과 동일하게 콜백 실행
	for (auto& name : triggeredEvents)
	{
		auto it = m_eventListeners.find(name);
		if (it != m_eventListeners.end())
		{
			for (auto& cb : it->second)
				cb(name);
		}
	}
	m_Blend.elapsed += fTimeDelta;
	_float t = min(m_Blend.elapsed / m_Blend.duration, 1.f);

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

	// 블렌드 완료 시
	if (t >= 1.f)
	{
		m_Blend.active = false;
		m_pCurrentAnim = m_Blend.dstAnim;
		m_Blend.srcAnim->ResetTrack();
	}
}

const string CAnimator::GetCurrentAnimName() const
{
	return m_pCurrentAnim->Get_Name();
}

void CAnimator::RegisterEventListener(const string& eventName, AnimEventCallback cb)
{
	m_eventListeners[eventName].push_back(move(cb));
}

_float CAnimator::GetStateLengthByName(const string& name) const
{
	return m_pAnimController->GetStateLength(name);
}


CAnimator* CAnimator::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAnimator* pInstance = new CAnimator(pDevice,pContext);
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
	Safe_Release(m_pAnimController); // 모델은 다른 곳에서 해제하므로 해제하지 않음
}
