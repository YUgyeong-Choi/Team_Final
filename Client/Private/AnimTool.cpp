#ifdef USE_IMGUI
#include "Bone.h"
#include "EventMag.h"
#include "AnimTool.h"

#include "GameInstance.h"
#include <queue>

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
		style.LinkLineSegmentsPerLength = 0.0f; // ����


		  style.Colors[ImNodesCol_NodeBackground] = IM_COL32(56, 56, 56, 255);           // �� ��ο� ���
    style.Colors[ImNodesCol_NodeBackgroundHovered] = IM_COL32(70, 70, 70, 255);    // ȣ�� ��
    style.Colors[ImNodesCol_NodeBackgroundSelected] = IM_COL32(90, 90, 90, 255);   // ���� ��
    style.Colors[ImNodesCol_NodeOutline] = IM_COL32(128, 128, 128, 255);           // �׵θ�

    // Ÿ��Ʋ�� ���� (����Ƽ�� ��Ȳ�� �迭)
    style.Colors[ImNodesCol_TitleBar] = IM_COL32(58, 58, 58, 255);
    style.Colors[ImNodesCol_TitleBarHovered] = IM_COL32(72, 72, 72, 255);
    style.Colors[ImNodesCol_TitleBarSelected] = IM_COL32(86, 86, 86, 255);

    // === ��(Pin) ��Ÿ�� ===
    // �Է�/��� �� ������ �ٸ��� ����
    style.Colors[ImNodesCol_Pin] = IM_COL32(200, 200, 200, 255);                   // �⺻ ��
    style.Colors[ImNodesCol_PinHovered] = IM_COL32(255, 255, 255, 255);            // ȣ�� �� ��

    // �� ũ�� �� ��� ����
    style.PinCircleRadius = 7.0f;        // ���� �� �� ũ��
    style.PinQuadSideLength = 10.0f;     // �簢�� �� ũ��
    style.PinTriangleSideLength = 12.0f; // �ﰢ�� �� ũ��

    // === ��ũ(���ἱ) ��Ÿ�� - �ٽ� ���� ���� ===
    
    // 1. ������ ��ũ�� ���� (� ����)
    style.LinkThickness = 3.0f;                    // �� �β�
    style.LinkLineSegmentsPerLength = 0.0f;        // 0���� �����ϸ� ����
    style.LinkHoverDistance = 10.0f;               // ���콺 ȣ�� ���� �Ÿ�
    
    // 2. ��ũ ���� ���� (����Ƽ ��Ÿ��)
    style.Colors[ImNodesCol_Link] = IM_COL32(150, 150, 150, 255);                // �⺻ ��ũ ����
    style.Colors[ImNodesCol_LinkHovered] = IM_COL32(255, 255, 255, 255);         // ȣ�� �� ���
    style.Colors[ImNodesCol_LinkSelected] = IM_COL32(255, 165, 0, 255);          // ���� �� ��Ȳ��

    // === ��� ��� ���� ===
    style.NodeCornerRounding = 6.0f;       // �𼭸� �ձ۱�
    style.NodePadding = ImVec2(10.0f, 6.0f); // ��� ���� ���� ����
    style.NodeBorderThickness = 2.0f;      // �׵θ� �β�

    // === �׸��� ��Ÿ�� ===
    style.Colors[ImNodesCol_GridBackground] = IM_COL32(40, 40, 40, 255);    // ����
    style.Colors[ImNodesCol_GridLine] = IM_COL32(60, 60, 60, 100);          // �׸��� ����
    style.GridSpacing = 32.0f;                                               // �׸��� ����

    // === ���� ���� ��Ÿ�� ===
    style.Colors[ImNodesCol_BoxSelector] = IM_COL32(100, 149, 237, 80);     // ���� �ڽ�
    style.Colors[ImNodesCol_BoxSelectorOutline] = IM_COL32(100, 149, 237, 255);

    // === �̴ϸ� ��Ÿ�� ===
    style.Colors[ImNodesCol_MiniMapBackground] = IM_COL32(25, 25, 25, 150);
    style.Colors[ImNodesCol_MiniMapBackgroundHovered] = IM_COL32(25, 25, 25, 200);
    style.Colors[ImNodesCol_MiniMapOutline] = IM_COL32(150, 150, 150, 100);
    style.Colors[ImNodesCol_MiniMapOutlineHovered] = IM_COL32(150, 150, 150, 200);
    style.Colors[ImNodesCol_MiniMapNodeBackground] = IM_COL32(200, 200, 200, 100);
    style.Colors[ImNodesCol_MiniMapNodeBackgroundHovered] = IM_COL32(200, 200, 200, 255);
    style.Colors[ImNodesCol_MiniMapNodeBackgroundSelected] = IM_COL32(255, 165, 0, 255);
    style.Colors[ImNodesCol_MiniMapNodeOutline] = IM_COL32(200, 200, 200, 100);
    style.Colors[ImNodesCol_MiniMapLink] = IM_COL32(200, 200, 200, 100);
    style.Colors[ImNodesCol_MiniMapLinkSelected] = IM_COL32(255, 165, 0, 255);

    // �̴ϸ� ũ�� �� ��ġ
    style.MiniMapPadding = ImVec2(8.0f, 8.0f);
    style.MiniMapOffset = ImVec2(4.0f, 4.0f);
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
		if (FAILED(Render_OverrideAnimControllers()))
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
		ImGui::Combo("##listener_combo", &m_iSelectedListenerIdx,
			listenerNames.data(), (int)listenerNames.size());
		ImGui::SameLine();
		if (ImGui::Button("Assign To Anim"))
		{
			m_pCurAnimation->AddEvent({ fCurTarckPos, listenerNames[m_iSelectedListenerIdx] });
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

	_float fRowH = ImGui::GetFrameHeightWithSpacing();
	ImGui::BeginChild("ParamScrollRegion", ImVec2(0, fRowH * 5 + ImGui::GetStyle().FramePadding.y * 2), true);

		// �÷� Name | Type | Value | Action
		ImGui::Columns(4, "ParamColumns", true);
		ImGui::Text("Name");   ImGui::NextColumn();
		ImGui::Text("Type");   ImGui::NextColumn();
		ImGui::Text("Value");  ImGui::NextColumn();
		ImGui::Text("Action"); ImGui::NextColumn();
		ImGui::Separator();

		unordered_map<string, Parameter>& parameters = m_pCurAnimator->GetParametersForEditor();
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
		ImGui::EndChild();

		// �Ķ���� �߰� �˾�
		if (ImGui::Button("Add Parameter"))
			ImGui::OpenPopup("AddParamPopup");

		if (ImGui::BeginPopup("AddParamPopup"))
		{
			ImGui::InputText("Name", m_NewParameterName, sizeof(m_NewParameterName));
			ImGui::Combo("Type", &m_iNewType, typeNames, IM_ARRAYSIZE(typeNames));
			if (ImGui::Button("Add"))
			{
				if (m_pCurAnimator)
				{
					_bool bExists = m_pCurAnimator->ExisitsParameter(m_NewParameterName);
					string diffName = m_NewParameterName + to_string(i);
					switch (m_iNewType)
					{	
					case 0: // Bool
						m_pCurAnimator->AddBool(bExists? diffName : m_NewParameterName);
						break;
					case 1: // Trigger
						m_pCurAnimator->AddTrigger(bExists ? diffName : m_NewParameterName);
						break;
					case 2: // Float
						m_pCurAnimator->AddFloat(bExists ? diffName : m_NewParameterName);
						break;
					case 3: // Int
						m_pCurAnimator->AddInt(bExists ? diffName : m_NewParameterName);
						break;
					default:
						break;
					}
					m_NewParameterName[0] = '\0'; // Ŭ����
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

	return S_OK;
}

HRESULT CAnimTool::Render_AnimControllers()
{
	ImGui::Begin("Controller"); 


	auto& ctrls = m_pCurAnimator->GetAnimControllers();
	vector<string> names;
	names.reserve(ctrls.size());
	for (auto& kv : ctrls) 
		names.push_back(kv.first);

	
	const _char* curName = names.empty() ? "" : names[m_iControllerIndex].c_str();
	if (ImGui::BeginCombo("##Controllers", curName))
	{
		for (_int i = 0; i < static_cast<_int>(names.size()); ++i)
		{
			_bool selected = (i == m_iControllerIndex);
			if (ImGui::Selectable(names[i].c_str(), selected))
			{
				m_iControllerIndex = i;
				m_pCurAnimator->SetCurrentAnimControllerForEditor(names[i]);
				// ���⼭�� ��Ʈ�ѷ� �ҷ��ͼ� ID �ٲ������
				auto pCtrl = m_pCurAnimator->Get_CurrentAnimController();
				if (pCtrl)
				{
					auto& states = pCtrl->GetStates();
					for (const auto& state : states)
					{
						m_iSpeicificNodeId = max(m_iSpeicificNodeId, state.iNodeId);
					}

					for (const auto& trns : pCtrl->GetTransitions())
					{
						m_iSpeicificNodeId = max(m_iSpeicificNodeId, trns.iFromNodeId);
						m_iSpeicificNodeId = max(m_iSpeicificNodeId, trns.iToNodeId);
					}
					m_iSpeicificNodeId++;
				}
			}
			if (selected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}


	if (!names.empty())
		strncpy_s(m_RenameControllerName, names[m_iControllerIndex].c_str(), 63);
	if (ImGui::InputText("Rename", m_RenameControllerName, sizeof(m_RenameControllerName),
		ImGuiInputTextFlags_EnterReturnsTrue))
	{
		string oldName = names[m_iControllerIndex];
		string newName = m_RenameControllerName;
		// �������� �� �̸� �ٲ㼭 �� ��������
		auto pCtrl= ctrls[oldName];
		m_pCurAnimator->RenameAnimController(oldName, newName);
		pCtrl->SetName(newName);
		// ���� �ε��� ����
		names[m_iControllerIndex] = newName;
	}


	ImGui::InputText("New Ctrl", m_NewControllerName, sizeof(m_NewControllerName));
	ImGui::SameLine();
	if (ImGui::Button("Create"))
	{
		string name = m_NewControllerName;
		if (!name.empty() && ctrls.find(name) == ctrls.end())
		{
			auto pNew = CAnimController::Create();
			pNew->SetName(name);
			m_pCurAnimator->RegisterAnimController(name, pNew);
			m_NewControllerName[0] = '\0';
		}
	}
	ImGui::End();

	return S_OK;
}

HRESULT CAnimTool::Render_OverrideAnimControllers()
{
	if (m_pCurAnimator == nullptr || m_pCurAnimator->Get_CurrentAnimController() == nullptr)
	{
		return S_OK;
	}

	ImGui::Begin("Override Anim Controllers");
	auto& ctrls = m_pCurAnimator->GetAnimControllers();
	ImGui::Checkbox("Use Override AnimCtrl", &m_bUseOverrideController);

	if (m_bUseOverrideController)
	{
		vector<string> overrideCtrlNames;
		overrideCtrlNames.reserve(m_pCurAnimator->GetOverrideAnimControllersMap().size()+1);
		overrideCtrlNames.push_back("None");
		for (const auto& Pair : m_pCurAnimator->GetOverrideAnimControllersMap())
		{
			overrideCtrlNames.push_back(Pair.first);
		}

		// ���� ��Ʈ�ѷ��� ������Ʈ�� �����ͼ� �����ϰ�
		auto pCtrl = m_pCurAnimator->Get_CurrentAnimController();
		auto& states = pCtrl->GetStates();
		if (m_NewOverrideAnimController.states.empty() || states.size()!= m_NewOverrideAnimController.states.size())
		{
			// �������̵� ��Ʈ�ѷ��� ��������� ���� ��Ʈ�ѷ��� ���µ�� �ʱ�ȭ
			for (const auto& state : states)
			{
				m_NewOverrideAnimController.states.emplace(state.stateName, OverrideAnimController::OverrideState{
					state.clip ? state.clip->Get_Name() : "",
					state.upperClipName,
					state.lowerClipName,
					state.maskBoneName,
					state.fBlendWeight
					});
			}
		}

		if (ImGui::BeginCombo("Override Ctrls", m_iSelectedOverrideControllerIndex >= 0 ? overrideCtrlNames[m_iSelectedOverrideControllerIndex].c_str() : "Select Ctrl"))
		{
			for (_int i = 0; i < overrideCtrlNames.size(); ++i)
			{
				_bool isSelected = (i == m_iSelectedOverrideControllerIndex);
				if (ImGui::Selectable(overrideCtrlNames[i].c_str(), isSelected))
				{
					m_iSelectedOverrideControllerIndex = i;
					// ���õ� �������̵� ��Ʈ�ѷ��� ���� �۾� ���� ��Ʈ�ѷ� ����
					m_NewOverrideAnimController = m_pCurAnimator->GetOverrideAnimControllersMap()[overrideCtrlNames[i]];
					// ���õ� ��Ʈ�ѷ��� �̸��� ���� �̸����� ����
					strcpy_s(m_OverrideControllerName, overrideCtrlNames[i].c_str());
				//	strcpy_s(m_NewOverrideControllerName, overrideCtrlNames[i].c_str());
					// ������ �ʵ� �ʱ�ȭ
					strcpy_s(m_OverrideControllerRename, overrideCtrlNames[i].c_str());
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// ���� ���õ� �������̵� ��Ʈ�ѷ��� �̸� ǥ��
		if (m_iSelectedOverrideControllerIndex >= 0)
		{
			ImGui::Text("Current Controller: %s", m_OverrideControllerName);

			// �̸� ���� �Է� �ʵ�
			if (ImGui::InputText("Rename Controller", m_OverrideControllerRename, sizeof(m_OverrideControllerRename),
				ImGuiInputTextFlags_EnterReturnsTrue))
			{
				string oldName = m_OverrideControllerName;
				string newName = m_OverrideControllerRename;
				if (newName != oldName && !newName.empty())
				{
					m_pCurAnimator->RenameOverrideAnimController(oldName, newName);
					strcpy_s(m_OverrideControllerName, newName.c_str());
					strcpy_s(m_NewOverrideControllerName, newName.c_str());
					// ���� �ε��� ������Ʈ (�̸��� �ٲ�����Ƿ�)
					m_iSelectedOverrideControllerIndex = -1;
				}
			}
		}

		// �������̵� �ؾ��� ������Ʈ�� �������̵� ��Ʈ�ѷ��� �߰�
		vector<string> stateNames;
		stateNames.reserve(m_NewOverrideAnimController.states.size());
		for (const auto& Pair : m_NewOverrideAnimController.states)
		{
			stateNames.push_back(Pair.first);
		}
		sort(stateNames.begin(), stateNames.end());

		if (ImGui::BeginCombo("Override States", m_iSelectedOverrideStateIndex >= 0 && m_iSelectedOverrideStateIndex < stateNames.size() ? stateNames[m_iSelectedOverrideStateIndex].c_str() : "Select State"))
		{
			for (_int i = 0; i < stateNames.size(); ++i)
			{
				_bool isSelected = (i == m_iSelectedOverrideStateIndex);
				if (ImGui::Selectable(stateNames[i].c_str(), isSelected))
				{
					m_iSelectedOverrideStateIndex = i;
					auto it = find_if(states.begin(), states.end(),
						[&](const CAnimController::AnimState& state) { return state.stateName == stateNames[i]; });

					if (it != states.end())
					{
						// ����ũ ���� ����ϴ� ������Ʈ���� Ȯ��
						m_bIsUesMaskBoneState = it->maskBoneName.empty() == false;
					}

					// ����� �������̵� ���¿��� �ִϸ��̼� �ε��� ã��
					auto& selectedState = m_NewOverrideAnimController.states[stateNames[i]];
					vector<CAnimation*> anims = m_LoadedAnimations[m_stSelectedModelName];

					// ���� �ִϸ��̼� �ε��� ã��
					m_iOverrideAnimIndex = -1;
					for (int j = 0; j < anims.size(); ++j)
					{
						if (anims[j]->Get_Name() == selectedState.clipName)
						{
							m_iOverrideAnimIndex = j;
							break;
						}
					}

					// ��ü �ִϸ��̼� �ε��� ã��
					m_iOverrideUpperAnimIndex = -1;
					for (int j = 0; j < anims.size(); ++j)
					{
						if (anims[j]->Get_Name() == selectedState.upperClipName)
						{
							m_iOverrideUpperAnimIndex = j;
							break;
						}
					}

					// ��ü �ִϸ��̼� �ε��� ã��
					m_iOverrideLowerAnimIndex = -1;
					for (int j = 0; j < anims.size(); ++j)
					{
						if (anims[j]->Get_Name() == selectedState.lowerClipName)
						{
							m_iOverrideLowerAnimIndex = j;
							break;
						}
					}
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// ���õ� �������̵� ���°� �ִٸ� �ش� ������ ������ �����ְ� ������ �� �ְ�
		if (m_iSelectedOverrideStateIndex >= 0 && m_iSelectedOverrideStateIndex < stateNames.size())
		{
			auto& selectedState = m_NewOverrideAnimController.states[stateNames[m_iSelectedOverrideStateIndex]];

			// ���� �̸�
			vector<CAnimation*> anims = m_LoadedAnimations[m_stSelectedModelName]; // ���� ���õ� ���� �ִϸ��̼ǵ�

			vector<string> animNames;
			animNames.reserve(anims.size());
			for (const auto& anim : anims)
			{
				animNames.push_back(anim->Get_Name());
			}

			if (ImGui::BeginCombo("Override Animations", m_iOverrideAnimIndex >= 0 && m_iOverrideAnimIndex < animNames.size() ? animNames[m_iOverrideAnimIndex].c_str() : "Override Select"))
			{
				for (_int i = 0; i < animNames.size(); ++i)
				{
					_bool isSelected = (i == m_iOverrideAnimIndex);
					if (ImGui::Selectable(animNames[i].c_str(), isSelected))
					{
						m_iOverrideAnimIndex = i;
						selectedState.clipName = anims[i]->Get_Name();
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (m_bIsUesMaskBoneState)
			{
				if (ImGui::BeginCombo("Override Upper Animations", m_iOverrideUpperAnimIndex >= 0 && m_iOverrideUpperAnimIndex < animNames.size() ? animNames[m_iOverrideUpperAnimIndex].c_str() : "Override Select Upper"))
				{
					for (_int i = 1; i < animNames.size(); ++i)
					{
						_bool isSelected = (i == m_iOverrideUpperAnimIndex);
						if (ImGui::Selectable(animNames[i].c_str(), isSelected))
						{
							m_iOverrideUpperAnimIndex = i;
							selectedState.upperClipName = anims[m_iOverrideUpperAnimIndex]->Get_Name();
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (ImGui::BeginCombo("Override Lower Animations", m_iOverrideLowerAnimIndex >= 0 && m_iOverrideLowerAnimIndex < animNames.size() ? animNames[m_iOverrideLowerAnimIndex].c_str() : "Override Select Lower"))
				{
					for (_int i = 1; i < animNames.size(); ++i)
					{
						_bool isSelected = (i == m_iOverrideLowerAnimIndex);
						if (ImGui::Selectable(animNames[i].c_str(), isSelected))
						{
							m_iOverrideLowerAnimIndex = i;
							selectedState.lowerClipName = anims[m_iOverrideLowerAnimIndex]->Get_Name();
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			else
			{
				m_iOverrideLowerAnimIndex = -1;
				m_iOverrideUpperAnimIndex = -1; // ����ũ ���� ������� �ʴ� ���¶�� ��ü, ��ü �ִϸ��̼� ���� �ʱ�ȭ
			}
		}

		// ��Ʈ�ѷ� �̸� ���� (���� ���� ����)
		if (m_iSelectedOverrideControllerIndex < 0)
		{
			if (ImGui::InputText("New Controller Name", m_NewOverrideControllerName, sizeof(m_NewOverrideControllerName),
				ImGuiInputTextFlags_EnterReturnsTrue))
			{
			}
		}

		if (ImGui::Button("Add Override Controller"))
		{
			if (m_bUseOverrideController)
			{
				if (m_OverrideControllerName[0] != '\0')
				{
					// ���� ��Ʈ�ѷ� �̸����� �߰�
					m_pCurAnimator->Add_OverrideAnimController(
						m_OverrideControllerName, m_NewOverrideAnimController);
				}
				else if (m_iSelectedOverrideControllerIndex >= 0)
				{
					// ���õ� ��Ʈ�ѷ��� �߰�
					m_pCurAnimator->Add_OverrideAnimController(
						m_OverrideControllerName, m_NewOverrideAnimController);
				}
				else
				if (m_NewOverrideControllerName[0] == '\0')
				{
					m_pCurAnimator->Add_OverrideAnimController(
						m_NewOverrideAnimController.name, m_NewOverrideAnimController);
				}
				
			}
		}

		if (ImGui::Button("Clear Override Controllers"))
		{
			m_NewOverrideAnimController.states.clear();
			m_NewOverrideControllerName[0] = '\0'; // Ŭ����
			m_OverrideControllerName[0] = '\0'; // Ŭ����
			m_OverrideControllerRename[0] = '\0'; // Ŭ����
			m_iSelectedOverrideControllerIndex = -1; // ���� �ʱ�ȭ
			m_iSelectedOverrideStateIndex = -1; // ���� �ʱ�ȭ
			m_iOverrideAnimIndex = -1; // �ִϸ��̼� ���� �ʱ�ȭ
			m_iOverrideUpperAnimIndex = -1; // ��ü �ִϸ��̼� ���� �ʱ�ȭ
			m_iOverrideLowerAnimIndex = -1; // ��ü �ִϸ��̼� ���� �ʱ�ȭ
		}

		if (ImGui::Button("Apply Override Controllers"))
		{
			if (m_bUseOverrideController)
			{
				if (m_iSelectedOverrideControllerIndex < 0)
				{
					// ���� �߰��� ��Ʈ�ѷ����
					m_pCurAnimator->Add_OverrideAnimController(
						m_NewOverrideControllerName, m_NewOverrideAnimController);
					// ���� �߰��� ��Ʈ�ѷ��� ���� ��Ʈ�ѷ��� ����
					m_pCurAnimator->ApplyOverrideAnimController(m_NewOverrideControllerName);
				}
				else
				{
					// ���� ��Ʈ�ѷ� ����
					m_pCurAnimator->ApplyOverrideAnimController(m_OverrideControllerName);
				}
			}
		}

		if (ImGui::Button("Cancel Override Controllers"))
		{
			m_pCurAnimator->CancelOverrideAnimController();
		}
	}
	ImGui::End();
	return S_OK;
}

HRESULT CAnimTool::Render_TransitionConditions()
{
	// ���� �߰��Ǿ��ִ� Ʈ�������� ���ǵ��� �����ְ� �޺��ڽ��� ������� ������ ������ �� �ְ� ó��
	if (m_pCurAnimator == nullptr || m_pCurAnimator->Get_CurrentAnimController() == nullptr)
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
		&m_bExpanded,
		&m_iSelectEntry,
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
	if (m_pCurAnimator == nullptr || m_pCurAnimator->Get_CurrentAnimController() == nullptr)
	{
		return S_OK;
	}
	if (FAILED(Render_AnimControllers()))
		return E_FAIL;

	_bool bApplyRootMotion = m_pCurAnimator->IsApplyRootMotion();
	ImGui::Checkbox("Apply Root Motion", &bApplyRootMotion);
	m_pCurAnimator->SetApplyRootMotion(bApplyRootMotion);

	if (ImGui::Button("Save AnimState This Model"))
	{
		SaveLoadAnimStates();
	}

	if (ImGui::Button("Load AnimState This Model"))
	{
		SaveLoadAnimStates(false);
		return S_OK; // �ҷ��� ���� �� ������ �ѱ��
	}

	// ��Ʈ�ѷ��� �ٲ�� �ĺ� ID �ʱ�ȭ ���ѳ���
	CAnimController* pCtrl = m_pCurAnimator->Get_CurrentAnimController();

	ImGui::Begin("Anim State Machine");

	if (ImGui::CollapsingHeader("Parameters"))
	{
		if (FAILED(Render_Parameters()))
			return E_FAIL;
	}

	ImGui::BeginChild("CategoryCheckList", ImVec2(120, 60), true); //  �ڽ� ������ ��ũ��

	ImGui::Checkbox("Show All Categories", &m_bShowAll);

	if (ImGui::IsItemEdited())
	{
		for (auto& Pair : m_CategoryVisibility)
		{
			Pair.second = m_bShowAll;
		}
	}

	for (auto& Pair : m_CategoryVisibility)
	{
		_bool& bIsVisible = Pair.second;
		if (ImGui::Checkbox(Pair.first.c_str(), &bIsVisible))
		{
			_bool allChecked = all_of(m_CategoryVisibility.begin(), m_CategoryVisibility.end(),
				[&](const auto& it) { return it.second; });
			m_bShowAll = allChecked;
		}
	}
	ImGui::EndChild();

	_int iNumSelectedNodes = ImNodes::NumSelectedNodes();
	_bool isAnyNodeSelected = iNumSelectedNodes > 0;
	_int iNumSelectedLinks = ImNodes::NumSelectedLinks();
	_bool isAnyLinkSelected = iNumSelectedLinks > 0;
	vector<_int> selectedLinkIds;
	if (isAnyLinkSelected)
	{
		selectedLinkIds.resize(ImNodes::NumSelectedLinks());
		ImNodes::GetSelectedLinks(selectedLinkIds.data());
	}
	else
	{
		selectedLinkIds.clear();
	}

	ImGui::Checkbox("Show All Link", &m_bShowAllLink);

	ImNodes::BeginNodeEditor();

	// ��Ŭ������ ��� �߰� �˾�
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImNodes::IsEditorHovered())
	{
		ImGui::OpenPopup("AddStatePopup");
	}

	if (ImGui::BeginPopup("AddStatePopup"))
	{

		static _bool bMaskBone = false;
		ImGui::InputText("State Name", m_NewStateName, IM_ARRAYSIZE(m_NewStateName));
		
		ImGui::Checkbox("Mask Bone", &bMaskBone);

		

		if (ImGui::Button("Add State"))
		{
			ImVec2 mouse = ImGui::GetMousePosOnOpeningCurrentPopup();
			ImVec2 canvasPos = ImNodes::EditorContextGetPanning();
			ImVec2 editorPos = ImVec2(mouse.x - canvasPos.x, mouse.y - canvasPos.y);
			CAnimation* selectedAnim = m_pCurAnimation;

			//size_t newIdx = pCtrl->AddState(stateName, selectedAnim, m_iSpeicificNodeId++);
			_int iNodeId = m_iSpeicificNodeId++; // ������ ��� ���̵� ����
			// ID Ȯ��
			for (auto& state : pCtrl->GetStates())
			{
				if (state.iNodeId == iNodeId)
				{
					iNodeId = m_iSpeicificNodeId++; // �ߺ��Ǹ� ���� ID��
					break;
				}
			}

			size_t newIdx = pCtrl->AddState(m_NewStateName, selectedAnim, iNodeId, bMaskBone);
			auto& newState = pCtrl->GetStatesForEditor()[newIdx];
			newState.fNodePos = { editorPos.x, editorPos.y };
			if (bMaskBone)
			{
				newState.maskBoneName = "Bip001-Spine2";
			}

			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}


	ImNodes::MiniMap(0.25f, 3);

	// �����ϱ�
	if (ImGui::IsKeyDown(ImGuiKey_L))
	{
		//ApplyHierarchicalLayout(pCtrl);
		ApplyCategoryLayout(pCtrl);
	//	ImNodes::EditorContextResetPanning(ImVec2(0.0f, 0.0f));
	}

	for (const auto& Pair : m_CategoryStates)
	{
		auto& category = Pair.first; // ī�װ�
		auto& states = Pair.second; // �ش� ī�װ��� ���µ�

		auto& transitions = pCtrl->GetTransitionsForEditor();
		for (const auto& visState : states)
		{
			_bool bIsVisible = m_bShowAll || m_CategoryVisibility[category];
			for (auto& state : pCtrl->GetStatesForEditor())
			{
				_bool isAny = (state.iNodeId == ANY_NODE_ID);
				_bool isExit = (state.iNodeId == EXIT_NODE_ID);


				if ((isAny || isExit) && !m_bShowAll)
					bIsVisible = true;
				else
					bIsVisible = m_bShowAll || m_CategoryVisibility[category];
				if (state.stateName != visState || !bIsVisible)
					continue;

				if (isAny) 
				{
					ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(60, 140, 170, 255));
					ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(75, 160, 190, 255));
					ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(90, 180, 210, 255));
					ImNodes::PushColorStyle(ImNodesCol_NodeOutline, IM_COL32(60, 140, 170, 255));
				}
				else if (isExit)
				{
					ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(185, 70, 70, 255));
					ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(205, 90, 90, 255));
					ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(225, 110, 110, 255));
					ImNodes::PushColorStyle(ImNodesCol_NodeOutline, IM_COL32(185, 70, 70, 255));
				}
				ImNodes::BeginNode(state.iNodeId);

				ImNodes::BeginNodeTitleBar();
				ImGui::TextUnformatted(state.stateName.c_str());

				if (state.iNodeId == pCtrl->GetEntryNodeId())
				{
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "[ENTRY]"); // ������� Entry ǥ��
				}
				//if (state.iNodeId == pCtrl->GetExitNodeId())
				//{
				//	ImGui::SameLine();
				//	ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "[EXIT]"); // ���������� Exit ǥ��
				//}
				ImNodes::EndNodeTitleBar();

				ImGui::BeginGroup();

				// ���� Ȱ�� �������� Ȯ�� (���� ��� ���� �ִϸ��̼� ����)
				_bool isCurrentState = (pCtrl->GetCurrentStateForEditor() &&
					pCtrl->GetCurrentStateForEditor()->iNodeId == state.iNodeId);

				if (isCurrentState)
				{
					ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "[ACTIVE]");

					// ���� �ִϸ��̼� ����� ǥ��
					_float progress = m_pCurAnimator->GetCurrentAnimProgress();
					ImGui::ProgressBar(progress, ImVec2(120, 0), "");
					ImGui::SameLine();
					ImGui::Text("%.1f%%", progress * 100.0f);
				}
				else
				{
					ImGui::Text("");
					ImGui::Dummy(ImVec2(120, ImGui::GetFrameHeight())); // ����� ũ�⸸ŭ 
				}

				// Pin
				ImGui::Columns(2, nullptr, false);
				ImGui::SetColumnWidth(0, 60);
				ImGui::SetColumnWidth(1, 60);

				if (!isAny)
				{
					const _int inPin = state.iNodeId * 10 + 1;
					ImNodes::BeginInputAttribute(inPin);
					ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.f), "In");
					ImNodes::EndInputAttribute();
				}

				ImGui::NextColumn();

				const _int outPin = state.iNodeId * 10 + 2;
				ImNodes::BeginOutputAttribute(outPin);
				ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.2f, 1.f), "Out");
				ImNodes::EndOutputAttribute();
				ImGui::Columns(1);
				ImGui::EndGroup();

				ImNodes::EndNode();

				// ��� ��ġ ����
				ImVec2 pos = ImNodes::GetNodeEditorSpacePos(state.iNodeId);
				state.fNodePos = { pos.x, pos.y };

				if (isAny || isExit)
				{
					ImNodes::PopColorStyle(); // NodeOutline
					ImNodes::PopColorStyle(); // TitleBarSelected
					ImNodes::PopColorStyle(); // TitleBarHovered
					ImNodes::PopColorStyle(); // TitleBar
				}
			}
		}
	}


	for (auto& t : pCtrl->GetTransitions())
	{
		//_bool bDrawLink = false;
		//_int startPinID = t.iFromNodeId * 10 + 2;  // Output Pin
		//_int endPinID = t.iToNodeId * 10 + 1;     // Input Pin
		//string fromName = pCtrl->GetStateNameByNodeId(t.iFromNodeId);
		//string toName = pCtrl->GetStateNameByNodeId(t.iToNodeId);
		//string fromCat = GetStateCategory(fromName);
		//string toCat = GetStateCategory(toName);

		//_bool fromVisible = m_bShowAll || m_CategoryVisibility[fromCat];
		//_bool toVisible = m_bShowAll || m_CategoryVisibility[toCat];

		//if (!fromVisible || !toVisible)
		//	continue;

		//if (isAnyLinkSelected)
		//{
		//	for (const auto& linkId : selectedLinkIds)
		//	{
		//		if (linkId == t.link.iLinkId)
		//		{
		//			bDrawLink = true;
		//			break;
		//		}
		//	}
		//}
		//else if (isAnyNodeSelected)
		//{
		//	_bool isFromSelected = ImNodes::IsNodeSelected(t.iFromNodeId);
		//	_bool isToSelected = ImNodes::IsNodeSelected(t.iToNodeId);

		//	if (isFromSelected || isToSelected)
		//	{
		//		bDrawLink = true;
		//		// �� �� �ϳ��� ������ ������
		//		//ImNodes::Link(t.link.iLinkId, startPinID, endPinID);
		//	}
		//}
		//else  // �ƹ��͵� ������ �ȵƴµ� ��ũ�� ���õȰ� ������
		//{
		//	bDrawLink = m_bShowAllLink;
		//}
		//if (bDrawLink)
		//{
		//	ImNodes::Link(t.link.iLinkId, startPinID, endPinID);
		//}

		const bool fromIsSpecial = (t.iFromNodeId == ANY_NODE_ID || t.iFromNodeId == EXIT_NODE_ID);
		const bool toIsSpecial = (t.iToNodeId == ANY_NODE_ID || t.iToNodeId == EXIT_NODE_ID);

		const _int startPinID = t.iFromNodeId * 10 + 2; // Out
		const _int endPinID = t.iToNodeId * 10 + 1; // In

		// ī�װ� ��� ���ü�
		string fromName = pCtrl->GetStateNameByNodeId(t.iFromNodeId);
		string toName = pCtrl->GetStateNameByNodeId(t.iToNodeId);
		string fromCat = GetStateCategory(fromName);
		string toCat = GetStateCategory(toName);

		_bool fromVisible = m_bShowAll || m_CategoryVisibility[fromCat];
		_bool toVisible = m_bShowAll || m_CategoryVisibility[toCat];

		// ���� ���� ���
		_bool thisLinkSelected = ImNodes::IsLinkSelected(t.link.iLinkId);
		_bool fromNodeSelected = ImNodes::IsNodeSelected(t.iFromNodeId);
		_bool toNodeSelected = ImNodes::IsNodeSelected(t.iToNodeId);
		_bool anySelectedThis = thisLinkSelected || fromNodeSelected || toNodeSelected;

		
		_bool passCategory = (fromVisible && toVisible);

	
		if (!passCategory && anySelectedThis && (fromIsSpecial || toIsSpecial))
			passCategory = true;

		if (!passCategory)
			continue;

		// �׸��� ���� ����
		_bool bDrawLink = false;
		if (isAnyLinkSelected) 
		{
			// ���õ� ��ũ��
			bDrawLink = thisLinkSelected;
		}
		else if (isAnyNodeSelected)
		{
			// ���õ� ���� ����� ��ũ��
			bDrawLink = (fromNodeSelected || toNodeSelected);
		}
		else 
		{
			// �Ϲ� ���
			bDrawLink = m_bShowAllLink;
		}

		if (bDrawLink)
			ImNodes::Link(t.link.iLinkId, startPinID, endPinID);
	}


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

				// Ʈ������ �߰�
				pCtrl->AddTransition(fromNodeID, toNodeID, link, 0.2f,true);
			}

		}
	}
	if (ImNodes::NumSelectedLinks() > 0 )
	{
		vector<int> selectedLinks(ImNodes::NumSelectedLinks());
		ImNodes::GetSelectedLinks(selectedLinks.data());
		auto& transitions = pCtrl->GetTransitionsForEditor();

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

			auto pCtrl = m_pCurAnimator->Get_CurrentAnimController();
		for (_int linkId : selectedLinks)
		{
			for (auto& transition : transitions)
			{
				if (transition.link.iLinkId == linkId)
				{
					ImGui::Begin("Transition Info");
					if (pCtrl->GetStateAnimationByNodeIdForEditor(transition.iFromNodeId))
					{

					auto FromNodeName = pCtrl->GetStateAnimationByNodeIdForEditor(transition.iFromNodeId)->Get_Name();
					if (FromNodeName.empty())
						FromNodeName = "Unknown";

					ImGui::Text("From Node: %s", FromNodeName.c_str());
					}

					if (pCtrl->GetStateAnimationByNodeIdForEditor(transition.iToNodeId))
					{
						auto ToNodeName = pCtrl->GetStateAnimationByNodeIdForEditor(transition.iToNodeId)->Get_Name();
						if (ToNodeName.empty())
							ToNodeName = "Unknown";
					ImGui::Text("To Node : %s", ToNodeName.c_str());
					}
					
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
		auto& states = pCtrl->GetStatesForEditor();
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
		m_iSelectedNodeID = hoveredNodeID;
	}
	ImGui::End();




	vector<CAnimation*> anims = m_LoadedAnimations[m_stSelectedModelName]; // ���� ���õ� ���� �ִϸ��̼ǵ�

	vector<string> animNames;
	animNames.reserve(anims.size()+1);
	animNames.push_back("None");
	for (const auto& anim : anims)
	{
		animNames.push_back(anim->Get_Name());
	}

	if (m_iSelectedNodeID != -1)
	{
		for (auto& state : pCtrl->GetStatesForEditor())
		{
			if (state.iNodeId == m_iSelectedNodeID)
			{
				ImGui::Begin("State Info");

				_char buf[64];
				strcpy_s(buf, state.stateName.c_str());
				ImGui::InputText("State Name", buf, IM_ARRAYSIZE(buf));
				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					state.stateName = buf; // �̸� ����
				}

				if (ImGui::Button("Active Upper MaskBone"))
				{
					if (state.maskBoneName.empty())
					{
						state.maskBoneName = "Bip001-Spine2"; // �⺻ ����ũ ��
					}
					else
					{
						state.maskBoneName.clear(); // ����ũ �� ����
					}
				}

				if (state.clip)
				{
					ImGui::Text("Clip: %s", state.clip->Get_Name().c_str());
					ImGui::Text("Node ID: %d", state.iNodeId);
					ImGui::Text("Duration: %.2f", state.clip->GetDuration());
					ImGui::Text("Current Track Position: %.2f", state.clip->GetCurrentTrackPosition());
					ImGui::Text("Tick Per Second: %.2f", state.clip->GetTickPerSecond());
					ImGui::Text("Loop: %s", state.clip->Get_isLoop() ? "True" : "False");
					if (m_pCurAnimation)
						ImGui::Text("CurAnimLoop: %s", m_pCurAnimation->Get_isLoop() ? "True" : "False");


					// Ŭ���� �־��� ��쿡�� ���� �ִϸ��̼��� state�� �ִϸ��̼����� ����
					m_pCurAnimation = state.clip;
					if (ImNodes::IsNodeSelected(m_iSelectedNodeID) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						pCtrl->SetState(m_iSelectedNodeID);
					}
				}
				else if (state.maskBoneName.empty() == false)
				{
					if (ImNodes::IsNodeSelected(m_iSelectedNodeID) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						pCtrl->SetState(m_iSelectedNodeID);
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
						const string& stateName = pCtrl->GetStateNameByNodeId(transition.iToNodeId);
						ImGui::Text("Transition to: %s", stateName.c_str());
						ImGui::Text("Transition to Node ID: %d", transition.iToNodeId);
						ImGui::Text("Link ID: %d", transition.link.iLinkId);
					}
				}


				if (state.maskBoneName.empty())
				{
					if (ImGui::BeginCombo("Clips", m_iDefualtSeletedAnimIndex >= 0 ? animNames[m_iDefualtSeletedAnimIndex].c_str() : "Select Clip"))
					{
						for (_int i = 0; i < animNames.size(); ++i)
						{
							_bool isSelected = (i == m_iDefualtSeletedAnimIndex);
							if (ImGui::Selectable(animNames[i].c_str(), isSelected))
							{
								m_iDefualtSeletedAnimIndex = i;
								if (i == 0) // "None"�� ���õ� ���
								{
									state.clip = nullptr;
								}
								else // �ٸ� �ִϸ��̼��� ���õ� ���
								{
									if (state.maskBoneName.empty())
									{
										state.clip = anims[i - 1];
									}
									else
									{
										state.clip = nullptr;
									}
								}
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

				}
				else if (state.maskBoneName.empty() == false)
				{
					ImGui::Text("Mask Bone Name: %s", state.maskBoneName.c_str());
					ImGui::Text("Upper Clip: %s", state.upperClipName.c_str());
					ImGui::Text("Lower Clip: %s", state.lowerClipName.c_str());

					// ����ũ �� ����
					auto& bones = m_pCurModel->Get_Bones();

					if (m_vecMaskBoneNames.empty())
					{
						m_vecMaskBoneNames.reserve(bones.size());
						for (const auto& bone : bones)
						{
							m_vecMaskBoneNames.push_back(bone->Get_Name());
						}
					}


					if (ImGui::BeginCombo("Mask Bone", m_iSelectedMaskBoneIndex >= 0 ? m_vecMaskBoneNames[m_iSelectedMaskBoneIndex].c_str() : "Select Mask Bone"))
					{
						for (_int i = 0; i < m_vecMaskBoneNames.size(); ++i)
						{
							_bool isSelected = (i == m_iSelectedMaskBoneIndex);
							if (ImGui::Selectable(m_vecMaskBoneNames[i].c_str(), isSelected))
							{
								m_iSelectedMaskBoneIndex = i;
								state.maskBoneName = m_vecMaskBoneNames[m_iSelectedMaskBoneIndex];
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					if (ImGui::BeginCombo("Upper Animations", m_iSelectedUpperAnimIndex >= 0 ? animNames[m_iSelectedUpperAnimIndex].c_str() : "Select Upper"))
					{
						for (_int i = 0; i < animNames.size(); ++i)
						{
							_bool isSelected = (i == m_iSelectedUpperAnimIndex);
							if (ImGui::Selectable(animNames[i].c_str(), isSelected))
							{
								m_iSelectedUpperAnimIndex = i;

								if (i == 0) // "None"�� ���õ� ���
								{
									state.upperClipName = ""; // ��ü �ִϸ��̼� �ʱ�ȭ
								}
								else // �ٸ� �ִϸ��̼��� ���õ� ���
								{
									state.upperClipName = anims[i-1]->Get_Name();
								}
					
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					if (ImGui::BeginCombo("Lower Animations", m_iSelectedLowerAnimIndex >= 0 ? animNames[m_iSelectedLowerAnimIndex].c_str() : "Select Lower"))
					{
						for (_int i = 0; i < animNames.size(); ++i)
						{
							_bool isSelected = (i == m_iSelectedLowerAnimIndex);
							if (ImGui::Selectable(animNames[i].c_str(), isSelected))
							{
								m_iSelectedLowerAnimIndex = i;
								if (i == 0) // "None"�� ���õ� ���
								{
									state.lowerClipName = ""; // ��ü �ִϸ��̼� �ʱ�ȭ
								}
								else // �ٸ� �ִϸ��̼��� ���õ� ���
								{
									state.lowerClipName = anims[i - 1]->Get_Name();
								}
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
				}
				if (Button("Set Entry This State"))
				{
					pCtrl->SetEntry(state.stateName);
				}

				// �ִϸ��̼� StartTime ���ϱ�
				// ����ũ ���� �ƴϸ� LowerStartTime�� ���� �ƴϸ� Upper����
				if (state.clip)
				{
					_float fStartTime = state.fLowerStartTime;
					if (ImGui::DragFloat("Start Time", &fStartTime, 0.01f, 0.f, 1.f, "%.2f"))
					{
						state.fLowerStartTime = fStartTime;
					}
				}
				else if (state.maskBoneName.empty() == false)
				{
					_float fUpperStartTime = state.fUpperStartTime;
					if (ImGui::DragFloat("Upper Start Time", &fUpperStartTime, 0.01f, 0.f, 1.f, "%.2f"))
					{
						state.fUpperStartTime = fUpperStartTime;
					}
					_float fLowerStartTime = state.fLowerStartTime;
					if (ImGui::DragFloat("Lower Start Time", &fLowerStartTime, 0.01f, 0.f, 1.f, "%.2f"))
					{
						state.fLowerStartTime = fLowerStartTime;
					}
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

	vector<string> modelNames;
	for (const auto& pair : m_LoadedModels)
		modelNames.push_back(pair.first);

	if (ImGui::BeginCombo("Models", m_iSelectedModelIndex >= 0 ? modelNames[m_iSelectedModelIndex].c_str() : "Select Model"))
	{
		for (_int i = 0; i < modelNames.size(); ++i)
		{
			_bool isSelected = (i == m_iSelectedModelIndex);
			if (ImGui::Selectable(modelNames[i].c_str(), isSelected))
			{
				m_iSelectedModelIndex = i;
				m_pCurModel = m_LoadedModels[modelNames[i]];

				m_pCurAnimator = m_LoadedAnimators[modelNames[i]];
				// ���� �ٲ� ���� ���� �ִϸ��̼��� nullptr
				m_pCurAnimation = nullptr;
				m_stSelectedModelName = modelNames[i];
				m_vecMaskBoneNames.clear(); // ����ũ �� �̸� �ʱ�ȭ
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

	m_pCurModel->Update_Bones();
}

void CAnimTool::SelectAnimation()
{
	if (m_pCurModel == nullptr || m_pCurAnimator == nullptr)
	{
		return;
	}

	vector<CAnimation*> anims = m_LoadedAnimations[m_stSelectedModelName]; // ���� ���õ� ���� �ִϸ��̼ǵ�

	vector<string> animNames;
	animNames.reserve(anims.size());
	for (const auto& anim : anims)
	{
		animNames.push_back(anim->Get_Name());
	}

	if (ImGui::BeginCombo("Animations", m_iSelectedAnimIndex >= 0 ? animNames[m_iSelectedAnimIndex].c_str() : "Select Animation"))
	{
		for (_int i = 0; i < animNames.size(); ++i)
		{
			_bool isSelected = (i == m_iSelectedAnimIndex);
			if (ImGui::Selectable(animNames[i].c_str(), isSelected))
			{
				m_iSelectedAnimIndex = i;
				m_pCurAnimation = anims[m_iSelectedAnimIndex];
				m_pCurAnimator->PlayClip(anims[m_iSelectedAnimIndex], false);
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// �ִϸ��̼� ������Ƽ�� ����
	Setting_AnimationProperties();
}

void CAnimTool::Setting_AnimationProperties()
{
	// ������ �ִϸ��̼ǿ� ������ �������
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
		_bool bRootMotion = m_pCurAnimation->IsRootMotionEnabled();
		if (ImGui::Checkbox("Loop", &bLoop))
		{
			m_pCurAnimation->SetLoop(bLoop);
			bChanged = true;
		}
		if (ImGui::Checkbox("Root Motion", &bRootMotion))
		{
			m_pCurAnimation->SetUseRootMotion(bRootMotion);
			bChanged = true;
		}
		if (bChanged)
		{
			m_pCurAnimation->SetTickPerSecond(fTickPerSecond);

			auto& anims = m_LoadedAnimations[m_stSelectedModelName];
			auto it = find_if(anims.begin(), anims.end(),
				[&](CAnimation* anim) { return anim->Get_Name() == m_pCurAnimation->Get_Name(); });

			if (it != anims.end())
			{
				(*it)->SetLoop(bLoop);
				(*it)->SetTickPerSecond(fTickPerSecond);
			}
		}
	}

}

void CAnimTool::ApplyHierarchicalLayout(CAnimController* pCtrl)
{
	set<_int>visited;
	//map<_int, _bool> visited;
	map<_int, _int> level;
	map<_int, _int> nodeOrderInLevel; // �� ���� ������ ����� ����

	
	queue<_int> q;

	// ENTRY ��带 ã�� ť�� �ְ�, �湮 ó�� �� 0������ ����
	_int entryNodeId = pCtrl->GetEntryNodeId();
	q.push(entryNodeId);
	visited.insert(entryNodeId);
	level[entryNodeId] = 0;
	nodeOrderInLevel[0] = 0;

	// BFS 
	while (!q.empty())
	{
		_int currentNodeId = q.front();
		q.pop();

		// ���� ��忡�� ������ ��ȯ ã��
		for (const auto& transition : pCtrl->GetTransitions())
		{
			if (transition.iFromNodeId == currentNodeId)
			{
				_int nextNodeId = transition.iToNodeId;
				if (!visited.count(nextNodeId))
				{
					visited.insert(nextNodeId); // �湮 ó��
					level[nextNodeId] = level[currentNodeId] + 1; // ���� ���� ���� ��庸�� �� �ܰ� �Ʒ�
					q.push(nextNodeId);
				}
			}
		}
	}

	// ���� ������ ������ ������� ��� ��ġ ����
	map<_int, _int> levelNodeCount;
	_float horizontalSpacing = 220.0f; // ��� �� ���� ����
	_float verticalSpacing = 200.0f;   // ��� �� ���� ����

	// ��带 �������� �׷�ȭ
	map<_int,vector<_int>> nodesByLevel;
	for (const auto& state : pCtrl->GetStates())
	{
		nodesByLevel[level[state.iNodeId]].push_back(state.iNodeId);
	}

	// �� ������ ��� ��ġ�� ����Ͽ� ����
	for (auto const& [nodeLevel, nodeIds] : nodesByLevel)
	{
		_float startX = 0.0f;
		// �ʿ信 ���� �� ������ �߾ӿ� ���� ����
		startX = -(static_cast<_float>(nodeIds.size() - 1) / 2.0f) * horizontalSpacing;

		for (size_t i = 0; i < nodeIds.size(); ++i)
		{
			_int nodeId = nodeIds[i];
			ImVec2 newPos = ImVec2(startX + i * horizontalSpacing, nodeLevel * verticalSpacing);

			ImNodes::SetNodeEditorSpacePos(nodeId, newPos);
		}
	}
}

void CAnimTool::ApplyCategoryLayout(CAnimController* pCtrl)
{
	//const auto& states = pCtrl->GetStates();

	//// ���¸� ī�װ����� �з�
	//map<string, vector<_int>> categories;
	//m_CategoryStates.clear();
	//for (const auto& state : states)
	//{
	//	string category = GetStateCategory(state.stateName);
	//	categories[category].push_back(state.iNodeId);
	//	if (m_CategoryVisibility.find(category) == m_CategoryVisibility.end())
	//		m_CategoryVisibility[category] = true; // ī�װ� vis �ʱ�ȭ
	//	m_CategoryStates[category].push_back(state.stateName); // ī�װ��� state �̸� �߰�
	//}


	//_float categorySpacing = 600.0f;
	//_float nodeSpacing = 300.0f;
	//_int categoryIndex = 0;

	//for (auto& [categoryName, nodeIds] : categories)
	//{
	//	if (!m_bShowAll && m_CategoryVisibility[categoryName] == false)
	//		continue;
	//	// �� ī�װ��� ���η� ����
	//	_float categoryX = categoryIndex * categorySpacing;

	//	// ī�װ� �� ������ ���ڷ� ��ġ
	//	_int nodesPerRow = max(1, (_int)sqrt(nodeIds.size()));

	//	for (_int i = 0; i < static_cast<_int>(nodeIds.size()); ++i)
	//	{
	//		_int row = i / nodesPerRow;
	//		_int col = i % nodesPerRow;

	//		ImVec2 pos = ImVec2(
	//			categoryX + col * nodeSpacing,
	//			row * nodeSpacing
	//		);
	//		ImNodes::SetNodeEditorSpacePos(nodeIds[i], pos);
	//	}

	//	categoryIndex++;
	//}

	const auto& states = pCtrl->GetStates();
	const auto& transitions = pCtrl->GetTransitions();

	// ---- 0) ��� ���(Any/Exit) ���� ���� ��ġ ----
	constexpr float kHubY = -220.f;     // ��ܿ� ����
	constexpr float kAnyX = -500.f;
	// Exit X�� ���߿� ī�װ� ������ ���

	// ---- 1) ī�װ� ���� + ���ü� �ʱ�ȭ ----
	std::map<std::string, std::vector<_int>> categories; // �̸��� ���� ����
	m_CategoryStates.clear();

	// ���� ��� ���� ���(���/�߰� ��� ���� ��ġ)
	std::unordered_map<_int, int> degree;
	degree.reserve(states.size());
	for (const auto& t : transitions) {
		degree[t.iFromNodeId]++; degree[t.iToNodeId]++;
	}

	for (const auto& st : states)
	{
		std::string cat = GetStateCategory(st.stateName);
		categories[cat].push_back(st.iNodeId);

		if (m_CategoryVisibility.find(cat) == m_CategoryVisibility.end())
			m_CategoryVisibility[cat] = true;

		m_CategoryStates[cat].push_back(st.stateName);
	}

	// ---- 2) ��ġ �Ķ���� ----
	const float categorySpacingX = 720.f;   // ī�װ�(�÷�) �� X ����
	const float nodeSpacingX = 280.f;   // ���� ī�װ� �� X ����
	const float nodeSpacingY = 170.f;   // ���� ī�װ� �� Y ����
	const int   MAX_COLS = 3;       // �� ī�װ� �ִ� �� ��(�ʹ� ���η� ������� �� ����)
	const bool  snakeLayout = true;    // ������� ��ġ�� ��ũ ���� ��ȭ

	// ---- 3) ���� ī�װ� ��� ���� ----
	std::vector<std::pair<std::string, std::vector<_int>>> visibleCats;
	visibleCats.reserve(categories.size());
	for (auto& kv : categories) {
		if (m_bShowAll || m_CategoryVisibility[kv.first])
			visibleCats.push_back(kv);
	}
	if (visibleCats.empty())
		return;

	// ---- 4) �� ī�װ��� rows/cols ���, ��ü���� maxRows ����(���� �߾����Ŀ�) ----
	vector<int> catCols(visibleCats.size());
	vector<int> catRows(visibleCats.size());
	int globalMaxRows = 1;

	for (size_t i = 0; i < visibleCats.size(); ++i)
	{
		int n = static_cast<int>(visibleCats[i].second.size());
		int cols = max(1, (int)floor(sqrt((float)n)));
		cols = min(cols, MAX_COLS);
		int rows = (n + cols - 1) / cols;

		catCols[i] = max(1, cols);
		catRows[i] = max(1, rows);
		globalMaxRows = max(globalMaxRows, rows);
	}

	// ---- 5) ī�װ��� ���� ��Ģ(���/�߰� ��� �켱) ----
	for (auto& kv : visibleCats) {
		auto& list = kv.second;
		sort(list.begin(), list.end(), [&](int a, int b) {
			int da = degree.count(a) ? degree[a] : 0;
			int db = degree.count(b) ? degree[b] : 0;
			if (da != db) return da > db;            // ���� ���� ��� ����
			return a < b;                             // �����ϸ� ID ��������
			});
	}

	auto ClusterKey = [](const string& name)->string
		{
			size_t p1 = name.find('_');
			if (p1 == string::npos) return name;

			size_t p2 = name.find('_', p1 + 1);
			if (p2 == string::npos) return name.substr(0, p1); // 1��ū
			return name.substr(0, p2);                          // 2��ū
		};

	// ---- 6) ī�װ��� �÷����� ��ġ(Ŭ������ + �ִ� ��� ����) ----
	const int   kMaxRowsPerCol = 6;     // �� ���� ����(�� ��) ����
	const float kColGapX = 40.f;  // ���� ��Ŷ(Column) �� ����

	for (size_t ci = 0; ci < visibleCats.size(); ++ci)
	{
		const auto& catName = visibleCats[ci].first;
		const auto& nodeIds = visibleCats[ci].second;

		// 6-1) �� ī�װ� ��带 �̸� �����Ƚ�(�� 1~2 ��ū)�� ��Ŷȭ
		map<string, vector<_int>> buckets;
		for (int id : nodeIds)
		{
			auto* st = pCtrl->GetStateByNodeIdForEditor(id);
			const string nm = st ? st->stateName : "";
			buckets[ClusterKey(nm)].push_back(id);
		}

		// 6-2) ī�װ� ���� X
		float baseX = (float)ci * categorySpacingX;

		// 6-3) ���� �߾� ���Ŀ� �뷫 ����(Y ������)
		const float yBase = -0.5f * (kMaxRowsPerCol - 1) * nodeSpacingY;

		// 6-4) ��Ŷ ������ ��ġ: ��Ŷ���� ���� kMaxRowsPerCol���� �װ� ��ġ�� �� ���� wrap
		float curX = baseX;
		for (auto& kv : buckets)
		{
			auto& ids = kv.second;
			sort(ids.begin(), ids.end()); // ������(���ϸ� ����)

			int row = 0;
			float colX = curX;

			for (int idx = 0; idx < (int)ids.size(); ++idx)
			{
				if (row >= kMaxRowsPerCol) // �ٹٲ�
				{
					row = 0;
					colX += nodeSpacingX + kColGapX;
				}

				ImVec2 pos = ImVec2(colX, yBase + row * nodeSpacingY);
				ImNodes::SetNodeEditorSpacePos(ids[idx], pos);
				++row;
			}

			// ���� ��Ŷ�� �� ĭ ����� ����
			curX = colX + nodeSpacingX + kColGapX * 2.f;
		}
	}
	// ---- 7) ��� ���(Any/Exit) ���� ��ġ ���� ----
	// Any�� ���� ���, Exit�� ������ ī�װ� ������ ������� ����
	const float exitX = (float)(visibleCats.size()) * categorySpacingX + 200.f;
	if (pCtrl->GetStateByNodeIdForEditor(ANY_NODE_ID))
		ImNodes::SetNodeEditorSpacePos(ANY_NODE_ID, ImVec2(kAnyX, kHubY));
	if (pCtrl->GetStateByNodeIdForEditor(EXIT_NODE_ID))
		ImNodes::SetNodeEditorSpacePos(EXIT_NODE_ID, ImVec2(exitX, kHubY));
}

void CAnimTool::Test_AnimEvents()
{

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
			json j = m_pCurAnimator->Serialize();
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
			auto pCtrl = m_pCurAnimator->Get_CurrentAnimController();
			auto states = pCtrl->GetStates();
			auto transitions = pCtrl->GetTransitions();

			for (const auto& state : states)
			{
				m_iSpeicificNodeId = max(m_iSpeicificNodeId, state.iNodeId);
			}

			for (const auto& transition : transitions)
			{
				m_iSpeicificNodeId = max(m_iSpeicificNodeId, transition.link.iLinkId);
				m_iSpeicificNodeId = max(m_iSpeicificNodeId, transition.iFromNodeId);
			}
			m_iSpeicificNodeId += 1;
		}
		else
		{
			MSG_BOX("�ִϸ����Ͱ� �����ϴ�. �ִϸ����͸� ���� �������ּ���.");
		}
		m_CategoryStates.clear();
		ApplyCategoryLayout(m_pCurAnimator->Get_CurrentAnimController());
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
		auto& pAnimations = pModel->GetAnimations();
		m_LoadedAnimations[modelName] = pAnimations;

		auto pAnimator = CAnimator::Create(m_pDevice, m_pContext);
		if (pAnimator)
		{
			CAnimator::ANIMATOR_DESC desc;
			desc.pModel = pModel;

			//pAnimator->Initialize_Test(pModel);
			pAnimator->Initialize_Test(&desc);
			m_LoadedAnimators[modelName] = pAnimator;
			pAnimator->RegisterEventListener("TestEvent", [&](const string& eventName)
				{
					MSG_BOX("�ִϸ��̼� �̺�Ʈ �߻�");
				});
		}

		// �� �ҷ����� ó�� �ִϸ��̼� ���� ������ �ҷ����� 
		SaveLoadEvents(false);
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

string CAnimTool::GetStateCategory(const string& stateName)
{
	if (stateName.empty())
		return "Other";
	if (stateName.find("Hit") != string::npos)
		return "Hit";
	if (stateName.find("Guard") != string::npos)
		return "Guard";
	else if (stateName.find("Walk") != string::npos)
		return "Walk";
	else if(stateName.find("Run") != string::npos)
		return "Run";
	else if(stateName.find("Dash") != string::npos)
		return "Dash";
	else if (stateName.find("Attack") != string::npos ||
		stateName.find("Skill") != string::npos|| stateName.find("Atk") != string::npos)
		return "Attack";
	else if (stateName.find("Idle") != string::npos)
		return "Idle";
	else if (stateName.find("Spawn") != string::npos)
		return "Spawn";
	else if (stateName.find("Any") != string::npos || stateName.find("Exit") != string::npos ||
		stateName.find("Entry") != string::npos)
		return "Default";
	else if (stateName.find("Death") != string::npos ||
		stateName.find("Die") != string::npos)
		return "Death";
	else
		return "Other";
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

if (ImGui::CollapsingHeader("Conditions", ImGuiTreeNodeFlags_DefaultOpen))
{
	// �Ķ���� ��� �غ�
	auto& parameters = m_pCurAnimator->GetParametersForEditor();
	vector<string> paramNames;
	paramNames.reserve(parameters.size());
	for (auto& kv : parameters)
		paramNames.push_back(kv.first);

	// ���� ���ǵ� ����
	for (_int idx = 0; idx < static_cast<_int>(transition.conditions.size());)
	{
		auto& cond = transition.conditions[idx];
		ImGui::PushID(idx);
		ImGui::Separator();

		//Param ����
		_int selParam = -1;
		for (_int i = 0; i < static_cast<_int>(paramNames.size()); ++i)
			if (paramNames[i] == cond.paramName) 
				selParam = i;

		if (ImGui::BeginCombo("Param", selParam >= 0 ? paramNames[selParam].c_str() : "Select"))
		{
			for (_int i = 0; i < static_cast<_int>(paramNames.size()); ++i)
			{
				bool isSelected = (i == selParam);
				if (ImGui::Selectable(paramNames[i].c_str(), isSelected))
				{
					if (cond.paramName != paramNames[i])
					{
						cond.paramName = paramNames[i];
						cond.type = parameters[cond.paramName].type; // �Ķ���� Ÿ�� ������Ʈ

						if (cond.type == ParamType::Float) 
						{
							cond.fThreshold = 0.0f;
						}
						else if (cond.type == ParamType::Int) 
						{
							cond.iThreshold = 0;
						}

					}
				}
				if (isSelected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		switch (cond.type)
		{
		case ParamType::Bool:
		{
			_int opIdx = 0;
			for (_int i = 0; i < IM_ARRAYSIZE(BoolOps); ++i)
				if (BoolOps[i] == cond.op) opIdx = i;
			ImGui::Combo("Operator", &opIdx, BoolOpNames, IM_ARRAYSIZE(BoolOpNames));
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
			_int opIdx = 0;
			for (_int i = 0; i < IM_ARRAYSIZE(CmpFloatOps); ++i)
				if (CmpFloatOps[i] == cond.op) opIdx = i;
			ImGui::Combo("Operator", &opIdx, CmpFloatOpNames, IM_ARRAYSIZE(CmpFloatOpNames));
				cond.op = CmpFloatOps[opIdx];
			break;
		}
		case ParamType::Int:
		{
			_int opIdx = 0;
			for (_int i = 0; i < IM_ARRAYSIZE(CmpIntOps); ++i)
				if (CmpIntOps[i] == cond.op) opIdx = i;
			ImGui::Combo("Operator", &opIdx, CmpIntOpNames, IM_ARRAYSIZE(CmpIntOpNames));
				cond.op = CmpIntOps[opIdx];
			break;
		}
		}

		if (cond.op == CAnimController::EOp::Greater ||
			cond.op == CAnimController::EOp::Less ||
			cond.op == CAnimController::EOp::NotEqual ||
			cond.op == CAnimController::EOp::Equal)
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
		newCond.fThreshold = 0.f;
		newCond.iThreshold = 0;
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


	////m_pContext->Flush();
	//if (FAILED(m_pAnimShader->Bind_SRV("g_FinalBoneMatrices", m_pCurAnimator->GetFinalBoneMatricesSRV())))
	//	return E_FAIL;

	
	//if (KEY_PRESSING(DIK_I))
	//{
	//	auto tmp = m_pCurAnimator->DebugGetFinalBoneMatrices();
	//	for (int i = 0;i<20;i++)
	//	{
	//		auto mat = tmp[i];
	//		cout << "---------GPU ���---------------" << endl;
	//		cout << mat.m[0][0] << " " << mat.m[0][1] << " " << mat.m[0][2] << " " << mat.m[0][3] << endl;
	//		cout << mat.m[1][0] << " " << mat.m[1][1] << " " << mat.m[1][2] << " " << mat.m[1][3] << endl;
	//		cout << mat.m[2][0] << " " << mat.m[2][1] << " " << mat.m[2][2] << " " << mat.m[2][3] << endl;
	//		cout << mat.m[3][0] << " " << mat.m[3][1] << " " << mat.m[3][2] << " " << mat.m[3][3] << endl;
	//		cout << "------------------------" << endl;

	//		auto mat2 = *m_pCurModel->Get_Bones()[i]->Get_CombinedTransformationMatrix();

	//		cout << "---------CPU ���---------------" << endl;
	//		cout << mat2.m[0][0] << " " << mat2.m[0][1] << " " << mat2.m[0][2] << " " << mat2.m[0][3] << endl;
	//		cout << mat2.m[1][0] << " " << mat2.m[1][1] << " " << mat2.m[1][2] << " " << mat2.m[1][3] << endl;
	//		cout << mat2.m[2][0] << " " << mat2.m[2][1] << " " << mat2.m[2][2] << " " << mat2.m[2][3] << endl;
	//		cout << mat2.m[3][0] << " " << mat2.m[3][1] << " " << mat2.m[3][2] << " " << mat2.m[3][3] << endl;
	//		cout << "------------------------" << endl;

	//	}
	////	m_pCurAnimator->DebugComputeShader();
	//}
	//auto tmp = m_pCurAnimator->DebugGetFinalBoneMatrices();
	//for (int i = 0;i<m_pCurModel->Get_Bones().size();i++)
	//{
	//	auto& bone = m_pCurModel->Get_Bones()[i];
	//	// bone->Get_CombinedTransformationMatrix()�� CPU���� ���� �� ���
	//	bone->Set_CombinedTransformationMatrix(XMLoadFloat4x4(&tmp[i]));
	//}

	_uint		iNumMesh = m_pCurModel->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pCurModel->Bind_Material(m_pAnimShader, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
		{

		}
			//	return E_FAIL;

		//m_pCurModel->Bind_SkinningSRVs(m_pAnimShader, i);
		m_pCurModel->Bind_Bone_Matrices(m_pAnimShader, "g_BoneMatrices", i);

		if (FAILED(m_pAnimShader->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pCurModel->Render(i)))
			return E_FAIL;
	}

	//ID3D11ShaderResourceView* cur[3]{};
	//m_pContext->VSGetShaderResources(0, 3, cur);
	//auto* expect = m_pCurAnimator->GetFinalBoneMatricesSRV();
	//assert(cur[0] == expect); // �� ���� ����ؾ� ����
	//for (auto& s : cur) if (s) s->Release();
	//ID3D11UnorderedAccessView* nullUAV = nullptr;
	//UINT counts = 0;
	//m_pContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, &counts);
	//ID3D11ShaderResourceView* nullSRV[3]{ nullptr, nullptr, nullptr };


	//// VS �ܰ� t0,t1,t2 ���Կ��� ����ε�
	//m_pContext->VSSetShaderResources(0, 3, nullSRV);

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

	//for (auto& pair : m_LoadedAnimations)
	//{
	//	for (auto& anim : pair.second)
	//		Safe_Release(anim);
	//	pair.second.clear();
	//}
	ImNodes::DestroyContext();
	Safe_Release(m_pEventMag);
	Safe_Delete(m_pMySequence);
	Safe_Release(m_pAnimShader);
	Safe_Release(m_pGameInstance);
}



#endif