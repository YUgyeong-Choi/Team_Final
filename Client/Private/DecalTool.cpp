#include "GameInstance.h"

#include "DecalTool.h"
#include "DecalToolObject.h"

#include "Camera_Manager.h"
#include "ImGuiFileDialog.h"


CDecalTool::CDecalTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CYWTool(pDevice, pContext)
{
}

CDecalTool::CDecalTool(const CDecalTool& Prototype)
	:CYWTool(Prototype)
{
}

HRESULT CDecalTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDecalTool::Initialize(void* pArg)
{
	/*m_pFocusObject = static_cast<CDecalToolObject*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_TestDecal")));
	Safe_AddRef(m_pFocusObject);*/


	return S_OK;
}

void CDecalTool::Priority_Update(_float fTimeDelta)
{
}

void CDecalTool::Update(_float fTimeDelta)
{
	Control(fTimeDelta);
}

void CDecalTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CDecalTool::Render()
{
	return S_OK;
}

HRESULT	CDecalTool::Render_ImGui()
{
	ImGui::Begin("Decal Tool", nullptr);

	if(ImGui::Button("Spawn Decal"))
	{
		Spawn_DecalObject();
	}

	ImGui::End();

	Render_Detail();

	return S_OK;
}

HRESULT CDecalTool::Save(const _char* Map)
{
	filesystem::create_directories("../Bin/Save/DecalTool");
	string DecalFilePath = string("../Bin/Save/DecalTool/Decal_") + Map + ".json";
	ofstream DecalDataFile(DecalFilePath);

	//맵을 만들어서, [프로토타입 태그, 파일 경로] 형태로 저장
	map<string, string> ResourceMap;//[프로토타입 태그, 파일 경로]

	if (!DecalDataFile.is_open())
		return E_FAIL;

	// 전체 데칼 데이터를 담을 JSON
	json DecalList = json::array();

	for (CGameObject* pObj : m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal")))
	{
		// 월드 행렬 얻기
		_matrix matWorld = pObj->Get_TransfomCom()->Get_WorldMatrix();
		_float4x4 matWorldFloat4x4;
		XMStoreFloat4x4(&matWorldFloat4x4, matWorld);

		// 4x4 행렬을 배열로 저장
		json MatrixJson = json::array();
		for (_int i = 0; i < 4; ++i)
		{
			json Row = json::array();
			for (_int j = 0; j < 4; ++j)
			{
				Row.push_back(matWorldFloat4x4.m[i][j]);
			}
			MatrixJson.push_back(Row);
		}

		CDecalToolObject* pDecalToolObj = static_cast<CDecalToolObject*>(pObj);

		//ARMT, N, BC 중 하나라도 디폴트 텍스쳐 껴있으면 저장하지 말자.
		if (pDecalToolObj->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] == TEXT("Prototype_Component_Texture_DefaultDecal") ||
			pDecalToolObj->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] == TEXT("Prototype_Component_Texture_DefaultDecal") ||
			pDecalToolObj->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] == TEXT("Prototype_Component_Texture_DefaultDecal"))
		{
			continue;
		}

		// 데칼 개체 정보 추가
		json DecalJson;
		DecalJson["WorldMatrix"] = MatrixJson;
		DecalJson["ARMT"] = WStringToString(pDecalToolObj->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)]);
		DecalJson["N"] = WStringToString(pDecalToolObj->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)]);
		DecalJson["BC"] = WStringToString(pDecalToolObj->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)]);

		ResourceMap[WStringToString(pDecalToolObj->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)])] =
			WStringToString(pDecalToolObj->m_FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)]);
		ResourceMap[WStringToString(pDecalToolObj->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)])] =
			WStringToString(pDecalToolObj->m_FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)]);
		ResourceMap[WStringToString(pDecalToolObj->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)])] =
			WStringToString(pDecalToolObj->m_FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)]);

		// 리스트에 추가
		DecalList.push_back(DecalJson);
	}

	// JSON 파일에 기록
	DecalDataFile << DecalList.dump(4); // 들여쓰기 4칸으로 보기 좋게 출력
	DecalDataFile.close();

	// 리소스 파일에 기록
	// JSON 객체 생성
	json ResourceJson;

	// map을 JSON에 넣기
	for (const auto& [prototypeTag, filePath] : ResourceMap)
	{
		ResourceJson[prototypeTag] = filePath;
	}

	// 파일로 저장
	string ResourceFilePath =  string("../Bin/Save/DecalTool/Resource_") + Map + ".json";
	ofstream ResourceDataFile(ResourceFilePath);
	if (ResourceDataFile.is_open())
	{
		ResourceDataFile << ResourceJson.dump(4); // 4는 들여쓰기
	}

	MSG_BOX("데칼 저장 성공!");

	return S_OK;
}

HRESULT CDecalTool::Load(const _char* Map)
{
	//현재 맵에 배치된 오브젝트를 모두 삭제하자
	Clear_All_Decal();

	Ready_Texture(Map);

	//현재 맵에 필요한 데칼 텍스쳐를 로드한다.

	string DecalDataPath = string("../Bin/Save/DecalTool/Decal_") + Map + ".json";
	//string ResourcePath = string("../Bin/Save/MapTool/Resource_") + Map + ".json"; //나중에 쓸듯 맵 바꿀때

	ifstream inFile(DecalDataPath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Decal_" + StringToWString(Map) + L".json 파일을 열 수 없습니다: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);
		return S_OK;
	}


	//파일 패스도 저장해주자
	string ResuorcePath = string("../Bin/Save/DecalTool/Resource_") + Map + ".json";
	json ResourceJson;
	{
		ifstream ResFile(ResuorcePath);
		if (ResFile.is_open())
			ResFile >> ResourceJson;
	}

	// JSON 파싱
	json JSON;
	inFile >> JSON;

	// 데이터 만큼 Decal 소환
	for (auto& item : JSON)
	{
		// 4x4 행렬 읽기
		_float4x4 WorldMatrix;
		for (_int iRow = 0; iRow < 4; ++iRow)
		{
			for (_int iCol = 0; iCol < 4; ++iCol)
			{
				WorldMatrix.m[iRow][iCol] = item["WorldMatrix"][iRow][iCol].get<_float>();
			}
		}

		//텍스쳐 프로토타입 이름 전달
		CDecalToolObject::DECALTOOLOBJECT_DESC Desc = {};
		Desc.WorldMatrix = WorldMatrix;
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] = StringToWString(item["ARMT"].get<string>());
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] = StringToWString(item["N"].get<string>());
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] = StringToWString(item["BC"].get<string>());

		//텍스쳐 파일 경로 전달
		auto it = ResourceJson.find(item["ARMT"].get<string>());
		Desc.FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] = StringToWString(it.value());

		it = ResourceJson.find(item["N"].get<string>());
		Desc.FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] = StringToWString(it.value());

		it = ResourceJson.find(item["BC"].get<string>());
		Desc.FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] = StringToWString(it.value());

		// Decal 객체 생성
		if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_DecalToolObject"),
			ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal"), &Desc)))
			return E_FAIL;

	}

	return S_OK;
}

void CDecalTool::Control(_float fTimeDelta)
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

		//포커스 비우기
		if (m_pGameInstance->Mouse_Up(DIM::WHEELBUTTON))
		{
			Safe_Release(m_pFocusObject);
			m_pFocusObject = nullptr;
		}
	}

	//클릭하면 가장 가까운 데칼을 포커스 한다.
	if (m_pGameInstance->Mouse_Up(DIM::LBUTTON) && ImGuizmo::IsOver() == false)
	{
		Select_Decal();
	}

	//F 키누르면 해당 오브젝트 위치로 이동
	if (m_pGameInstance->Key_Down(DIK_F))
	{
		Focus();
	}

	//딜리트키 누르면 현재 선택된거 삭제
	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		Delete_FocusObject();
	}

	//알트 클릭하면 해당 위치로 데칼 이동
	if (m_pGameInstance->Key_Pressing(DIK_LALT) && m_pGameInstance->Mouse_Up(DIM::LBUTTON))
	{
		SnapTo();
	}

	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_D))
	{
		Duplicate();
	}
}

void CDecalTool::Select_Decal()
{
	//알트키 누르고 있으면 피킹하지 않음(오브젝트 붙이고나서 오브젝트 변경되는거 막기 위함임)
	if (m_pGameInstance->Key_Pressing(DIK_LALT))
		return;

	// ImGui가 마우스 입력을 가져가면 피킹을 하지 않음
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	_float4 vWorldPos = {};
	if (m_pGameInstance->Picking(&vWorldPos))
	{
		Safe_Release(m_pFocusObject);
		m_pFocusObject = Get_ClosestDecalObject(XMLoadFloat4(&vWorldPos));
		Safe_AddRef(m_pFocusObject);
	}
}

void CDecalTool::Focus()
{
	if (nullptr == m_pFocusObject)
		return;

	_vector vObjectPos = m_pFocusObject->Get_TransfomCom()->Get_State(STATE::POSITION);
	_vector vCameraPos = XMVectorAdd(vObjectPos, XMVectorSet(0.f, 3.f, -3.f, 0.f));

	CTransform* pCameraTransformCom = CCamera_Manager::Get_Instance()->GetFreeCam()->Get_TransfomCom();

	//여유를 두고 이동한후
	pCameraTransformCom->Set_State(STATE::POSITION, vCameraPos);

	//LookAt 하자
	pCameraTransformCom->LookAt(vObjectPos);
}

void CDecalTool::SnapTo()
{
	if (m_pFocusObject)
	{
		CTransform* pTransform = m_pFocusObject->Get_TransfomCom();

		_float4 vPickedPos = {};
		if (m_pGameInstance->Picking(&vPickedPos))
		{
			pTransform->Set_State(STATE::POSITION, XMLoadFloat4(&vPickedPos));

		}
	}
}

void CDecalTool::Duplicate()
{
	if (m_pFocusObject == nullptr)
		return;

	CDecalToolObject::DECALTOOLOBJECT_DESC Desc = {};
	Desc.FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] = m_pFocusObject->m_FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)];
	Desc.FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] = m_pFocusObject->m_FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)];
	Desc.FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] = m_pFocusObject->m_FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)];

	Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] = m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)];
	Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] = m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)];
	Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] = m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)];

	Desc.WorldMatrix = m_pFocusObject->Get_TransfomCom()->Get_World4x4();

	//소환하고 포커스 변경
	m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_DecalToolObject"),
		ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal"), &Desc);

	Safe_Release(m_pFocusObject);
	m_pFocusObject = static_cast<CDecalToolObject*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal")));
	Safe_AddRef(m_pFocusObject);
}

void CDecalTool::Clear_All_Decal()
{
	Safe_Release(m_pFocusObject);
	m_pFocusObject = nullptr;

	list<CGameObject*> List = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal"));

	for (CGameObject* pObj : List)
	{
		pObj->Set_bDead();
	}
}


HRESULT CDecalTool::Ready_Texture(const _char* Map)
{
	string ResourcePath = string("../Bin/Save/DecalTool/Resource_") + Map + ".json";

	ifstream inFile(ResourcePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Resource_" + StringToWString(Map) + L".json 파일을 열 수 없습니다: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);

		return S_OK;
	}

	json ReadyTextureJson;
	try
	{
		inFile >> ReadyTextureJson;
		inFile.close();
	}
	catch (const exception& e)
	{
		inFile.close();
		MessageBoxA(nullptr, e.what(), "JSON 파싱 실패", MB_OK);
		return E_FAIL;
	}

	//텍스쳐 프로토타입 생성
	for (auto& [PrototypeTag, Path] : ReadyTextureJson.items())
	{
		//이미 있으면 넘기고
		if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::YW), StringToWString(PrototypeTag)) != nullptr)
			continue;

		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), StringToWString(PrototypeTag),
			CTexture::Create(m_pDevice, m_pContext, StringToWString(Path).c_str()))))
			return E_FAIL;
	}

	return S_OK;
}

CDecalToolObject* CDecalTool::Get_ClosestDecalObject(_fvector vPosition)
{
	list<CGameObject*> List = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal"));

	_float fMinDistance = FLT_MAX;
	CGameObject* pClosestObject = nullptr;

	for (CGameObject* pObj : List)
	{
		_float fDist = XMVectorGetX(XMVector3LengthSq(vPosition - pObj->Get_TransfomCom()->Get_State(STATE::POSITION)));

		if (fDist < fMinDistance)
		{
			fMinDistance = fDist;
			pClosestObject = pObj;
		}

	}

	return  static_cast<CDecalToolObject*>(pClosestObject);
}

HRESULT CDecalTool::Spawn_DecalObject()
{
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
	_float fDist = PRE_TRANSFORMMATRIX_SCALE * 1000.f;

	// 룩 벡터에 거리 곱하기
	_vector vOffset = XMVectorScale(vLook, fDist);

	// 카메라 위치 벡터
	_vector vCamPos = XMLoadFloat4(&CamPos);

	// 최종 위치 계산 (카메라 위치 + 룩 * 거리)
	_vector vSpawnPos = XMVectorAdd(vCamPos, vOffset);

	// 최종 월드 행렬 생성 (위치만)
	_matrix SpawnWorldMatrix = XMMatrixTranslationFromVector(vSpawnPos);

#pragma endregion

	CDecalToolObject::DECALTOOLOBJECT_DESC DecalDesc = {};

	// 오브젝트 월드 행렬에 적용
	XMStoreFloat4x4(&DecalDesc.WorldMatrix, SpawnWorldMatrix);

	//소환하고 포커스 변경
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_DecalToolObject"),
		ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal"), &DecalDesc)))
		return E_FAIL;

	Safe_Release(m_pFocusObject);
	m_pFocusObject = static_cast<CDecalToolObject*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal")));
	Safe_AddRef(m_pFocusObject);

	return S_OK;
}

void CDecalTool::Delete_FocusObject()
{
	Safe_Release(m_pFocusObject);
	m_pFocusObject->Set_bDead();
	m_pFocusObject = nullptr;
}


void CDecalTool::Render_Detail()
{
#pragma region 디테일
	ImGui::Begin("Detail", nullptr);

	Detail_Transform();

	ImGui::Separator();

	Detail_Texture();


	ImGui::End();
#pragma endregion
}

void CDecalTool::Detail_Transform()
{
	ImGui::Text("Transform");

	if (m_pFocusObject != nullptr)
	{
		CTransform* pTransform = m_pFocusObject->Get_TransfomCom();

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

		}
		else if (bPositionChanged || bRotationChanged || bScaleChanged)
		{
			// 수동 입력으로 바뀐 값 → matrix 재구성 후 적용
			ImGuizmo::RecomposeMatrixFromComponents(position, rotation, scale, matrix);
			memcpy(&worldMat, matrix, sizeof(_float) * 16);
			pTransform->Set_WorldMatrix(worldMat);
		}

#pragma endregion
	}
}


void CDecalTool::Detail_Texture()
{
	//여기서 데칼의 ARM, N, BC 세개의 텍스쳐를 갈아 낄 수 있게 하고싶다.
	//Imgui 솔루션 탐색기를 열어서 dds텍스쳐를 찾아서 꽂아넣어야지

	ImGui::Text("Texture");
	if (m_pFocusObject == nullptr)
		return;

	IGFD::FileDialogConfig Config;
	Config.path = "../Bin/Resources/Textures/Decal/";

#pragma region ARM 텍스처
	string ARMT_Name = WStringToString(m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)]);
	string N_Name = WStringToString(m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)]);
	string BC_Name = WStringToString(m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)]);

	ImGui::TextWrapped(ARMT_Name.c_str());
	// ARM 텍스처 선택 버튼
	if (ImGui::Button("Change ARMT Texture"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ChooseARMTTexture", "Select ARMT Texture (.dds)", ".dds", Config);
	}
	if (ImGuiFileDialog::Instance()->Display("ChooseARMTTexture"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			//상대경로로 저장하자
			filePath = ToRelativePath(filePath);

			string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

			//파일 이름을 가져와서, 프로토타입을 만들고, 갈아껴주자
			if (FAILED(m_pFocusObject->Set_Texture(CDecal::TEXTURE_TYPE::ARMT, filePath, fileName)))
				MSG_BOX("ARMT 텍스쳐 갈아끼기 실패");
		}
		ImGuiFileDialog::Instance()->Close();
	}
#pragma endregion

#pragma region Normal 텍스쳐
	ImGui::TextWrapped(N_Name.c_str());
	// Normal 텍스처 선택 버튼
	if (ImGui::Button("Change Normal Texture"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ChooseNormalTexture", "Select Normal Texture (.dds)", ".dds", Config);
	}
	if (ImGuiFileDialog::Instance()->Display("ChooseNormalTexture"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			//상대경로로 저장하자
			filePath = ToRelativePath(filePath);

			string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

			//파일 이름을 가져와서, 프로토타입을 만들고, 갈아껴주자
			if (FAILED(m_pFocusObject->Set_Texture(CDecal::TEXTURE_TYPE::N, filePath, fileName)))
				MSG_BOX("Normal 텍스쳐 갈아끼기 실패");
		}
		ImGuiFileDialog::Instance()->Close();
	}
#pragma endregion

#pragma region BaseColor 텍스쳐
	ImGui::TextWrapped(BC_Name.c_str());
	// BaseColor 텍스처 선택 버튼
	if (ImGui::Button("Change BaseColor Texture"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ChooseBaseColorTexture", "Select BaseColor Texture (.dds)", ".dds", Config);
	}
	if (ImGuiFileDialog::Instance()->Display("ChooseBaseColorTexture"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			//상대경로로 저장하자
			filePath = ToRelativePath(filePath);

			string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

			//파일 이름을 가져와서, 프로토타입을 만들고, 갈아껴주자
			if (FAILED(m_pFocusObject->Set_Texture(CDecal::TEXTURE_TYPE::BC, filePath, fileName)))
				MSG_BOX("BaseColor 텍스쳐 갈아끼기 실패");
		}
		ImGuiFileDialog::Instance()->Close();
	}
#pragma endregion


	
}


CDecalTool* CDecalTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CDecalTool* pInstance = new CDecalTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CDecalTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDecalTool::Clone(void* pArg)
{
	CDecalTool* pInstance = new CDecalTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDecalTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CDecalTool::Free()
{
	__super::Free();

	Safe_Release(m_pFocusObject);
}
