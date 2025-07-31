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
					//m_NewOverrideAnimController = m_pCurAnimator->GetOverrideAnimControllersMap()[overrideCtrlNames[i]];
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
			if (m_bUseOverrideController && !m_NewOverrideControllerName[0] == '\0')
			{
				m_pCurAnimator->Add_OverrideAnimController(
					m_NewOverrideControllerName, m_NewOverrideAnimController);
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
			const ImVec2 mousePos = ImNodes::EditorContextGetPanning(); // ���� ���콺 ��ġ
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
			auto& newState = pCtrl->GetStates()[newIdx];
			newState.fNodePos = { mousePos.x, mousePos.y };
			if (bMaskBone)
			{
				newState.maskBoneName = "Bip001-Spine2";
			}

			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	auto& transitions = pCtrl->GetTransitions();

	for (auto& state : pCtrl->GetStates())
	{
		ImNodes::BeginNode(state.iNodeId);

		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted(state.stateName.c_str());

		if (state.iNodeId == pCtrl->GetEntryNodeId())
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "[ENTRY]"); // ������� Entry ǥ��
		}
		if (state.iNodeId == pCtrl->GetExitNodeId())
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "[EXIT]"); // ���������� Exit ǥ��
		}
		ImNodes::EndNodeTitleBar();

		ImGui::BeginGroup();

		// ���� Ȱ�� �������� Ȯ�� (���� ��� ���� �ִϸ��̼� ����)
		_bool isCurrentState = (pCtrl->GetCurrentState() &&
			pCtrl->GetCurrentState()->iNodeId == state.iNodeId);

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

		_int inCount = 0;
		for (auto& t : transitions) 
			if (t.iToNodeId == state.iNodeId) 
				++inCount;

		int pinId = state.iNodeId * 10 + 1;
		ImNodes::BeginInputAttribute(pinId);
		ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.f), "In");
		ImNodes::EndInputAttribute();

		ImGui::NextColumn();
		_int outCount = 0;
		for (auto& t : transitions) 
			if (t.iFromNodeId == state.iNodeId)
				++outCount;

		pinId = state.iNodeId * 10 + 2;
		ImNodes::BeginOutputAttribute(pinId);
		ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.2f, 1.f), "Out");
		ImNodes::EndOutputAttribute();

		ImGui::Columns(1);
		ImGui::EndGroup();

		ImNodes::EndNode();

		// ��� ��ġ ����
		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(state.iNodeId);
		state.fNodePos = { pos.x, pos.y };
	}

	for (auto& t : pCtrl->GetTransitions())
	{
		_int startPinID = t.iFromNodeId * 10 + 2;  // Output Pin
		_int endPinID = t.iToNodeId * 10 + 1;     // Input Pin
		ImNodes::Link(t.link.iLinkId, startPinID, endPinID);
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

				// Ʈ������ �߰�
				pCtrl->AddTransition(fromNodeID, toNodeID, link, 0.2f,true);
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

			auto pCtrl = m_pCurAnimator->Get_CurrentAnimController();
		for (_int linkId : selectedLinks)
		{
			for (auto& transition : transitions)
			{
				if (transition.link.iLinkId == linkId)
				{
					ImGui::Begin("Transition Info");
					if (pCtrl->GetStateAnimationByNodeId(transition.iFromNodeId))
					{

					auto FromNodeName = pCtrl->GetStateAnimationByNodeId(transition.iFromNodeId)->Get_Name();
					if (FromNodeName.empty())
						FromNodeName = "Unknown";

					ImGui::Text("From Node: %s", FromNodeName.c_str());
					}

					if (pCtrl->GetStateAnimationByNodeId(transition.iToNodeId))
					{
						auto ToNodeName = pCtrl->GetStateAnimationByNodeId(transition.iToNodeId)->Get_Name();
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
		for (auto& state : pCtrl->GetStates())
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
								if (state.maskBoneName.empty()) // ����ũ �� �̸��� ���� ���� 
									state.clip = anims[m_iDefualtSeletedAnimIndex];
								else
								{
									state.clip = nullptr;
								}

								if ("None" == animNames[m_iDefualtSeletedAnimIndex])
								{
									state.clip = nullptr; // "None" ���ý� Ŭ�� �ʱ�ȭ
								}
								else
								{
									state.clip = anims[m_iDefualtSeletedAnimIndex];
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
								state.upperClipName = anims[m_iSelectedUpperAnimIndex]->Get_Name();
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
								state.lowerClipName = anims[m_iSelectedLowerAnimIndex]->Get_Name();
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
			pAnimator->Initialize(pModel);
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
	auto& parameters = m_pCurAnimator->GetParameters();
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
