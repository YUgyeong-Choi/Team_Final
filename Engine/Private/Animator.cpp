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
	// 디폴트 컨트롤러 생성해서 설정
	m_pCurAnimController->SetAnimator(this);
	m_AnimControllers["Default"] = m_pCurAnimController;
	m_pCurAnimController->SetName(m_pModel->Get_ModelName() + "_Default");
	return S_OK;
}

void CAnimator::Update(_float fDeltaTime)
{
	if (m_bPlaying == false)
		return;


	// 그냥 현재 애니메이션 업데이트 
	if (!m_Blend.active)
	{
		// 애니메이션 컨트롤러 업데이트 
		// 컨트롤러 업데이트 하면서 트랜지션 확인해서 상태 전환
		// 블렌드 할 애니메이션 클립 2개와 전환 시간 받아오기

		if (m_pCurAnimController)
		{
			m_pCurAnimController->Update(fDeltaTime);
			auto& transitionResult = m_pCurAnimController->CheckTransition();
			if (transitionResult.bTransition)
			{
				StartTransition(transitionResult.pFromAnim,transitionResult.pToAnim,transitionResult.fDuration);
				m_pCurAnimController->ResetTransitionResult();
			}
		}
 
		// 블렌드 중이 아니면 그냥 현재 애니메이션만 업데이트
		if (m_pCurrentAnim)
		{


		vector<string> triggeredEvents;
		// Update_Bones 호출 (outEvents 전달)
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

	// 애니메이션 트랙 초기화
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

_float CAnimator::GetStateLengthByName(const string& name) const
{
	return m_pCurAnimController->GetStateLength(name);
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
		static unordered_map<string, Parameter> emptyParams; // 빈 맵 반환
		return emptyParams;
	}
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
	for (auto& Pair : m_AnimControllers)
	{
		Safe_Release(Pair.second);
	}
}

json CAnimator::Serialize()
{
	//json j = m_pAnimController->Serialize();
	//// 애니메이션 컨트롤러 각각의 이름으로 
	//// 애니메이터의 파라미터들 저장
	//for (const auto& [name, param] : m_Params)
	//{
	//	j["Parameters"][name] = {
	//		{"bValue", param.bValue},
	//		{"fValue", param.fValue},
	//		{"iValue", param.iValue},
	//		{"bTriggered", param.bTriggered},
	//		{"Type", static_cast<int>(param.type)} // ParamType을 정수로 저장
	//	};
	//}
	//return j;

	CHAR exeFullPath[MAX_PATH] = {};
	GetModuleFileNameA(nullptr, exeFullPath, MAX_PATH);

	// 2) EXE가 있는 폴더를 기준으로 경로 계산
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
	return j;
}

void CAnimator::Deserialize(const json& j)
{
	m_pCurAnimController = nullptr;
	m_pCurrentAnim = nullptr; // 현재 애니메이션 초기화
	CHAR exeFullPath[MAX_PATH] = {};
	GetModuleFileNameA(nullptr, exeFullPath, MAX_PATH);
	filesystem::path exeDir = filesystem::path(exeFullPath).parent_path();
	auto loadDir = exeDir  / "Save" / "AnimationStates" / "AnimControllers";

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

			// 첫 번째 컨트롤러를 current로
			if (!m_pCurAnimController)
				m_pCurAnimController = pCtrl;
		}
	}
}
