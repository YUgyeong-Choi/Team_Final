#include "Bone.h"
#include "EventMag.h"
#include "AnimTool.h"
#include "Animator.h"
#include "Animation.h"
#include "AnimController.h"
#include "GameInstance.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감

CAnimTool::CAnimTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
	, m_pGameInstance(CGameInstance::Get_Instance())
	, m_pEventMag(CEventMag::Get_Instance())
{
	Safe_AddRef(m_pGameInstance);
}

CAnimTool::CAnimTool(const CAnimTool& Prototype)
	: CGameObject(Prototype)
	, m_pGameInstance(CGameInstance::Get_Instance())
	, m_pEventMag(CEventMag::Get_Instance())
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CAnimTool::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CAnimTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"), TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pAnimShader))))
		return E_FAIL;

	_matrix ModelWorldMatrix = XMMatrixIdentity();
	XMStoreFloat4x4(&m_ModelWorldMatrix, ModelWorldMatrix);

	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));

	m_pMySequence = new CMySequence();


	return S_OK;
}

void CAnimTool::Priority_Update(_float fTimeDelta)
{

}

void CAnimTool::Update(_float fTimeDelta)
{
	UpdateCurrentModel(fTimeDelta);

}

void CAnimTool::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
	m_bRenerLevel = false;
}

HRESULT CAnimTool::Render()
{
	if (m_bRenerLevel)
	{
		if (FAILED(Render_Load_Model()))
			return E_FAIL;

		Setting_Sequence();
		if (m_bUseAnimSequence)
		{
			if (FAILED(Render_AnimationSequence()))
				return E_FAIL;
			if (FAILED(Render_AnimEvents()))
				return E_FAIL;
		}
		if (FAILED(Render_AnimStatesByNode()))
			return E_FAIL;
	}
	else
	{
		m_bRenerLevel = true;
	}

	if (FAILED(Bind_Shader()))
		return E_FAIL;


	return S_OK;
}

HRESULT CAnimTool::Render_Load_Model()
{
	_bool open = true;
	Begin("Load Model", &open, NULL);
	ImGui::Checkbox("Load Model", &m_bActiveLoadModel);
	if (!m_bActiveLoadModel)
	{
		ImGui::End();
		return S_OK;
	}

	IGFD::FileDialogConfig config;
	if (Button(u8"파일 경로"))
	{
		config.path = R"(../Bin/Resources/Models/Bin_Anim)";
		config.countSelectionMax = 0; // 하나만 선택 가능
		IFILEDIALOG->OpenDialog("Model Dialog", "Select Model File", ".bin", config);
	}

	if (IFILEDIALOG->Display("Model Dialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			auto selections = IFILEDIALOG->GetSelection();
			// 처리
			// first: 파일명.확장자
			// second: 전체 경로 (파일명포함)
			if (!selections.empty())
			{
				for (const auto& FilePath : selections)
				{
					CreateModel(FilePath.first, FilePath.second); // 전체 경로로 모델 생성

				}
			}
		}
		IFILEDIALOG->Close();
	}



	if (FAILED(Render_Loaded_Models()))
	{
		ImGui::End();
		return E_FAIL;
	}

	ImGui::End();
	return S_OK;
}

HRESULT CAnimTool::Render_AnimEvents()
{
	if (m_pCurAnimation == nullptr || m_pCurAnimator == nullptr || m_pMySequence == nullptr)
	{
		return S_OK;
	}

	static _int   selectedListenerIdx = 0;
	_float fCurTarckPos = m_pCurAnimation->GetCurrentTrackPosition();
	_float fDuration = m_pCurAnimation->GetDuration();
	ImGui::Separator();

	if (ImGui::Button("Add Manual Event"))
	{
		m_pCurAnimation->AddEvent({ fCurTarckPos, "NewEvent" });
	}

	auto& events = m_pCurAnimation->GetEvents();
	for (_int i = 0; i < (_int)events.size(); ++i)
	{
		auto& ev = events[i];
		ImGui::PushID(i);
		char buf[64];
		strncpy_s(buf, ev.name.c_str(), sizeof(buf));
		if (ImGui::InputText("Name", buf, sizeof(buf)))
			ev.name = buf;
		ImGui::Separator();
		if (ImGui::DragFloat("Time", &ev.fTime, 0.01f, 0.f, fDuration, "%.2f"))
			ImGui::SameLine();
		if (ImGui::Button("Remove"))
		{
			events.erase(events.begin() + i);
			ImGui::PopID();
			break;
		}
		ImGui::PopID();
		ImGui::Separator();
	}

	const auto& listeners = m_pCurAnimator->GetEventListeners();
	vector<const char*> listenerNames;
	listenerNames.reserve(listeners.size());
	for (auto& kv : listeners)
		listenerNames.push_back(kv.first.c_str());

	if (!listenerNames.empty())
	{
		ImGui::Text("Available Animator Events:");
		ImGui::Combo("##listener_combo", &selectedListenerIdx,
			listenerNames.data(), (int)listenerNames.size());
		ImGui::SameLine();
		if (ImGui::Button("Assign To Anim"))
		{
			m_pCurAnimation->AddEvent({ fCurTarckPos, listenerNames[selectedListenerIdx] });
		}
		ImGui::Separator();
	}
	if (ImGui::Button("Save All Clips Events to JSON"))
	{
		SaveLoadEvents();
	}

	if (ImGui::Button("Load All Clips Events from JSON"))
	{
		SaveLoadEvents(false);
	}
	return S_OK;
}

HRESULT CAnimTool::Render_AnimationSequence()
{
	if (m_pCurAnimator == nullptr || m_pCurAnimation == nullptr || m_pMySequence == nullptr)
	{
		return S_OK;
	}

	static _int           selectedEntry = -1;
	static _bool          expanded = true;// 트랙 확장 여부
	static const _float FRAME = 60.f; // 1초당 60프레임 기준

	ImGui::Begin("Animation Sequence");
	m_bIsPlaying = m_pCurAnimator->IsPlaying();
	if (m_bIsPlaying)
	{
		if (ImGui::Button("Stop"))
		{
			m_bIsPlaying = false;
			if (m_pCurAnimator)
			{
				m_pCurAnimator->StopAnimation();
			}
		}
	}
	else
	{
		if (ImGui::Button("Play"))
		{
			m_bIsPlaying = true;
			if (m_pCurAnimator)
			{
				m_pCurAnimator->PlayClip(m_pCurAnimation, false);
			}
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{
		m_iSequenceFrame = m_pMySequence->GetFrameMin();
		if (m_pCurAnimator)
		{
			m_pCurAnimator->GetCurrentAnim()->ResetTrack();
			m_pCurAnimator->SetPlaying(true);
		}
	}

	// 재생 속도 조절
	ImGui::SameLine();
	ImGui::SliderInt("Speed", &m_playSpeed, 1, 10);


	_bool bChanged = ImSequencer::Sequencer(
		m_pMySequence,
		&m_iSequenceFrame,
		&expanded,
		&selectedEntry,
		&m_iFirstFrame,
		ImSequencer::SEQUENCER_EDIT_ALL | ImSequencer::SEQUENCER_CHANGE_FRAME
	);

	if (!m_bIsPlaying)
	{
		auto pCurAnim = m_pCurAnimator->GetCurrentAnim();
		if (pCurAnim)
		{
			pCurAnim->SetCurrentTrackPosition(static_cast<_float>(m_iSequenceFrame)); // 프레임을 초 단위로 변환
			m_pCurAnimator->SetPlaying(true);
			m_pCurAnimator->Update(0.f);
			m_pCurAnimator->StopAnimation(); // 다시 pause
		}
	}


	ImGui::End();
	return S_OK;
}

HRESULT CAnimTool::Render_AnimStatesByNode()
{
	if (m_pCurAnimator == nullptr || m_pCurAnimator->GetAnimController() == nullptr)
	{
		return S_OK;
	}
	static _int g_nodeID = 100; // 전역 노드로 ID 증가용으로 사용 
	static _int iIndex = 0;
	static _int selectedNodeID = -1;  // 선택한 노드 아이디
	CAnimController* pCtrl = m_pCurAnimator->GetAnimController();

	ImGui::Begin("Anim State Machine");

	ImNodes::BeginNodeEditor();

	// 우클릭으로 상대 추가 팝업
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImNodes::IsEditorHovered())
	{
		ImGui::OpenPopup("AddStatePopup");
	}

	if (ImGui::BeginPopup("AddStatePopup"))
	{
		static char stateName[64] = "NewState";

		ImGui::InputText("State Name", stateName, IM_ARRAYSIZE(stateName));

		if (ImGui::Button("Add State"))
		{
			const ImVec2 mousePos = ImNodes::EditorContextGetPanning(); // 현재 마우스 위치
			CAnimation* selectedAnim = m_pCurAnimation;

			size_t newIdx = pCtrl->AddState(stateName, selectedAnim, iIndex++);
			auto& newState = pCtrl->GetStates()[newIdx];
			newState.iNodeId = g_nodeID++;
			newState.fNodePos = { mousePos.x, mousePos.y };


			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	for (auto& state : pCtrl->GetStates())
	{
		ImNodes::BeginNode(state.iNodeId);

		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted(state.stateName.c_str());
		ImNodes::EndNodeTitleBar();

		// 노드 아이디 
		ImNodes::BeginInputAttribute(state.iNodeId * 10 + 1);
		ImGui::Text("In");
		ImNodes::EndInputAttribute();

		ImNodes::BeginOutputAttribute(state.iNodeId * 10 + 2);
		ImGui::Text("Out");
		ImNodes::EndOutputAttribute();

		ImNodes::EndNode();

		/*if (state.fNodePos.x == 0.f && state.fNodePos.y == 0.f)
		{
			ImVec2 initPos = ImNodes::EditorContextGetPanning();
			ImNodes::SetNodeEditorSpacePos(state.iNodeId, initPos);

			state.fNodePos = { initPos.x, initPos.y };
		}
		else
		{
	
		}*/
		
		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(state.iNodeId);
		state.fNodePos = { pos.x, pos.y };
	}

	
	ImNodes::EndNodeEditor();
	_int hoveredNodeID = -1;
	if (ImNodes::IsNodeHovered(&hoveredNodeID) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		selectedNodeID = hoveredNodeID;
	}
	ImGui::End();


	if (selectedNodeID != -1)
	{
		for (auto& state : pCtrl->GetStates())
		{
			if (state.iNodeId == selectedNodeID)
			{
				ImGui::Begin("Anim Info");
				ImGui::Text("State: %s", state.stateName.c_str());
				if (state.clip)
				{
					ImGui::Text("Clip: %s", state.clip->Get_Name().c_str());
					ImGui::Text("Duration: %.2f", state.clip->GetDuration());
					ImGui::Text("Current Track Position: %.2f", state.clip->GetCurrentTrackPosition());
					ImGui::Text("Tick Per Second: %.2f", state.clip->GetTickPerSecond());
					ImGui::Text("Loop: %s", state.clip->Get_isLoop() ? "True" : "False");
				}
				else
				{
					ImGui::Text("No Clip Assigned");
				}
				ImGui::End();
				break;
			}
		}
	}

	return S_OK;
}

HRESULT CAnimTool::Render_Loaded_Models()
{
	if (m_LoadedModels.empty())
	{
		return S_OK;
	}

	static _int iSelectedModelIndex = -1;

	vector<string> modelNames;
	for (const auto& pair : m_LoadedModels)
		modelNames.push_back(pair.first);

	if (ImGui::BeginCombo("Models", iSelectedModelIndex >= 0 ? modelNames[iSelectedModelIndex].c_str() : "Select Model"))
	{
		for (_int i = 0; i < modelNames.size(); ++i)
		{
			_bool isSelected = (i == iSelectedModelIndex);
			if (ImGui::Selectable(modelNames[i].c_str(), isSelected))
			{
				iSelectedModelIndex = i;
				m_pCurModel = m_LoadedModels[modelNames[i]];

				m_pCurAnimator = m_LoadedAnimators[modelNames[i]];
				m_stSelectedModelName = modelNames[i];
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// 시퀀스 바꾸기
	if (m_pMySequence)
	{
		m_pMySequence->SetAnimator(m_pCurAnimator);
	}

	// 선택된 모델의 애니메이션들
	SelectAnimation();
	return S_OK;
}

void CAnimTool::UpdateCurrentModel(_float fTimeDelta)
{
	if (m_pCurModel == nullptr || m_pCurAnimator == nullptr)
	{
		return;
	}


	if (m_bUseAnimSequence && !m_bIsPlaying)
	{
		CAnimation* pAnim = m_pCurAnimator->GetCurrentAnim();
		if (pAnim)
		{
			_float normalized = _float(m_iSequenceFrame - m_pMySequence->GetFrameMin())
				/ _float(m_pMySequence->GetFrameMax() - m_pMySequence->GetFrameMin());
			_float ticks = normalized * pAnim->GetDuration();
			pAnim->SetCurrentTrackPosition(ticks);
			m_pCurAnimator->GetAnimController()->Update(0.f);
		}
	}
	else
	{
		m_pCurAnimator->GetAnimController()->Update(fTimeDelta);
	}
	m_pCurModel->Play_Animation();
}

void CAnimTool::SelectAnimation()
{
	if (m_pCurModel == nullptr || m_pCurAnimator == nullptr)
	{
		return;
	}
	static _int iSelectedAnimIndex = -1;


	vector<CAnimation*> anims = m_LoadedAnimations[m_stSelectedModelName]; // 현재 선택된 모델의 애니메이션들

	vector<string> animNames;
	animNames.reserve(anims.size());
	for (const auto& anim : anims)
	{
		animNames.push_back(anim->Get_Name());
	}

	if (ImGui::BeginCombo("Animations", iSelectedAnimIndex >= 0 ? animNames[iSelectedAnimIndex].c_str() : "Select Animation"))
	{
		for (_int i = 0; i < animNames.size(); ++i)
		{
			_bool isSelected = (i == iSelectedAnimIndex);
			if (ImGui::Selectable(animNames[i].c_str(), isSelected))
			{
				iSelectedAnimIndex = i;
				m_pCurAnimation = anims[iSelectedAnimIndex];
				m_pCurAnimator->PlayClip(anims[iSelectedAnimIndex], false);
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (m_pGameInstance->Key_Down(DIK_COMMA))
	{
		iSelectedAnimIndex--;
		if (iSelectedAnimIndex < 0)
			iSelectedAnimIndex = static_cast<_int>(anims.size()) - 1; // 마지막으로 순서

		if (m_pCurAnimation)
		{
			m_pCurAnimator->StartTransition(m_pCurAnimation, anims[iSelectedAnimIndex]);
		}
		else
		{
			m_pCurAnimator->PlayClip(anims[iSelectedAnimIndex], false);
		}

		m_pCurAnimation = anims[iSelectedAnimIndex];
	}
	if (m_pGameInstance->Key_Down(DIK_PERIOD))
	{
		iSelectedAnimIndex++;
		if (iSelectedAnimIndex >= static_cast<_int>(anims.size()))
			iSelectedAnimIndex = 0;
		if (m_pCurAnimation)
		{
			m_pCurAnimator->StartTransition(m_pCurAnimation, anims[iSelectedAnimIndex]);
		}
		else
		{
			m_pCurAnimator->PlayClip(anims[iSelectedAnimIndex], false);
		}
		m_pCurAnimation = anims[iSelectedAnimIndex];
	}

	// 애니메이션 프로퍼티들 설정
	Setting_AnimationProperties();
}

void CAnimTool::Setting_AnimationProperties()
{
	if (m_pCurAnimation)
	{
		ImGui::Checkbox("Use Animation Sequence", &m_bUseAnimSequence);
		ImGui::Separator();
		_bool bChanged = false;

		ImGui::Text("Animation Properties");

		// 애니메이션 이름
		const string& animName = m_pCurAnimation->Get_Name();
		ImGui::Text("Name: %s", animName.c_str());
		_float fCurTarckPos = m_pCurAnimation->GetCurrentTrackPosition();
		_float fDuration = m_pCurAnimation->GetDuration();
		ImGui::Text("Current Track Position: %.2f / Duration: %.2f", fCurTarckPos, fDuration);
		_float fTickPerSecond = m_pCurAnimation->GetTickPerSecond();
		bChanged |= ImGui::DragFloat("Tick Per Second", &fTickPerSecond, 0.1f, 0.1f, 100.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		_bool bLoop = m_pCurAnimation->Get_isLoop();
		if (ImGui::Checkbox("Loop", &bLoop))
		{
			m_pCurAnimation->SetLoop(bLoop);
		}
		if (bChanged)
		{
			m_pCurAnimation->SetTickPerSecond(fTickPerSecond);
		}
	}

}

void CAnimTool::Test_AnimEvents()
{
	//if (m_pCurAnimator)
	//{
	//	m_pCurAnimator->
	//}
}

void CAnimTool::SaveLoadEvents(_bool isSave)
{
	if (m_pCurModel == nullptr)
		return;
	string path = string("../Bin/Save/AnimationEvents/") + m_stSelectedModelName + "_events.json";
	if (isSave)
	{
		json root;
		root["animations"] = json::array();
		for (auto* anim : m_LoadedAnimations[m_stSelectedModelName])
		{
			root["animations"].push_back(anim->Serialize());
		}
	
		ofstream ofs(path);
		ofs << root.dump(4);
	}
	else
	{
		ifstream ifs(path);
		if (!ifs.is_open())
			return; // 파일이 없으면 로드하지 않음
		json root;
		ifs >> root;

		if (root.contains("animations"))
		{
			auto& animationsJson = root["animations"];
			auto& clonedAnims = m_LoadedAnimations[m_stSelectedModelName];

			for (const auto& animData : animationsJson)
			{
				const string& clipName = animData["ClipName"];

				for (auto& pAnim : clonedAnims)
				{
					if (pAnim->Get_Name() == clipName)
					{
						pAnim->Deserialize(animData);
						break;
					}
				}
			}
		}
	}
}

void CAnimTool::CreateModel(const string& fileName, const string& filePath)
{
	string modelName = fileName.find(".bin") != string::npos ?
		fileName.substr(0, fileName.find(".bin")) :
		fileName;

	if (m_LoadedModels.find(modelName) != m_LoadedModels.end())
	{
		// 이미 로드된 모델이면 새로 로드하지 않음
		MSG_BOX("이미 로드된 모델입니다.");
		return;
	}

	auto pModel = CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, filePath.c_str());
	if (pModel)
	{
		m_LoadedModels[modelName] = pModel;
		auto pAnimations = pModel->GetAnimations();

		vector<CAnimation*>& pModelCloneAnims = m_LoadedAnimations[modelName];

		const auto& Bones = pModel->Get_Bones();
		pModelCloneAnims.reserve(pAnimations.size());
		for (_uint i = 0; i < pAnimations.size(); i++)
		{
			pModelCloneAnims.push_back(pAnimations[i]->Clone(Bones));
		}

		auto pAnimator = CAnimator::Create(m_pDevice, m_pContext);
		if (pAnimator)
		{
			pAnimator->Initialize(pModel);
			m_LoadedAnimators[modelName] = pAnimator;
			pAnimator->RegisterEventListener("TestEvent", [&](const string& eventName)
				{
					MSG_BOX("애니메이션 이벤트 발생");
				});
		}
	}
}

void CAnimTool::Setting_Sequence()
{

	if (m_bUseAnimSequence)
	{
		if (m_pMySequence)
		{
			if (m_pCurAnimator)
			{
				// 현재 재생중이던 애니메이션의 트랙포지션으로 갱신
				auto pAnim = m_pCurAnimator->GetCurrentAnim();
				if (pAnim)
				{
					m_iSequenceFrame = static_cast<_int>(pAnim->GetCurrentTrackPosition());
				}
			}
		}
	}
}

void CAnimTool::Manipulate(Operation op, const _float snapT[3], const _float snapR[3], const _float snapS[3])
{
	if (!m_pTransformCom) return;

	ImGuizmo::BeginFrame();
	auto& io = ImGui::GetIO();
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	// 뷰·프로젝션·월드 매트릭스 float[16] 준비
	_float matV[16], matP[16], matW[16];
	XMStoreFloat4x4((XMFLOAT4X4*)matV, CGameInstance::Get_Instance()->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4((XMFLOAT4X4*)matP, CGameInstance::Get_Instance()->Get_Transform_Matrix(D3DTS::PROJ));

	_matrix xmW = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr());
	XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(matW), xmW);

	// 조작 모드 결정
	ImGuizmo::OPERATION gizOp =
		op == Operation::TRANSLATE ? ImGuizmo::TRANSLATE :
		op == Operation::ROTATE ? ImGuizmo::ROTATE :
		ImGuizmo::SCALE;

	// Shift 누르고 있을 때만 스냅 적용
	const _float* snapPtr = nullptr;
	if (ImGui::IsKeyDown(ImGuiMod_Shift))
	{
		switch (op)
		{
		case Operation::TRANSLATE: snapPtr = snapT; break;
		case Operation::ROTATE:    snapPtr = snapR; break;
		case Operation::SCALE:     snapPtr = snapS; break;
		}
	}

	// Manipulate에 스냅 배열 전달
	ImGuizmo::Manipulate(matV, matP, gizOp, ImGuizmo::WORLD, matW, nullptr, snapPtr);

	//// 드래그 중이면 WorldMatrix 업데이트
	//if (ImGuizmo::IsUsing())
	//{
	//	XMFLOAT4X4 newW;
	//	memcpy(&newW, matW, sizeof(newW));
	//	m_pTransformCom->Set_WorldMatrix(newW);

	//	if (op == Operation::ROTATE)
	//	{
	//		pTransform->UpdateEulerAngles();
	//	}
	//}

}

HRESULT CAnimTool::Bind_Shader()
{
	if (m_pCurModel == nullptr || m_pCurAnimator == nullptr)
		return S_OK;

	if (m_pAnimShader == nullptr)
		return E_FAIL;
	if (FAILED(m_pAnimShader->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;

	_float4x4 ViewMatrix, ProjViewMatrix;
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));
	if (FAILED(m_pAnimShader->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pAnimShader->Bind_Matrix("g_ProjMatrix", &ProjViewMatrix)))
		return E_FAIL;

	_uint		iNumMesh = m_pCurModel->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pCurModel->Bind_Material(m_pAnimShader, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		m_pCurModel->Bind_Bone_Matrices(m_pAnimShader, "g_BoneMatrices", i);

		if (FAILED(m_pAnimShader->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pCurModel->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}


CAnimTool* CAnimTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CAnimTool* pInstance = new CAnimTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CAnimTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAnimTool::Clone(void* pArg)
{
	CAnimTool* pInstance = new CAnimTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAnimTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CAnimTool::Free()
{
	__super::Free();

	for (auto& pair : m_LoadedModels)
	{
		Safe_Release(pair.second);
	}

	for (auto& pair : m_LoadedAnimators)
	{
		Safe_Release(pair.second);
	}

	for (auto& pair : m_LoadedAnimations)
	{
		for (auto& anim : pair.second)
			Safe_Release(anim);
		pair.second.clear();
	}
	ImNodes::DestroyContext();
	Safe_Release(m_pEventMag);
	Safe_Delete(m_pMySequence);
	Safe_Release(m_pAnimShader);
	Safe_Release(m_pGameInstance);
}
