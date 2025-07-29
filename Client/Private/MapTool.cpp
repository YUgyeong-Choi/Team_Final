#include "MapTool.h"
#include "GameInstance.h"

#include <filesystem>
#include "MapToolObject.h"

#include "Layer.h"
#include "RenderTarget.h"

#include "PreviewObject.h"

#include "Camera_Free.h"

#include "Camera_Manager.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() �̷� �̱������� ����� ���� ���ϰ� ��

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

	//MapData�� ��ġ�Ǿ��ִ� �𵨵��� �̸� �ҷ����δ�.
	if (FAILED(Ready_Model()))
		return E_FAIL;

	//MapData�� ���� ���� �ε��Ѵ�.
	if (FAILED(Load_Map()))
		return E_FAIL;

	m_pPreviewObject = static_cast<CPreviewObject*>(m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_PreviewObject")));
	if (m_pPreviewObject == nullptr)
		return E_FAIL;

	Safe_AddRef(m_pPreviewObject);

	return S_OK;
}

void CMapTool::Priority_Update(_float fTimeDelta)
{

}

void CMapTool::Update(_float fTimeDelta)
{
	Control(fTimeDelta);

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

void CMapTool::Control(_float fTimeDelta)
{
	//E ȸ��, R ũ��, T�� ��ġ
	if (m_pGameInstance->Key_Down(DIK_E))
		m_currentOperation = ImGuizmo::ROTATE;
	else if (m_pGameInstance->Key_Down(DIK_R))
		m_currentOperation = ImGuizmo::SCALE;
	else if (m_pGameInstance->Key_Down(DIK_T))
		m_currentOperation = ImGuizmo::TRANSLATE;
	else if (m_pGameInstance->Key_Down(DIK_ESCAPE))
		m_iSelectedHierarchyIndex = -1;

	//Ctrl + S �� ����
	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_S))
	{
		Save_Map();
	}

	//Ctrl + D ���õ� ������Ʈ ����
	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_D))
	{
		Duplicate_Selected_Object();
	}

	//Ctrl + Z �ش� ������Ʈ�� ����� ������ġ �̵�
	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_Z))
	{
		Undo_Selected_Object();
	}

	//��ŷ���� �� ������Ʈ ���� �ϴ� ���(����� �ٸ� ��ü���� �ڿ� ������ �����Ϸ��� ��ü�� �ٲ����� IsOver()�� �ذ�)
	if (m_pGameInstance->Mouse_Down(DIM::LBUTTON) && m_pGameInstance->Key_Pressing(DIK_LCONTROL) == false && ImGuizmo::IsOver() == false)
	{
		Picking();
	}

	//����ƮŰ ������ ���� ���õȰ� ����
	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		printf("Delete\n");
		DeleteMapToolObject();
	}

	Control_PreviewObject(fTimeDelta);

}

HRESULT CMapTool::Ready_Model()
{
	ifstream inFile("../Bin/Save/MapTool/ReadyModel.json");
	if (!inFile.is_open())
	{
		MSG_BOX("ReadyModel.json ������ �� �� �����ϴ�.");
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
		MessageBoxA(nullptr, e.what(), "JSON �Ľ� ����", MB_OK);
		return E_FAIL;
	}

	// JSON ������ Ȯ��
	for (const auto& element : ReadyModelJson)
	{
		string ModelName = element.value("ModelName", "");
		string Path = element.value("Path", "");

		//�� ������ Ÿ�� ����
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
	json MapDataJson; // �𵨰� ������Ʈ ���� �����

	//���� �ʵ忡 �����ϴ� �𵨵��� ���̾� �̸����� �����´�.
	vector<wstring> LayerNames = m_pGameInstance->Find_LayerNamesContaining(ENUM_CLASS(LEVEL::YW), TEXT("Layer_MapToolObject_"));

	//�� ���� ����
	_uint iModelCount = static_cast<_uint>(LayerNames.size());
	MapDataJson["ModelCount"] = iModelCount;
	MapDataJson["Models"] = json::array();

	for (wstring& LayerName : LayerNames)
	{
		list<CGameObject*>& MapObjectList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::YW), LayerName);

		//�� �̸�����
		wstring Prefix = L"Layer_MapToolObject_";
		wstring ModelName = {};
		if (LayerName.find(Prefix) == 0) // ���λ� Ȯ��
		{
			ModelName = LayerName.substr(Prefix.length());
		}

		//Json ���Ͽ� ���� �̸��� ���ϰ�� ����
		string strModelName = WStringToString(ModelName);
		filesystem::path path = filesystem::path(PATH_NONANIM) / (strModelName + ".bin"); //�̷��� �ϵ��ڵ� ���帻��
		string FullPath = path.generic_string();

		json ObjectJson;
		ObjectJson["ModelName"] = strModelName;
		ObjectJson["Path"] = FullPath; // �Ǵ� Path
		ObjectJson["ObjectCount"] = static_cast<_uint>(MapObjectList.size());//���� �����ؼ� �ν��Ͻ� ���� �ε����� ���� 

		ReadyModelJsonArray.push_back(ObjectJson);

		// ���ڿ� ���� ���
		_uint iLength = static_cast<_uint>(ModelName.length());

		// ������Ʈ ���� ����
		_uint iObjectCount = static_cast<_uint>(MapObjectList.size());

		// �� JSON
		json ModelJson;
		ModelJson["ModelName"] = strModelName;
		ModelJson["ObjectCount"] = static_cast<_uint>(MapObjectList.size());
		ModelJson["Objects"] = json::array();

		for (CGameObject* pGameObject : MapObjectList)
		{
			// JSON �ϳ� ����
			json ObjectJson;

			//�� ��ġ�� ����
			_matrix matWorld = pGameObject->Get_TransfomCom()->Get_WorldMatrix();
			_float4x4 matWorldFloat4x4;
			XMStoreFloat4x4(&matWorldFloat4x4, matWorld);

			// ����� 4x4 �迭�� ����
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

			ModelJson["Objects"].push_back(ObjectJson);
		}

		MapDataJson["Models"].push_back(ModelJson);
	}

	// ���Ͽ� JSON ����
	ReadyModelFile << ReadyModelJsonArray.dump(4);
	MapDataFile << MapDataJson.dump(4);

	MapDataFile.close();
	ReadyModelFile.close();

	MSG_BOX("�� ���� ����");

	return S_OK;
}

HRESULT CMapTool::Load_Map()
{
	ifstream inFile("../Bin/Save/MapTool/MapData.json");
	if (!inFile.is_open())
	{
		MSG_BOX("MapData.json ������ �� �� �����ϴ�.");
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

			//������Ʈ ����, ��ġ

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

			//Ÿ�ϸ�
			if (objects[j].contains("TileDensity"))
			{
				MapToolObjDesc.bUseTiling = true;

				const json& TileDensityJson = objects[j]["TileDensity"];
				MapToolObjDesc.vTileDensity = {
					TileDensityJson[0].get<_float>(),
					TileDensityJson[1].get<_float>()
				};
			}


			if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
				ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
				return E_FAIL;

			//��� �߰��Ѽ��� �� �׷쿡 �з��ؼ� ����
			Add_ModelGroup(ModelName, m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), LayerTag));

		}
	}

	return S_OK;
}

HRESULT CMapTool::Render_MapTool()
{
	Render_Hierarchy();

	Render_Asset();
	Render_Favorite();

	Render_Preview();

	Render_Detail();

	return S_OK;
}

void CMapTool::Render_Hierarchy()
{
#pragma region ���̾��Ű
	ImGui::Begin("Hierarchy", nullptr);

	if (ImGui::BeginListBox("##HierarchyList", ImVec2(-FLT_MIN, 300)))
	{
		_uint i = 0; // ��ü Hierarchy �׸� �ε����� ���� ī���� (���� �ε���)
		for (auto& group : m_ModelGroups) // �� �̸����� �׷�ȭ�� GameObject ����� �ݺ�
		{
			const string& ModelName = group.first; // ���� �׷��� �� �̸�
			// Ʈ�� ��� ���� (�����ְ� �⺻ ����) - �� �̸��� �������� �׷�ȭ�� �׸�
			_bool bOpen = ImGui::TreeNodeEx(ModelName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

			for (auto pGameObject : group.second)
			{
				// ���� ������Ʈ�� ���� �̸� ���� (ID �� ���� ���߿�)
				string strHierarchyName = "(ID:" + to_string(static_cast<CMapToolObject*>(pGameObject)->Get_ID()) + ')' + ModelName;

				// ���� �ε����� ���õ� �������� Ȯ��
				_bool isSelected = (m_iSelectedHierarchyIndex == i);

				if (bOpen) // Ʈ�� ��尡 ���� ���� ���� Selectable �׸��� �׸���
				{
					// �ش� �׸��� Ŭ���Ǹ� �ε����� ����Ͽ� ���� ���·� �����
					if (ImGui::Selectable(strHierarchyName.c_str(), isSelected))
					{
						m_iSelectedHierarchyIndex = i; // ���� ���õ� �׸����� ����
						m_pSelectedObject = static_cast<CMapToolObject*>(Get_Selected_GameObject());
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus(); // ��Ŀ���� �ش� �׸� �����ش� (Ű���� �׺���̼ǿ�)
				}

				++i; // ��ü �ε��� ���� (Ʈ�� ��� �����ֵ� ���� �������Ѿ� ��)
			}

			if (bOpen)
				ImGui::TreePop(); // Ʈ�� ��� �ݱ� (Ʈ�� UI�� �ݾ���)
		}

		ImGui::EndListBox(); // ����Ʈ�ڽ� ��

	}
#pragma endregion
	if (ImGui::Button("Delete"))
	{
		DeleteMapToolObject();
	}

	if (ImGui::Button("Save Map"))
	{
		if (FAILED(Save_Map()))
			MSG_BOX("�� ���� ����");
	}


	//if (ImGui::Button("Load Map"))
	//{

	//}
	ImGui::End();
}

void CMapTool::Render_Asset()
{
#pragma region ����
	ImGui::Begin("Asset", nullptr);

	if (ImGui::Button("Spawn"))
	{
		if (FAILED(Spawn_MapToolObject()))
		{
			MSG_BOX("���� ����");
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Add Favorites"))
	{
		if (false == m_ModelNames.empty())
		{
			// �̹� �̸��� ��Ͽ� �����ϸ� �ߺ� �߰����� ����
			if (find(m_FavoriteModelNames.begin(), m_FavoriteModelNames.end(), m_ModelNames[m_iSelectedModelIndex]) == m_FavoriteModelNames.end())
			{
				m_FavoriteModelNames.push_back(m_ModelNames[m_iSelectedModelIndex]); // �̸� �߰�
			}
		}
	}

	// â ��Ŀ�� ����
	const _bool bWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootAndChildWindows);

	if (ImGui::BeginListBox("##Model List", ImVec2(-FLT_MIN, 200)))
	{
		/*
		* �� ������ �����Կ� ����ִ� ��� <���̸�>�� ���
		*/

		for (_int i = 0; i < m_ModelNames.size(); ++i)
		{
			const _bool isSelected = (m_iSelectedModelIndex == i);
			if (ImGui::Selectable(m_ModelNames[i].c_str(), isSelected))
			{
				if (bWindowFocused)
				{
					m_iSelectedModelIndex = i;

					//�̸����� �� ����
					//TEXT("Prototype_Component_Model_���̸�"),
					wstring ModelName = wstring(m_ModelNames[m_iSelectedModelIndex].begin(), m_ModelNames[m_iSelectedModelIndex].end());
					wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
					ModelPrototypeTag += ModelName;

					m_pPreviewObject->Change_Model(ModelPrototypeTag);
				}
			}

			// ���õ� �׸� ��Ŀ��
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();

	}

	/*
		���� Ž���� ����
		�ش� ��.bin �� ��θ� �����ͼ�
		�� ������ �����.

		_matrix		PreTransformMatrix = XMMatrixIdentity();

		PreTransformMatrix = XMMatrixIdentity();
		PreTransformMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f);
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_���̸�"),
			CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, "../Bin/Resources/Models/Bin_NonAnim/���̸�.bin", PreTransformMatrix))))
			return E_FAIL;

		�� �������� �����Ѵ�.
	*/
	if (ImGui::Button("Import Model"))
	{
		IGFD::FileDialogConfig config;
		config.path = PATH_NONANIM;
		config.countSelectionMax = 0; //������ ���� �ε�

		IFILEDIALOG->OpenDialog("Import Model", "Import .bin model", ".bin", config);
	}

	// �� �����Ӹ��� Display ȣ��
	if (IFILEDIALOG->Display("Import Model"))
	{
		if (IFILEDIALOG->IsOk())
		{
			//filesystem::path ModelPath = IFILEDIALOG->GetFilePathName();

			auto selections = IFILEDIALOG->GetSelection();
			// ó��
			// first: ���ϸ�.Ȯ����
			// second: ��ü ��� (���ϸ�����)
			if (!selections.empty())
			{
				for (auto FilePath : selections)
				{
					filesystem::path ModelPath = FilePath.second;

					if (!FilePath.second.empty())
					{
						// Prototype �̸� ����
						wstring PrototypeTag = L"Prototype_Component_Model_" + ModelPath.stem().wstring();
						string strPrototypeTag = ModelPath.stem().string(); // Ȯ���� ���� ���� �̸���

						string ModelFilePath = ModelPath.string();
						const _char* pModelFilePath = ModelFilePath.c_str();

						if (FAILED(Load_Model(PrototypeTag, pModelFilePath)))
						{
							MSG_BOX("�ε� ����");
						}
						else
						{
							// �̹� �̸��� ��Ͽ� �����ϸ� �ߺ� �߰����� ����
							if (find(m_ModelNames.begin(), m_ModelNames.end(), strPrototypeTag) == m_ModelNames.end())
							{
								m_ModelNames.push_back(strPrototypeTag); // �̸� �߰�
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

	// â ��Ŀ�� ����
	const _bool bFavoriteWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootAndChildWindows);

	if (ImGui::Button("Spawn"))
	{
		if (m_iSelectedFavoriteModelIndex != -1)
		{
			if (FAILED(Spawn_MapToolObject(m_FavoriteModelNames[m_iSelectedFavoriteModelIndex])))
			{
				MSG_BOX("���� ����");
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
			// ã�Ƽ� ����
			if (iter != m_FavoriteModelNames.end())
			{
				m_FavoriteModelNames.erase(iter);
				m_iSelectedFavoriteModelIndex = -1;
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
				if (bFavoriteWindowFocused) // ��Ŀ�� �� ��쿡�� �� ����
				{

					m_iSelectedFavoriteModelIndex = i;

					//�̸����� �� ����
					//TEXT("Prototype_Component_Model_���̸�"),
					wstring ModelName = wstring(m_FavoriteModelNames[m_iSelectedFavoriteModelIndex].begin(), m_FavoriteModelNames[m_iSelectedFavoriteModelIndex].end());
					wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
					ModelPrototypeTag += ModelName;

					m_pPreviewObject->Change_Model(ModelPrototypeTag);
				}
			}

			// ���õ� �׸� ��Ŀ��
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndListBox();
	}

	ImGui::End();
}

void CMapTool::Render_Detail()
{
#pragma region ������
	ImGui::Begin("Detail", nullptr);

	ImGui::Separator();

	Detail_Transform();

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

		// ī�޶� ���� ��ư
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

		//������ ������Ʈ ȸ�� ��� ��ư
		ImGui::SameLine();
		_bool* pIsRotate = m_pPreviewObject->Get_IsRotate_Ptr();
		const _char* label = *pIsRotate ? "ON" : "OFF";
		ImGui::Text("Rotate");
		ImGui::SameLine();
		if (ImGui::Button(label))
		{
			*pIsRotate = !*pIsRotate;
		}

		// 2. ����Ÿ���� �ؽ�óó�� ImGui�� ǥ��
		ID3D11ShaderResourceView* pSRV = m_pGameInstance->Find_RenderTarget(TEXT("Target_Preview"))->Get_SRV();

		// ���� â�� ���� ���� ũ�� (�е� ����)
		ImVec2 availableSize = ImGui::GetContentRegionAvail();

		ImGui::Image(reinterpret_cast<ImTextureID>(pSRV), availableSize, ImVec2(0.f, 0.f), ImVec2(1.f, 1.f));

	}

	ImGui::End();
}

HRESULT CMapTool::Spawn_MapToolObject()
{
	if (m_iSelectedModelIndex == -1)
		return E_FAIL;

	//���� ���õ� �𵨷� �ʿ�����Ʈ�� ����

	CMapToolObject::MAPTOOLOBJ_DESC MapToolObjDesc = {};

	//TEXT("Prototype_Component_Model_���̸�"),
	wstring ModelName = wstring(m_ModelNames[m_iSelectedModelIndex].begin(), m_ModelNames[m_iSelectedModelIndex].end());
	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
	ModelPrototypeTag += ModelName;
	lstrcpy(MapToolObjDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());
	lstrcpy(MapToolObjDesc.szModelName, ModelName.c_str());
	/*
	������ ������ �� � ���̾ ��Ҵ��� �����ϴ� ���̾� ����Ʈ�� �ʿ���
	������ �� ���̾ �����ϴ� �� Ȯ��(���̾ ������ �������� �ʴٴ� ��)
	�� ���̾ ������� ����Ʈ���� ���� �ؾ���
	*/
	wstring LayerTag = TEXT("Layer_MapToolObject_");
	LayerTag += ModelName;

	MapToolObjDesc.iID = m_iID++;

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
	_float fDist = PRE_TRANSFORMMATRIX_SCALE * 500.f;

	// �� ���Ϳ� �Ÿ� ���ϱ�
	_vector vOffset = XMVectorScale(vLook, fDist);

	// ī�޶� ��ġ ����
	_vector vCamPos = XMLoadFloat4(&CamPos);

	// ���� ��ġ ��� (ī�޶� ��ġ + �� * �Ÿ�)
	_vector vSpawnPos = XMVectorAdd(vCamPos, vOffset);

	// ���� ���� ��� ���� (��ġ��)
	_matrix SpawnWorldMatrix = XMMatrixTranslationFromVector(vSpawnPos);

	// ������Ʈ ���� ��Ŀ� ����
	XMStoreFloat4x4(&MapToolObjDesc.WorldMatrix, SpawnWorldMatrix);
#pragma endregion

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
		ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
		return E_FAIL;

	//��� �߰��Ѽ��� �� �׷쿡 �з��ؼ� ����
	Add_ModelGroup(WStringToString(ModelName), m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), LayerTag));

	return S_OK;
}

HRESULT CMapTool::Spawn_MapToolObject(string ModelName)
{
	//���� ���õ� �𵨷� �ʿ�����Ʈ�� ����

	CMapToolObject::MAPTOOLOBJ_DESC MapToolObjDesc = {};

	//TEXT("Prototype_Component_Model_���̸�"),
	wstring wModelName = StringToWString(ModelName);
	wstring wModelPrototypeTag = TEXT("Prototype_Component_Model_");
	wModelPrototypeTag += wModelName;
	lstrcpy(MapToolObjDesc.szModelPrototypeTag, wModelPrototypeTag.c_str());
	lstrcpy(MapToolObjDesc.szModelName, wModelName.c_str());
	/*
	������ ������ �� � ���̾ ��Ҵ��� �����ϴ� ���̾� ����Ʈ�� �ʿ���
	������ �� ���̾ �����ϴ� �� Ȯ��(���̾ ������ �������� �ʴٴ� ��)
	�� ���̾ ������� ����Ʈ���� ���� �ؾ���
	*/
	wstring LayerTag = TEXT("Layer_MapToolObject_");
	LayerTag += wModelName;

	MapToolObjDesc.iID = m_iID++;

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
	_float fDist = PRE_TRANSFORMMATRIX_SCALE * 500.f;

	// �� ���Ϳ� �Ÿ� ���ϱ�
	_vector vOffset = XMVectorScale(vLook, fDist);

	// ī�޶� ��ġ ����
	_vector vCamPos = XMLoadFloat4(&CamPos);

	// ���� ��ġ ��� (ī�޶� ��ġ + �� * �Ÿ�)
	_vector vSpawnPos = XMVectorAdd(vCamPos, vOffset);

	// ���� ���� ��� ���� (��ġ��)
	_matrix SpawnWorldMatrix = XMMatrixTranslationFromVector(vSpawnPos);

	// ������Ʈ ���� ��Ŀ� ����
	XMStoreFloat4x4(&MapToolObjDesc.WorldMatrix, SpawnWorldMatrix);
#pragma endregion

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
		ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
		return E_FAIL;

	//��� �߰��Ѽ��� �� �׷쿡 �з��ؼ� ����
	Add_ModelGroup(ModelName, m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), LayerTag));

	return S_OK;
}

HRESULT CMapTool::Duplicate_Selected_Object()
{
	//���� ���õ� ������Ʈ�� �ʿ�����Ʈ�� ����

	CMapToolObject* pMapToolObject = static_cast<CMapToolObject*>(Get_Selected_GameObject());
	if (pMapToolObject == nullptr)
		return S_OK;

	CMapToolObject::MAPTOOLOBJ_DESC MapToolObjDesc = {};

	//TEXT("Prototype_Component_Model_���̸�"),
	string ModelName = pMapToolObject->Get_ModelName();

	lstrcpy(MapToolObjDesc.szModelPrototypeTag, pMapToolObject->Get_ModelPrototypeTag().c_str());
	lstrcpy(MapToolObjDesc.szModelName, StringToWString(ModelName).c_str());
	/*
	������ ������ �� � ���̾ ��Ҵ��� �����ϴ� ���̾� ����Ʈ�� �ʿ���
	������ �� ���̾ �����ϴ� �� Ȯ��(���̾ ������ �������� �ʴٴ� ��)
	�� ���̾ ������� ����Ʈ���� ���� �ؾ���
	*/
	wstring LayerTag = TEXT("Layer_MapToolObject_");
	LayerTag += StringToWString(ModelName);

	MapToolObjDesc.iID = m_iID++;

#pragma region �ش� ������Ʈ �����ٰ� ��ȯ
	_matrix SpawnWorldMatrix = pMapToolObject->Get_TransfomCom()->Get_WorldMatrix();

	// x������ 3.f ��ŭ �̵��ϴ� ��ȯ ���
	_matrix matOffset = XMMatrixTranslation(3.f, 0.f, 0.f);

	// ��ȯ ����: ���� ��Ŀ� offset�� �����ش�
	_matrix matResult = matOffset * SpawnWorldMatrix;

	// ��� ����
	XMStoreFloat4x4(&MapToolObjDesc.WorldMatrix, matResult);
#pragma endregion

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
		ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
		return E_FAIL;

	//��� �߰��� ���� �� �׷쿡 �з��ؼ� ����
	Add_ModelGroup(ModelName, m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), LayerTag));


	m_iSelectedHierarchyIndex = Find_HierarchyIndex_By_ID(MapToolObjDesc.iID);

	return S_OK;
}

HRESULT CMapTool::Undo_Selected_Object()
{
	CMapToolObject* pMapToolObject = static_cast<CMapToolObject*>(Get_Selected_GameObject());
	if (pMapToolObject == nullptr)
		return S_OK;

	pMapToolObject->Undo_WorldMatrix();

	return S_OK;
}

void CMapTool::DeleteMapToolObject()
{
	CGameObject* pGameObject = Get_Selected_GameObject();

	if (nullptr != pGameObject)
	{
		//�׷쿡�� ����
		Delete_ModelGroup(pGameObject);
		//������ ����
		pGameObject->Set_bDead();

		m_iSelectedHierarchyIndex = -1;
		m_pSelectedObject = nullptr;
	}
}

HRESULT CMapTool::Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath)
{
	//�̹� ������Ÿ���������ϴ� ��Ȯ��
	
	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::YW), strPrototypeTag) != nullptr)
	{
		//MSG_BOX("�̹� ������Ÿ���� ������");
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
	// �� �̸��� Ű�� �Ͽ� �׷쿡 GameObject�� �߰�
	m_ModelGroups[ModelName].push_back(pMapToolObject);
}

void CMapTool::Delete_ModelGroup(CGameObject* pMapToolObject)
{
	// map<string, vector<CGameObject*>> m_ModelGroups; ����
	// �׷��� �̸���, ���̵� �������� ����

	string ModelName = static_cast<CMapToolObject*>(pMapToolObject)->Get_ModelName();

	auto iterGroup = m_ModelGroups.find(ModelName);

	// �ش� �׷��� �������� ������ ����
	if (iterGroup == m_ModelGroups.end())
	{
		MSG_BOX("�ɰ��� ����) ������� �ش� �׷��� �������� ����");
		return;
	}

	std::list<CGameObject*>& objList = iterGroup->second;

	// ����Ʈ���� �ش� ������Ʈ ����
	objList.remove(pMapToolObject);

	// ���� �ش� �׷��� ����ٸ� map���� �׷� ��ü�� ����
	if (objList.empty())
	{
		m_ModelGroups.erase(iterGroup);
	}

}

CGameObject* CMapTool::Get_Selected_GameObject()
{
	_uint index = m_iSelectedHierarchyIndex;

	for (auto& group : m_ModelGroups)
	{
		for (auto pGameObject : group.second)
		{
			if (index == 0)
				return pGameObject;

			--index;
		}
	}

	return nullptr; // �ε��� �ʰ� �� null
}

_int CMapTool::Find_HierarchyIndex_By_ID(_uint iID)
{
	//���� ���̵� ���� ������Ʈ�� ��Ŀ��
	//�� �׷��߿��� ���� ���̵� ���� ������Ʈ�� ������������ȸ

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

void CMapTool::Picking()
{
	_int iID = -1;
	if (m_pGameInstance->Picking(&iID))
	{
		printf("ID: %d\n", iID);
		m_iSelectedHierarchyIndex = Find_HierarchyIndex_By_ID(iID);
		m_pSelectedObject = static_cast<CMapToolObject*>(Get_Selected_GameObject());
	}
}

void CMapTool::Control_PreviewObject(_float fTimeDelta)
{
	CCamera_Free* pCameraFree = CCamera_Manager::Get_Instance()->GetFreeCam(); //static_cast<CCamera_Free*> (m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Layer_Camera")));

	/*if (m_bPreviewHovered)
	{
		_int a = 10;
	}

	if (m_pGameInstance->Mouse_Pressing(DIM::RBUTTON))
	{
		_int a = 10;

	}*/

	if (m_bPreviewHovered && m_pGameInstance->Mouse_Pressing(DIM::RBUTTON))
	{
		pCameraFree->Set_Moveable(false);

		//CPreviewObject* pPreviewObject = static_cast<CPreviewObject*> (m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), TEXT("Layer_PreviewObject")));

		//if (pPreviewObject == nullptr)
		//	return;

		CTransform* pCamTransformCom = m_pPreviewObject->Get_CameraTransformCom();

		_float3 vPos = {};
		XMStoreFloat3(&vPos, pCamTransformCom->Get_State(STATE::POSITION));

		printf("x: %0.1f, y: %0.1f, z: %0.1f\n", vPos.x, vPos.y, vPos.z);

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
			pCamTransformCom->Go_Straight(fTimeDelta * 10.f);
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
		pCameraFree->Set_Moveable(true);
	}

}

void CMapTool::Detail_Transform()
{
	ImGui::Text("Transform");

	if (m_pSelectedObject != nullptr)
	{
		CTransform* pTransform = m_pSelectedObject->Get_TransfomCom();

		//���� ��ư
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			_float4x4 MatrixIdentity = {};
			XMStoreFloat4x4(&MatrixIdentity, XMMatrixIdentity());
			pTransform->Set_WorldMatrix(MatrixIdentity);
		}

		//��Ʈ�� Ŭ�� �ϸ� ��ŷ�� ��ġ�� �̵�
		if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Mouse_Down(DIM::LBUTTON))
		{
			_float4 vPickedPos = {};
			if (m_pGameInstance->Picking(&vPickedPos))
			{
				//���� ���� ��� ����
				m_pSelectedObject->Set_UndoWorldMatrix(pTransform->Get_WorldMatrix());

				pTransform->Set_State(STATE::POSITION, XMLoadFloat4(&vPickedPos));

			}
		}


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

		//���� �ȴ��Ȱ� ���� �������� ����
		if (m_bWasUsingGizmoLastFrame == false && ImGuizmo::IsUsing())
		{
			//���� ���� ��� ����
			m_pSelectedObject->Set_UndoWorldMatrix(pTransform->Get_WorldMatrix());

			m_bWasUsingGizmoLastFrame = true;
		}
		m_bWasUsingGizmoLastFrame = ImGuizmo::IsUsing();

		if (ImGuizmo::IsUsing())
		{
			// ImGuizmo�� ���۵� matrix �״�� ����
			memcpy(&worldMat, matrix, sizeof(_float) * 16);
			pTransform->Set_WorldMatrix(worldMat);
		}
		else if (bPositionChanged || bRotationChanged || bScaleChanged)
		{
			m_pSelectedObject->Set_UndoWorldMatrix(pTransform->Get_WorldMatrix());

			// ���� �Է����� �ٲ� �� �� matrix �籸�� �� ����
			ImGuizmo::RecomposeMatrixFromComponents(position, rotation, scale, matrix);
			memcpy(&worldMat, matrix, sizeof(_float) * 16);
			pTransform->Set_WorldMatrix(worldMat);
		}

#pragma endregion
	}
}

void CMapTool::Detail_Tile()
{
	if (m_pSelectedObject)
	{
		ImGui::Text("Tile Settings");

		// Ÿ�ϸ� ���� üũ�ڽ�
		ImGui::Checkbox("Enable Tiling", &m_pSelectedObject->m_bUseTiling);

		// Ÿ�ϸ� �� �����̴� (X, Z)
		if (m_pSelectedObject->m_bUseTiling)
		{
			ImGui::DragFloat2("Tiling (X,Z)", m_pSelectedObject->m_TileDensity, 0.01f, 0.01f, 32.0f, "%.2f");
		}
	}
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

}
