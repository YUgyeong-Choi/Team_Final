#include "MapTool.h"
#include "GameInstance.h"

#include <filesystem>
#include "MapToolObject.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감

CMapTool::CMapTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CMapTool::CMapTool(const CMapTool& Prototype)
	: CGameObject(Prototype)
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

	//MapData에 배치되어있는 모델들을 미리 불러들인다.
	if (FAILED(Ready_Model()))
		return E_FAIL;

	//MapData를 따라 맵을 로드한다.
	if (FAILED(Load_Map()))
		return E_FAIL;

	return S_OK;
}

void CMapTool::Priority_Update(_float fTimeDelta)
{

}

void CMapTool::Update(_float fTimeDelta)
{
}

void CMapTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CMapTool::Render()
{
	if (FAILED(Render_MapTool()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapTool::Ready_Model()
{
	ifstream inFile("../Bin/Save/MapTool/ReadyModel.json");
	if (!inFile.is_open())
	{
		MSG_BOX("ReadyModel.json 파일을 열 수 없습니다.");
		return S_OK;
	}

	json ReadyModelJson;
	try
	{
		inFile >> ReadyModelJson;
		inFile.close();
	}
	catch (const exception& e)
	{
		inFile.close();
		MessageBoxA(nullptr, e.what(), "JSON 파싱 실패", MB_OK);
		return E_FAIL;
	}

	// JSON 데이터 확인
	for (const auto& element : ReadyModelJson)
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
	}

	return S_OK;
}

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
		filesystem::path path = filesystem::path(PATH_NONANIM) / (strModelName + ".bin");
		string FullPath = path.generic_string();

		json ObjectJson;
		ObjectJson["ModelName"] = strModelName;
		ObjectJson["Path"] = FullPath; // 또는 Path

		ReadyModelJsonArray.push_back(ObjectJson);

		// 문자열 길이 계산
		_uint iLength = static_cast<_uint>(ModelName.length());

		// 오브젝트 갯수 저장
		_uint iObjectCount = static_cast<_uint>(MapObjectList.size());

		// 모델 JSON
		json ModelJson;
		ModelJson["ModelName"] = strModelName;
		ModelJson["ObjectCount"] = static_cast<_uint>(MapObjectList.size());
		ModelJson["Objects"] = json::array();

		for (CGameObject* pGameObject : MapObjectList)
		{
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

			ModelJson["Objects"].push_back({ {"WorldMatrix", MatrixJson} });
		}

		MapDataJson["Models"].push_back(ModelJson);
	}

	// 파일에 JSON 쓰기
	ReadyModelFile << ReadyModelJsonArray.dump(4);
	MapDataFile << MapDataJson.dump(4);

	MapDataFile.close();
	ReadyModelFile.close();

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
		const json& objects = Models[i]["Objects"];

		for (_uint j = 0; j < iObjectCount; ++j)
		{
			const json& WorldMatrixJson = objects[j]["WorldMatrix"];
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

			lstrcpy(MapToolObjDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());
			MapToolObjDesc.WorldMatrix = WorldMatrix;

			if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
				ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CMapTool::Render_MapTool()
{
	Hierarchy();
	Asset();

	return S_OK;
}

void CMapTool::Hierarchy()
{
#pragma region 하이어라키
	ImGui::Begin("Hierarchy", nullptr);

	//if (ImGui::BeginListBox("##HierarchyList", ImVec2(-FLT_MIN, 300)))
	//{
	//	_uint i = 0; // 전체 Hierarchy 항목 인덱스를 위한 카운터 (전역 인덱스)
	//	for (auto& group : m_ModelGroups) // 모델 이름별로 그룹화된 GameObject 목록을 반복
	//	{
	//		const string& ModelName = group.first; // 현재 그룹의 모델 이름
	//		// 트리 노드 생성 (열려있게 기본 설정) - 모델 이름을 기준으로 그룹화된 항목
	//		_bool bOpen = ImGui::TreeNodeEx(ModelName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

	//		//_uint j = 0; // 각 모델 그룹 내에서의 개별 오브젝트 인덱스
	//		for (auto pGameObject : group.second)
	//		{
	//			// 개별 오브젝트에 대한 이름 생성 (ID 값 주자 나중에)
	//			string strHierarchyName = ModelName + '_';// + to_string(ID);

	//			// 현재 인덱스가 선택된 상태인지 확인
	//			_bool isSelected = (m_iSelectedHierarchyIndex == i);

	//			if (bOpen) // 트리 노드가 열려 있을 때만 Selectable 항목을 그린다
	//			{
	//				// 해당 항목이 클릭되면 인덱스를 기록하여 선택 상태로 만든다
	//				if (ImGui::Selectable(strHierarchyName.c_str(), isSelected))
	//				{
	//					m_iSelectedHierarchyIndex = i; // 현재 선택된 항목으로 갱신
	//				}

	//				if (isSelected)
	//					ImGui::SetItemDefaultFocus(); // 포커스를 해당 항목에 맞춰준다 (키보드 네비게이션용)
	//			}

	//			++i; // 전체 인덱스 증가 (트리 노드 열려있든 말든 증가시켜야 함)
	//		}

	//		if (bOpen)
	//			ImGui::TreePop(); // 트리 노드 닫기 (트리 UI를 닫아줌)
	//	}

	//	ImGui::EndListBox(); // 리스트박스 끝

	//}


	if (ImGui::Button("Save Map"))
	{
		if (FAILED(Save_Map()))
			MSG_BOX("맵 저장 실패");
	}


	if (ImGui::Button("Load Map"))
	{

	}
	ImGui::End();
#pragma endregion
}

void CMapTool::Asset()
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
				m_iSelectedModelIndex = i;
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
	if (ImGui::Button("Load Model"))
	{
		IGFD::FileDialogConfig config;
		config.path = PATH_NONANIM;
		config.countSelectionMax = 1;

		IFILEDIALOG->OpenDialog("Load Model", "Load .bin model", ".bin", config);
	}

	// 매 프레임마다 Display 호출
	if (IFILEDIALOG->Display("Load Model"))
	{
		if (IFILEDIALOG->IsOk())
		{
			filesystem::path ModelPath = IFILEDIALOG->GetFilePathName();

			if (!ModelPath.empty())
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
					m_ModelNames.push_back(strPrototypeTag);
				}
			}
		}


		IFILEDIALOG->Close();
	}

	ImGui::End();

#pragma endregion
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

	/*
	스폰을 눌렀을 때 어떤 레이어에 담았는지 저장하는 레이어 리스트가 필요함
	삭제할 때 레이어가 존재하는 지 확인(레이어가 없으면 존재하지 않다는 것)
	이 레이어가 비었으면 리스트에서 제외 해야함
	*/
	wstring LayerTag = TEXT("Layer_MapToolObject_");
	LayerTag += ModelName;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
		ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapTool::Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath)
{
	//이미 프로토타입이존재하는 지확인
	
	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::YW), strPrototypeTag) != nullptr)
	{
		MSG_BOX("이미 프로토타입이 존재함");
		return S_OK;
	}

	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f);

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), strPrototypeTag,
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
		return E_FAIL;

	return S_OK;
}

void CMapTool::UpdateHierarchy()
{
	//// 기존 데이터 초기화
	//m_ModelGroups.clear();      // 모델 이름별로 그룹화된 GameObject 리스트 초기화
	//m_HierarchyNames.clear();   // Hierarchy UI에서 사용할 이름 리스트 초기화

	//// 현재 레벨(YW)의 모든 레이어들에 대해 반복
	//for (auto& pLayer : m_pGameInstance->Get_Layers(ENUM_CLASS(LEVEL::YW)))
	//{
	//	// 레이어 이름에 "MapObject"가 포함되지 않으면 무시 (맵 에디터용 객체만 필터링)
	//	if (pLayer.first.find(L"MapToolObject") == wstring::npos)
	//		continue;

	//	// 해당 레이어의 모든 GameObject에 대해 반복
	//	for (auto pGameObject : pLayer.second->Get_GameObjects())
	//	{
	//		// 모델 컴포넌트 이름에서 "Prototype_Component_Model_" 접두사를 기준으로 모델명 추출
	//		wstring prefix = L"Prototype_Component_Model_";
	//		wstring wstrModelCom = static_cast<CMapToolObject*>(pGameObject)->Get_ModelCom(); // 모델 이름 얻기

	//		// 접두사가 없는 경우는 무시
	//		size_t PosPrefix = wstrModelCom.find(prefix);
	//		if (PosPrefix == wstring::npos)
	//			continue;

	//		// 접두사 이후의 실제 모델 이름 부분을 잘라내어 string으로 변환
	//		string ModelName = WStringToString(wstrModelCom.substr(PosPrefix + prefix.length()));

	//		// 모델 이름을 키로 하여 그룹에 GameObject를 추가
	//		m_ModelGroups[ModelName].push_back(pGameObject);
	//	}
	//}

	//// 모델 그룹들로부터 UI에 사용할 Hierarchy 이름들을 생성
	//_uint i = 0; // 전체 인덱스
	//for (auto& group : m_ModelGroups)
	//{
	//	_uint j = 0; // 각 모델 그룹 내 인덱스
	//	for (auto pGameObject : group.second)
	//	{
	//		// "모델이름_숫자" 형태의 이름 생성하여 Hierarchy UI 항목으로 사용
	//		string strHierarchyName = group.first + "_" + to_string(j++);

	//		// 이름 리스트에 추가
	//		m_HierarchyNames.push_back(strHierarchyName);

	//		++i; // 전체 인덱스 증가
	//	}
	//}

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

}
