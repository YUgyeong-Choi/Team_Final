#ifdef USE_IMGUI
#include "AnimTool.h"

#include "Bone.h"
#include "Mesh.h"
#include "Fuoco.h"
#include "Player.h"
#include "EventMag.h"
#include "Elite_Police.h"
#include "GameInstance.h"
#include "Buttler_Range.h"
#include "Buttler_Train.h"
#include "FestivalLeader.h"	
#include "EditorObjectFactory.h"


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

	m_pEditorObjectFactory = CEditorObjectFactory::Create(m_pDevice, m_pContext);
	if (nullptr == m_pEditorObjectFactory)
		return E_FAIL;
	if (FAILED(Register_Objects()))
		return E_FAIL;

	InitImNodesStyle();

	return S_OK;
}


void CAnimTool::InitImNodesStyle()
{
	ImNodesStyle& style = ImNodes::GetStyle();

	style.NodeCornerRounding = 6.0f;
	style.NodePadding = ImVec2(10.0f, 6.0f);
	style.NodeBorderThickness = 2.0f;
	style.PinCircleRadius = 7.0f;
	style.PinQuadSideLength = 10.0f;
	style.PinTriangleSideLength = 12.0f;
	style.LinkThickness = 3.0f;
	style.LinkLineSegmentsPerLength = 0.0f;
	style.LinkHoverDistance = 10.0f;
	style.GridSpacing = 32.0f;
	style.MiniMapPadding = ImVec2(8.0f, 8.0f);
	style.MiniMapOffset = ImVec2(4.0f, 4.0f);

	style.Colors[ImNodesCol_NodeBackground] = IM_COL32(56, 56, 56, 255);
	style.Colors[ImNodesCol_NodeBackgroundHovered] = IM_COL32(70, 70, 70, 255);
	style.Colors[ImNodesCol_NodeBackgroundSelected] = IM_COL32(90, 90, 90, 255);
	style.Colors[ImNodesCol_NodeOutline] = IM_COL32(128, 128, 128, 255);
	style.Colors[ImNodesCol_TitleBar] = IM_COL32(58, 58, 58, 255);
	style.Colors[ImNodesCol_TitleBarHovered] = IM_COL32(72, 72, 72, 255);
	style.Colors[ImNodesCol_TitleBarSelected] = IM_COL32(86, 86, 86, 255);
	style.Colors[ImNodesCol_Pin] = IM_COL32(200, 200, 200, 255);
	style.Colors[ImNodesCol_PinHovered] = IM_COL32(255, 255, 255, 255);
	style.Colors[ImNodesCol_Link] = IM_COL32(150, 150, 150, 255);
	style.Colors[ImNodesCol_LinkHovered] = IM_COL32(255, 255, 255, 255);
	style.Colors[ImNodesCol_LinkSelected] = IM_COL32(255, 165, 0, 255);
	style.Colors[ImNodesCol_GridBackground] = IM_COL32(0, 0, 0, 255);
	style.Colors[ImNodesCol_GridLine] = IM_COL32(60, 60, 60, 100);
	style.Colors[ImNodesCol_BoxSelector] = IM_COL32(100, 149, 237, 80);
	style.Colors[ImNodesCol_BoxSelectorOutline] = IM_COL32(100, 149, 237, 255);
	style.Colors[ImNodesCol_MiniMapBackground] = IM_COL32(25, 25, 25, 150);
	style.Colors[ImNodesCol_MiniMapBackgroundHovered] = IM_COL32(25, 25, 25, 200);
	style.Colors[ImNodesCol_MiniMapOutline] = IM_COL32(150, 150, 150, 100);
	style.Colors[ImNodesCol_MiniMapNodeBackground] = IM_COL32(200, 200, 200, 100);
	style.Colors[ImNodesCol_MiniMapNodeBackgroundHovered] = IM_COL32(200, 200, 200, 255);
	style.Colors[ImNodesCol_MiniMapNodeBackgroundSelected] = IM_COL32(255, 165, 0, 255);
	style.Colors[ImNodesCol_MiniMapNodeOutline] = IM_COL32(200, 200, 200, 100);
	style.Colors[ImNodesCol_MiniMapLink] = IM_COL32(200, 200, 200, 100);
	style.Colors[ImNodesCol_MiniMapLinkSelected] = IM_COL32(255, 165, 0, 255);
}

void CAnimTool::Priority_Update(_float fTimeDelta)
{
	if (m_pSelectedObject)
	{
		m_pSelectedObject->Priority_Update(fTimeDelta);
	}
}

void CAnimTool::Update(_float fTimeDelta)
{
	if (m_bIsObject == false)
		UpdateCurrentModel(fTimeDelta);

	if (m_pSelectedObject)
	{
		m_pSelectedObject->Update(fTimeDelta);
	}
}

void CAnimTool::Late_Update(_float fTimeDelta)
{
	if (m_pSelectedObject)
	{
		m_pSelectedObject->Late_Update(fTimeDelta);
	}
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
	m_bRenderLevel = false;
}

HRESULT CAnimTool::Render()
{
	if (m_bRenderLevel)
	{
		if (ImGui::Checkbox("Load Object", &m_bIsObject))
		{
			m_pCurAnimation = nullptr;
			m_pCurAnimator = nullptr;
			m_pCurModel = nullptr;
			m_pSelectedObject = nullptr; // 오브젝트 선택 초기화
			m_stSelectedModelName.clear();
			m_stSelectedObjectName.clear();
		}
		;
		if (m_bIsObject == false)
		{
			if (FAILED(Render_Load_Model()))
				return E_FAIL;
		}
		else
		{
			if (FAILED(Render_Spawn_Object()))
				return E_FAIL;
		}

		Setting_Sequence();
		if (m_bUseAnimSequence)
		{
			if (FAILED(Render_AnimationSequence()))
				return E_FAIL;
			if (FAILED(Render_AnimEvents()))
				return E_FAIL;
		}
		if (FAILED(Render_AnimationStateMachine()))
			return E_FAIL;
		if (FAILED(Render_OverrideAnimControllers()))
			return E_FAIL;
	}
	else
	{
		m_bRenderLevel = true;
	}

	if (m_bIsObject == false)
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
	if (ImGui::Button("Save All Clips Events to JSON"))
	{
		SaveOrLoadEvents();
	}

	if (ImGui::Button("Load All Clips Events from JSON"))
	{
		SaveOrLoadEvents(false);
	}
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

	return S_OK;
}

HRESULT CAnimTool::Render_Parameters()
{
	ImGui::Separator();

	_float fRowH = ImGui::GetFrameHeightWithSpacing();
	ImGui::BeginChild("ParamScrollRegion", ImVec2(0, fRowH * 5 + ImGui::GetStyle().FramePadding.y * 2), true);

	// 컬럼 Name | Type | Value | Action
	ImGui::Columns(4, "ParamColumns", true);
	ImGui::Text("Name");   ImGui::NextColumn();
	ImGui::Text("Type");   ImGui::NextColumn();
	ImGui::Text("Value");  ImGui::NextColumn();
	ImGui::Text("Action"); ImGui::NextColumn();
	ImGui::Separator();

	unordered_map<string, Parameter>& parameters = m_pCurAnimator->GetParametersForEditor();
	// 파라미터 타입들
	const _char* typeNames[] = { "Bool", "Trigger", "Float", "Int" };

	_int i = 0;

	for (auto it = parameters.begin(); it != parameters.end();)
	{
		auto& parmeter = it->second;
		string id = to_string(i); // 식별하는 아이디용

		// Name
		_char buf[64];
		strncpy_s(buf, it->first.c_str(), sizeof(buf));
		if (ImGui::InputText(("##Name" + id).c_str(), buf, sizeof(buf)))
		{
			string oldName = parmeter.name;
			string newName = buf;
			if (newName != oldName)
			{
				Parameter newParameter = parmeter; // 기존 파라미터 복사
				newParameter.name = newName; // 새 이름으로 변경
				it = parameters.erase(it); // 기존 이름 삭제
				m_pCurAnimator->AddParameter(newName, newParameter);
				continue;
			}
		}
		ImGui::NextColumn();

		// Type 콤보
		_int iType = static_cast<_int>(parmeter.type);

		if (ImGui::Combo(("##Type" + id).c_str(), &iType, typeNames, IM_ARRAYSIZE(typeNames)))
		{
			ParamType newType = static_cast<ParamType>(iType);
			if (newType != parmeter.type)
			{
				// 타입 변경 시 값들을 적절히 초기화
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


				// 바뀐 타입에 맞게 다시 조절
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

		// Value 위젯
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

		// Delete 버튼
		if (ImGui::Button(("X##" + id).c_str()))
		{
			it = parameters.erase(it);
			--i; // 삭제했으니 인덱스 보정
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

	// 파라미터 추가 팝업
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
					m_pCurAnimator->AddBool(bExists ? diffName : m_NewParameterName);
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
				m_NewParameterName[0] = '\0'; // 클리어
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
				// 여기서도 컨트롤러 불러와서 ID 바꿔줘야함
				auto pCtrl = m_pCurAnimator->Get_CurrentAnimController();
				if (pCtrl)
				{
					auto& states = pCtrl->GetStates();
					for (const auto& state : states)
					{
						m_iSpecificNodeId = max(m_iSpecificNodeId, state.iNodeId);
					}

					for (const auto& trns : pCtrl->GetTransitions())
					{
						m_iSpecificNodeId = max(m_iSpecificNodeId, trns.iFromNodeId);
						m_iSpecificNodeId = max(m_iSpecificNodeId, trns.iToNodeId);
					}
					m_iSpecificNodeId++;
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
		// 언레지스터 → 이름 바꿔서 → 리지스터
		auto pCtrl = ctrls[oldName];
		m_pCurAnimator->RenameAnimController(oldName, newName);
		pCtrl->SetName(newName);
		// 선택 인덱스 보정
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
		overrideCtrlNames.reserve(m_pCurAnimator->GetOverrideAnimControllersMap().size() + 1);
		overrideCtrlNames.push_back("None");
		for (const auto& Pair : m_pCurAnimator->GetOverrideAnimControllersMap())
		{
			overrideCtrlNames.push_back(Pair.first);
		}

		// 현재 컨트롤러의 스테이트들 가져와서 설정하게
		auto pCtrl = m_pCurAnimator->Get_CurrentAnimController();
		auto& states = pCtrl->GetStates();
		if (m_NewOverrideAnimController.states.empty() || states.size() != m_NewOverrideAnimController.states.size())
		{
			// 오버라이드 컨트롤러가 비어있으면 현재 컨트롤러의 상태들로 초기화
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
					// 선택된 오버라이드 컨트롤러로 현재 작업 중인 컨트롤러 설정
					m_NewOverrideAnimController = m_pCurAnimator->GetOverrideAnimControllersMap()[overrideCtrlNames[i]];
					// 선택된 컨트롤러의 이름을 현재 이름으로 설정
					strcpy_s(m_OverrideControllerName, overrideCtrlNames[i].c_str());
					//	strcpy_s(m_NewOverrideControllerName, overrideCtrlNames[i].c_str());
						// 리네임 필드 초기화
					strcpy_s(m_OverrideControllerRename, overrideCtrlNames[i].c_str());
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// 현재 선택된 오버라이드 컨트롤러의 이름 표시
		if (m_iSelectedOverrideControllerIndex >= 0)
		{
			ImGui::Text("Current Controller: %s", m_OverrideControllerName);

			// 이름 변경 입력 필드
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
					// 선택 인덱스 업데이트 (이름이 바뀌었으므로)
					m_iSelectedOverrideControllerIndex = -1;
				}
			}
		}

		// 오버라이드 해야할 스테이트들 오버라이드 컨트롤러에 추가
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
						// 마스크 본을 사용하는 스테이트인지 확인
						m_bIsUseMaskBoneState = it->maskBoneName.empty() == false;
					}

					// 저장된 오버라이드 상태에서 애니메이션 인덱스 찾기
					auto& selectedState = m_NewOverrideAnimController.states[stateNames[i]];
					vector<CAnimation*>& anims = m_bIsObject ? m_pCurModel->GetAnimations() : m_LoadedAnimations[m_stSelectedModelName];

					// 메인 애니메이션 인덱스 찾기
					m_iOverrideAnimIndex = -1;
					for (int j = 0; j < anims.size(); ++j)
					{
						if (anims[j]->Get_Name() == selectedState.clipName)
						{
							m_iOverrideAnimIndex = j;
							break;
						}
					}

					// 상체 애니메이션 인덱스 찾기
					m_iOverrideUpperAnimIndex = -1;
					for (int j = 0; j < anims.size(); ++j)
					{
						if (anims[j]->Get_Name() == selectedState.upperClipName)
						{
							m_iOverrideUpperAnimIndex = j;
							break;
						}
					}

					// 하체 애니메이션 인덱스 찾기
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

		// 선택된 오버라이드 상태가 있다면 해당 상태의 정보를 보여주고 수정할 수 있게
		if (m_iSelectedOverrideStateIndex >= 0 && m_iSelectedOverrideStateIndex < stateNames.size())
		{
			auto& selectedState = m_NewOverrideAnimController.states[stateNames[m_iSelectedOverrideStateIndex]];

			// 상태 이름
			vector<CAnimation*>& anims = m_bIsObject ? m_pCurModel->GetAnimations() : m_LoadedAnimations[m_stSelectedModelName]; // 현재 선택된 모델의 애니메이션들

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

			if (m_bIsUseMaskBoneState)
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
				m_iOverrideUpperAnimIndex = -1; // 마스크 본을 사용하지 않는 상태라면 상체, 하체 애니메이션 선택 초기화
			}
		}

		// 컨트롤러 이름 설정 (새로 만들 때만)
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
					// 기존 컨트롤러 이름으로 추가
					m_pCurAnimator->Add_OverrideAnimController(
						m_OverrideControllerName, m_NewOverrideAnimController);
				}
				else if (m_iSelectedOverrideControllerIndex >= 0)
				{
					// 선택된 컨트롤러로 추가
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
			m_NewOverrideControllerName[0] = '\0'; // 클리어
			m_OverrideControllerName[0] = '\0'; // 클리어
			m_OverrideControllerRename[0] = '\0'; // 클리어
			m_iSelectedOverrideControllerIndex = -1; // 선택 초기화
			m_iSelectedOverrideStateIndex = -1; // 선택 초기화
			m_iOverrideAnimIndex = -1; // 애니메이션 선택 초기화
			m_iOverrideUpperAnimIndex = -1; // 상체 애니메이션 선택 초기화
			m_iOverrideLowerAnimIndex = -1; // 하체 애니메이션 선택 초기화
		}

		if (ImGui::Button("Apply Override Controllers"))
		{
			if (m_bUseOverrideController)
			{
				if (m_iSelectedOverrideControllerIndex < 0)
				{
					// 새로 추가한 컨트롤러라면
					m_pCurAnimator->Add_OverrideAnimController(
						m_NewOverrideControllerName, m_NewOverrideAnimController);
					// 새로 추가한 컨트롤러를 현재 컨트롤러로 설정
					m_pCurAnimator->ApplyOverrideAnimController(m_NewOverrideControllerName);
				}
				else
				{
					// 기존 컨트롤러 적용
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

HRESULT CAnimTool::Render_AnimationSequence()
{
	if (m_pCurAnimator == nullptr || m_pCurAnimation == nullptr || m_pMySequence == nullptr)
	{
		return S_OK;
	}

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

	// 재생 속도 조절
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
			pCurAnim->SetCurrentTrackPosition(static_cast<_float>(m_iSequenceFrame)); // 프레임을 초 단위로 변환
			m_pCurAnimator->SetPlaying(true);
			m_pCurAnimator->Update(0.f);
			m_pCurAnimator->StopAnimation(); // 다시 pause
			vector<string> events;

			pCurAnim->Update_Bones(0.f, m_pCurModel->Get_Bones(), pCurAnim->Get_isLoop(), &events);
			m_pCurAnimator->DispatchAnimEventsForEditor(events);
		}
	}


	ImGui::End();
	return S_OK;
}

HRESULT CAnimTool::Render_AnimationStateMachine()
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
		SaveOrLoadAnimStates();
	}

	if (ImGui::Button("Load AnimState This Model"))
	{
		SaveOrLoadAnimStates(false);
		return S_OK; // 불렀올 때는 한 프레임 넘기기
	}

	// 컨트롤러가 바뀌면 식별 ID 초기화 시켜놓기
	CAnimController* pCtrl = m_pCurAnimator->Get_CurrentAnimController();

	ImGui::Begin("Anim State Machine");

	if (ImGui::CollapsingHeader("Parameters"))
	{
		if (FAILED(Render_Parameters()))
			return E_FAIL;
	}

	ImGui::BeginChild("CategoryCheckList", ImVec2(120, 60), true); //  박스 내에서 스크롤

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

	// 우클릭으로 상대 추가 팝업
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

			//size_t newIdx = pCtrl->AddState(stateName, selectedAnim, m_iSpecificNodeId++);
			_int iNodeId = m_iSpecificNodeId++; // 고유한 노드 아이디 생성
			// ID 확인
			for (auto& state : pCtrl->GetStates())
			{
				if (state.iNodeId == iNodeId)
				{
					iNodeId = m_iSpecificNodeId++; // 중복되면 다음 ID로
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

	// 정렬하기

	_bool requestLayout = false;
	if (ImGui::IsKeyPressed(ImGuiKey_L))
		requestLayout = true;

	m_DrawnInPins.clear();
	m_DrawnOutPins.clear();

	for (const auto& Pair : m_CategoryStates)
	{
		auto& category = Pair.first; // 카테고리
		auto& states = Pair.second; // 해당 카테고리의 상태들

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
					ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "[ENTRY]"); // 녹색으로 Entry 표시
				}
		
				ImNodes::EndNodeTitleBar();

				ImGui::BeginGroup();

				// 현재 활성 상태인지 확인 (현재 재생 중인 애니메이션 상태)
				_bool isCurrentState = (pCtrl->GetCurrentStateForEditor() &&
					pCtrl->GetCurrentStateForEditor()->iNodeId == state.iNodeId);

				if (isCurrentState)
				{
					ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "[ACTIVE]");

					// 현재 애니메이션 진행률 표시
					_float progress = m_pCurAnimator->GetCurrentAnimProgress();
					ImGui::ProgressBar(progress, ImVec2(120, 0), "");
					ImGui::SameLine();
					ImGui::Text("%.1f%%", progress * 100.0f);
				}
				else
				{
					ImGui::Text("");
					ImGui::Dummy(ImVec2(120, ImGui::GetFrameHeight())); // 진행바 크기만큼 
				}

				const ImVec4 kInColor = ImVec4(0.3f, 0.8f, 0.3f, 1.f);
				const ImVec4 kOutColor = ImVec4(0.8f, 0.4f, 0.2f, 1.f);

				if (ImGui::BeginTable(
					("pins##" + std::to_string(state.iNodeId)).c_str(), // 노드별 고유 ID
					2,
					ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoBordersInBody,
					ImVec2(140.0f, 0.0f))) // 최소 가로폭 
				{
					ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 60.0f);
					ImGui::TableSetupColumn("R", ImGuiTableColumnFlags_WidthFixed, 60.0f);

					ImGui::TableNextRow();

					// 왼쪽 컬럼: In 핀
					ImGui::TableSetColumnIndex(0);
					if (!isAny)
					{
						const _int inPin = state.iNodeId * 10 + 1;
						ImNodes::BeginInputAttribute(inPin);
						ImGui::TextColored(kInColor, "In");
						ImNodes::EndInputAttribute();
						m_DrawnInPins.insert(inPin);
					}

					// 오른쪽 컬럼: Out 핀
					ImGui::TableSetColumnIndex(1);
					const _int outPin = state.iNodeId * 10 + 2;
					ImNodes::BeginOutputAttribute(outPin);
					ImGui::TextColored(kOutColor, "Out");
					ImNodes::EndOutputAttribute();
					m_DrawnOutPins.insert(outPin);

					ImGui::EndTable();
				}
				ImGui::EndGroup();

				ImNodes::EndNode();

				// 노드 위치 저장
				ImVec2 pos = ImNodes::GetNodeEditorSpacePos(state.iNodeId);
				state.fNodePos = { pos.x, pos.y };

				if (isAny || isExit)
				{
					ImNodes::PopColorStyle(); // 노드 아웃라인
					ImNodes::PopColorStyle(); // 타이틀 바 선택
					ImNodes::PopColorStyle(); // 타이틀 바 호버
					ImNodes::PopColorStyle(); // 타이틀 바
				}
			}
		}
	}

	Render_Transitions(pCtrl, isAnyLinkSelected,isAnyNodeSelected);

	//for (auto& t : pCtrl->GetTransitions())
	//{

	//	const _bool fromIsSpecial = (t.iFromNodeId == ANY_NODE_ID || t.iFromNodeId == EXIT_NODE_ID);
	//	const _bool toIsSpecial = (t.iToNodeId == ANY_NODE_ID || t.iToNodeId == EXIT_NODE_ID);

	//	const _int startPinID = t.iFromNodeId * 10 + 2; // Out
	//	const _int endPinID = t.iToNodeId * 10 + 1; // In
	//	_bool startDrawn = (m_DrawnInPins.count(endPinID) > 0);
	//	_bool endDrawn = (m_DrawnOutPins.count(startPinID) > 0);
	//	if (!startDrawn || !endDrawn)
	//		continue; //노드 그린거 없으면 안그리기
	//	// 카테고리 기반 가시성
	//	string fromName = pCtrl->GetStateNameByNodeId(t.iFromNodeId);
	//	string toName = pCtrl->GetStateNameByNodeId(t.iToNodeId);
	//	string fromCat = GetStateCategory(fromName);
	//	string toCat = GetStateCategory(toName);

	//	_bool fromVisible = m_bShowAll || m_CategoryVisibility[fromCat];
	//	_bool toVisible = m_bShowAll || m_CategoryVisibility[toCat];

	//	// 선택 상태 계산
	//	_bool thisLinkSelected = ImNodes::IsLinkSelected(t.link.iLinkId);
	//	_bool fromNodeSelected = ImNodes::IsNodeSelected(t.iFromNodeId);
	//	_bool toNodeSelected = ImNodes::IsNodeSelected(t.iToNodeId);
	//	_bool anySelectedThis = thisLinkSelected || fromNodeSelected || toNodeSelected;


	//	_bool passCategory = (fromVisible && toVisible);


	//	if (!passCategory && anySelectedThis && (fromIsSpecial || toIsSpecial))
	//		passCategory = true;

	//	if (!passCategory)
	//		continue;

	//	// 그릴지 최종 결정
	//	_bool bDrawLink = false;
	//	if (isAnyLinkSelected)
	//	{
	//		// 선택된 링크만
	//		bDrawLink = thisLinkSelected;
	//	}
	//	else if (isAnyNodeSelected)
	//	{
	//		// 선택된 노드와 연결된 링크만
	//		bDrawLink = (fromNodeSelected || toNodeSelected);
	//	}
	//	else
	//	{
	//		// 일반 모드
	//		bDrawLink = m_bShowAllLink;
	//	}

	//	if (bDrawLink)
	//		ImNodes::Link(t.link.iLinkId, startPinID, endPinID);
	//}

	ImNodes::EndNodeEditor();

	if (requestLayout)
	{
		ApplyCategoryLayout(pCtrl);
		ImNodes::ClearLinkSelection();
		ImNodes::ClearNodeSelection();
	}

	Handle_Links(pCtrl);

	//_bool bCanLink = pCtrl->GetStates().size() >= 2;
	//if (bCanLink)
	//{
	//	_int startPinID = -1, endPinID = -1;
	//	if (ImNodes::IsLinkCreated(&startPinID, &endPinID))
	//	{
	//		_int fromNodeID = (startPinID - 2) / 10;
	//		_int toNodeID = (endPinID - 1) / 10;

	//		// 유효한 아이디인지 
	//		_bool validFromNode = false, validToNode = false;
	//		for (const auto& state : pCtrl->GetStates())
	//		{
	//			if (state.iNodeId == fromNodeID)
	//				validFromNode = true;
	//			if (state.iNodeId == toNodeID)
	//				validToNode = true;
	//		}

	//		if (validFromNode && validToNode && fromNodeID != toNodeID)
	//		{
	//			Engine::Link link;
	//			link.iLinkId = m_iSpecificNodeId++;
	//			link.iLinkStartID = startPinID;
	//			link.iLinkEndID = endPinID;

	//			// 트랜지션 추가
	//			pCtrl->AddTransition(fromNodeID, toNodeID, link, 0.2f, true);
	//		}

	//	}
	//}
	//if (ImNodes::NumSelectedLinks() > 0)
	//{
	//	vector<int> selectedLinks(ImNodes::NumSelectedLinks());
	//	ImNodes::GetSelectedLinks(selectedLinks.data());
	//	auto& transitions = pCtrl->GetTransitionsForEditor();

	//	_bool bDeleteLink = false;
	//	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	//	{

	//		for (_int linkId : selectedLinks)
	//		{
	//			for (_int i = static_cast<_int>(transitions.size()) - 1; i >= 0; --i)
	//			{
	//				if (transitions[i].link.iLinkId == linkId)
	//				{
	//					bDeleteLink = true;
	//					transitions.erase(transitions.begin() + i);
	//					break;
	//				}
	//			}
	//		}
	//		ImNodes::ClearLinkSelection();
	//	}

	//	// 링크 선택시 트랜지션 상태 표시
	//	// 선택된 링크의 Condition을 찾아서 표시

	//	if (bDeleteLink == false) // 삭제 안한 경우에만
	//	{

	//		auto pCtrl = m_pCurAnimator->Get_CurrentAnimController();
	//		for (_int linkId : selectedLinks)
	//		{
	//			for (auto& transition : transitions)
	//			{
	//				if (transition.link.iLinkId == linkId)
	//				{
	//					ImGui::Begin("Transition Info");
	//					if (pCtrl->GetStateAnimationByNodeIdForEditor(transition.iFromNodeId))
	//					{

	//						auto FromNodeName = pCtrl->GetStateAnimationByNodeIdForEditor(transition.iFromNodeId)->Get_Name();
	//						if (FromNodeName.empty())
	//							FromNodeName = "Unknown";

	//						ImGui::Text("From Node: %s", FromNodeName.c_str());
	//					}

	//					if (pCtrl->GetStateAnimationByNodeIdForEditor(transition.iToNodeId))
	//					{
	//						auto ToNodeName = pCtrl->GetStateAnimationByNodeIdForEditor(transition.iToNodeId)->Get_Name();
	//						if (ToNodeName.empty())
	//							ToNodeName = "Unknown";
	//						ImGui::Text("To Node : %s", ToNodeName.c_str());
	//					}

	//					ImGui::Text("Link ID: %d", transition.link.iLinkId);
	//					//ImGui::Text("Condition: %s", transition.condition.paramName.c_str());
	//					ImGui::End();

	//					if (FAILED(Modify_Transition(transition)))
	//					{
	//						ImGui::End();
	//						return E_FAIL;
	//					}
	//					break;
	//				}
	//			}
	//		}
	//	}
	//}

	// 노드 삭제 (스테이트 삭제)
	if (ImNodes::NumSelectedNodes() > 0 && ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		vector<_int> selectedNodes(ImNodes::NumSelectedNodes());
		ImNodes::GetSelectedNodes(selectedNodes.data());
		auto& states = pCtrl->GetStatesForEditor();
		for (_int nodeId : selectedNodes)
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


	vector<CAnimation*>& anims = m_bIsObject ? m_pCurModel->GetAnimations() : m_LoadedAnimations[m_stSelectedModelName]; // 현재 선택된 모델의 애니메이션들

	vector<string> animNames;
	animNames.reserve(anims.size() + 1);
	animNames.push_back("None");
	auto& animsByName = m_pCurModel->GetAnimationsByIndex();
	for (_int i = 0; i < anims.size(); ++i)
	{
		animNames.push_back(animsByName[i]);
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
					state.stateName = buf; // 이름 변경
				}

				if (ImGui::Button("Active Upper MaskBone"))
				{
					if (state.maskBoneName.empty())
					{
						state.maskBoneName = "Bip001-Spine2"; // 기본 마스크 본
					}
					else
					{
						state.maskBoneName.clear(); // 마스크 본 제거
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


					// 클립이 있었던 경우에는 현재 애니메이션을 state의 애니메이션으로 변경
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

				// 트랜지션 리스트
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
					if (ImGui::BeginCombo("Clips", m_iDefaultSelectedAnimIndex >= 0 ? animNames[m_iDefaultSelectedAnimIndex].c_str() : "Select Clip"))
					{
						for (_int i = 0; i < animNames.size(); ++i)
						{
							_bool isSelected = (i == m_iDefaultSelectedAnimIndex);
							if (ImGui::Selectable(animNames[i].c_str(), isSelected))
							{
								m_iDefaultSelectedAnimIndex = i;
								if (i == 0) // "None"이 선택된 경우
								{
									state.clip = nullptr;
								}
								else // 다른 애니메이션이 선택된 경우
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

					// 마스크 본 선택
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

								if (i == 0) // "None"이 선택된 경우
								{
									state.upperClipName = ""; // 상체 애니메이션 초기화
								}
								else // 다른 애니메이션이 선택된 경우
								{
									state.upperClipName = anims[i - 1]->Get_Name();
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
								if (i == 0) // "None"이 선택된 경우
								{
									state.lowerClipName = ""; // 상체 애니메이션 초기화
								}
								else // 다른 애니메이션이 선택된 경우
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

				// 애니메이션 StartTime 정하기
				// 마스크 본이 아니면 LowerStartTime만 설정 아니면 Upper까지
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
				_bool bCanSameAnimReset = state.bCanSameAnimReset;

				if (ImGui::Checkbox("Can Same Anim Reset", &bCanSameAnimReset))
				{
					state.bCanSameAnimReset = bCanSameAnimReset;
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
				// 모델을 바꿀 때는 현재 애니메이션을 nullptr
				m_pCurAnimation = nullptr;
				m_stSelectedModelName = modelNames[i];
				m_vecMaskBoneNames.clear(); // 마스크 본 이름 초기화
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

HRESULT CAnimTool::Render_Spawn_Object()
{
	_bool open = true;
	Begin("Spawn Object", &open, NULL);
	ImGui::Checkbox("Load Object", &m_bIsObjectToolActive);
	if (!m_bIsObjectToolActive)
	{
		ImGui::End();
		return S_OK;
	}
	if (ImGui::BeginCombo("Object List", m_iSelectedSpawnIndex >= 0 ? m_vecObjectNames[m_iSelectedSpawnIndex].c_str() : "Object Name"))
	{
		for (_int i = 0; i < m_vecObjectNames.size(); ++i)
		{
			_bool isSelected = (i == m_iSelectedSpawnIndex);
			if (ImGui::Selectable(m_vecObjectNames[i].c_str(), isSelected))
			{
				m_iSelectedSpawnIndex = i;

			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Spawn"))
	{
		if (m_iSelectedSpawnIndex >= 0 && m_iSelectedSpawnIndex < m_vecObjectNames.size())
		{
			auto pObj = m_pEditorObjectFactory->CreateObject(StringToWString(m_vecObjectNames[m_iSelectedSpawnIndex]));
			if (pObj)
			{
				m_vecObjects.emplace_back(pObj);
				if (m_vecObjects.back()->Get_Name() != L"Player")
				{
				pObj->Get_TransfomCom()->Set_State(STATE::POSITION, XMVectorSet(0.f, 0.f, 0.f, 1.f));
				}
			}
		}
	}

	if (FAILED(Render_SpawnedObject()))
	{
		ImGui::End();
		return E_FAIL;
	}

	ImGui::End();
	return S_OK;
}

HRESULT CAnimTool::Render_Transitions(CAnimController* pCtrl, _bool bIsAnyLinkSelected, _bool bIsAnyNodeSelected)
{
	if (!pCtrl)
		return E_FAIL;
	for (auto& t : pCtrl->GetTransitions())
	{

		const _bool fromIsSpecial = (t.iFromNodeId == ANY_NODE_ID || t.iFromNodeId == EXIT_NODE_ID);
		const _bool toIsSpecial = (t.iToNodeId == ANY_NODE_ID || t.iToNodeId == EXIT_NODE_ID);

		const _int startPinID = t.iFromNodeId * 10 + 2; // Out
		const _int endPinID = t.iToNodeId * 10 + 1; // In
		_bool startDrawn = (m_DrawnInPins.count(endPinID) > 0);
		_bool endDrawn = (m_DrawnOutPins.count(startPinID) > 0);
		if (!startDrawn || !endDrawn)
			continue; //노드 그린거 없으면 안그리기
		// 카테고리 기반 가시성
		string fromName = pCtrl->GetStateNameByNodeId(t.iFromNodeId);
		string toName = pCtrl->GetStateNameByNodeId(t.iToNodeId);
		string fromCat = GetStateCategory(fromName);
		string toCat = GetStateCategory(toName);

		_bool fromVisible = m_bShowAll || m_CategoryVisibility[fromCat];
		_bool toVisible = m_bShowAll || m_CategoryVisibility[toCat];

		// 선택 상태 계산
		_bool thisLinkSelected = ImNodes::IsLinkSelected(t.link.iLinkId);
		_bool fromNodeSelected = ImNodes::IsNodeSelected(t.iFromNodeId);
		_bool toNodeSelected = ImNodes::IsNodeSelected(t.iToNodeId);
		_bool anySelectedThis = thisLinkSelected || fromNodeSelected || toNodeSelected;


		_bool passCategory = (fromVisible && toVisible);


		if (!passCategory && anySelectedThis && (fromIsSpecial || toIsSpecial))
			passCategory = true;

		if (!passCategory)
			continue;

		// 그릴지 최종 결정
		_bool bDrawLink = false;
		if (bIsAnyLinkSelected)
		{
			// 선택된 링크만
			bDrawLink = thisLinkSelected;
		}
		else if (bIsAnyNodeSelected)
		{
			// 선택된 노드와 연결된 링크만
			bDrawLink = (fromNodeSelected || toNodeSelected);
		}
		else
		{
			// 일반 모드
			bDrawLink = m_bShowAllLink;
		}

		if (bDrawLink)
			ImNodes::Link(t.link.iLinkId, startPinID, endPinID);
	}

	return S_OK;
}

HRESULT CAnimTool::Handle_Links(CAnimController* pCtrl)
{
	_bool bCanLink = pCtrl->GetStates().size() >= 2;
	if (bCanLink)
	{
		_int startPinID = -1, endPinID = -1;
		if (ImNodes::IsLinkCreated(&startPinID, &endPinID))
		{
			_int fromNodeID = (startPinID - 2) / 10;
			_int toNodeID = (endPinID - 1) / 10;

			// 유효한 아이디인지 
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
				link.iLinkId = m_iSpecificNodeId++;
				link.iLinkStartID = startPinID;
				link.iLinkEndID = endPinID;

				// 트랜지션 추가
				pCtrl->AddTransition(fromNodeID, toNodeID, link, 0.2f, true);
			}

		}
	}
	if (ImNodes::NumSelectedLinks() > 0)
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

		// 링크 선택시 트랜지션 상태 표시
		// 선택된 링크의 Condition을 찾아서 표시

		if (bDeleteLink == false) // 삭제 안한 경우에만
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
	return S_OK;
}



HRESULT CAnimTool::Render_SpawnedObject()
{
	if (m_vecObjects.empty())
	{
		return S_OK;
	}

	if (ImGui::BeginCombo("Objects", m_iSelectedObjectIndex >= 0 ? WStringToString(m_vecObjects[m_iSelectedObjectIndex]->Get_Name()).c_str() : "Select Object"))
	{
		for (_int i = 0; i < m_vecObjects.size(); ++i)
		{
			_bool isSelected = (i == m_iSelectedObjectIndex);
			if (ImGui::Selectable(WStringToString(m_vecObjects[i]->Get_Name()).c_str(), isSelected))
			{
				if (i < 0 || i >= m_vecObjects.size())
				{
					continue; // 유효하지 않은 인덱스는 무시
				}
				m_iSelectedObjectIndex = i;
				m_pCurModel = dynamic_cast<CModel*>(m_vecObjects[m_iSelectedObjectIndex]->Get_Component(TEXT("Com_Model")));
				m_pSelectedObject = m_vecObjects[m_iSelectedObjectIndex];
				m_pCurAnimator = dynamic_cast<CUnit*>(m_vecObjects[m_iSelectedObjectIndex])->Get_Animator();
				m_pCurAnimation = nullptr;
				m_stSelectedObjectName = WStringToString(m_vecObjects[m_iSelectedObjectIndex]->Get_Name());
				m_stSelectedModelName = m_pCurModel->Get_ModelName();
				m_vecMaskBoneNames.clear(); // 마스크 본 이름 초기화
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
			_float fNormalized = _float(m_iSequenceFrame - m_pMySequence->GetFrameMin())
				/ _float(m_pMySequence->GetFrameMax() - m_pMySequence->GetFrameMin());
			_float fTicks = fNormalized * pAnim->GetDuration();
			pAnim->SetCurrentTrackPosition(fTicks);
			vector<string> events;

			m_pCurAnimator->Update(0.f);
			pAnim->Update_Bones(0.f, m_pCurModel->Get_Bones(), pAnim->Get_isLoop(), &events);
			m_pCurAnimator->DispatchAnimEventsForEditor(events);
		}
	}
	else
	{
		m_pCurAnimator->Update(fTimeDelta * m_iPlaySpeed);
	}

	m_pCurModel->Update_Bones();
}

void CAnimTool::SelectAnimation()
{
	if (m_pCurModel == nullptr || m_pCurAnimator == nullptr)
	{
		return;
	}
	vector<CAnimation*>& anims = m_bIsObject
		? m_pCurModel->GetAnimations()
		: m_LoadedAnimations[m_stSelectedModelName]; // 현재 선택된 모델의 애니메이션들

	vector<string> animNames;
	animNames.reserve(anims.size());
	auto animsNameByIndexMap = m_pCurModel->GetAnimationsByIndex();
	for (_int i = 0; i < static_cast<_int>(anims.size()); i++)
	{
		animNames.push_back(animsNameByIndexMap[i]);
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

	// 애니메이션 프로퍼티들 설정
	Setting_AnimationProperties();
}



void CAnimTool::Setting_AnimationProperties()
{
	// 원본의 애니메이션에 적용을 해줘야함
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
		_bool bRootMotion = m_pCurAnimation->IsRootMotionEnabled();
		_bool bReverse = m_pCurAnimation->IsReverse();
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
		if (ImGui::Checkbox("Reverse", &bReverse))
		{
			m_pCurAnimation->SetReverse(bReverse);
			bChanged = true;
		}
		if (bChanged)
		{
			m_pCurAnimation->SetTickPerSecond(fTickPerSecond);

			auto& anims = m_bIsObject ? m_pCurModel->GetAnimations() : m_LoadedAnimations[m_stSelectedModelName];
			auto it = find_if(anims.begin(), anims.end(),
				[&](CAnimation* anim) { return anim->Get_Name() == m_pCurAnimation->Get_Name(); });

			if (it != anims.end())
			{
				(*it)->SetLoop(bLoop);
				(*it)->SetTickPerSecond(fTickPerSecond);
			}
		}
	}

	// 현재 모델의 모든 애니메이션 속도 조절
	if (m_pCurModel)
	{
		_bool bChanged = false;
		bChanged |= ImGui::DragFloat("Set All Anim Tick Per Second ", &m_fAllAnimTickperSec, 0.1f, 0.1f, 100.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (bChanged)
		{
			if (m_bIsObject)
			{
				m_pCurModel->Set_Animation_TickPerSecond_All(m_fAllAnimTickperSec);
			}
			else
			{
				for (auto& anim : m_LoadedAnimations[m_stSelectedModelName])
				{
					anim->SetTickPerSecond(m_fAllAnimTickperSec);
				}
			}
		}
	}
}

void CAnimTool::ApplyCategoryLayout(CAnimController* pCtrl)
{

	const auto& states = pCtrl->GetStates();
	const auto& transitions = pCtrl->GetTransitions();

	// 허브 노드 위치 상수
	constexpr _float HUB_Y = -300.f;     // 더 위쪽으로 이동
	constexpr _float ANY_X = -800.f;     // 더 왼쪽으로
	constexpr _float EXIT_MARGIN = 600.f; // Exit 노드 여유 공간

	// 카테고리 구성
	map<string, vector<_int>> categories;
	m_CategoryStates.clear();

	// 연결 허브 정도 계산
	unordered_map<_int, _int> degree;
	degree.reserve(states.size());
	for (const auto& t : transitions)
	{
		degree[t.iFromNodeId]++;
		degree[t.iToNodeId]++;
	}

	for (const auto& st : states)
	{
		string cat = GetStateCategory(st.stateName);
		categories[cat].push_back(st.iNodeId);

		if (m_CategoryVisibility.find(cat) == m_CategoryVisibility.end())
			m_CategoryVisibility[cat] = true;

		m_CategoryStates[cat].push_back(st.stateName);
	}

	const _float CATEGORY_SPACING_X = 100.f;   // 카테고리 간격 증가
	const _float NODE_SPACING_X = 250.f;   // 노드 간격
	const _float NODE_SPACING_Y = 165.f;   // 노드 세로 간격
	const _float COL_GAP_X = 200.f;   // 컬럼 간 간격

	// 보이는 카테고리 수집
	vector<pair<string, vector<_int>>> visibleCats;
	visibleCats.reserve(categories.size());
	for (auto& kv : categories)
	{
		if (m_bShowAll || m_CategoryVisibility[kv.first])
			visibleCats.push_back(kv);
	}

	if (visibleCats.empty())
		return;

	// 카테고리별 정렬 
	for (auto& kv : visibleCats) 
	{
		auto& list = kv.second;
		sort(list.begin(), list.end(), [&](_int a, _int b) 
			{
			_int da = degree.count(a) ? degree[a] : 0;
			_int db = degree.count(b) ? degree[b] : 0;
			if (da != db) return da > db;
			return a < b;
			});
	}


	auto ClusterKey = [](const string& name) -> string {
		string result = name;

		// 언더스코어 기반 처리
		size_t p1 = name.find('_');
		if (p1 != string::npos) {
			size_t p2 = name.find('_', p1 + 1);
			result = (p2 == string::npos) ? name.substr(0, p1) : name.substr(0, p2);
		}

		// 끝에 붙은 숫자 제거
		while (!result.empty() && isdigit(result.back())) {
			result.pop_back();
		}

		return result.empty() ? name : result;
		};

	const _int MAX_ROWS_PER_COL = 4;  // 행 수 증가


	_float fTotalLayoutWidth = 0.f;
	vector<_float> categoryWidths(visibleCats.size());

	// 각 카테고리의 예상 너비 계산
	for (size_t ci = 0; ci < visibleCats.size(); ++ci)
	{
		const auto& nodeIds = visibleCats[ci].second;

		// 버킷 생성
		map<string, vector<_int>> buckets;
		for (_int id : nodeIds)
		{
			auto* st = pCtrl->GetStateByNodeIdForEditor(id);
			const string nm = st ? st->stateName : "";
			buckets[nm].push_back(id);
		}

		// 이 카테고리가 차지할 너비 계산
		_int iTotalCols = 0;
		for (auto& kv : buckets)
		{
			_int iBucketCols = (static_cast<_int>(kv.second.size()) + MAX_ROWS_PER_COL - 1) / MAX_ROWS_PER_COL;
			iTotalCols += iBucketCols;
		}

		categoryWidths[ci] = iTotalCols * (NODE_SPACING_X + COL_GAP_X) + COL_GAP_X;
		fTotalLayoutWidth += categoryWidths[ci];
	}

	// 전체 레이아웃의 중앙 정렬을 위한 시작 X 계산
	_float fStartX = -fTotalLayoutWidth * 0.5f;

	// 카테고리별 노드 배치
	_float fCurrentX = fStartX;
	for (size_t ci = 0; ci < visibleCats.size(); ++ci)
	{
		const auto& catName = visibleCats[ci].first;
		const auto& nodeIds = visibleCats[ci].second;

		// 버킷화
		map<string, vector<_int>> buckets;
		for (_int id : nodeIds)
		{
			auto* st = pCtrl->GetStateByNodeIdForEditor(id);
			const string nm = st ? st->stateName : "";
			buckets[ClusterKey(nm)].push_back(id);
		}

		// 이 카테고리의 시작 X 위치
		_float fCategoryStartX = fCurrentX;

		// 버킷별 배치
		_float fBucketX = fCategoryStartX;
		for (auto& kv : buckets)
		{
			auto& ids = kv.second;
			sort(ids.begin(), ids.end());

			_int iRow = 0;
			_int iCol = 0;

			for (size_t idx = 0; idx < ids.size(); ++idx)
			{
				if (iRow >= MAX_ROWS_PER_COL)
				{
					iRow = 0;
					iCol++;
				}

				_float fPosX = fBucketX + iCol * (NODE_SPACING_X + COL_GAP_X);
				_float fPosY = iRow * NODE_SPACING_Y;

				ImVec2 pos = ImVec2(fPosX, fPosY);
				ImNodes::SetNodeEditorSpacePos(ids[idx], pos);
				++iRow;
			}

			// 다음 버킷 위치 계산
			_int iBucketCols = (static_cast<_int>(ids.size()) + MAX_ROWS_PER_COL - 1) / MAX_ROWS_PER_COL;
			fBucketX += iBucketCols * (NODE_SPACING_X + COL_GAP_X) + COL_GAP_X * 2;
		}

		fCurrentX += categoryWidths[ci] + CATEGORY_SPACING_X;
	}

	// Any/Exit 노드 배치 - 항상 양 끝에 위치하도록 보장
	_float fMinNodeX = fStartX - 100.f;  // 가장 왼쪽 노드보다 더 왼쪽
	_float fMaxNodeX = fCurrentX - CATEGORY_SPACING_X + 100.f; // 가장 오른쪽 노드보다 더 오른쪽

	// Any 노드를 가장 왼쪽에
	if (pCtrl->GetStateByNodeIdForEditor(ANY_NODE_ID))
	{
		_float fAnyX = min(ANY_X, fMinNodeX - 300.f);
		ImNodes::SetNodeEditorSpacePos(ANY_NODE_ID, ImVec2(fAnyX, HUB_Y));
	}

	// Exit 노드를 가장 오른쪽에
	if (pCtrl->GetStateByNodeIdForEditor(EXIT_NODE_ID))
	{
		_float fExitX = max(fMaxNodeX + EXIT_MARGIN, fTotalLayoutWidth * 0.5f + 400.f);
		ImNodes::SetNodeEditorSpacePos(EXIT_NODE_ID, ImVec2(fExitX, HUB_Y));
	}
}

void CAnimTool::SaveOrLoadEvents(_bool isSave)
{
	if (m_pCurModel == nullptr)
		return;
	string path = string("../Bin/Save/AnimationEvents/") + m_stSelectedModelName + "_events.json";
	if (isSave)
	{
		json root;
		root["animations"] = json::array();
		if (m_bIsObject)
		{
			for (auto* anim : m_pCurModel->GetAnimations())
			{
				root["animations"].push_back(anim->Serialize());
			}
		}
		else
		{
			for (auto* anim : m_LoadedAnimations[m_stSelectedModelName])
			{
				root["animations"].push_back(anim->Serialize());
			}
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
			// 애니메이션 이름 중복용
			vector<json> animJsonVec;
			for (const auto& animData : animationsJson)
			{
				animJsonVec.push_back(animData);
			}
			auto& clonedAnims = m_LoadedAnimations[m_stSelectedModelName];

			if (m_bIsObject)
			{
				clonedAnims = m_pCurModel->GetAnimations();
			}


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

void CAnimTool::SaveOrLoadAnimStates(_bool isSave)
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
			MSG_BOX("애니메이터가 없습니다. 애니메이터를 먼저 생성해주세요.");
			return;
		}
	}
	else
	{
		ifstream ifs(path);
		if (!ifs.is_open())
			return; // 파일이 없으면 로드하지 않음
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
				m_iSpecificNodeId = max(m_iSpecificNodeId, state.iNodeId);
			}

			for (const auto& transition : transitions)
			{
				m_iSpecificNodeId = max(m_iSpecificNodeId, transition.link.iLinkId);
				m_iSpecificNodeId = max(m_iSpecificNodeId, transition.iFromNodeId);
			}
			m_iSpecificNodeId += 1;
		}
		else
		{
			MSG_BOX("애니메이터가 없습니다. 애니메이터를 먼저 생성해주세요.");
		}
		m_CategoryStates.clear();

		// 불러오고 정렬
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
		// 이미 로드된 모델이면 새로 로드하지 않음
		MSG_BOX("이미 로드된 모델입니다.");
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

			pAnimator->Initialize(pModel);
			m_LoadedAnimators[modelName] = pAnimator;
		}

		// 모델 불러오면 처음 애니메이션 관련 정보들 불러오기 
		m_pCurModel = m_LoadedModels[modelName];
		m_stSelectedModelName = modelName;
		SaveOrLoadEvents(false);
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


string CAnimTool::GetStateCategory(const string& stateName)
{
	if (stateName.empty())
		return "Other";
	if (stateName.find("Item") != string::npos || stateName.find("Heal") != string::npos ||
		stateName.find("Grinder") != string::npos)
		return "Item";
	else if (stateName.find("Hit") != string::npos)
		return "Hit";
	if (stateName.find("Guard") != string::npos)
		return "Guard";
	else if (stateName.find("Walk") != string::npos)
		return "Walk";
	else if (stateName.find("Run") != string::npos)
		return "Run";
	else if (stateName.find("Dash") != string::npos)
		return "Dash";
	else if (stateName.find("Attack") != string::npos ||
		stateName.find("Skill") != string::npos || stateName.find("Atk") != string::npos)
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

	// 컬럼 Name | Type | Value | Action

	if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_CollapsingHeader))
	{
		ImGui::Indent(10);
		ImGui::Checkbox("Has Exit Time", &transition.hasExitTime);
		ImGui::SliderFloat("Exit Time", &transition.minTime, 0.f, 1.f);
		ImGui::SliderFloat("Transition Duration ", &transition.duration, 0.f, 1.f);
		ImGui::Unindent(10);
	}

	// 현재 트랜지션의 컨디션 조건 가져오기
	// 가져왔을 때 현재 조건을 포함해서 컨디션을 정할 수 있게
	// 각각의 컨디션마다 조건을 정할 수 있게 
	if (ImGui::CollapsingHeader("Conditions", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// 파라미터 목록 준비
		auto& parameters = m_pCurAnimator->GetParametersForEditor();
		vector<string> paramNames;
		paramNames.reserve(parameters.size());
		for (auto& kv : parameters)
			paramNames.push_back(kv.first);

		// 기존 조건들 편집
		for (_int idx = 0; idx < static_cast<_int>(transition.conditions.size());)
		{
			auto& cond = transition.conditions[idx];
			ImGui::PushID(idx);
			ImGui::Separator();

			//Param 선택
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
							cond.type = parameters[cond.paramName].type; // 파라미터 타입 업데이트

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
				// Trigger는 단일
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
				continue; // 현재 idx 항목이 지워졌으니 증가 생략
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

HRESULT CAnimTool::Register_Objects()
{
	if (FAILED(m_pEditorObjectFactory->RegisterObject<CFuoco>(TEXT("Fuoco"))))
		return E_FAIL;
	m_vecObjectNames.push_back("Fuoco");

	CMonster_Base::MONSTER_BASE_DESC* pDesc = new CMonster_Base::MONSTER_BASE_DESC();
	pDesc->fSpeedPerSec = 5.f;
	pDesc->fRotationPerSec = XMConvertToRadians(600.0f);
	pDesc->eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
	pDesc->InitPos = _float3(0.f, 0.f, 0.f);
	pDesc->InitScale = _float3(1.f, 1.f, 1.f);
	lstrcpy(pDesc->szName, TEXT("Buttler_Train"));
	pDesc->szMeshID = TEXT("Buttler_Train");
	pDesc->fHeight = 1.f;
	pDesc->vExtent = { 0.5f,1.f,0.5f };
	if (FAILED(m_pEditorObjectFactory->RegisterObject<CButtler_Train>(TEXT("Buttler_Train"), pDesc)))
		return E_FAIL;
	m_vecObjectNames.push_back("Buttler_Train");
	m_SpawnObjectDesc["Buttler_Train"] = pDesc;

	CMonster_Base::MONSTER_BASE_DESC* pDesc1 = new CMonster_Base::MONSTER_BASE_DESC();
	pDesc1->fSpeedPerSec = 5.f;
	pDesc1->fRotationPerSec = XMConvertToRadians(600.0f);
	pDesc1->eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
	pDesc1->InitPos = _float3(0.f, 0.f, 0.f);
	pDesc1->InitScale = _float3(1.f, 1.f, 1.f);
	lstrcpy(pDesc1->szName, TEXT("Buttler_Basic"));
	pDesc1->szMeshID = TEXT("Buttler_Basic");
	pDesc1->fHeight = 1.f;
	pDesc1->vExtent = { 0.5f,1.f,0.5f };
	if (FAILED(m_pEditorObjectFactory->RegisterObject<CButtler_Train>(TEXT("Buttler_Basic"), pDesc1)))
		return E_FAIL;
	m_vecObjectNames.push_back("Buttler_Basic");
	m_SpawnObjectDesc["Buttler_Basic"] = pDesc1;

	CMonster_Base::MONSTER_BASE_DESC* pDesc2 = new CMonster_Base::MONSTER_BASE_DESC();
	pDesc2->fSpeedPerSec = 5.f;
	pDesc2->fRotationPerSec = XMConvertToRadians(600.0f);
	pDesc2->eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
	pDesc2->InitPos = _float3(0.f, 0.f, 0.f);
	pDesc2->InitScale = _float3(1.f, 1.f, 1.f);
	lstrcpy(pDesc2->szName, TEXT("Buttler_Range"));
	pDesc2->szMeshID = TEXT("Buttler_Range");
	pDesc2->fHeight = 1.f;
	pDesc2->vExtent = { 0.5f,1.f,0.5f };
	if (FAILED(m_pEditorObjectFactory->RegisterObject<CButtler_Range>(TEXT("Buttler_Range"), pDesc2)))
		return E_FAIL;
	m_vecObjectNames.push_back("Buttler_Range");
	m_SpawnObjectDesc["Buttler_Range"] = pDesc2;

	CPlayer::PLAYER_DESC* pPlayerDesc = new CPlayer::PLAYER_DESC();
	pPlayerDesc->fSpeedPerSec = 5.f;
	pPlayerDesc->fRotationPerSec = XMConvertToRadians(600.0f);
	pPlayerDesc->eMeshLevelID = LEVEL::JW;
	pPlayerDesc->InitPos = _float3(0.f, 0.f, 0.f);
	pPlayerDesc->InitScale = _float3(1.f, 1.f, 1.f);
	pPlayerDesc->iLevelID = ENUM_CLASS(LEVEL::JW);
	lstrcpy(pPlayerDesc->szName, TEXT("Player"));
	pPlayerDesc->szMeshID = TEXT("Player");
	if (FAILED(m_pEditorObjectFactory->RegisterObject<CPlayer>(TEXT("Player"), pPlayerDesc)))
		return E_FAIL;
	m_vecObjectNames.push_back("Player");
	m_SpawnObjectDesc["Player"] = pPlayerDesc;

	if (FAILED(m_pEditorObjectFactory->RegisterObject<CElite_Police>(TEXT("Elite_Police"))))
		return E_FAIL;
	m_vecObjectNames.push_back("Elite_Police");

	if (FAILED(m_pEditorObjectFactory->RegisterObject<CFestivalLeader>(TEXT("FestivalLeader"))))
		return E_FAIL;
	m_vecObjectNames.push_back("FestivalLeader");
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
	m_LoadedModels.clear();

	for (auto& pair : m_LoadedAnimators)
	{
		Safe_Release(pair.second);
	}
	m_LoadedAnimators.clear();


	for (auto& obj : m_vecObjects)
	{
		Safe_Release(obj);
	}
	m_vecObjects.clear();
	for (auto& Pair : m_SpawnObjectDesc)
	{
		Safe_Delete(Pair.second);
	}
	m_SpawnObjectDesc.clear();

	Safe_Release(m_pTransformCom);

	ImNodes::DestroyContext();
	Safe_Release(m_pEventMag);
	Safe_Delete(m_pMySequence);
	Safe_Release(m_pAnimShader);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pEditorObjectFactory);
}
#endif