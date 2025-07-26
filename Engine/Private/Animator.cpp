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


	// �׳� ���� �ִϸ��̼� ������Ʈ 
	if (!m_Blend.active)
	{
		// �ִϸ��̼� ��Ʈ�ѷ� ������Ʈ 
		// ��Ʈ�ѷ� ������Ʈ �ϸ鼭 Ʈ������ Ȯ���ؼ� ���� ��ȯ
		// ���� �� �ִϸ��̼� Ŭ�� 2���� ��ȯ �ð� �޾ƿ���

		if (m_pAnimController)
		{
			m_pAnimController->Update(fDeltaTime);
			auto& transitionResult = m_pAnimController->CheckTransition();
			if (transitionResult.bTransition)
			{
				StartTransition(transitionResult.pFromAnim,transitionResult.pToAnim,transitionResult.fDuration);
				m_pAnimController->ResetTransitionResult();
			}
		}
 
		// ���� ���� �ƴϸ� �׳� ���� �ִϸ��̼Ǹ� ������Ʈ
		if (m_pCurrentAnim)
		{


		vector<string> triggeredEvents;
		// Update_Bones ȣ�� (outEvents ����)
		m_bIsFinished = m_pCurrentAnim->Update_Bones(
			fDeltaTime,
			m_Bones,
			m_pCurrentAnim->Get_isLoop(),
			&triggeredEvents
		);

		// �ݹ� ����
		for (auto& name : triggeredEvents)
		{
			// �̺�Ʈ �����ʿ� ��� �صаſ� �ִϸ��̼� �̺�Ʈ�� �ִ��� ã��
			auto it = m_eventListeners.find(name);
			if (it != m_eventListeners.end()) 
			{
				for (auto& cb : it->second)
					cb(name);
			}
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
	if (from == nullptr || to == nullptr)
		return;

	m_Blend.active = true;
	m_Blend.srcAnim = from;
	m_Blend.dstAnim = to;
	m_Blend.elapsed = 0.f;
	m_Blend.duration = duration;
	m_Blend.isLoop = to->Get_isLoop();

	// �ִϸ��̼� Ʈ�� �ʱ�ȭ
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

	//  src / dst �ִϸ��̼��� �̺�Ʈ �ݹ� ���� ������Ʈ
	m_Blend.srcAnim->Update_Bones(
		fTimeDelta,
		m_Bones,
		m_Blend.srcAnim->Get_isLoop(),
		&triggeredEvents       // �̺�Ʈ ����
	);
	m_Blend.dstAnim->Update_Bones(
		fTimeDelta,
		m_Bones,
		m_Blend.dstAnim->Get_isLoop(),
		&triggeredEvents
	);

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

	// ���� �Ϸ� ��
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
	Safe_Release(m_pAnimController); // ���� �ٸ� ������ �����ϹǷ� �������� ����
}

json CAnimator::Serialize()
{
	json j = m_pAnimController->Serialize();

	// �ִϸ������� �Ķ���͵� ����
	for (const auto& [name, param] : m_Params)
	{
		j["Parameters"][name] = {
			{"bValue", param.bValue},
			{"fValue", param.fValue},
			{"iValue", param.iValue},
			{"bTriggered", param.bTriggered},
			{"Type", static_cast<int>(param.type)} // ParamType�� ������ ����
		};
	}
	return j;
}

void CAnimator::Deserialize(const json& j)
{
	// �ִϸ������� �Ķ���͵� ����
	if (j.contains("Parameters"))
	{
		for (const auto& [name, param] : j["Parameters"].items())
		{
			Parameter p;
			p.bValue = param["bValue"].get<_bool>();
			p.fValue = param["fValue"].get<_float>();
			p.iValue = param["iValue"].get<_int>();
			p.bTriggered = param["bTriggered"].get<_bool>();
			p.type = static_cast<ParamType>(param["Type"].get<int>());
			m_Params[name] = p;
		}
	}
	m_pAnimController->Deserialize(j);
}
