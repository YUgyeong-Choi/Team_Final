#include "MapTool.h"
#include "GameInstance.h"

#include <filesystem>
#include "MapToolObject.h"

#include "Layer.h"
#include "RenderTarget.h"

#include "PreviewObject.h"

#include "Camera_Free.h"

#include "Camera_Manager.h"

#include "Client_Function.h"



//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감

CMapTool::CMapTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CYWTool(pDevice, pContext)
{

}

CMapTool::CMapTool(const CMapTool& Prototype)
	: CYWTool(Prototype)
{
}

HRESULT CMapTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	//즐겨찾기 로드
	if (FAILED(Load_Favorite()))
		E_FAIL;

	//MapData에 배치되어있는 모델들을 미리 불러들인다.
	/*if (FAILED(Ready_Model()))
		return E_FAIL;*/

	//MapData를 따라 맵을 로드한다.
	if (FAILED(Load_Map()))
		return E_FAIL;

	m_pPreviewObject = static_cast<CPreviewObject*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_PreviewObject")));
	if (m_pPreviewObject == nullptr)
		return E_FAIL;

	Safe_AddRef(m_pPreviewObject);

	XMStoreFloat4x4(&m_CopyWorldMatrix, XMMatrixIdentity());

	m_pCamera_Free = CCamera_Manager::Get_Instance()->GetFreeCam();
	Safe_AddRef(m_pCamera_Free);

	return S_OK;
}

void CMapTool::Priority_Update(_float fTimeDelta)
{

}

void CMapTool::Update(_float fTimeDelta)
{
	Control(fTimeDelta);

	//포커스 된놈만 업데이트 시켜주자
	if (m_pFocusObject != nullptr)
	{
		m_pFocusObject->Update_ColliderPos();
	}

}

void CMapTool::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONLIGHT, this);
}

HRESULT CMapTool::Render()
{
	//모두 렌더 하는것도 만들자
	if (m_bRenderAllCollider)
	{
		for (auto& Group : m_ModelGroups) // 모델 이름별로 그룹화된 GameObject 목록을 반복
		{
			for (auto pGameObject : Group.second)
			{
				if (FAILED(static_cast<CMapToolObject*>(pGameObject)->Render_Collider()))
					return E_FAIL;
			}
		}

	}
	//포커스 된놈만 랜더
	else if (m_pFocusObject)
	{
		if (FAILED(m_pFocusObject->Render_Collider()))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMapTool::Render_ImGui()
{
	if (FAILED(Render_MapTool()))
		return E_FAIL;

	return S_OK;

}

void CMapTool::Control(_float fTimeDelta)
{
	if (GetForegroundWindow() != g_hWnd)
		return;

	if (ImGuizmo::IsUsing() == false)
	{
		//E 회전, R 크기, T는 위치
		if (m_pGameInstance->Key_Down(DIK_E))
			m_currentOperation = ImGuizmo::ROTATE;
		else if (m_pGameInstance->Key_Down(DIK_R))
			m_currentOperation = ImGuizmo::SCALE;
		else if (m_pGameInstance->Key_Down(DIK_T))
			m_currentOperation = ImGuizmo::TRANSLATE;

		if (m_pGameInstance->Mouse_Up(DIM::WHEELBUTTON))
		{
			//모든 오브젝트 선택 제거

			m_SelectedIndexies.clear();

			for (CMapToolObject* pObj : m_SelectedObjects)
				Safe_Release(pObj);
			m_SelectedObjects.clear();

			m_iFocusIndex = -1;
			Safe_Release(m_pFocusObject);
			m_pFocusObject = nullptr;

		}
	}
	

	//컨트롤 클릭 하면 피킹된 위치로 이동
	if (m_pGameInstance->Key_Pressing(DIK_LALT) && m_pGameInstance->Mouse_Up(DIM::LBUTTON))
	{
		if (m_pFocusObject)
		{
			CTransform* pTransform = m_pFocusObject->Get_TransfomCom();
	
			_float4 vPickedPos = {};
			if (m_pGameInstance->Picking(&vPickedPos))
			{
				//이전 월드 행렬 저장
				m_pFocusObject->Set_UndoWorldMatrix(pTransform->Get_WorldMatrix());
				pTransform->Set_State(STATE::POSITION, XMLoadFloat4(&vPickedPos));

			}
		}
	}

	//Ctrl + S 맵 저장
	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_S))
	{
		Save_Map();
	}

	//Ctrl + D 선택된 오브젝트 복제
	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_D))
	{
		Duplicate_Selected_Object();
	}

	//Ctrl + Z 해당 오브젝트에 저장된 이전위치 이동
	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_Z))
	{
		Undo_Selected_Object();
	}


	//마우스 드래그 시작
	if (m_pGameInstance->Mouse_Down(DIM::LBUTTON) && ImGuizmo::IsOver() == false)
	{
		m_vMouseDragStart = Get_MousePos();
		m_bDragging = true;
	}

	//드래그 끝
	if (m_pGameInstance->Mouse_Up(DIM::LBUTTON))
	{
		m_bDragging = false;
	}

	//피킹했을 때 오브젝트 선택 하는 기능(기즈모가 다른 물체보다 뒤에 있으면 조작하려는 물체가 바뀌어버림 IsOver()로 해결)
	if (m_pGameInstance->Mouse_Up(DIM::LBUTTON) && ImGuizmo::IsOver() == false)
	{
		//알트키 누르고 있으면 피킹하지 않음(오브젝트 붙이고나서 오브젝트 변경되는거 막기 위함임)
		if (m_pGameInstance->Key_Pressing(DIK_LALT))
			return;

		// ImGui가 마우스 입력을 가져가면 피킹을 하지 않음
		if (ImGui::GetIO().WantCaptureMouse)
			return;

		_float2 vMouseDragEnd = Get_MousePos();
		_float	fDragLength = XMVectorGetX(XMVector2Length(XMLoadFloat2(&vMouseDragEnd) - XMLoadFloat2(&m_vMouseDragStart)));
		_bool	bIsDrag = { false };

		if (fDragLength > 3.f)
			bIsDrag = true;

		if (bIsDrag)
			SelectByDrag(vMouseDragEnd);
		else
			SelectByClick();
		
	}

	//딜리트키 누르면 현재 선택된거 삭제
	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		printf("Delete\n");
		DeleteMapToolObject();
	}
	
	//F 키누르면 해당 오브젝트 위치로 이동
	if (m_pGameInstance->Key_Down(DIK_F))
	{
		if (m_SelectedObjects.empty() == false)
		{
			_vector vObjectPos = (*m_SelectedObjects.begin())->Get_TransfomCom()->Get_State(STATE::POSITION);
			_vector vCameraPos = XMVectorAdd(vObjectPos, XMVectorSet(0.f, 3.f, -3.f, 0.f));

			CTransform* pCameraTransformCom = CCamera_Manager::Get_Instance()->GetFreeCam()->Get_TransfomCom();

			//여유를 두고 이동한후
			pCameraTransformCom->Set_State(STATE::POSITION, vCameraPos);

			//LookAt 하자
			pCameraTransformCom->LookAt(vObjectPos);
		}
	}

	Control_PreviewObject(fTimeDelta);

}

//HRESULT CMapTool::Ready_Model()
//{
//	ifstream inFile("../Bin/Save/MapTool/ReadyModel.json");
//	if (!inFile.is_open())
//	{
//		MSG_BOX("ReadyModel.json 파일을 열 수 없습니다.");
//		return S_OK;
//	}
//
//	json ReadyModelJson;
//	try
//	{
//		inFile >> ReadyModelJson;
//		inFile.close();
//	}
//	catch (const exception& e)
//	{
//		inFile.close();
//		MessageBoxA(nullptr, e.what(), "JSON 파싱 실패", MB_OK);
//		return E_FAIL;
//	}
//
//	// JSON 데이터 확인
//	for (const auto& element : ReadyModelJson)
//	{
//		string ModelName = element.value("ModelName", "");
//		string Path = element.value("Path", "");
//
//		//모델 프로토 타입 생성
//		wstring PrototypeTag = L"Prototype_Component_Model_" + StringToWString(ModelName);
//
//		const _char* pModelFilePath = Path.c_str();
//
//		if (FAILED(Load_Model(PrototypeTag, pModelFilePath)))
//		{
//			return E_FAIL;
//		}
//	}
//
//	return S_OK;
//}

HRESULT CMapTool::Save_Map()
{
	filesystem::create_directories("../Bin/Save/MapTool");
	ofstream MapDataFile("../Bin/Save/MapTool/MapData.json");
	ofstream ReadyModelFile("../Bin/Save/MapTool/ReadyModel.json");

	json ReadyModelJsonArray = json::array();
	json MapDataJson; // 모델과 오브젝트 정보 저장용

	//현재 필드에 존재하는 모델들의 레이어 이름들을 가져온다.
	vector<wstring> LayerNames = m_pGameInstance->Find_LayerNamesContaining(ENUM_CLASS(LEVEL::YW), TEXT("Layer_MapToolObject_"));

	//모델 갯수 저장
	_uint iModelCount = static_cast<_uint>(LayerNames.size());
	MapDataJson["ModelCount"] = iModelCount;
	MapDataJson["Models"] = json::array();

	for (wstring& LayerName : LayerNames)
	{
		list<CGameObject*>& MapObjectList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::YW), LayerName);

		//모델 이름저장
		wstring Prefix = L"Layer_MapToolObject_";
		wstring ModelName = {};
		if (LayerName.find(Prefix) == 0) // 접두사 확인
		{
			ModelName = LayerName.substr(Prefix.length());
		}

		//Json 파일에 모델의 이름과 파일경로 저장
		string strModelName = WStringToString(ModelName);
		filesystem::path path = filesystem::path(PATH_NONANIM) / (strModelName + ".bin"); //이렇게 하드코드 저장말고
		string FullPath = path.generic_string();

		json ReadyModelJson;
		ReadyModelJson["ModelName"] = strModelName;
		ReadyModelJson["Path"] = FullPath; // 또는 Path
		ReadyModelJson["Collision"] = false;
		ReadyModelJson["ObjectCount"] = static_cast<_uint>(MapObjectList.size());//갯수 저장해서 인스턴싱 모델을 로드할지 결정(아니 충돌여부로 할거야)

		// 문자열 길이 계산
		_uint iLength = static_cast<_uint>(ModelName.length());

		// 오브젝트 갯수 저장
		_uint iObjectCount = static_cast<_uint>(MapObjectList.size());

		// 모델 JSON
		json ModelJson;
		ModelJson["ModelName"] = strModelName;
		ModelJson["ObjectCount"] = static_cast<_uint>(MapObjectList.size());
		ModelJson["Collision"] = false;
		ModelJson["Objects"] = json::array();

		for (CGameObject* pGameObject : MapObjectList)
		{
			// JSON 하나 구성
			json ObjectJson;

			//모델 위치들 저장
			_matrix matWorld = pGameObject->Get_TransfomCom()->Get_WorldMatrix();
			_float4x4 matWorldFloat4x4;
			XMStoreFloat4x4(&matWorldFloat4x4, matWorld);

			// 행렬을 4x4 배열로 저장
			json MatrixJson = json::array();
			for (int i = 0; i < 4; ++i)
			{
				json Row = json::array();
				for (int j = 0; j < 4; ++j)
				{
					Row.push_back(matWorldFloat4x4.m[i][j]);
				}
				MatrixJson.push_back(Row);
			}

			CMapToolObject* pMapToolObject = static_cast<CMapToolObject*>(pGameObject);

			ObjectJson["WorldMatrix"] = MatrixJson;
			if(pMapToolObject->m_bUseTiling)
				ObjectJson["TileDensity"] = { pMapToolObject->m_TileDensity[0], pMapToolObject->m_TileDensity[1] };

			ObjectJson["ColliderType"] = static_cast<_int>(pMapToolObject->m_eColliderType);
			//한번이라도 충돌체가 있으면 인스턴싱으로 못부르게 만들 것임
			if (pMapToolObject->m_eColliderType != COLLIDER_TYPE::NONE)
			{
				ModelJson["Collision"] = true;
				ReadyModelJson["Collision"] = true;
			}

			ModelJson["Objects"].push_back(ObjectJson);
		}

		ReadyModelJsonArray.push_back(ReadyModelJson);
		MapDataJson["Models"].push_back(ModelJson);
	}

	// 파일에 JSON 쓰기
	ReadyModelFile << ReadyModelJsonArray.dump(4);
	MapDataFile << MapDataJson.dump(4);

	MapDataFile.close();
	ReadyModelFile.close();

	MSG_BOX("맵 저장 성공");

	return S_OK;
}

HRESULT CMapTool::Load_Map()
{
	ifstream inFile("../Bin/Save/MapTool/MapData.json");
	if (!inFile.is_open())
	{
		MSG_BOX("MapData.json 파일을 열 수 없습니다.");
		return S_OK;
	}

	json MapDataJson;
	inFile >> MapDataJson;
	inFile.close();

	_uint iModelCount = MapDataJson["ModelCount"];
	const json& Models = MapDataJson["Models"];

	for (_uint i = 0; i < iModelCount; ++i)
	{
		string ModelName = Models[i]["ModelName"];
		_uint iObjectCount = Models[i]["ObjectCount"];
		const json& Objects = Models[i]["Objects"];

		for (_uint j = 0; j < iObjectCount; ++j)
		{
			const json& WorldMatrixJson = Objects[j]["WorldMatrix"];
			_float4x4 WorldMatrix = {};

			for (_int row = 0; row < 4; ++row)
				for (_int col = 0; col < 4; ++col)
					WorldMatrix.m[row][col] = WorldMatrixJson[row][col];

			//오브젝트 생성, 배치

			wstring LayerTag = TEXT("Layer_MapToolObject_");
			LayerTag += StringToWString(ModelName);

			CMapToolObject::MAPTOOLOBJ_DESC MapToolObjDesc = {};

			wstring wstrModelName = StringToWString(ModelName);
			wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
			ModelPrototypeTag += wstrModelName;

			lstrcpy(MapToolObjDesc.szModelName, wstrModelName.c_str());
			lstrcpy(MapToolObjDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());
			MapToolObjDesc.WorldMatrix = WorldMatrix;

			MapToolObjDesc.iID = m_iID++;

			//타일링
			if (Objects[j].contains("TileDensity"))
			{
				MapToolObjDesc.bUseTiling = true;

				const json& TileDensityJson = Objects[j]["TileDensity"];
				MapToolObjDesc.vTileDensity = {
					TileDensityJson[0].get<_float>(),
					TileDensityJson[1].get<_float>()
				};
			}

			//콜라이더
			if (Objects[j].contains("ColliderType") && Objects[j]["ColliderType"].is_number_integer())
			{
				MapToolObjDesc.eColliderType = static_cast<COLLIDER_TYPE>(Objects[j]["ColliderType"].get<_int>());
			}
			else
				return E_FAIL;

			if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
				ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
				return E_FAIL;

			//방금 추가한서을 모델 그룹에 분류해서 저장
			Add_ModelGroup(ModelName, m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), LayerTag));

		}
	}

	return S_OK;
}

HRESULT CMapTool::Save_Favorite()
{
	//즐겨찾기 추가 삭제 시
	//현재 즐겨찾기에 포함된 이름들을 저장한다.
	filesystem::create_directories("../Bin/Save/MapTool");
	ofstream Favorite("../Bin/Save/MapTool/Favorite.json");

	json FavoritJsonArray = json::array();

	//즐겨찾기 목록
	//m_FavoriteModelNames;

	for (string& ModelName : m_FavoriteModelNames)
	{
		//Json 파일에 모델의 이름과 파일경로 저장
		filesystem::path path = filesystem::path(PATH_NONANIM) / (ModelName + ".bin"); //이렇게 하드코드 저장말고
		string FullPath = path.generic_string();

		json ObjectJson;
		ObjectJson["ModelName"] = ModelName;
		ObjectJson["Path"] = FullPath; // 또는 Path

		FavoritJsonArray.push_back(ObjectJson);

	}

	// 파일에 JSON 쓰기
	Favorite << FavoritJsonArray.dump(4);
	Favorite.close();

	//MSG_BOX("즐겨 찾기 저장 성공");

	return S_OK;
}

HRESULT CMapTool::Load_Favorite()
{
	//레벨이 시작할 때
	//제이슨에 저장된 이름의 모델들을 모두 로드하고
	//즐겨 찾기 목록에 포함 시킨다.

	ifstream inFile("../Bin/Save/MapTool/Favorite.json");
	if (!inFile.is_open())
	{
		MSG_BOX("Favorite.json 파일을 열 수 없습니다.");
		return S_OK;
	}

	json FavoriteJson;
	try
	{
		inFile >> FavoriteJson;
		inFile.close();
	}
	catch (const exception& e)
	{
		inFile.close();
		MessageBoxA(nullptr, e.what(), "JSON 파싱 실패", MB_OK);
		return E_FAIL;
	}

	// JSON 데이터 확인
	for (const auto& element : FavoriteJson)
	{
		string ModelName = element.value("ModelName", "");
		string Path = element.value("Path", "");

		//모델 프로토 타입 생성
		wstring PrototypeTag = L"Prototype_Component_Model_" + StringToWString(ModelName);

		const _char* pModelFilePath = Path.c_str();

		if (FAILED(Load_Model(PrototypeTag, pModelFilePath)))
		{
			return E_FAIL;
		}

		if (FAILED(Add_Favorite(ModelName, false)))
			return E_FAIL;
	}

	return S_OK;

}

HRESULT CMapTool::Render_MapTool()
{
	if (GetForegroundWindow() != g_hWnd)
		return S_OK;

	Render_Hierarchy();

	Render_Asset();
	Render_Favorite();

	Render_Preview();

	Render_Detail();

	//드래그 사각형 그리기
	if (m_bDragging && ImGui::GetIO().WantCaptureMouse == false)
	{
		_float2 vDragEnd = Get_MousePos();

		ImDrawList* draw_list = ImGui::GetBackgroundDrawList(); // 배경에 그려짐
		ImVec2 start(m_vMouseDragStart.x, m_vMouseDragStart.y);
		ImVec2 end(vDragEnd.x, vDragEnd.y);

		draw_list->AddRectFilled(start, end, IM_COL32(100, 150, 255, 40));  // 반투명 파란색
		draw_list->AddRect(start, end, IM_COL32(100, 150, 255, 255));       // 외곽선
	}

	return S_OK;
}

void CMapTool::Render_Hierarchy()
{
#pragma region 하이어라키
	ImGui::Begin("Hierarchy", nullptr);

	if (ImGui::BeginListBox("##HierarchyList", ImVec2(-FLT_MIN, 300)))
	{
		_uint i = 0; // 전체 Hierarchy 항목 인덱스를 위한 카운터 (전역 인덱스)
		for (auto& group : m_ModelGroups) // 모델 이름별로 그룹화된 GameObject 목록을 반복
		{
			const string& ModelName = group.first; // 현재 그룹의 모델 이름
			// 트리 노드 생성 (열려있게 기본 설정) - 모델 이름을 기준으로 그룹화된 항목
			_bool bOpen = ImGui::TreeNodeEx(ModelName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

			for (auto pGameObject : group.second)
			{
				// 개별 오브젝트에 대한 이름 생성 (ID 값 주자 나중에)
				string strHierarchyName = "(ID:" + to_string(static_cast<CMapToolObject*>(pGameObject)->Get_ID()) + ')' + ModelName;

				// 현재 인덱스가 선택된 상태인지 확인
				_bool isSelected = (m_SelectedIndexies.count(i) > 0);

				if (bOpen) // 트리 노드가 열려 있을 때만 Selectable 항목을 그린다
				{
					// 해당 항목이 클릭되면 인덱스를 기록하여 선택 상태로 만든다
					if (ImGui::Selectable(strHierarchyName.c_str(), isSelected))
					{
						if (ImGui::GetIO().KeyCtrl)
						{
							// Ctrl 눌렀으면 선택 토글
							if (isSelected)
							{
								m_SelectedObjects.erase(static_cast<CMapToolObject*>(pGameObject));
								Safe_Release(pGameObject);

								m_SelectedIndexies.erase(i);
							}
							else
							{
								m_SelectedObjects.insert(static_cast<CMapToolObject*>(pGameObject));
								Safe_AddRef(pGameObject);

								m_SelectedIndexies.insert(i);
							}
						}
						else
						{
							for (CMapToolObject* pObj : m_SelectedObjects)
								Safe_Release(pObj);
							m_SelectedObjects.clear();

							m_SelectedObjects.insert(static_cast<CMapToolObject*>(pGameObject));
							Safe_AddRef(pGameObject);

							// Ctrl 안 눌렀으면 단일 선택
							m_SelectedIndexies.clear();
							m_SelectedIndexies.insert(i);
						}

						//마지막 클릭한 항목
						m_iFocusIndex = i;

						Safe_Release(m_pFocusObject);
						m_pFocusObject = static_cast<CMapToolObject*>(Get_Focused_Object());
						Safe_AddRef(m_pFocusObject);
					}
				}

				// 포커스만 마지막 항목만
				if (m_iFocusIndex == i)
					ImGui::SetItemDefaultFocus();

				++i; // 전체 인덱스 증가 (트리 노드 열려있든 말든 증가시켜야 함)
			}

			if (bOpen)
				ImGui::TreePop(); // 트리 노드 닫기 (트리 UI를 닫아줌)
		}

		ImGui::EndListBox(); // 리스트박스 끝

	}
#pragma endregion
	if (ImGui::Button("Delete"))
	{
		DeleteMapToolObject();
	}

	if (ImGui::Button("Save Map"))
	{
		if (FAILED(Save_Map()))
			MSG_BOX("맵 저장 실패");
	}


	//if (ImGui::Button("Load Map"))
	//{

	//}
	ImGui::End();
}

void CMapTool::Render_Asset()
{
#pragma region 에셋
	ImGui::Begin("Asset", nullptr);

	if (ImGui::Button("Spawn"))
	{
		if (FAILED(Spawn_MapToolObject()))
		{
			MSG_BOX("스폰 실패");
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Add Favorites"))
	{
		if (false == m_ModelNames.empty())
		{
			if (FAILED(Add_Favorite(m_ModelNames[m_iSelectedModelIndex], true)))
				MSG_BOX("즐겨찾기 추가 실패");
		}
	}

	ImGui::SameLine();
	// 빨간색 버튼 스타일 적용
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); // 버튼 배경색
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // 호버 시
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f)); // 클릭 시
	if (ImGui::Button("Clear List"))
	{
		m_iSelectedModelIndex = -1;
		m_ModelNames.clear();
	}

	ImGui::PopStyleColor(3); // 적용한 색상 3개 되돌리기


	// 창 포커스 여부
	const _bool bWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootAndChildWindows);

	if (ImGui::BeginListBox("##Model List", ImVec2(-FLT_MIN, 200)))
	{
		/*
		* 모델 원형의 보관함에 들어있는 모든 <모델이름>을 출력
		*/

		for (_int i = 0; i < m_ModelNames.size(); ++i)
		{
			const _bool isSelected = (m_iSelectedModelIndex == i);
			if (ImGui::Selectable(m_ModelNames[i].c_str(), isSelected))
			{
				if (bWindowFocused)
				{
					m_iSelectedModelIndex = i;

					//미리보기 모델 변경
					//TEXT("Prototype_Component_Model_모델이름"),
					wstring ModelName = wstring(m_ModelNames[m_iSelectedModelIndex].begin(), m_ModelNames[m_iSelectedModelIndex].end());
					wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
					ModelPrototypeTag += ModelName;

					m_pPreviewObject->Change_Model(ModelPrototypeTag);
				}
			}

			// 선택된 항목에 포커스
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();

	}

	/*
		파일 탐색기 띄우고
		해당 모델.bin 의 경로를 가져와서
		모델 원형을 만든다.

		_matrix		PreTransformMatrix = XMMatrixIdentity();

		PreTransformMatrix = XMMatrixIdentity();
		PreTransformMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f);
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_모델이름"),
			CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/모델이름.bin", PreTransformMatrix))))
			return E_FAIL;

		모델 원형들을 보관한다.
	*/
	if (ImGui::Button("Import Model"))
	{
		IGFD::FileDialogConfig config;
		config.path = PATH_NONANIM;
		config.countSelectionMax = 0; //무제한 갯수 로드

		IFILEDIALOG->OpenDialog("Import Model", "Import .bin model", ".bin", config);
	}

	// 매 프레임마다 Display 호출
	if (IFILEDIALOG->Display("Import Model"))
	{
		if (IFILEDIALOG->IsOk())
		{
			//filesystem::path ModelPath = IFILEDIALOG->GetFilePathName();

			auto selections = IFILEDIALOG->GetSelection();
			// 처리
			// first: 파일명.확장자
			// second: 전체 경로 (파일명포함)
			if (!selections.empty())
			{
				for (auto FilePath : selections)
				{
					filesystem::path ModelPath = FilePath.second;

					if (!FilePath.second.empty())
					{
						// Prototype 이름 설정
						wstring PrototypeTag = L"Prototype_Component_Model_" + ModelPath.stem().wstring();
						string strPrototypeTag = ModelPath.stem().string(); // 확장자 없이 파일 이름만

						string ModelFilePath = ModelPath.string();
						const _char* pModelFilePath = ModelFilePath.c_str();

						if (FAILED(Load_Model(PrototypeTag, pModelFilePath)))
						{
							MSG_BOX("로드 실패");
						}
						else
						{
							// 이미 이름이 목록에 존재하면 중복 추가하지 않음
							if (find(m_ModelNames.begin(), m_ModelNames.end(), strPrototypeTag) == m_ModelNames.end())
							{
								m_ModelNames.push_back(strPrototypeTag); // 이름 추가
							}
						}
					}
				}
			}
		}
		IFILEDIALOG->Close();
	}

	ImGui::End();

#pragma endregion

}

void CMapTool::Render_Favorite()
{
	ImGui::Begin("Favorite", nullptr);

	// 창 포커스 여부
	const _bool bFavoriteWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootAndChildWindows);

	if (ImGui::Button("Spawn"))
	{
		if (m_iSelectedFavoriteModelIndex != -1)
		{
			if (FAILED(Spawn_MapToolObject(m_FavoriteModelNames[m_iSelectedFavoriteModelIndex])))
			{
				MSG_BOX("스폰 실패");
			}
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Delete Favorite"))
	{
		if (m_iSelectedFavoriteModelIndex != -1 && m_FavoriteModelNames.empty() == false)
		{
			const string& selectModel = m_FavoriteModelNames[m_iSelectedFavoriteModelIndex];

			auto iter = find(m_FavoriteModelNames.begin(), m_FavoriteModelNames.end(), selectModel);
			// 찾아서 제거
			if (iter != m_FavoriteModelNames.end())
			{
				m_FavoriteModelNames.erase(iter);
				m_iSelectedFavoriteModelIndex = -1;

				if (FAILED(Save_Favorite()))
				{
					MSG_BOX("즐겨 찾기 저장 실패");
				}
			}
		}
	}
	

	if (ImGui::BeginListBox("##Favorite List", ImVec2(-FLT_MIN, 200)))
	{
		for (_int i = 0; i < m_FavoriteModelNames.size(); ++i)
		{
			const _bool isSelected = (m_iSelectedFavoriteModelIndex == i);
			if (ImGui::Selectable(m_FavoriteModelNames[i].c_str(), isSelected))
			{
				if (bFavoriteWindowFocused) // 포커스 된 경우에만 모델 변경
				{

					m_iSelectedFavoriteModelIndex = i;

					//미리보기 모델 변경
					//TEXT("Prototype_Component_Model_모델이름"),
					wstring ModelName = wstring(m_FavoriteModelNames[m_iSelectedFavoriteModelIndex].begin(), m_FavoriteModelNames[m_iSelectedFavoriteModelIndex].end());
					wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
					ModelPrototypeTag += ModelName;

					m_pPreviewObject->Change_Model(ModelPrototypeTag);
				}
			}

			// 선택된 항목에 포커스
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndListBox();
	}

	ImGui::End();
}

void CMapTool::Render_Detail()
{
#pragma region 디테일
	ImGui::Begin("Detail", nullptr);

	ImGui::Separator();

	Detail_Name();

	ImGui::Separator();

	Detail_Transform();

	ImGui::Separator();

	Detail_Collider();

	ImGui::Separator();

	Detail_Tile();



	ImGui::End();
#pragma endregion
}

void CMapTool::Render_Preview()
{
	
	if (ImGui::Begin("Preview"))
	{
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
			m_bPreviewHovered = true;
		else
			m_bPreviewHovered = false;

		// 카메라 리셋 버튼
		if (ImGui::Button("Reset Camera"))
		{
			//CPreviewObject* pPreviewObject = static_cast<CPreviewObject*>(
			//	m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_PreviewObject")));

			if (m_pPreviewObject)
			{
				CTransform* pCamTransformCom = m_pPreviewObject->Get_CameraTransformCom();
				if (pCamTransformCom)
				{
					m_pPreviewObject->Reset_CameraWorldMatrix();
				}
			}
		}

		//프리뷰 오브젝트 회전 토글 버튼
		ImGui::SameLine();
		_bool* pIsRotate = m_pPreviewObject->Get_IsRotate_Ptr();
		const _char* label = *pIsRotate ? "ON" : "OFF";
		ImGui::Text("Rotate");
		ImGui::SameLine();
		if (ImGui::Button(label))
		{
			*pIsRotate = !*pIsRotate;
		}

		// 2. 렌더타겟을 텍스처처럼 ImGui에 표시
		ID3D11ShaderResourceView* pSRV = m_pGameInstance->Find_RenderTarget(TEXT("Target_Preview"))->Get_SRV();

		// 현재 창의 가용 영역 크기 (패딩 제외)
		ImVec2 availableSize = ImGui::GetContentRegionAvail();

		ImGui::Image(reinterpret_cast<ImTextureID>(pSRV), availableSize, ImVec2(0.f, 0.f), ImVec2(1.f, 1.f));

	}
	else
	{
		// 창이 닫힌 상태일 때
		m_bPreviewHovered = false;
	}

	ImGui::End();
}

HRESULT CMapTool::Spawn_MapToolObject()
{
	if (m_iSelectedModelIndex == -1)
		return E_FAIL;

	//현재 선택된 모델로 맵오브젝트를 생성

	CMapToolObject::MAPTOOLOBJ_DESC MapToolObjDesc = {};

	//TEXT("Prototype_Component_Model_모델이름"),
	wstring ModelName = wstring(m_ModelNames[m_iSelectedModelIndex].begin(), m_ModelNames[m_iSelectedModelIndex].end());
	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
	ModelPrototypeTag += ModelName;
	lstrcpy(MapToolObjDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());
	lstrcpy(MapToolObjDesc.szModelName, ModelName.c_str());
	/*
	스폰을 눌렀을 때 어떤 레이어에 담았는지 저장하는 레이어 리스트가 필요함
	삭제할 때 레이어가 존재하는 지 확인(레이어가 없으면 존재하지 않다는 것)
	이 레이어가 비었으면 리스트에서 제외 해야함
	*/
	wstring LayerTag = TEXT("Layer_MapToolObject_");
	LayerTag += ModelName;

	MapToolObjDesc.iID = m_iID++;

#pragma region 카메라 앞에다가 소환
	//카메라 앞에다가 소환
	//카메라 위치에서, 뷰행렬 Look 만큼 앞으로
	// 카메라 위치
	_float4 CamPos = *m_pGameInstance->Get_CamPosition();

	// 위치만 반영한 행렬 생성
	_matrix matWorld = XMMatrixTranslation(CamPos.x, CamPos.y, CamPos.z);

	// 카메라 월드 행렬 (뷰 행렬 역행렬)
	_matrix CamWorldMatrix = XMMatrixInverse(nullptr, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	_float4x4 CamWM = {};
	XMStoreFloat4x4(&CamWM, CamWorldMatrix);

	// 룩 벡터 추출 (3번째 행)
	_vector vLook = XMVectorSet(CamWM._31, CamWM._32, CamWM._33, 0.f);

	// 룩 벡터 정규화
	vLook = XMVector3Normalize(vLook);

	// 거리 설정
	_float fDist = PRE_TRANSFORMMATRIX_SCALE * 500.f;

	// 룩 벡터에 거리 곱하기
	_vector vOffset = XMVectorScale(vLook, fDist);

	// 카메라 위치 벡터
	_vector vCamPos = XMLoadFloat4(&CamPos);

	// 최종 위치 계산 (카메라 위치 + 룩 * 거리)
	_vector vSpawnPos = XMVectorAdd(vCamPos, vOffset);

	// 최종 월드 행렬 생성 (위치만)
	_matrix SpawnWorldMatrix = XMMatrixTranslationFromVector(vSpawnPos);

	// 오브젝트 월드 행렬에 적용
	XMStoreFloat4x4(&MapToolObjDesc.WorldMatrix, SpawnWorldMatrix);
#pragma endregion

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
		ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
		return E_FAIL;

	//방금 추가한서을 모델 그룹에 분류해서 저장
	Add_ModelGroup(WStringToString(ModelName), m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), LayerTag));

	//포커스 변경
	m_iFocusIndex = Find_HierarchyIndex_By_ID(MapToolObjDesc.iID);

	Safe_Release(m_pFocusObject);
	m_pFocusObject = Find_Object_By_Index(m_iFocusIndex);
	Safe_AddRef(m_pFocusObject);

	//선택된것들 모두 초기화 후 새로 추가한것 넣기
	m_SelectedIndexies.clear();

	for (CMapToolObject* pObj : m_SelectedObjects)
		Safe_Release(pObj);
	m_SelectedObjects.clear();

	m_SelectedIndexies.insert(m_iFocusIndex);
	m_SelectedObjects.insert(m_pFocusObject);
	Safe_AddRef(m_pFocusObject);

	return S_OK;
}

HRESULT CMapTool::Spawn_MapToolObject(string ModelName)
{
	//현재 선택된 모델로 맵오브젝트를 생성

	CMapToolObject::MAPTOOLOBJ_DESC MapToolObjDesc = {};

	//TEXT("Prototype_Component_Model_모델이름"),
	wstring wModelName = StringToWString(ModelName);
	wstring wModelPrototypeTag = TEXT("Prototype_Component_Model_");
	wModelPrototypeTag += wModelName;
	lstrcpy(MapToolObjDesc.szModelPrototypeTag, wModelPrototypeTag.c_str());
	lstrcpy(MapToolObjDesc.szModelName, wModelName.c_str());
	/*
	스폰을 눌렀을 때 어떤 레이어에 담았는지 저장하는 레이어 리스트가 필요함
	삭제할 때 레이어가 존재하는 지 확인(레이어가 없으면 존재하지 않다는 것)
	이 레이어가 비었으면 리스트에서 제외 해야함
	*/
	wstring LayerTag = TEXT("Layer_MapToolObject_");
	LayerTag += wModelName;

	MapToolObjDesc.iID = m_iID++;

#pragma region 카메라 앞에다가 소환
	//카메라 앞에다가 소환
	//카메라 위치에서, 뷰행렬 Look 만큼 앞으로
	// 카메라 위치
	_float4 CamPos = *m_pGameInstance->Get_CamPosition();

	// 위치만 반영한 행렬 생성
	_matrix matWorld = XMMatrixTranslation(CamPos.x, CamPos.y, CamPos.z);

	// 카메라 월드 행렬 (뷰 행렬 역행렬)
	_matrix CamWorldMatrix = XMMatrixInverse(nullptr, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	_float4x4 CamWM = {};
	XMStoreFloat4x4(&CamWM, CamWorldMatrix);

	// 룩 벡터 추출 (3번째 행)
	_vector vLook = XMVectorSet(CamWM._31, CamWM._32, CamWM._33, 0.f);

	// 룩 벡터 정규화
	vLook = XMVector3Normalize(vLook);

	// 거리 설정
	_float fDist = PRE_TRANSFORMMATRIX_SCALE * 500.f;

	// 룩 벡터에 거리 곱하기
	_vector vOffset = XMVectorScale(vLook, fDist);

	// 카메라 위치 벡터
	_vector vCamPos = XMLoadFloat4(&CamPos);

	// 최종 위치 계산 (카메라 위치 + 룩 * 거리)
	_vector vSpawnPos = XMVectorAdd(vCamPos, vOffset);

	// 최종 월드 행렬 생성 (위치만)
	_matrix SpawnWorldMatrix = XMMatrixTranslationFromVector(vSpawnPos);

	// 오브젝트 월드 행렬에 적용
	XMStoreFloat4x4(&MapToolObjDesc.WorldMatrix, SpawnWorldMatrix);
#pragma endregion

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
		ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
		return E_FAIL;

	//방금 추가한서을 모델 그룹에 분류해서 저장
	Add_ModelGroup(ModelName, m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), LayerTag));

	//포커스 변경
	m_iFocusIndex = Find_HierarchyIndex_By_ID(MapToolObjDesc.iID);

	Safe_Release(m_pFocusObject);
	m_pFocusObject = Find_Object_By_Index(m_iFocusIndex);
	Safe_AddRef(m_pFocusObject);

	//선택된것들 모두 초기화 후 새로 추가한것 넣기
	m_SelectedIndexies.clear();

	for (CMapToolObject* pObj : m_SelectedObjects)
		Safe_Release(pObj);
	m_SelectedObjects.clear();

	m_SelectedIndexies.insert(m_iFocusIndex);
	m_SelectedObjects.insert(m_pFocusObject);
	Safe_AddRef(m_pFocusObject);

	return S_OK;
}

HRESULT CMapTool::Duplicate_Selected_Object()
{
	//현재 선택된 오브젝트로 맵오브젝트를 생성
	/*CMapToolObject* pMapToolObject = static_cast<CMapToolObject*>(Get_Focused_Object());
	if (pMapToolObject == nullptr)
		return S_OK;*/

	if (m_SelectedObjects.empty())
		return E_FAIL;

	//임시 저장
	set<CMapToolObject*> pTempSelectObjects = m_SelectedObjects;

	//선택된것들 모두 초기화 후 복제한것만 넣기
	m_SelectedIndexies.clear();

	for (CMapToolObject* pObj : m_SelectedObjects)
		Safe_Release(pObj);
	m_SelectedObjects.clear();

	for (CMapToolObject* pObj : pTempSelectObjects)
	{
		CMapToolObject::MAPTOOLOBJ_DESC MapToolObjDesc = {};

		//TEXT("Prototype_Component_Model_모델이름"),
		string ModelName = pObj->Get_ModelName();

		lstrcpy(MapToolObjDesc.szModelPrototypeTag, pObj->Get_ModelPrototypeTag().c_str());
		lstrcpy(MapToolObjDesc.szModelName, StringToWString(ModelName).c_str());
		/*
		스폰을 눌렀을 때 어떤 레이어에 담았는지 저장하는 레이어 리스트가 필요함
		삭제할 때 레이어가 존재하는 지 확인(레이어가 없으면 존재하지 않다는 것)
		이 레이어가 비었으면 리스트에서 제외 해야함
		*/
		wstring LayerTag = TEXT("Layer_MapToolObject_");
		LayerTag += StringToWString(ModelName);

		MapToolObjDesc.iID = m_iID++;

#pragma region 해당 오브젝트 옆에다가 소환
		_matrix SpawnWorldMatrix = pObj->Get_TransfomCom()->Get_WorldMatrix();

		//// x축으로 3.f 만큼 이동하는 변환 행렬
		//_matrix matOffset = XMMatrixTranslation(3.f, 0.f, 0.f);

		//// 변환 적용: 기존 행렬에 offset을 곱해준다
		//_matrix matResult = matOffset * SpawnWorldMatrix;

		// 결과 저장
		XMStoreFloat4x4(&MapToolObjDesc.WorldMatrix, SpawnWorldMatrix);
#pragma endregion

		if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
			ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
			return E_FAIL;

		//방금 추가한 것을 모델 그룹에 분류해서 저장
		CGameObject* pLastObject = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), LayerTag);

		Add_ModelGroup(ModelName, pLastObject);
		m_iFocusIndex = Find_HierarchyIndex_By_ID(m_iID);

		//포커스 되는 오브젝트 변경
		Safe_Release(m_pFocusObject);
		m_pFocusObject = static_cast<CMapToolObject*>(pLastObject);
		Safe_AddRef(m_pFocusObject);

		//복제된 것 넣기
		m_SelectedIndexies.insert(m_iFocusIndex);
		m_SelectedObjects.insert(m_pFocusObject);
		Safe_AddRef(m_pFocusObject);
	}

	return S_OK;
}

HRESULT CMapTool::Undo_Selected_Object()
{
	for (CMapToolObject* pObj : m_SelectedObjects)
	{
		pObj->Undo_WorldMatrix();
	}

	return S_OK;
}

void CMapTool::DeleteMapToolObject()
{
	for (CGameObject* pObj : m_SelectedObjects)
	{
		//m_SelectedObjects 릴리즈
		Safe_Release(pObj);
		//그룹에서 삭제
		Delete_ModelGroup(pObj);
		//실제로 삭제
		pObj->Set_bDead();
	}

	m_SelectedIndexies.clear();
	m_SelectedObjects.clear();

	m_iFocusIndex = -1;

	//포커스 릴리즈
	Safe_Release(m_pFocusObject);
	m_pFocusObject = nullptr;
}

HRESULT CMapTool::Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath)
{
	//이미 프로토타입이존재하는 지확인
	
	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::YW), strPrototypeTag) != nullptr)
	{
		//MSG_BOX("이미 프로토타입이 존재함");
		return S_OK;
	}

	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE);

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), strPrototypeTag,
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
		return E_FAIL;

	return S_OK;
}

void CMapTool::Add_ModelGroup(string ModelName, CGameObject* pMapToolObject)
{
	Safe_AddRef(pMapToolObject);
	// 모델 이름을 키로 하여 그룹에 GameObject를 추가
	m_ModelGroups[ModelName].push_back(pMapToolObject);
}

void CMapTool::Delete_ModelGroup(CGameObject* pMapToolObject)
{
	// map<string, vector<CGameObject*>> m_ModelGroups; 에서
	// 그룹중 이름과, 아이디가 같은놈을 제거

	string ModelName = static_cast<CMapToolObject*>(pMapToolObject)->Get_ModelName();

	auto iterGroup = m_ModelGroups.find(ModelName);

	// 해당 그룹이 존재하지 않으면 리턴
	if (iterGroup == m_ModelGroups.end())
	{
		MSG_BOX("심각한 에러) 지우려는 해당 그룹이 존재하지 않음");
		return;
	}

	std::list<CGameObject*>& objList = iterGroup->second;

	Safe_Release(pMapToolObject);
	// 리스트에서 해당 오브젝트 제거
	objList.remove(pMapToolObject);

	// 만약 해당 그룹이 비었다면 map에서 그룹 자체도 제거
	if (objList.empty())
	{
		m_ModelGroups.erase(iterGroup);
	}

}

CGameObject* CMapTool::Get_Focused_Object()
{
	_uint index = m_iFocusIndex;

	for (auto& group : m_ModelGroups)
	{
		for (auto pGameObject : group.second)
		{
			if (index == 0)
				return pGameObject;

			--index;
		}
	}

	return nullptr; // 인덱스 초과 시 null
}

_int CMapTool::Find_HierarchyIndex_By_ID(_uint iID)
{
	//같은 아이디를 가진 오브젝트에 포커스
	//모델 그룹중에서 같은 아이디를 가진 오브젝트를 만날때까지순회

	_bool bFind = false;
	_uint i = 0;
	for (auto& Group : m_ModelGroups)
	{
		if (bFind)
			break;

		const string& ModelName = Group.first;

		for (auto pGameObject : Group.second)
		{
			if (static_cast<CMapToolObject*>(pGameObject)->Get_ID() == iID)
			{
				bFind = true;
				break;
			}

			++i;
		}
	}

	return i;
}

CMapToolObject* CMapTool::Find_Object_By_Index(_int iIndex)
{
	for (auto& group : m_ModelGroups)
	{
		for (auto pGameObject : group.second)
		{
			if (iIndex == 0)
				return static_cast<CMapToolObject*>(pGameObject);

			--iIndex;
		}
	}

	return nullptr; // 인덱스 초과 시 null
}

void CMapTool::SelectByClick()
{
	_int iID = -1;
	if (m_pGameInstance->PickByClick(&iID))
	{
		printf("ID: %d\n", iID);
		m_iFocusIndex = Find_HierarchyIndex_By_ID(iID);

		Safe_Release(m_pFocusObject);
		m_pFocusObject = static_cast<CMapToolObject*>(Get_Focused_Object());
		Safe_AddRef(m_pFocusObject);

		//다중선택 안함
		if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) == false)
		{
			//기존 선택된 것들 클리어
			m_SelectedIndexies.clear();
			for (CMapToolObject* pObj : m_SelectedObjects)
				Safe_Release(pObj);
			m_SelectedObjects.clear();
		}

		//새로 추가
		m_SelectedIndexies.insert(Find_HierarchyIndex_By_ID(m_pFocusObject->Get_ID()));
		m_SelectedObjects.insert(m_pFocusObject);
		Safe_AddRef(m_pFocusObject);
	}
}

void CMapTool::SelectByDrag(const _float2& vMouseDragEnd)
{
	set<_int> IDs = {};

	if (m_pGameInstance->PickInRect(m_vMouseDragStart, vMouseDragEnd, &IDs))
	{
		//기존 선택된 것들 클리어
		m_SelectedIndexies.clear();
		for (CMapToolObject* pObj : m_SelectedObjects)
			Safe_Release(pObj);
		m_SelectedObjects.clear();

		_bool bSetFocusObject = { false };

		for (_int iID : IDs)
		{

			//새로 추가
			_int iIndex = Find_HierarchyIndex_By_ID(iID);

			m_SelectedIndexies.insert(iIndex);

			CMapToolObject* pObj = Find_Object_By_Index(iIndex);
			m_SelectedObjects.insert(pObj);
			Safe_AddRef(pObj);

			if (bSetFocusObject == false)
			{
				bSetFocusObject = true;
				m_iFocusIndex = iIndex;

				Safe_Release(m_pFocusObject);
				m_pFocusObject = pObj;
				Safe_AddRef(m_pFocusObject);
			}
		}

	}
}

void CMapTool::Control_PreviewObject(_float fTimeDelta)
{

	if (m_bPreviewHovered && m_pGameInstance->Mouse_Pressing(DIM::RBUTTON))
	{
		m_pCamera_Free->Set_Moveable(false);

		CTransform* pCamTransformCom = m_pPreviewObject->Get_CameraTransformCom();

		_float3 vPos = {};
		XMStoreFloat3(&vPos, pCamTransformCom->Get_State(STATE::POSITION));

		//printf("x: %0.1f, y: %0.1f, z: %0.1f\n", vPos.x, vPos.y, vPos.z);

		if (m_pGameInstance->Key_Pressing(DIK_A))
		{
			pCamTransformCom->Go_Left(fTimeDelta * 10.f);
		}
		if (m_pGameInstance->Key_Pressing(DIK_D))
		{
			pCamTransformCom->Go_Right(fTimeDelta * 10.f);
		}
		if (m_pGameInstance->Key_Pressing(DIK_W))
		{
			pCamTransformCom->Go_Front(fTimeDelta * 10.f);
		}
		if (m_pGameInstance->Key_Pressing(DIK_S))
		{
			pCamTransformCom->Go_Backward(fTimeDelta * 10.f);
		}

		_long			MouseMove = {};

		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMM::X))
		{
			pCamTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), MouseMove * fTimeDelta * 1.f);
		}

		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMM::Y))
		{
			pCamTransformCom->Turn(pCamTransformCom->Get_State(STATE::RIGHT), MouseMove * fTimeDelta * 1.f);
		}
	}
	else
	{
		m_pCamera_Free->Set_Moveable(true);
	}

}

void CMapTool::Detail_Name()
{
	if (m_pFocusObject == nullptr)
		return;

	ImGui::Text(m_pFocusObject->Get_ModelName().c_str());
}

void CMapTool::Detail_Transform()
{
	ImGui::Text("Transform");

	if (m_pFocusObject != nullptr)
	{
		CTransform* pTransform = m_pFocusObject->Get_TransfomCom();

		//행렬 복사 버튼
		ImGui::SameLine();
		if (ImGui::Button("Copy"))
		{
			XMStoreFloat4x4(&m_CopyWorldMatrix, m_pFocusObject->Get_TransfomCom()->Get_WorldMatrix());
		}

		//행렬 붙이기 버튼
		ImGui::SameLine();
		if (ImGui::Button("Paste"))
		{
			m_pFocusObject->Get_TransfomCom()->Set_WorldMatrix(XMLoadFloat4x4(&m_CopyWorldMatrix));
		}

		// 리셋 버튼 오른쪽 정렬
		_float fButtonWidth = ImGui::CalcTextSize("Reset").x + ImGui::GetStyle().FramePadding.x * 2;
		_float fRegionWidth = ImGui::GetContentRegionAvail().x;
		ImGui::SameLine(ImGui::GetCursorPosX() + fRegionWidth - fButtonWidth);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));

		if (ImGui::Button("Reset"))
		{
			_float4x4 MatrixIdentity = {};
			XMStoreFloat4x4(&MatrixIdentity, XMMatrixIdentity());
			pTransform->Set_WorldMatrix(MatrixIdentity);
		}

		ImGui::PopStyleColor(3);

#pragma region 기즈모 및 행렬 분해
		_float4x4 worldMat;
		XMStoreFloat4x4(&worldMat, pTransform->Get_WorldMatrix());

		_float matrix[16];
		memcpy(matrix, &worldMat, sizeof(float) * 16);

		// 분해
		_float position[3], rotation[3], scale[3];
		ImGuizmo::DecomposeMatrixToComponents(matrix, position, rotation, scale);

		// ImGuizmo 설정
		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

		ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		ImGuizmo::SetRect(0, 0, displaySize.x, displaySize.y);

		_float viewMat[16], projMat[16];
		XMStoreFloat4x4(reinterpret_cast<_float4x4*>(viewMat), m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
		XMStoreFloat4x4(reinterpret_cast<_float4x4*>(projMat), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));

		// Gizmo 조작
		ImGuizmo::Manipulate(viewMat, projMat, m_currentOperation, ImGuizmo::LOCAL, matrix);
#pragma endregion

#pragma region 포지션
		_bool bPositionChanged = ImGui::InputFloat3("##Position", position, "%.2f");
		ImGui::SameLine();
		if (ImGui::RadioButton("Position", m_currentOperation == ImGuizmo::TRANSLATE))
			m_currentOperation = ImGuizmo::TRANSLATE;
#pragma endregion

#pragma region 회전
		_bool bRotationChanged = ImGui::InputFloat3("##Rotation", rotation, "%.2f");
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotation", m_currentOperation == ImGuizmo::ROTATE))
			m_currentOperation = ImGuizmo::ROTATE;
#pragma endregion

#pragma region 스케일
		_bool bScaleChanged = ImGui::InputFloat3("##Scale", scale, "%.2f");
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", m_currentOperation == ImGuizmo::SCALE))
			m_currentOperation = ImGuizmo::SCALE;
#pragma endregion

#pragma region 적용

		//전에 안눌렸고 지금 눌렸으면 저장
		if (m_bWasUsingGizmoLastFrame == false && ImGuizmo::IsUsing())
		{
			for (CMapToolObject* pObj : m_SelectedObjects)
			{
				pObj->Set_UndoWorldMatrix(pObj->Get_TransfomCom()->Get_WorldMatrix());
			}
			m_bWasUsingGizmoLastFrame = true;
		}
		m_bWasUsingGizmoLastFrame = ImGuizmo::IsUsing();

		if (ImGuizmo::IsUsing())
		{
			// ImGuizmo로 조작된 matrix 그대로 적용
			memcpy(&worldMat, matrix, sizeof(_float) * 16);

			// 대표 오브젝트의 이전 행렬과 새 행렬 비교
			_matrix matPrevMain = m_pFocusObject->Get_TransfomCom()->Get_WorldMatrix();
			_matrix matNewMain = XMLoadFloat4x4(&worldMat);

			// 대표 오브젝트의 이동/회전/스케일 변화 행렬 계산
			_matrix matOffset = matNewMain * XMMatrixInverse(nullptr, matPrevMain);

			pTransform->Set_WorldMatrix(worldMat);

			//선택된 모든오브젝트가 같이 움직이게
			for (CGameObject* pObj : m_SelectedObjects)
			{
				if (pObj == m_pFocusObject)
					continue;

				_matrix matOld = pObj->Get_TransfomCom()->Get_WorldMatrix();
				_matrix matNew = matOffset * matOld;

				pObj->Get_TransfomCom()->Set_WorldMatrix(matNew);
			}

		}
		else if (bPositionChanged || bRotationChanged || bScaleChanged)
		{
			m_pFocusObject->Set_UndoWorldMatrix(pTransform->Get_WorldMatrix());

			// 수동 입력으로 바뀐 값 → matrix 재구성 후 적용
			ImGuizmo::RecomposeMatrixFromComponents(position, rotation, scale, matrix);
			memcpy(&worldMat, matrix, sizeof(_float) * 16);
			pTransform->Set_WorldMatrix(worldMat);
		}

#pragma endregion
	}
}

void CMapTool::Detail_Tile()
{
	ImGui::Text("Tile");
	if (m_pFocusObject)
	{
		// 타일링 여부 체크박스
		ImGui::Checkbox("Enable Tiling", &m_pFocusObject->m_bUseTiling);

		// 타일링 값 슬라이더 (X, Z)
		if (m_pFocusObject->m_bUseTiling)
		{
			ImGui::DragFloat2("Tiling (X,Z)", m_pFocusObject->m_TileDensity, 0.01f, 0.01f, 32.0f, "%.2f");
		}
	}
}

void CMapTool::Detail_Collider()
{
	ImGui::Text("Collider");
	ImGui::SameLine();

	if (m_bRenderAllCollider)
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));  // 초록색
	else
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));  // 붉은색

	if (ImGui::Button(m_bRenderAllCollider ? "Show All" : "Focus Only"))
	{
		m_bRenderAllCollider = !m_bRenderAllCollider;
	}
	ImGui::PopStyleColor();

	if (m_pFocusObject)
	{
		// 콜라이더 타입 선택 콤보박스
		const _char* ColliderTypes[] = { "None", "Convex", "Triangle" };
		_int CurrentCollider = static_cast<_int>(m_pFocusObject->m_eColliderType);

		if (ImGui::Combo("Collider Type", &CurrentCollider, ColliderTypes, IM_ARRAYSIZE(ColliderTypes)))
		{
			m_pFocusObject->Set_Collider(static_cast<COLLIDER_TYPE>(CurrentCollider));
		}

	}
}

HRESULT CMapTool::Add_Favorite(const string& ModelName, _bool bSave)
{
	// 이미 이름이 목록에 존재하면 중복 추가하지 않음
	if (find(m_FavoriteModelNames.begin(), m_FavoriteModelNames.end(), ModelName) == m_FavoriteModelNames.end())
	{
		m_FavoriteModelNames.push_back(ModelName); // 이름 추가

		if (bSave)
		{
			if (FAILED(Save_Favorite()))
			{
				return E_FAIL;
				MSG_BOX("즐겨 찾기 저장 실패");
			}
		}
	}

	return S_OK;
}


CMapTool* CMapTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CMapTool* pInstance = new CMapTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CMapTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapTool::Clone(void* pArg)
{
	CMapTool* pInstance = new CMapTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMapTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CMapTool::Free()
{
	__super::Free();	

	Safe_Release(m_pPreviewObject);
	Safe_Release(m_pCamera_Free);


	for (auto Group : m_ModelGroups)
	{
		for (auto pObj : Group.second )
			Safe_Release(pObj);
		Group.second.clear();
	}
	m_ModelGroups.clear();

	Safe_Release(m_pFocusObject);

	for (CMapToolObject* pObj : m_SelectedObjects)
		Safe_Release(pObj);
	m_SelectedObjects.clear();
}
