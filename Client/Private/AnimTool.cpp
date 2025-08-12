#ifdef USE_IMGUI
#include "Bone.h"
#include "EventMag.h"
#include "AnimTool.h"

#include "GameInstance.h"
#include <queue>

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


	ImNodesStyle& style = ImNodes::GetStyle();

		// 유니티 스타일 색상
		style.Colors[ImNodesCol_NodeBackground] = IM_COL32(60, 60, 70, 255);
		style.Colors[ImNodesCol_NodeBackgroundHovered] = IM_COL32(75, 75, 85, 255);
		style.Colors[ImNodesCol_NodeBackgroundSelected] = IM_COL32(85, 85, 95, 255);
		style.Colors[ImNodesCol_NodeOutline] = IM_COL32(100, 100, 110, 255);

		// 둥근 모서리
		style.NodeCornerRounding = 5.0f;
		style.NodePadding = ImVec2(8.0f, 4.0f);

		// 핀 스타일
		style.PinCircleRadius = 6.0f;
		style.PinQuadSideLength = 8.0f;
		style.LinkThickness = 3.0f;
		style.LinkLineSegmentsPerLength = 0.0f; // 직선


		  style.Colors[ImNodesCol_NodeBackground] = IM_COL32(56, 56, 56, 255);           // 더 어두운 배경
    style.Colors[ImNodesCol_NodeBackgroundHovered] = IM_COL32(70, 70, 70, 255);    // 호버 시
    style.Colors[ImNodesCol_NodeBackgroundSelected] = IM_COL32(90, 90, 90, 255);   // 선택 시
    style.Colors[ImNodesCol_NodeOutline] = IM_COL32(128, 128, 128, 255);           // 테두리

    // 타이틀바 색상 (유니티의 주황색 계열)
    style.Colors[ImNodesCol_TitleBar] = IM_COL32(58, 58, 58, 255);
    style.Colors[ImNodesCol_TitleBarHovered] = IM_COL32(72, 72, 72, 255);
    style.Colors[ImNodesCol_TitleBarSelected] = IM_COL32(86, 86, 86, 255);

    // === 핀(Pin) 스타일 ===
    // 입력/출력 핀 색상을 다르게 설정
    style.Colors[ImNodesCol_Pin] = IM_COL32(200, 200, 200, 255);                   // 기본 핀
    style.Colors[ImNodesCol_PinHovered] = IM_COL32(255, 255, 255, 255);            // 호버 시 핀

    // 핀 크기 및 모양 설정
    style.PinCircleRadius = 7.0f;        // 핀을 좀 더 크게
    style.PinQuadSideLength = 10.0f;     // 사각형 핀 크기
    style.PinTriangleSideLength = 12.0f; // 삼각형 핀 크기

    // === 링크(연결선) 스타일 - 핵심 개선 사항 ===
    
    // 1. 직선형 링크로 변경 (곡선 제거)
    style.LinkThickness = 3.0f;                    // 선 두께
    style.LinkLineSegmentsPerLength = 0.0f;        // 0으로 설정하면 직선
    style.LinkHoverDistance = 10.0f;               // 마우스 호버 감지 거리
    
    // 2. 링크 색상 설정 (유니티 스타일)
    style.Colors[ImNodesCol_Link] = IM_COL32(150, 150, 150, 255);                // 기본 링크 색상
    style.Colors[ImNodesCol_LinkHovered] = IM_COL32(255, 255, 255, 255);         // 호버 시 흰색
    style.Colors[ImNodesCol_LinkSelected] = IM_COL32(255, 165, 0, 255);          // 선택 시 주황색

    // === 노드 모양 개선 ===
    style.NodeCornerRounding = 6.0f;       // 모서리 둥글기
    style.NodePadding = ImVec2(10.0f, 6.0f); // 노드 내부 여백 증가
    style.NodeBorderThickness = 2.0f;      // 테두리 두께

    // === 그리드 스타일 ===
    style.Colors[ImNodesCol_GridBackground] = IM_COL32(40, 40, 40, 255);    // 배경색
    style.Colors[ImNodesCol_GridLine] = IM_COL32(60, 60, 60, 100);          // 그리드 라인
    style.GridSpacing = 32.0f;                                               // 그리드 간격

    // === 선택 영역 스타일 ===
    style.Colors[ImNodesCol_BoxSelector] = IM_COL32(100, 149, 237, 80);     // 선택 박스
    style.Colors[ImNodesCol_BoxSelectorOutline] = IM_COL32(100, 149, 237, 255);

    // === 미니맵 스타일 ===
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

    // 미니맵 크기 및 위치
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
			_int t = static_cast<_int>(parmeter.type);
	/*		if (ImGui::Combo(("##Type" + id).c_str(), &t, typeNames, IM_ARRAYSIZE(typeNames)))
				parmeter.type = (ParamType)t;*/

			if (ImGui::Combo(("##Type" + id).c_str(), &t, typeNames, IM_ARRAYSIZE(typeNames)))
			{
				ParamType newType = (ParamType)t;
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
		// 언레지스터 → 이름 바꿔서 → 리지스터
		auto pCtrl= ctrls[oldName];
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
		overrideCtrlNames.reserve(m_pCurAnimator->GetOverrideAnimControllersMap().size()+1);
		overrideCtrlNames.push_back("None");
		for (const auto& Pair : m_pCurAnimator->GetOverrideAnimControllersMap())
		{
			overrideCtrlNames.push_back(Pair.first);
		}

		// 현재 컨트롤러의 스테이트들 가져와서 설정하게
		auto pCtrl = m_pCurAnimator->Get_CurrentAnimController();
		auto& states = pCtrl->GetStates();
		if (m_NewOverrideAnimController.states.empty() || states.size()!= m_NewOverrideAnimController.states.size())
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
						m_bIsUesMaskBoneState = it->maskBoneName.empty() == false;
					}

					// 저장된 오버라이드 상태에서 애니메이션 인덱스 찾기
					auto& selectedState = m_NewOverrideAnimController.states[stateNames[i]];
					vector<CAnimation*> anims = m_LoadedAnimations[m_stSelectedModelName];

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
			vector<CAnimation*> anims = m_LoadedAnimations[m_stSelectedModelName]; // 현재 선택된 모델의 애니메이션들

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

HRESULT CAnimTool::Render_TransitionConditions()
{
	// 현재 추가되어있는 트랜지션의 조건들을 보여주고 콤보박스로 컨디션이 있으면 수정할 수 있게 처리
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

			//size_t newIdx = pCtrl->AddState(stateName, selectedAnim, m_iSpeicificNodeId++);
			_int iNodeId = m_iSpeicificNodeId++; // 고유한 노드 아이디 생성
			// ID 확인
			for (auto& state : pCtrl->GetStates())
			{
				if (state.iNodeId == iNodeId)
				{
					iNodeId = m_iSpeicificNodeId++; // 중복되면 다음 ID로
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
	if (ImGui::IsKeyDown(ImGuiKey_L))
	{
		//ApplyHierarchicalLayout(pCtrl);
		ApplyCategoryLayout(pCtrl);
	//	ImNodes::EditorContextResetPanning(ImVec2(0.0f, 0.0f));
	}

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
				//if (state.iNodeId == pCtrl->GetExitNodeId())
				//{
				//	ImGui::SameLine();
				//	ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "[EXIT]"); // 빨간색으로 Exit 표시
				//}
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

				// 노드 위치 저장
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
		//		// 둘 중 하나라도 선택이 됐으면
		//		//ImNodes::Link(t.link.iLinkId, startPinID, endPinID);
		//	}
		//}
		//else  // 아무것도 선택이 안됐는데 링크도 선택된게 없으면
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
		if (isAnyLinkSelected) 
		{
			// 선택된 링크만
			bDrawLink = thisLinkSelected;
		}
		else if (isAnyNodeSelected)
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


	ImNodes::EndNodeEditor();

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
				link.iLinkId = m_iSpeicificNodeId++;
				link.iLinkStartID = startPinID;
				link.iLinkEndID = endPinID;

				// 트랜지션 추가
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

		// 링크 선택시 트랜지션 상태 표시
		// 선택된 링크의 Condition을 찾아서 표시

		// CAnimController::Condition testCondition{ "Test", CAnimController::EOp::Finished, 0.f, 0.95f }; 컨디션 예시

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

	// 노드 삭제 (스테이트 삭제)
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




	vector<CAnimation*> anims = m_LoadedAnimations[m_stSelectedModelName]; // 현재 선택된 모델의 애니메이션들

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
					if (ImGui::BeginCombo("Clips", m_iDefualtSeletedAnimIndex >= 0 ? animNames[m_iDefualtSeletedAnimIndex].c_str() : "Select Clip"))
					{
						for (_int i = 0; i < animNames.size(); ++i)
						{
							_bool isSelected = (i == m_iDefualtSeletedAnimIndex);
							if (ImGui::Selectable(animNames[i].c_str(), isSelected))
							{
								m_iDefualtSeletedAnimIndex = i;
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

	vector<CAnimation*> anims = m_LoadedAnimations[m_stSelectedModelName]; // 현재 선택된 모델의 애니메이션들

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
	map<_int, _int> nodeOrderInLevel; // 각 계층 내에서 노드의 순서

	
	queue<_int> q;

	// ENTRY 노드를 찾아 큐에 넣고, 방문 처리 및 0레벨로 설정
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

		// 현재 노드에서 나가는 전환 찾기
		for (const auto& transition : pCtrl->GetTransitions())
		{
			if (transition.iFromNodeId == currentNodeId)
			{
				_int nextNodeId = transition.iToNodeId;
				if (!visited.count(nextNodeId))
				{
					visited.insert(nextNodeId); // 방문 처리
					level[nextNodeId] = level[currentNodeId] + 1; // 다음 노드는 현재 노드보다 한 단계 아래
					q.push(nextNodeId);
				}
			}
		}
	}

	// 계산된 레벨과 순서를 기반으로 노드 위치 설정
	map<_int, _int> levelNodeCount;
	_float horizontalSpacing = 220.0f; // 노드 간 가로 간격
	_float verticalSpacing = 200.0f;   // 노드 간 세로 간격

	// 노드를 레벨별로 그룹화
	map<_int,vector<_int>> nodesByLevel;
	for (const auto& state : pCtrl->GetStates())
	{
		nodesByLevel[level[state.iNodeId]].push_back(state.iNodeId);
	}

	// 각 레벨의 노드 위치를 계산하여 설정
	for (auto const& [nodeLevel, nodeIds] : nodesByLevel)
	{
		_float startX = 0.0f;
		// 필요에 따라 각 레벨을 중앙에 오게 조정
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

	//// 상태를 카테고리별로 분류
	//map<string, vector<_int>> categories;
	//m_CategoryStates.clear();
	//for (const auto& state : states)
	//{
	//	string category = GetStateCategory(state.stateName);
	//	categories[category].push_back(state.iNodeId);
	//	if (m_CategoryVisibility.find(category) == m_CategoryVisibility.end())
	//		m_CategoryVisibility[category] = true; // 카테고리 vis 초기화
	//	m_CategoryStates[category].push_back(state.stateName); // 카테고리에 state 이름 추가
	//}


	//_float categorySpacing = 600.0f;
	//_float nodeSpacing = 300.0f;
	//_int categoryIndex = 0;

	//for (auto& [categoryName, nodeIds] : categories)
	//{
	//	if (!m_bShowAll && m_CategoryVisibility[categoryName] == false)
	//		continue;
	//	// 각 카테고리를 세로로 나열
	//	_float categoryX = categoryIndex * categorySpacing;

	//	// 카테고리 내 노드들을 격자로 배치
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

	// ---- 0) 허브 노드(Any/Exit) 별도 고정 배치 ----
	constexpr float kHubY = -220.f;     // 상단에 고정
	constexpr float kAnyX = -500.f;
	// Exit X는 나중에 카테고리 개수로 계산

	// ---- 1) 카테고리 구성 + 가시성 초기화 ----
	std::map<std::string, std::vector<_int>> categories; // 이름순 정렬 보장
	m_CategoryStates.clear();

	// 연결 허브 정도 계산(허브/중간 노드 위쪽 배치)
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

	// ---- 2) 배치 파라미터 ----
	const float categorySpacingX = 720.f;   // 카테고리(컬럼) 간 X 간격
	const float nodeSpacingX = 280.f;   // 같은 카테고리 내 X 간격
	const float nodeSpacingY = 170.f;   // 같은 카테고리 내 Y 간격
	const int   MAX_COLS = 3;       // 한 카테고리 최대 열 수(너무 가로로 길어지는 것 방지)
	const bool  snakeLayout = true;    // 지그재그 배치로 링크 교차 완화

	// ---- 3) 가시 카테고리 목록 추출 ----
	std::vector<std::pair<std::string, std::vector<_int>>> visibleCats;
	visibleCats.reserve(categories.size());
	for (auto& kv : categories) {
		if (m_bShowAll || m_CategoryVisibility[kv.first])
			visibleCats.push_back(kv);
	}
	if (visibleCats.empty())
		return;

	// ---- 4) 각 카테고리별 rows/cols 계산, 전체에서 maxRows 산출(세로 중앙정렬용) ----
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

	// ---- 5) 카테고리별 정렬 규칙(허브/중간 노드 우선) ----
	for (auto& kv : visibleCats) {
		auto& list = kv.second;
		sort(list.begin(), list.end(), [&](int a, int b) {
			int da = degree.count(a) ? degree[a] : 0;
			int db = degree.count(b) ? degree[b] : 0;
			if (da != db) return da > db;            // 연결 많은 노드 먼저
			return a < b;                             // 동일하면 ID 오름차순
			});
	}

	auto ClusterKey = [](const string& name)->string
		{
			size_t p1 = name.find('_');
			if (p1 == string::npos) return name;

			size_t p2 = name.find('_', p1 + 1);
			if (p2 == string::npos) return name.substr(0, p1); // 1토큰
			return name.substr(0, p2);                          // 2토큰
		};

	// ---- 6) 카테고리를 컬럼으로 배치(클러스터 + 최대 행수 고정) ----
	const int   kMaxRowsPerCol = 6;     // ★ 세로 길이(행 수) 제한
	const float kColGapX = 40.f;  // 같은 버킷(Column) 간 간격

	for (size_t ci = 0; ci < visibleCats.size(); ++ci)
	{
		const auto& catName = visibleCats[ci].first;
		const auto& nodeIds = visibleCats[ci].second;

		// 6-1) 이 카테고리 노드를 이름 프리픽스(앞 1~2 토큰)로 버킷화
		map<string, vector<_int>> buckets;
		for (int id : nodeIds)
		{
			auto* st = pCtrl->GetStateByNodeIdForEditor(id);
			const string nm = st ? st->stateName : "";
			buckets[ClusterKey(nm)].push_back(id);
		}

		// 6-2) 카테고리 시작 X
		float baseX = (float)ci * categorySpacingX;

		// 6-3) 세로 중앙 정렬용 대략 기준(Y 시작점)
		const float yBase = -0.5f * (kMaxRowsPerCol - 1) * nodeSpacingY;

		// 6-4) 버킷 단위로 배치: 버킷마다 세로 kMaxRowsPerCol까지 쌓고 넘치면 옆 열로 wrap
		float curX = baseX;
		for (auto& kv : buckets)
		{
			auto& ids = kv.second;
			sort(ids.begin(), ids.end()); // 안정성(원하면 유지)

			int row = 0;
			float colX = curX;

			for (int idx = 0; idx < (int)ids.size(); ++idx)
			{
				if (row >= kMaxRowsPerCol) // 줄바꿈
				{
					row = 0;
					colX += nodeSpacingX + kColGapX;
				}

				ImVec2 pos = ImVec2(colX, yBase + row * nodeSpacingY);
				ImNodes::SetNodeEditorSpacePos(ids[idx], pos);
				++row;
			}

			// 다음 버킷은 한 칸 띄워서 시작
			curX = colX + nodeSpacingX + kColGapX * 2.f;
		}
	}
	// ---- 7) 허브 노드(Any/Exit) 최종 위치 지정 ----
	// Any는 왼쪽 상단, Exit은 마지막 카테고리 오른쪽 상단으로 고정
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
			MSG_BOX("애니메이터가 없습니다. 애니메이터를 먼저 생성해주세요.");
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

			//pAnimator->Initialize_Test(pModel);
			pAnimator->Initialize_Test(&desc);
			m_LoadedAnimators[modelName] = pAnimator;
			pAnimator->RegisterEventListener("TestEvent", [&](const string& eventName)
				{
					MSG_BOX("애니메이션 이벤트 발생");
				});
		}

		// 모델 불러오면 처음 애니메이션 관련 정보들 불러오기 
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

	// 컬럼 Name | Type | Value | Action


	if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_CollapsingHeader))
	{
		// 이 안이 접힌 상태일 때 숨겨지고, 펼쳐지면 보입니다.
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
	//		cout << "---------GPU 계산---------------" << endl;
	//		cout << mat.m[0][0] << " " << mat.m[0][1] << " " << mat.m[0][2] << " " << mat.m[0][3] << endl;
	//		cout << mat.m[1][0] << " " << mat.m[1][1] << " " << mat.m[1][2] << " " << mat.m[1][3] << endl;
	//		cout << mat.m[2][0] << " " << mat.m[2][1] << " " << mat.m[2][2] << " " << mat.m[2][3] << endl;
	//		cout << mat.m[3][0] << " " << mat.m[3][1] << " " << mat.m[3][2] << " " << mat.m[3][3] << endl;
	//		cout << "------------------------" << endl;

	//		auto mat2 = *m_pCurModel->Get_Bones()[i]->Get_CombinedTransformationMatrix();

	//		cout << "---------CPU 계산---------------" << endl;
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
	//	// bone->Get_CombinedTransformationMatrix()는 CPU에서 계산된 본 행렬
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
	//assert(cur[0] == expect); // ★ 이제 통과해야 정상
	//for (auto& s : cur) if (s) s->Release();
	//ID3D11UnorderedAccessView* nullUAV = nullptr;
	//UINT counts = 0;
	//m_pContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, &counts);
	//ID3D11ShaderResourceView* nullSRV[3]{ nullptr, nullptr, nullptr };


	//// VS 단계 t0,t1,t2 슬롯에서 언바인드
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