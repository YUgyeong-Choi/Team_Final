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

	//���� ����, [������Ÿ�� �±�, ���� ���] ���·� ����
	map<string, string> ResourceMap;//[������Ÿ�� �±�, ���� ���]

	if (!DecalDataFile.is_open())
		return E_FAIL;

	// ��ü ��Į �����͸� ���� JSON
	json DecalList = json::array();

	for (CGameObject* pObj : m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::YW), TEXT("Layer_Decal")))
	{
		// ���� ��� ���
		_matrix matWorld = pObj->Get_TransfomCom()->Get_WorldMatrix();
		_float4x4 matWorldFloat4x4;
		XMStoreFloat4x4(&matWorldFloat4x4, matWorld);

		// 4x4 ����� �迭�� ����
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

		//ARMT, N, BC �� �ϳ��� ����Ʈ �ؽ��� �������� �������� ����.
		if (pDecalToolObj->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] == TEXT("Prototype_Component_Texture_DefaultDecal") ||
			pDecalToolObj->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] == TEXT("Prototype_Component_Texture_DefaultDecal") ||
			pDecalToolObj->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] == TEXT("Prototype_Component_Texture_DefaultDecal"))
		{
			continue;
		}

		// ��Į ��ü ���� �߰�
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

		// ����Ʈ�� �߰�
		DecalList.push_back(DecalJson);
	}

	// JSON ���Ͽ� ���
	DecalDataFile << DecalList.dump(4); // �鿩���� 4ĭ���� ���� ���� ���
	DecalDataFile.close();

	// ���ҽ� ���Ͽ� ���
	// JSON ��ü ����
	json ResourceJson;

	// map�� JSON�� �ֱ�
	for (const auto& [prototypeTag, filePath] : ResourceMap)
	{
		ResourceJson[prototypeTag] = filePath;
	}

	// ���Ϸ� ����
	string ResourceFilePath =  string("../Bin/Save/DecalTool/Resource_") + Map + ".json";
	ofstream ResourceDataFile(ResourceFilePath);
	if (ResourceDataFile.is_open())
	{
		ResourceDataFile << ResourceJson.dump(4); // 4�� �鿩����
	}

	MSG_BOX("��Į ���� ����!");

	return S_OK;
}

HRESULT CDecalTool::Load(const _char* Map)
{
	//���� �ʿ� ��ġ�� ������Ʈ�� ��� ��������
	Clear_All_Decal();

	Ready_Texture(Map);

	//���� �ʿ� �ʿ��� ��Į �ؽ��ĸ� �ε��Ѵ�.

	string DecalDataPath = string("../Bin/Save/DecalTool/Decal_") + Map + ".json";
	//string ResourcePath = string("../Bin/Save/MapTool/Resource_") + Map + ".json"; //���߿� ���� �� �ٲܶ�

	ifstream inFile(DecalDataPath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Decal_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);
		return S_OK;
	}


	//���� �н��� ����������
	string ResuorcePath = string("../Bin/Save/DecalTool/Resource_") + Map + ".json";
	json ResourceJson;
	{
		ifstream ResFile(ResuorcePath);
		if (ResFile.is_open())
			ResFile >> ResourceJson;
	}

	// JSON �Ľ�
	json JSON;
	inFile >> JSON;

	// ������ ��ŭ Decal ��ȯ
	for (auto& item : JSON)
	{
		// 4x4 ��� �б�
		_float4x4 WorldMatrix;
		for (_int iRow = 0; iRow < 4; ++iRow)
		{
			for (_int iCol = 0; iCol < 4; ++iCol)
			{
				WorldMatrix.m[iRow][iCol] = item["WorldMatrix"][iRow][iCol].get<_float>();
			}
		}

		//�ؽ��� ������Ÿ�� �̸� ����
		CDecalToolObject::DECALTOOLOBJECT_DESC Desc = {};
		Desc.WorldMatrix = WorldMatrix;
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] = StringToWString(item["ARMT"].get<string>());
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] = StringToWString(item["N"].get<string>());
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] = StringToWString(item["BC"].get<string>());

		//�ؽ��� ���� ��� ����
		auto it = ResourceJson.find(item["ARMT"].get<string>());
		Desc.FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] = StringToWString(it.value());

		it = ResourceJson.find(item["N"].get<string>());
		Desc.FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] = StringToWString(it.value());

		it = ResourceJson.find(item["BC"].get<string>());
		Desc.FilePath[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] = StringToWString(it.value());

		// Decal ��ü ����
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
		//E ȸ��, R ũ��, T�� ��ġ
		if (m_pGameInstance->Key_Down(DIK_E))
			m_currentOperation = ImGuizmo::ROTATE;
		else if (m_pGameInstance->Key_Down(DIK_R))
			m_currentOperation = ImGuizmo::SCALE;
		else if (m_pGameInstance->Key_Down(DIK_T))
			m_currentOperation = ImGuizmo::TRANSLATE;

		//��Ŀ�� ����
		if (m_pGameInstance->Mouse_Up(DIM::WHEELBUTTON))
		{
			Safe_Release(m_pFocusObject);
			m_pFocusObject = nullptr;
		}
	}

	//Ŭ���ϸ� ���� ����� ��Į�� ��Ŀ�� �Ѵ�.
	if (m_pGameInstance->Mouse_Up(DIM::LBUTTON) && ImGuizmo::IsOver() == false)
	{
		Select_Decal();
	}

	//F Ű������ �ش� ������Ʈ ��ġ�� �̵�
	if (m_pGameInstance->Key_Down(DIK_F))
	{
		Focus();
	}

	//����ƮŰ ������ ���� ���õȰ� ����
	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		Delete_FocusObject();
	}

	//��Ʈ Ŭ���ϸ� �ش� ��ġ�� ��Į �̵�
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
	//��ƮŰ ������ ������ ��ŷ���� ����(������Ʈ ���̰��� ������Ʈ ����Ǵ°� ���� ������)
	if (m_pGameInstance->Key_Pressing(DIK_LALT))
		return;

	// ImGui�� ���콺 �Է��� �������� ��ŷ�� ���� ����
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

	//������ �ΰ� �̵�����
	pCameraTransformCom->Set_State(STATE::POSITION, vCameraPos);

	//LookAt ����
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

	//��ȯ�ϰ� ��Ŀ�� ����
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
		wstring ErrorMessage = L"Resource_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);

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
		MessageBoxA(nullptr, e.what(), "JSON �Ľ� ����", MB_OK);
		return E_FAIL;
	}

	//�ؽ��� ������Ÿ�� ����
	for (auto& [PrototypeTag, Path] : ReadyTextureJson.items())
	{
		//�̹� ������ �ѱ��
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
#pragma region ī�޶� �տ��ٰ� ��ȯ
	//ī�޶� �տ��ٰ� ��ȯ
	//ī�޶� ��ġ����, ����� Look ��ŭ ������
	// ī�޶� ��ġ
	_float4 CamPos = *m_pGameInstance->Get_CamPosition();

	// ��ġ�� �ݿ��� ��� ����
	_matrix matWorld = XMMatrixTranslation(CamPos.x, CamPos.y, CamPos.z);

	// ī�޶� ���� ��� (�� ��� �����)
	_matrix CamWorldMatrix = XMMatrixInverse(nullptr, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	_float4x4 CamWM = {};
	XMStoreFloat4x4(&CamWM, CamWorldMatrix);

	// �� ���� ���� (3��° ��)
	_vector vLook = XMVectorSet(CamWM._31, CamWM._32, CamWM._33, 0.f);

	// �� ���� ����ȭ
	vLook = XMVector3Normalize(vLook);

	// �Ÿ� ����
	_float fDist = PRE_TRANSFORMMATRIX_SCALE * 1000.f;

	// �� ���Ϳ� �Ÿ� ���ϱ�
	_vector vOffset = XMVectorScale(vLook, fDist);

	// ī�޶� ��ġ ����
	_vector vCamPos = XMLoadFloat4(&CamPos);

	// ���� ��ġ ��� (ī�޶� ��ġ + �� * �Ÿ�)
	_vector vSpawnPos = XMVectorAdd(vCamPos, vOffset);

	// ���� ���� ��� ���� (��ġ��)
	_matrix SpawnWorldMatrix = XMMatrixTranslationFromVector(vSpawnPos);

#pragma endregion

	CDecalToolObject::DECALTOOLOBJECT_DESC DecalDesc = {};

	// ������Ʈ ���� ��Ŀ� ����
	XMStoreFloat4x4(&DecalDesc.WorldMatrix, SpawnWorldMatrix);

	//��ȯ�ϰ� ��Ŀ�� ����
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
#pragma region ������
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

		// ���� ��ư ������ ����
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

#pragma region ����� �� ��� ����
		_float4x4 worldMat;
		XMStoreFloat4x4(&worldMat, pTransform->Get_WorldMatrix());

		_float matrix[16];
		memcpy(matrix, &worldMat, sizeof(float) * 16);

		// ����
		_float position[3], rotation[3], scale[3];
		ImGuizmo::DecomposeMatrixToComponents(matrix, position, rotation, scale);

		// ImGuizmo ����
		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

		ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		ImGuizmo::SetRect(0, 0, displaySize.x, displaySize.y);

		_float viewMat[16], projMat[16];
		XMStoreFloat4x4(reinterpret_cast<_float4x4*>(viewMat), m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
		XMStoreFloat4x4(reinterpret_cast<_float4x4*>(projMat), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));

		// Gizmo ����
		ImGuizmo::Manipulate(viewMat, projMat, m_currentOperation, ImGuizmo::LOCAL, matrix);
#pragma endregion

#pragma region ������
		_bool bPositionChanged = ImGui::InputFloat3("##Position", position, "%.2f");
		ImGui::SameLine();
		if (ImGui::RadioButton("Position", m_currentOperation == ImGuizmo::TRANSLATE))
			m_currentOperation = ImGuizmo::TRANSLATE;
#pragma endregion

#pragma region ȸ��
		_bool bRotationChanged = ImGui::InputFloat3("##Rotation", rotation, "%.2f");
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotation", m_currentOperation == ImGuizmo::ROTATE))
			m_currentOperation = ImGuizmo::ROTATE;
#pragma endregion

#pragma region ������
		_bool bScaleChanged = ImGui::InputFloat3("##Scale", scale, "%.2f");
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", m_currentOperation == ImGuizmo::SCALE))
			m_currentOperation = ImGuizmo::SCALE;
#pragma endregion

#pragma region ����

		if (ImGuizmo::IsUsing())
		{
			// ImGuizmo�� ���۵� matrix �״�� ����
			memcpy(&worldMat, matrix, sizeof(_float) * 16);

			// ��ǥ ������Ʈ�� ���� ��İ� �� ��� ��
			_matrix matPrevMain = m_pFocusObject->Get_TransfomCom()->Get_WorldMatrix();
			_matrix matNewMain = XMLoadFloat4x4(&worldMat);

			// ��ǥ ������Ʈ�� �̵�/ȸ��/������ ��ȭ ��� ���
			_matrix matOffset = matNewMain * XMMatrixInverse(nullptr, matPrevMain);

			pTransform->Set_WorldMatrix(worldMat);

		}
		else if (bPositionChanged || bRotationChanged || bScaleChanged)
		{
			// ���� �Է����� �ٲ� �� �� matrix �籸�� �� ����
			ImGuizmo::RecomposeMatrixFromComponents(position, rotation, scale, matrix);
			memcpy(&worldMat, matrix, sizeof(_float) * 16);
			pTransform->Set_WorldMatrix(worldMat);
		}

#pragma endregion
	}
}


void CDecalTool::Detail_Texture()
{
	//���⼭ ��Į�� ARM, N, BC ������ �ؽ��ĸ� ���� �� �� �ְ� �ϰ�ʹ�.
	//Imgui �ַ�� Ž���⸦ ��� dds�ؽ��ĸ� ã�Ƽ� �ȾƳ־����

	ImGui::Text("Texture");
	if (m_pFocusObject == nullptr)
		return;

	IGFD::FileDialogConfig Config;
	Config.path = "../Bin/Resources/Textures/Decal/";

#pragma region ARM �ؽ�ó
	string ARMT_Name = WStringToString(m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)]);
	string N_Name = WStringToString(m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)]);
	string BC_Name = WStringToString(m_pFocusObject->m_PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)]);

	ImGui::TextWrapped(ARMT_Name.c_str());
	// ARM �ؽ�ó ���� ��ư
	if (ImGui::Button("Change ARMT Texture"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ChooseARMTTexture", "Select ARMT Texture (.dds)", ".dds", Config);
	}
	if (ImGuiFileDialog::Instance()->Display("ChooseARMTTexture"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			//����η� ��������
			filePath = ToRelativePath(filePath);

			string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

			//���� �̸��� �����ͼ�, ������Ÿ���� �����, ���Ʋ�����
			if (FAILED(m_pFocusObject->Set_Texture(CDecal::TEXTURE_TYPE::ARMT, filePath, fileName)))
				MSG_BOX("ARMT �ؽ��� ���Ƴ��� ����");
		}
		ImGuiFileDialog::Instance()->Close();
	}
#pragma endregion

#pragma region Normal �ؽ���
	ImGui::TextWrapped(N_Name.c_str());
	// Normal �ؽ�ó ���� ��ư
	if (ImGui::Button("Change Normal Texture"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ChooseNormalTexture", "Select Normal Texture (.dds)", ".dds", Config);
	}
	if (ImGuiFileDialog::Instance()->Display("ChooseNormalTexture"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			//����η� ��������
			filePath = ToRelativePath(filePath);

			string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

			//���� �̸��� �����ͼ�, ������Ÿ���� �����, ���Ʋ�����
			if (FAILED(m_pFocusObject->Set_Texture(CDecal::TEXTURE_TYPE::N, filePath, fileName)))
				MSG_BOX("Normal �ؽ��� ���Ƴ��� ����");
		}
		ImGuiFileDialog::Instance()->Close();
	}
#pragma endregion

#pragma region BaseColor �ؽ���
	ImGui::TextWrapped(BC_Name.c_str());
	// BaseColor �ؽ�ó ���� ��ư
	if (ImGui::Button("Change BaseColor Texture"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ChooseBaseColorTexture", "Select BaseColor Texture (.dds)", ".dds", Config);
	}
	if (ImGuiFileDialog::Instance()->Display("ChooseBaseColorTexture"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			//����η� ��������
			filePath = ToRelativePath(filePath);

			string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

			//���� �̸��� �����ͼ�, ������Ÿ���� �����, ���Ʋ�����
			if (FAILED(m_pFocusObject->Set_Texture(CDecal::TEXTURE_TYPE::BC, filePath, fileName)))
				MSG_BOX("BaseColor �ؽ��� ���Ƴ��� ����");
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
