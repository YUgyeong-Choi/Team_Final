#include "MapTool.h"
#include "GameInstance.h"

#include <filesystem>
#include "MapToolObject.h"

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
		filesystem::path path = filesystem::path(PATH_NONANIM) / (strModelName + ".bin");
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
#pragma region ���̾��Ű
	ImGui::Begin("Hierarchy", nullptr);

	//if (ImGui::BeginListBox("##HierarchyList", ImVec2(-FLT_MIN, 300)))
	//{
	//	_uint i = 0; // ��ü Hierarchy �׸� �ε����� ���� ī���� (���� �ε���)
	//	for (auto& group : m_ModelGroups) // �� �̸����� �׷�ȭ�� GameObject ����� �ݺ�
	//	{
	//		const string& ModelName = group.first; // ���� �׷��� �� �̸�
	//		// Ʈ�� ��� ���� (�����ְ� �⺻ ����) - �� �̸��� �������� �׷�ȭ�� �׸�
	//		_bool bOpen = ImGui::TreeNodeEx(ModelName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

	//		//_uint j = 0; // �� �� �׷� �������� ���� ������Ʈ �ε���
	//		for (auto pGameObject : group.second)
	//		{
	//			// ���� ������Ʈ�� ���� �̸� ���� (ID �� ���� ���߿�)
	//			string strHierarchyName = ModelName + '_';// + to_string(ID);

	//			// ���� �ε����� ���õ� �������� Ȯ��
	//			_bool isSelected = (m_iSelectedHierarchyIndex == i);

	//			if (bOpen) // Ʈ�� ��尡 ���� ���� ���� Selectable �׸��� �׸���
	//			{
	//				// �ش� �׸��� Ŭ���Ǹ� �ε����� ����Ͽ� ���� ���·� �����
	//				if (ImGui::Selectable(strHierarchyName.c_str(), isSelected))
	//				{
	//					m_iSelectedHierarchyIndex = i; // ���� ���õ� �׸����� ����
	//				}

	//				if (isSelected)
	//					ImGui::SetItemDefaultFocus(); // ��Ŀ���� �ش� �׸� �����ش� (Ű���� �׺���̼ǿ�)
	//			}

	//			++i; // ��ü �ε��� ���� (Ʈ�� ��� �����ֵ� ���� �������Ѿ� ��)
	//		}

	//		if (bOpen)
	//			ImGui::TreePop(); // Ʈ�� ��� �ݱ� (Ʈ�� UI�� �ݾ���)
	//	}

	//	ImGui::EndListBox(); // ����Ʈ�ڽ� ��

	//}


	if (ImGui::Button("Save Map"))
	{
		if (FAILED(Save_Map()))
			MSG_BOX("�� ���� ����");
	}


	if (ImGui::Button("Load Map"))
	{

	}
	ImGui::End();
#pragma endregion
}

void CMapTool::Asset()
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

	//���� ���õ� �𵨷� �ʿ�����Ʈ�� ����

	CMapToolObject::MAPTOOLOBJ_DESC MapToolObjDesc = {};

	//TEXT("Prototype_Component_Model_���̸�"),
	wstring ModelName = wstring(m_ModelNames[m_iSelectedModelIndex].begin(), m_ModelNames[m_iSelectedModelIndex].end());
	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
	ModelPrototypeTag += ModelName;
	lstrcpy(MapToolObjDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	/*
	������ ������ �� � ���̾ ��Ҵ��� �����ϴ� ���̾� ����Ʈ�� �ʿ���
	������ �� ���̾ �����ϴ� �� Ȯ��(���̾ ������ �������� �ʴٴ� ��)
	�� ���̾ ������� ����Ʈ���� ���� �ؾ���
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
	//�̹� ������Ÿ���������ϴ� ��Ȯ��
	
	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::YW), strPrototypeTag) != nullptr)
	{
		MSG_BOX("�̹� ������Ÿ���� ������");
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
	//// ���� ������ �ʱ�ȭ
	//m_ModelGroups.clear();      // �� �̸����� �׷�ȭ�� GameObject ����Ʈ �ʱ�ȭ
	//m_HierarchyNames.clear();   // Hierarchy UI���� ����� �̸� ����Ʈ �ʱ�ȭ

	//// ���� ����(YW)�� ��� ���̾�鿡 ���� �ݺ�
	//for (auto& pLayer : m_pGameInstance->Get_Layers(ENUM_CLASS(LEVEL::YW)))
	//{
	//	// ���̾� �̸��� "MapObject"�� ���Ե��� ������ ���� (�� �����Ϳ� ��ü�� ���͸�)
	//	if (pLayer.first.find(L"MapToolObject") == wstring::npos)
	//		continue;

	//	// �ش� ���̾��� ��� GameObject�� ���� �ݺ�
	//	for (auto pGameObject : pLayer.second->Get_GameObjects())
	//	{
	//		// �� ������Ʈ �̸����� "Prototype_Component_Model_" ���λ縦 �������� �𵨸� ����
	//		wstring prefix = L"Prototype_Component_Model_";
	//		wstring wstrModelCom = static_cast<CMapToolObject*>(pGameObject)->Get_ModelCom(); // �� �̸� ���

	//		// ���λ簡 ���� ���� ����
	//		size_t PosPrefix = wstrModelCom.find(prefix);
	//		if (PosPrefix == wstring::npos)
	//			continue;

	//		// ���λ� ������ ���� �� �̸� �κ��� �߶󳻾� string���� ��ȯ
	//		string ModelName = WStringToString(wstrModelCom.substr(PosPrefix + prefix.length()));

	//		// �� �̸��� Ű�� �Ͽ� �׷쿡 GameObject�� �߰�
	//		m_ModelGroups[ModelName].push_back(pGameObject);
	//	}
	//}

	//// �� �׷��κ��� UI�� ����� Hierarchy �̸����� ����
	//_uint i = 0; // ��ü �ε���
	//for (auto& group : m_ModelGroups)
	//{
	//	_uint j = 0; // �� �� �׷� �� �ε���
	//	for (auto pGameObject : group.second)
	//	{
	//		// "���̸�_����" ������ �̸� �����Ͽ� Hierarchy UI �׸����� ���
	//		string strHierarchyName = group.first + "_" + to_string(j++);

	//		// �̸� ����Ʈ�� �߰�
	//		m_HierarchyNames.push_back(strHierarchyName);

	//		++i; // ��ü �ε��� ����
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
