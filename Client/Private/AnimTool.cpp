#include "Bone.h"
#include "EventMag.h"
#include "AnimTool.h"

#include "GameInstance.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() �̷� �̱������� ����� ���� ���ϰ� ��

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


	ImNodesStyle& style = ImNodes::GetStyle();

		// ����Ƽ ��Ÿ�� ����
		style.Colors[ImNodesCol_NodeBackground] = IM_COL32(60, 60, 70, 255);
		style.Colors[ImNodesCol_NodeBackgroundHovered] = IM_COL32(75, 75, 85, 255);
		style.Colors[ImNodesCol_NodeBackgroundSelected] = IM_COL32(85, 85, 95, 255);
		style.Colors[ImNodesCol_NodeOutline] = IM_COL32(100, 100, 110, 255);

		// �ձ� �𼭸�
		style.NodeCornerRounding = 5.0f;
		style.NodePadding = ImVec2(8.0f, 4.0f);

		// �� ��Ÿ��
		style.PinCircleRadius = 6.0f;
		style.PinQuadSideLength = 8.0f;
		style.LinkThickness = 3.0f;
		style.LinkLineSegmentsPerLength = 0.1f;

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
	if (Button(u8"���� ���"))
	{
		config.path = R"(../Bin/Resources/Models/Bin_Anim)";
		config.countSelectionMax = 0; // �ϳ��� ���� ����
		IFILEDIALOG->OpenDialog("Model Dialog", "Select Model File", ".bin", config);
	}

	if (IFILEDIALOG->Display("Model Dialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			auto selections = IFILEDIALOG->GetSelection();
			// ó��
			// first: ���ϸ�.Ȯ����
			// second: ��ü ��� (���ϸ�����)
			if (!selections.empty())
			{
				for (const auto& FilePath : selections)
				{
					CreateModel(FilePath.first, FilePath.second); // ��ü ��η� �� ����

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

HRESULT CAnimTool::Render_Parameters()
{
	ImGui::Separator();

		// �÷� Name | Type | Value | Action
		ImGui::Columns(4, "ParamColumns", true);
		ImGui::Text("Name");   ImGui::NextColumn();
		ImGui::Text("Type");   ImGui::NextColumn();
		ImGui::Text("Value");  ImGui::NextColumn();
		ImGui::Text("Action"); ImGui::NextColumn();
		ImGui::Separator();

		unordered_map<string, Parameter>& parameters = m_pCurAnimator->GetParameters();
		// �Ķ���� Ÿ�Ե�
		const _char* typeNames[] = { "Bool", "Trigger", "Float", "Int" };

		_int i = 0;

		for (auto it = parameters.begin(); it != parameters.end();)
		{
			auto& parmeter = it->second;
			string id = to_string(i); // �ĺ��ϴ� ���̵��

			// Name
			_char buf[64];
			strncpy_s(buf, it->first.c_str(), sizeof(buf));
			if (ImGui::InputText(("##Name" + id).c_str(), buf, sizeof(buf)))
			{
				string oldName = parmeter.name;
				string newName = buf;
				if (newName != oldName)
				{
					Parameter newParameter = parmeter; // ���� �Ķ���� ����
					newParameter.name = newName; // �� �̸����� ����
					it = parameters.erase(it); // ���� �̸� ����
					m_pCurAnimator->AddParameter(newName, newParameter);
					continue;
				}
			}
			ImGui::NextColumn();

			// Type �޺�
			_int t = static_cast<_int>(parmeter.type);
	/*		if (ImGui::Combo(("##Type" + id).c_str(), &t, typeNames, IM_ARRAYSIZE(typeNames)))
				parmeter.type = (ParamType)t;*/

			if (ImGui::Combo(("##Type" + id).c_str(), &t, typeNames, IM_ARRAYSIZE(typeNames)))
			{
				ParamType newType = (ParamType)t;
				if (newType != parmeter.type)
				{
					// Ÿ�� ���� �� ������ ������ �ʱ�ȭ
					parmeter.type = newType;

					switch (newType)
					{
					case ParamType::Bool:
						parmeter.bValue = false;
						parmeter.bTriggered = false;
						parmeter.iValue = 0;
						parmeter.fValue = 0.0f;
						break;
					case ParamType::Trigger:
						parmeter.bValue = false;
						parmeter.bTriggered = false;
						parmeter.iValue = 0;
						parmeter.fValue = 0.0f;
						break;
					case ParamType::Float:
						parmeter.bValue = false;
						parmeter.bTriggered = false;
						parmeter.iValue = 0;
						parmeter.fValue = 0.0f;
						break;
					case ParamType::Int:
						parmeter.bValue = false;
						parmeter.bTriggered = false;
						parmeter.iValue = 0;
						parmeter.fValue = 0.0f;
						break;
					}


					// �ٲ� Ÿ�Կ� �°� �ٽ� ����
					if (m_pCurAnimator)
					{
						switch (newType)
						{
						case ParamType::Bool:
							m_pCurAnimator->SetBool(parmeter.name, parmeter.bValue);
							break;
						case ParamType::Trigger:
							m_pCurAnimator->ResetTrigger(parmeter.name); 
							break;
						case ParamType::Float:
							m_pCurAnimator->SetFloat(parmeter.name, parmeter.fValue);
							break;
						case ParamType::Int:
							m_pCurAnimator->SetInt(parmeter.name, parmeter.iValue);
							break;
						}
					}
				}
			}
			ImGui::NextColumn();

			// Value ����
			switch (parmeter.type)
			{
			case ParamType::Bool:
			{

				_bool bFlag = parmeter.bValue;
				if (ImGui::Checkbox(("##Val" + id).c_str(), &bFlag))
				{
					parmeter.bValue = bFlag;
					if (m_pCurAnimator)
						m_pCurAnimator->SetBool(parmeter.name, parmeter.bValue);
				}
			}
				break;
			case ParamType::Int:
			{
				_int iVal = parmeter.iValue;
				if (ImGui::DragInt(("##Val" + id).c_str(), &iVal, 1, -10000, 10000))
				{
					parmeter.iValue = iVal;
					if (m_pCurAnimator)
						m_pCurAnimator->SetInt(parmeter.name, parmeter.iValue);
				}
				break;
			}
			case ParamType::Float:
			{
				_float fVal = parmeter.fValue;
				if (ImGui::DragFloat(("##Val" + id).c_str(), &fVal, 0.01f, -1000.f, 1000.f))
				{
					parmeter.fValue = fVal;
					if (m_pCurAnimator)
						m_pCurAnimator->SetFloat(parmeter.name, parmeter.fValue);
				}
				break;
			}
			case ParamType::Trigger:
				if (ImGui::Button(("Trigger##" + id).c_str()))
					m_pCurAnimator->SetTrigger(parmeter.name);
				break;
			}
			ImGui::NextColumn();

			// Delete ��ư
			if (ImGui::Button(("X##" + id).c_str()))
			{
				it = parameters.erase(it);
				--i; // ���������� �ε��� ����
			}
			else
			{
				++it;
				i++;
			}
			ImGui::NextColumn();
		}


		ImGui::Columns(1);

		// �Ķ���� �߰� �˾�
		if (ImGui::Button("Add Parameter"))
			ImGui::OpenPopup("AddParamPopup");

		if (ImGui::BeginPopup("AddParamPopup"))
		{
			static _char newName[64] = "";
			static _int  newType = 0; // 0: Bool, 1: Trigger, 2: Float, 3: Int
			ImGui::InputText("Name", newName, sizeof(newName));
			ImGui::Combo("Type", &newType, typeNames, IM_ARRAYSIZE(typeNames));
			if (ImGui::Button("Add"))
			{
				if (m_pCurAnimator)
				{
					_bool bExists = m_pCurAnimator->ExisitsParameter(newName);
					string diffName = newName + to_string(i);
					switch (newType)
					{	
					case 0: // Bool
						m_pCurAnimator->AddBool(bExists? diffName : newName);
						break;
					case 1: // Trigger
						m_pCurAnimator->AddTrigger(bExists ? diffName : newName);
						break;
					case 2: // Float
						m_pCurAnimator->AddFloat(bExists ? diffName : newName);
						break;
					case 3: // Int
						m_pCurAnimator->AddInt(bExists ? diffName : newName);
						break;
					default:
						break;
					}
				newName[0] = '\0'; // Ŭ����
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

	return S_OK;
}

HRESULT CAnimTool::Render_TransitionConditions()
{
	// ���� �߰��Ǿ��ִ� Ʈ�������� ���ǵ��� �����ְ� �޺��ڽ��� ������� ������ ������ �� �ְ� ó��
	if (m_pCurAnimator == nullptr || m_pCurAnimator->GetAnimController() == nullptr)
	{
		return S_OK;
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
	static _bool          expanded = true;// Ʈ�� Ȯ�� ����
	static const _float FRAME = 60.f; // 1�ʴ� 60������ ����

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
			if (m_pCurAnimator->GetCurrentAnim())
			{
				m_pCurAnimator->GetCurrentAnim()->ResetTrack();
				m_pCurAnimator->SetPlaying(true);
			}
		}
	}

	// ��� �ӵ� ����
	ImGui::SameLine();
	ImGui::SliderInt("Speed", &m_iPlaySpeed, 1, 10);


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
			pCurAnim->SetCurrentTrackPosition(static_cast<_float>(m_iSequenceFrame)); // �������� �� ������ ��ȯ
			m_pCurAnimator->SetPlaying(true);
			m_pCurAnimator->Update(0.f);
			m_pCurAnimator->StopAnimation(); // �ٽ� pause
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
	if (ImGui::Button("Save AnimState This Model"))
	{
		SaveLoadAnimStates();
	}

	if (ImGui::Button("Load AnimState This Model"))
	{
		SaveLoadAnimStates(false);
		return S_OK; // �ҷ��� ���� �� ������ �ѱ��
	}
	static _int selectedNodeID = -1;  // ������ ��� ���̵�

	// ��Ʈ�ѷ��� �ٲ�� �ĺ� ID �ʱ�ȭ ���ѳ���
	CAnimController* pCtrl = m_pCurAnimator->GetAnimController();

	ImGui::Begin("Anim State Machine");

	if (ImGui::CollapsingHeader("Parameters"))
	{
		if (FAILED(Render_Parameters()))
			return E_FAIL;
	}


	ImNodes::BeginNodeEditor();

	// ��Ŭ������ ��� �߰� �˾�
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImNodes::IsEditorHovered())
	{
		ImGui::OpenPopup("AddStatePopup");
	}

	if (ImGui::BeginPopup("AddStatePopup"))
	{
		static _char stateName[64] = "NewState";

		ImGui::InputText("State Name", stateName, IM_ARRAYSIZE(stateName));

		if (ImGui::Button("Add State"))
		{
			const ImVec2 mousePos = ImNodes::EditorContextGetPanning(); // ���� ���콺 ��ġ
			CAnimation* selectedAnim = m_pCurAnimation;

			size_t newIdx = pCtrl->AddState(stateName, selectedAnim, m_iSpeicificNodeId++);
			// ���� �� ���� �ε��� ��ȯ�Ѱɷ� state ã��
			auto& newState = pCtrl->GetStates()[newIdx];
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

		// ��� ���̵� 
		ImNodes::BeginInputAttribute(state.iNodeId *10 + 1);
		ImGui::Text("In");
		ImNodes::EndInputAttribute();

		ImNodes::BeginOutputAttribute(state.iNodeId * 10 + 2);
		ImGui::Text("Out");
		ImNodes::EndOutputAttribute();

		ImNodes::EndNode();

		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(state.iNodeId);
		state.fNodePos = { pos.x, pos.y };
	}

	// Ʈ������	��ũ �׸���
	for(const auto& link : pCtrl->GetTransitions())
	{
		ImNodes::Link(link.link.iLinkId, link.link.iLinkStartID, link.link.iLinkEndID);
	}



	ImNodes::MiniMap();
	ImNodes::EndNodeEditor();

	_bool bCanLink = pCtrl->GetStates().size() >= 2;
	if (bCanLink)
	{

		_int startPinID = -1, endPinID = -1;
		if (ImNodes::IsLinkCreated(&startPinID, &endPinID))
		{
			_int fromNodeID = (startPinID - 2) / 10;
			_int toNodeID = (endPinID - 1) / 10;

			// ��ȿ�� ���̵����� 
			_bool validFromNode = false, validToNode = false;
			for (const auto& state : pCtrl->GetStates())
			{
				if (state.iNodeId == fromNodeID) 
					validFromNode = true;
				if (state.iNodeId == toNodeID)
					validToNode = true;
			}

			if (validFromNode && validToNode && fromNodeID != toNodeID)
			{
				Engine::Link link;
				link.iLinkId = m_iSpeicificNodeId++;
				link.iLinkStartID = startPinID;
				link.iLinkEndID = endPinID;

				// �ӽ� ���� ���� (���߿� UI�� �����ϵ��� ���� �ʿ�)
				//m_pCurAnimator->AddBool("Test");
				//CAnimController::Condition testCondition{ "",ParamType::Bool, CAnimController::EOp::None,0, 0.f, 0.95f };

				// Ʈ������ �߰�
				pCtrl->AddTransition(fromNodeID, toNodeID, link, 0.5f,true);
			}

		}
	}
	if (ImNodes::NumSelectedLinks() > 0 )
	{
		vector<int> selectedLinks(ImNodes::NumSelectedLinks());
		ImNodes::GetSelectedLinks(selectedLinks.data());
		auto& transitions = pCtrl->GetTransitions();

		_bool bDeleteLink = false;
		if (ImGui::IsKeyPressed(ImGuiKey_Delete))
		{

		for (_int linkId : selectedLinks)
		{
			for (_int i = static_cast<_int>(transitions.size()) - 1; i >= 0; --i)
			{
				if (transitions[i].link.iLinkId == linkId)
				{
					bDeleteLink = true;
					transitions.erase(transitions.begin() + i);
					break;
				}
			}
		}
			ImNodes::ClearLinkSelection();
		}

		// ��ũ ���ý� Ʈ������ ���� ǥ��
		// ���õ� ��ũ�� Condition�� ã�Ƽ� ǥ��

		// CAnimController::Condition testCondition{ "Test", CAnimController::EOp::Finished, 0.f, 0.95f }; ����� ����

		if (bDeleteLink == false) // ���� ���� ��쿡��
		{

		for (_int linkId : selectedLinks)
		{
			for (auto& transition : transitions)
			{
				if (transition.link.iLinkId == linkId)
				{
					ImGui::Begin("Transition Info");
					ImGui::Text("From Node ID: %d", transition.iFromNodeId);
					ImGui::Text("To Node ID: %d", transition.iToNodeId);
					ImGui::Text("Link ID: %d", transition.link.iLinkId);
					//ImGui::Text("Condition: %s", transition.condition.paramName.c_str());
					ImGui::End();

					if (FAILED(Modify_Transition(transition)))
					{
						ImGui::End();
						return E_FAIL;
					}
					break;
				}
			}
		}
		}
	}

	// ��� ���� (������Ʈ ����)
	if (ImNodes::NumSelectedNodes() > 0 && ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		vector<int> selectedNodes(ImNodes::NumSelectedNodes());
		ImNodes::GetSelectedNodes(selectedNodes.data());
		auto& states = pCtrl->GetStates();
		for (_int nodeId: selectedNodes)
		{

				for (auto it = states.begin(); it != states.end(); ++it)
				{
					if (it->iNodeId == nodeId)
					{
						states.erase(it); 
						break;
					}
				}
		}
		ImNodes::ClearNodeSelection();
		ImGui::End();
		return S_OK;
	}

	_int hoveredNodeID = -1;
	if (ImNodes::IsNodeHovered(&hoveredNodeID) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		selectedNodeID = hoveredNodeID;
	}
	ImGui::End();




	vector<CAnimation*> anims = m_LoadedAnimations[m_stSelectedModelName]; // ���� ���õ� ���� �ִϸ��̼ǵ�

	vector<string> animNames;
	animNames.reserve(anims.size());
	for (const auto& anim : anims)
	{
		animNames.push_back(anim->Get_Name());
	}

	if (selectedNodeID != -1)
	{
		for (auto& state : pCtrl->GetStates())
		{
			if (state.iNodeId == selectedNodeID)
			{
				ImGui::Begin("Anim Info");

				_char buf[64];
				strcpy_s(buf, state.stateName.c_str());
				ImGui::InputText("State Name", buf, IM_ARRAYSIZE(buf));
				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					state.stateName = buf; // �̸� ����
				}
				if (state.clip)
				{
					ImGui::Text("Clip: %s", state.clip->Get_Name().c_str());
					ImGui::Text("Node ID: %d", state.iNodeId);
					ImGui::Text("Duration: %.2f", state.clip->GetDuration());
					ImGui::Text("Current Track Position: %.2f", state.clip->GetCurrentTrackPosition());
					ImGui::Text("Tick Per Second: %.2f", state.clip->GetTickPerSecond());
					ImGui::Text("Loop: %s", state.clip->Get_isLoop() ? "True" : "False");

					// Ŭ���� �־��� ��쿡�� ���� �ִϸ��̼��� state�� �ִϸ��̼����� ����
					m_pCurAnimation = state.clip;
					if (ImNodes::IsNodeSelected(selectedNodeID) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						pCtrl->SetState(selectedNodeID);
					}
				}
				else
				{
					ImGui::Text("No Clip Assigned");
				}

				auto& transitions = pCtrl->GetTransitions();
				// Ʈ������ ����Ʈ

				for (const auto& transition : transitions)
				{
					if (transition.iFromNodeId == state.iNodeId)
					{
						const string& stateName = pCtrl-> GetStateNameByNodeId(transition.iToNodeId);
						ImGui::Text("Transition to: %s", stateName.c_str());
						ImGui::Text("Transition to Node ID: %d", transition.iToNodeId);
						ImGui::Text("Link ID: %d", transition.link.iLinkId);
					}
				}


				// �ش� ��忡 �ִϸ��̼� �����ϴ� â
				_int iSelectedAnimIndex = -1;
				if (ImGui::BeginCombo("Animations", iSelectedAnimIndex >= 0 ? animNames[iSelectedAnimIndex].c_str() : "Select Animation"))
				{
					for (_int i = 0; i < animNames.size(); ++i)
					{
						_bool isSelected = (i == iSelectedAnimIndex);
						if (ImGui::Selectable(animNames[i].c_str(), isSelected))
						{
							iSelectedAnimIndex = i;
							state.clip = anims[iSelectedAnimIndex];
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
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
				// ���� �ٲ� ���� ���� �ִϸ��̼��� nullptr
				m_pCurAnimation = nullptr;
				m_stSelectedModelName = modelNames[i];
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// ������ �ٲٱ�
	if (m_pMySequence)
	{
		m_pMySequence->SetAnimator(m_pCurAnimator);
	}

	// ���õ� ���� �ִϸ��̼ǵ�
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
			m_pCurAnimator->Update(0.f);
		}
	}
	else
	{
		m_pCurAnimator->Update(fTimeDelta* m_iPlaySpeed);
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


	vector<CAnimation*> anims = m_LoadedAnimations[m_stSelectedModelName]; // ���� ���õ� ���� �ִϸ��̼ǵ�

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
			iSelectedAnimIndex = static_cast<_int>(anims.size()) - 1; // ���������� ����

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

	// �ִϸ��̼� ������Ƽ�� ����
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

		// �ִϸ��̼� �̸�
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
			return; // ������ ������ �ε����� ����
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

void CAnimTool::SaveLoadAnimStates(_bool isSave)
{
	if (m_pCurModel == nullptr)
		return;
	string path = string("../Bin/Save/AnimationStates/") + m_stSelectedModelName + "_States.json";
	if (isSave)
	{
		if (m_pCurAnimator)
		{
		json j = 	m_pCurAnimator->Serialize();
		ofstream ofs(path);
		ofs << j.dump(4);
		}
		else
		{
			MSG_BOX("�ִϸ����Ͱ� �����ϴ�. �ִϸ����͸� ���� �������ּ���.");
			return;
		}
	
	}
	else
	{
		ifstream ifs(path);
		if (!ifs.is_open())
			return; // ������ ������ �ε����� ����
		json root;
		ifs >> root;
		if (m_pCurAnimator && path.find(m_stSelectedModelName) != string::npos)
		{
			m_pCurAnimator->Deserialize(root);
			auto pCtrl = m_pCurAnimator->GetAnimController();
			auto states = pCtrl->GetStates();
			auto transitions = pCtrl->GetTransitions();

			for (const auto& state : states)
			{
				m_iSpeicificNodeId = max(m_iSpeicificNodeId, state.iNodeId + 1);
			}

			for (const auto& transition : transitions)
			{
				m_iSpeicificNodeId = max(m_iSpeicificNodeId, transition.link.iLinkId + 1);
			}
		}
		else
		{
			MSG_BOX("�ִϸ����Ͱ� �����ϴ�. �ִϸ����͸� ���� �������ּ���.");
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
		// �̹� �ε�� ���̸� ���� �ε����� ����
		MSG_BOX("�̹� �ε�� ���Դϴ�.");
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
			//Safe_AddRef(pModelCloneAnims.back());
		}

		auto pAnimator = CAnimator::Create(m_pDevice, m_pContext);
		if (pAnimator)
		{
			pAnimator->Initialize(pModel);
			m_LoadedAnimators[modelName] = pAnimator;
			pAnimator->RegisterEventListener("TestEvent", [&](const string& eventName)
				{
					MSG_BOX("�ִϸ��̼� �̺�Ʈ �߻�");
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
				// ���� ������̴� �ִϸ��̼��� Ʈ������������ ����
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

	// �䡤�������ǡ����� ��Ʈ���� float[16] �غ�
	_float matV[16], matP[16], matW[16];
	XMStoreFloat4x4((XMFLOAT4X4*)matV, CGameInstance::Get_Instance()->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4((XMFLOAT4X4*)matP, CGameInstance::Get_Instance()->Get_Transform_Matrix(D3DTS::PROJ));

	_matrix xmW = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr());
	XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(matW), xmW);

	// ���� ��� ����
	ImGuizmo::OPERATION gizOp =
		op == Operation::TRANSLATE ? ImGuizmo::TRANSLATE :
		op == Operation::ROTATE ? ImGuizmo::ROTATE :
		ImGuizmo::SCALE;

	// Shift ������ ���� ���� ���� ����
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

	// Manipulate�� ���� �迭 ����
	ImGuizmo::Manipulate(matV, matP, gizOp, ImGuizmo::WORLD, matW, nullptr, snapPtr);

	//// �巡�� ���̸� WorldMatrix ������Ʈ
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

void CAnimTool::SelectNode()
{
}

void CAnimTool::CreateLink()
{
}

HRESULT CAnimTool::Modify_Transition(CAnimController::Transition& transition)
{
	ImGui::Separator();

	// �÷� Name | Type | Value | Action


	if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_CollapsingHeader))
	{
		// �� ���� ���� ������ �� ��������, �������� ���Դϴ�.
		ImGui::Indent(10);
		ImGui::Checkbox("Has Exit Time", &transition.hasExitTime);
		ImGui::SliderFloat("Exit Time", &transition.minTime, 0.f, 1.f);
		ImGui::SliderFloat("Transition Duration ", &transition.duration, 0.f, 1.f);
		ImGui::Unindent(10);
	}



	// ���� Ʈ�������� ����� ���� ��������
	// �������� �� ���� ������ �����ؼ� ������� ���� �� �ְ�
	// ������ ����Ǹ��� ������ ���� �� �ְ� 

//	vector<string> paramNames;
//	auto& parameters = m_pCurAnimator->GetParameters();
//	paramNames.reserve(parameters.size());
//	for (const auto& parm : parameters)
//	{
//		paramNames.push_back(parm.first);
//	}
//	static _int selectedParamIdx = -1;
//
//
//	if (ImGui::BeginCombo("Params", selectedParamIdx >= 0 ? paramNames[selectedParamIdx].c_str() : "Select Parameter"))
//	{
//		for (_int i = 0; i < paramNames.size(); ++i)
//		{
//			_bool isSelected = (i == selectedParamIdx);
//			if(paramNames[i].empty())
//				continue; // �� �̸��� ����
//			if (ImGui::Selectable(paramNames[i].c_str(), isSelected))
//			{
//				selectedParamIdx = i;
//				if (parameters.find(paramNames[i]) == parameters.end())
//					continue; // ���� �Ķ���ʹ� ����
//				Parameter& sel = parameters[paramNames[i]];
//				transition.condition.paramName = paramNames[i];
//				transition.condition.type = sel.type;
//				transition.condition.fThreshold = sel.fValue;
//				transition.condition.iThreshold = sel.iValue;
//			}
//			if (isSelected)
//				ImGui::SetItemDefaultFocus();
//		}
//		ImGui::EndCombo();
//	}	
//	
////	const _char* operationNames[] = { "True", "False", "Greater", "Less", "NotEqual","Equal","None" };
//
//	switch (transition.condition.type)
//	{
//	case ParamType::Bool:
//	{
//		
//		_int opIdx = 0;
//		for (_int i = 0; i < IM_ARRAYSIZE(BoolOps); ++i)
//			if (transition.condition.op == BoolOps[i])
//				opIdx = i;
//
//		// �޺��ڽ�
//		if (ImGui::Combo("Operator", &opIdx, BoolOpNames, IM_ARRAYSIZE(BoolOpNames)))
//			transition.condition.op = BoolOps[opIdx];
//		break;
//	}
//	case ParamType::Trigger:
//	{
//		transition.condition.op = CAnimController::EOp::Trigger;
//		break;
//	}
//	case ParamType::Float:
//	{
//		_int opIdx = 0;
//		for (_int i = 0; i < IM_ARRAYSIZE(CmpFloatOps); ++i)
//			if (transition.condition.op == CmpFloatOps[i])
//				opIdx = i;
//
//		if (ImGui::Combo("Operator", &opIdx, CmpFloatOpNames, IM_ARRAYSIZE(CmpFloatOpNames)))
//			transition.condition.op = CmpFloatOps[opIdx];
//		break;
//	}
//	case ParamType::Int:
//	{
//		_int opIdx = 0;
//		for (_int i = 0; i < IM_ARRAYSIZE(CmpIntOps); ++i)
//			if (transition.condition.op == CmpIntOps[i])
//				opIdx = i;
//
//		if (ImGui::Combo("Operator", &opIdx, CmpIntOpNames, IM_ARRAYSIZE(CmpIntOpNames)))
//			transition.condition.op = CmpIntOps[opIdx];
//		break;
//	}
//	}
//
//	switch (transition.condition.op)
//	{
//	case CAnimController::EOp::Greater:
//	case CAnimController::EOp::Less:
//	case CAnimController::EOp::Equal:
//	case CAnimController::EOp::NotEqual:
//		if (transition.condition.type == ParamType::Float)
//			ImGui::DragFloat("Threshold", &transition.condition.fThreshold, 0.01f);
//		else  // ParamType::Int
//			ImGui::DragInt("Threshold", &transition.condition.iThreshold, 1);
//		break;
//	default:
//		break;
//	}
//
//	return S_OK;

if (ImGui::CollapsingHeader("Conditions", ImGuiTreeNodeFlags_DefaultOpen))
{
	// �Ķ���� ��� �غ�
	auto& parameters = m_pCurAnimator->GetParameters();
	std::vector<std::string> paramNames;
	paramNames.reserve(parameters.size());
	for (auto& kv : parameters)
		paramNames.push_back(kv.first);

	// ���� ���ǵ� ����
	for (int idx = 0; idx < (int)transition.conditions.size(); /*���� inside*/)
	{
		auto& cond = transition.conditions[idx];
		ImGui::PushID(idx);
		ImGui::Separator();

		// 1) Param ����
		int selParam = -1;
		for (int i = 0; i < (int)paramNames.size(); ++i)
			if (paramNames[i] == cond.paramName) selParam = i;

		if (ImGui::BeginCombo("Param", selParam >= 0 ? paramNames[selParam].c_str() : "Select"))
		{
			for (int i = 0; i < (int)paramNames.size(); ++i)
			{
				bool isSelected = (i == selParam);
				if (ImGui::Selectable(paramNames[i].c_str(), isSelected))
				{
					cond.paramName = paramNames[i];
					cond.type = parameters[cond.paramName].type;
					cond.fThreshold = parameters[cond.paramName].fValue;
					cond.iThreshold = parameters[cond.paramName].iValue;
				}
				if (isSelected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		switch (cond.type)
		{
		case ParamType::Bool:
		{
			int opIdx = 0;
			for (int i = 0; i < IM_ARRAYSIZE(BoolOps); ++i)
				if (BoolOps[i] == cond.op) opIdx = i;
			if (ImGui::Combo("Operator", &opIdx, BoolOpNames, IM_ARRAYSIZE(BoolOpNames)))
				cond.op = BoolOps[opIdx];
			break;
		}
		case ParamType::Trigger:
		{
			// Trigger�� ����
			ImGui::Text("Operator: Trigger");
			cond.op = CAnimController::EOp::Trigger;
			break;
		}
		case ParamType::Float:
		{
			int opIdx = 0;
			for (int i = 0; i < IM_ARRAYSIZE(CmpFloatOps); ++i)
				if (CmpFloatOps[i] == cond.op) opIdx = i;
			if (ImGui::Combo("Operator", &opIdx, CmpFloatOpNames, IM_ARRAYSIZE(CmpFloatOpNames)))
				cond.op = CmpFloatOps[opIdx];
			break;
		}
		case ParamType::Int:
		{
			int opIdx = 0;
			for (int i = 0; i < IM_ARRAYSIZE(CmpIntOps); ++i)
				if (CmpIntOps[i] == cond.op) opIdx = i;
			if (ImGui::Combo("Operator", &opIdx, CmpIntOpNames, IM_ARRAYSIZE(CmpIntOpNames)))
				cond.op = CmpIntOps[opIdx];
			break;
		}
		}

		if (cond.op == CAnimController::EOp::Greater ||
			cond.op == CAnimController::EOp::Less ||
			cond.op == CAnimController::EOp::Equal ||
			cond.op == CAnimController::EOp::NotEqual)
		{
			if (cond.type == ParamType::Float)
				ImGui::DragFloat("Threshold", &cond.fThreshold, 0.01f);
			else
				ImGui::DragInt("Threshold", &cond.iThreshold, 1);
		}

		ImGui::SameLine();
		if (ImGui::Button("Remove"))
		{
			transition.conditions.erase(transition.conditions.begin() + idx);
			ImGui::PopID();
			continue; // ���� idx �׸��� ���������� ���� ����
		}

		ImGui::PopID();
		++idx;
	}

	if (ImGui::Button("Add Condition"))
	{
		CAnimController::Condition newCond{};
		newCond.op = CAnimController::EOp::None;
		transition.conditions.push_back(newCond);
	}
}

return S_OK;
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
		{

		}
			//	return E_FAIL;

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
