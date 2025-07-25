#include "MapTool.h"
#include "GameInstance.h"

#include <filesystem>
#include "MapToolObject.h"

#include "Layer.h"

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

	return S_OK;
}

void CMapTool::Priority_Update(_float fTimeDelta)
{

}

void CMapTool::Update(_float fTimeDelta)
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

			ModelJson["Objects"].push_back({ {"WorldMatrix", MatrixJson} });
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
		CGameObject* pGameObject = Get_Selected_GameObject();

		if (nullptr != pGameObject)
		{
			//�׷쿡�� ����
			Delete_ModelGroup(pGameObject);
			//������ ����
			pGameObject->Set_bDead();
		}
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
				m_iSelectedModelIndex = i;
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
	if (ImGui::Button("Load Model"))
	{
		IGFD::FileDialogConfig config;
		config.path = PATH_NONANIM;
		config.countSelectionMax = 1;

		IFILEDIALOG->OpenDialog("Load Model", "Load .bin model", ".bin", config);
	}

	// �� �����Ӹ��� Display ȣ��
	if (IFILEDIALOG->Display("Load Model"))
	{
		if (IFILEDIALOG->IsOk())
		{
			filesystem::path ModelPath = IFILEDIALOG->GetFilePathName();

			if (!ModelPath.empty())
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


		IFILEDIALOG->Close();
	}

	ImGui::End();

#pragma endregion
}

void CMapTool::Render_Detail()
{
#pragma region ������
	ImGui::Begin("Detail", nullptr);

	ImGui::Separator();

	ImGui::Text("Transform");

	CGameObject* pGameObject = Get_Selected_GameObject();
	if (pGameObject != nullptr)
	{
		CTransform* pTransform = static_cast<CTransform*>(pGameObject->Get_Component(g_strTransformTag));

		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			_float4x4 MatrixIdentity = {};
			XMStoreFloat4x4(&MatrixIdentity, XMMatrixIdentity());
			pTransform->Set_WorldMatrix(MatrixIdentity);
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
		if (ImGuizmo::IsUsing())
		{
			// ImGuizmo�� ���۵� matrix �״�� ����
			memcpy(&worldMat, matrix, sizeof(_float) * 16);
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

	ImGui::End();
#pragma endregion
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

HRESULT CMapTool::Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath)
{
	//�̹� ������Ÿ���������ϴ� ��Ȯ��
	
	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::YW), strPrototypeTag) != nullptr)
	{
		MSG_BOX("�̹� ������Ÿ���� ������");
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
