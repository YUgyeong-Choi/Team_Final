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
//ImGuiFileDialog::Instance() �̷� �̱������� ����� ���� ���ϰ� ��

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

	//���ã�� �ε�
	if (FAILED(Load_Favorite()))
		E_FAIL;

	//MapData�� ��ġ�Ǿ��ִ� �𵨵��� �̸� �ҷ����δ�.
	/*if (FAILED(Ready_Model()))
		return E_FAIL;*/

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

	//��Ŀ�� �ȳ� ������Ʈ ��������
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
	//��� ���� �ϴ°͵� ������
	if (m_bRenderAllCollider)
	{
		for (auto& Group : m_ModelGroups) // �� �̸����� �׷�ȭ�� GameObject ����� �ݺ�
		{
			for (auto pGameObject : Group.second)
			{
				if (FAILED(static_cast<CMapToolObject*>(pGameObject)->Render_Collider()))
					return E_FAIL;
			}
		}

	}
	//��Ŀ�� �ȳ� ����
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

	Change_Operation();

	DeselectObject();
	
	Change_ColliderType();

	SnapTo();

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


	//���콺 �巡�� ����
	if (m_pGameInstance->Mouse_Down(DIM::LBUTTON) && ImGuizmo::IsOver() == false)
	{
		m_vMouseDragStart = Get_MousePos();
		m_bDragging = true;
	}

	//�巡�� ��
	if (m_pGameInstance->Mouse_Up(DIM::LBUTTON))
	{
		m_bDragging = false;
	}

	//��ŷ���� �� ������Ʈ ���� �ϴ� ���(����� �ٸ� ��ü���� �ڿ� ������ �����Ϸ��� ��ü�� �ٲ����� IsOver()�� �ذ�)
	if (m_pGameInstance->Mouse_Up(DIM::LBUTTON) && ImGuizmo::IsOver() == false)
	{
		//��ƮŰ ������ ������ ��ŷ���� ����(������Ʈ ���̰��� ������Ʈ ����Ǵ°� ���� ������)
		if (m_pGameInstance->Key_Pressing(DIK_LALT))
			return;

		// ImGui�� ���콺 �Է��� �������� ��ŷ�� ���� ����
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

	//����ƮŰ ������ ���� ���õȰ� ����
	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		DeleteMapToolObject();
	}
	
	//F Ű������ �ش� ������Ʈ ��ġ�� �̵�
	if (m_pGameInstance->Key_Down(DIK_F))
	{
		if (m_SelectedObjects.empty() == false)
		{
			_vector vObjectPos = (*m_SelectedObjects.begin())->Get_TransfomCom()->Get_State(STATE::POSITION);
			_vector vCameraPos = XMVectorAdd(vObjectPos, XMVectorSet(0.f, 3.f, -3.f, 0.f));

			CTransform* pCameraTransformCom = CCamera_Manager::Get_Instance()->GetFreeCam()->Get_TransfomCom();

			//������ �ΰ� �̵�����
			pCameraTransformCom->Set_State(STATE::POSITION, vCameraPos);

			//LookAt ����
			pCameraTransformCom->LookAt(vObjectPos);
		}
	}

	Control_PreviewObject(fTimeDelta);

}

void CMapTool::Change_Operation()
{
	if (ImGuizmo::IsUsing() == false)
	{
		if (m_pGameInstance->Key_Pressing(DIK_LALT))
			return;

		if (m_pGameInstance->Key_Pressing(DIK_LCONTROL))
			return;

		//E ȸ��, R ũ��, T�� ��ġ
		if (m_pGameInstance->Key_Down(DIK_E))
			m_currentOperation = ImGuizmo::ROTATE;
		else if (m_pGameInstance->Key_Down(DIK_R))
			m_currentOperation = ImGuizmo::SCALE;
		else if (m_pGameInstance->Key_Down(DIK_T))
			m_currentOperation = ImGuizmo::TRANSLATE;
	}
}

void CMapTool::DeselectObject()
{
	if (m_pGameInstance->Mouse_Up(DIM::WHEELBUTTON))
	{
		//��� ������Ʈ ���� ����

		m_SelectedIndexies.clear();

		/*for (CMapToolObject* pObj : m_SelectedObjects)
			Safe_Release(pObj);*/
		m_SelectedObjects.clear();

		m_iFocusIndex = -1;
		//Safe_Release(m_pFocusObject);
		m_pFocusObject = nullptr;

	}
}

void CMapTool::Change_ColliderType()
{
	if (m_pFocusObject)
	{
		//C�� ������ 1(NONE), 2(CONVEX), 3(TRIANGLE) ���ϸ� �ݶ��̴� ����
		if (m_pGameInstance->Key_Pressing(DIK_C))
		{
			if (m_pGameInstance->Key_Up(DIK_1))
			{
				m_pFocusObject->Set_Collider(COLLIDER_TYPE::NONE);
			}
			if (m_pGameInstance->Key_Up(DIK_2))
			{
				m_pFocusObject->Set_Collider(COLLIDER_TYPE::CONVEX);
			}
			if (m_pGameInstance->Key_Up(DIK_3))
			{
				m_pFocusObject->Set_Collider(COLLIDER_TYPE::TRIANGLE);
			}
		}
	}
}

void CMapTool::SnapTo()
{
	//��Ʈ Ŭ�� �ϸ� ��ŷ�� ��ġ�� �̵�
	if (m_pGameInstance->Key_Pressing(DIK_LALT) && m_pGameInstance->Mouse_Up(DIM::LBUTTON))
	{
		if (m_pFocusObject)
		{
			CTransform* pTransform = m_pFocusObject->Get_TransfomCom();

			_float4 vPickedPos = {};
			if (m_pGameInstance->Picking(&vPickedPos))
			{
				//���� ���� ��� ����
				m_pFocusObject->Set_UndoWorldMatrix(pTransform->Get_WorldMatrix());
				pTransform->Set_State(STATE::POSITION, XMLoadFloat4(&vPickedPos));

			}
		}
	}
}

HRESULT CMapTool::Ready_Model(const _char* Map)
{
	string ResourcePath = string("../Bin/Save/MapTool/Resource_") + Map + ".json";

	ifstream inFile(ResourcePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Resource_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);

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

		if (FAILED(Load_Mesh(PrototypeTag, pModelFilePath)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CMapTool::Save(const _char* Map)
{
	string MapPath = string("../Bin/Save/MapTool/Map_") + Map + ".json";
	string ResourcePath = string("../Bin/Save/MapTool/Resource_") + Map + ".json";

	filesystem::create_directories("../Bin/Save/MapTool");
	ofstream MapDataFile(MapPath); //�ʺ��� �����͸� �����ؾ���, Map_Station; Map_Hotel; Map_Test;
	ofstream ReadyModelFile(ResourcePath);//�ʺ��ε� ���ҽ� ����, Resource_Station; Resource_Hotel; Resoucce_Test;

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

		json ReadyModelJson;
		ReadyModelJson["ModelName"] = strModelName;
		ReadyModelJson["Path"] = FullPath; // �Ǵ� Path
		ReadyModelJson["Collision"] = false;
		ReadyModelJson["ObjectCount"] = static_cast<_uint>(MapObjectList.size());//���� �����ؼ� �ν��Ͻ� ���� �ε����� ����(�ƴ� �浹���η� �Ұž�)

		// ���ڿ� ���� ���
		_uint iLength = static_cast<_uint>(ModelName.length());

		// ������Ʈ ���� ����
		_uint iObjectCount = static_cast<_uint>(MapObjectList.size());

		// �� JSON
		json ModelJson;
		ModelJson["ModelName"] = strModelName;
		ModelJson["ObjectCount"] = static_cast<_uint>(MapObjectList.size());
		ModelJson["Collision"] = false;
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

			ObjectJson["ColliderType"] = static_cast<_int>(pMapToolObject->m_eColliderType);
			//�ѹ��̶� �浹ü�� ������ �ν��Ͻ����� ���θ��� ���� ����
			if (pMapToolObject->m_eColliderType != COLLIDER_TYPE::NONE)
			{
				ModelJson["Collision"] = true;
				ReadyModelJson["Collision"] = true;
			}
			
			//����Ʈ ���
			ObjectJson["LightShape"] = pMapToolObject->m_iLightShape;

			//�ν��Ͻ� ����
			ModelJson["NoInstancing"] = pMapToolObject->m_bNoInstancing;
			ReadyModelJson["NoInstancing"] = pMapToolObject->m_bNoInstancing;

			ModelJson["Objects"].push_back(ObjectJson);
		}

		ReadyModelJsonArray.push_back(ReadyModelJson);
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

HRESULT CMapTool::Load(const _char* Map)
{
	//���� �ʿ� ��ġ�� ������Ʈ�� ��� ��������
	Clear_Map();

	//�ε� �� �ϱ����� ���� �غ�����
	Ready_Model(Map);

	string MapPath = string("../Bin/Save/MapTool/Map_") + Map + ".json";
	//string ResourcePath = string("../Bin/Save/MapTool/Resource_") + Map + ".json"; //���߿� ���� �� �ٲܶ�

	ifstream inFile(MapPath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Map_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);
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

			MapToolObjDesc.iID = ++m_iID;

			//Ÿ�ϸ�
			if (Objects[j].contains("TileDensity"))
			{
				MapToolObjDesc.bUseTiling = true;

				const json& TileDensityJson = Objects[j]["TileDensity"];
				MapToolObjDesc.vTileDensity = {
					TileDensityJson[0].get<_float>(),
					TileDensityJson[1].get<_float>()
				};
			}

			//�ݶ��̴�
			if (Objects[j].contains("ColliderType") && Objects[j]["ColliderType"].is_number_integer())
			{
				MapToolObjDesc.eColliderType = static_cast<COLLIDER_TYPE>(Objects[j]["ColliderType"].get<_int>());
			}
			else
				return E_FAIL;

			//����Ʈ ���
			MapToolObjDesc.iLightShape = Objects[j].value("LightShape", 0);

			//�ν��Ͻ� ���� ����
			if (Models[i].contains("NoInstancing"))
				MapToolObjDesc.bNoInstancing = Models[i]["NoInstancing"].get<_bool>();

			if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
				ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
				return E_FAIL;

			//��� �߰��Ѽ��� �� �׷쿡 �з��ؼ� ����
			Add_ModelGroup(ModelName, m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), LayerTag));

		}
	}

	return S_OK;
}

HRESULT CMapTool::Save_Favorite()
{
	//���ã�� �߰� ���� ��
	//���� ���ã�⿡ ���Ե� �̸����� �����Ѵ�.
	filesystem::create_directories("../Bin/Save/MapTool");
	ofstream Favorite("../Bin/Save/MapTool/Favorite.json");

	json FavoritJsonArray = json::array();

	//���ã�� ���
	//m_FavoriteModelNames;

	for (string& ModelName : m_FavoriteModelNames)
	{
		//Json ���Ͽ� ���� �̸��� ���ϰ�� ����
		filesystem::path path = filesystem::path(PATH_NONANIM) / (ModelName + ".bin"); //�̷��� �ϵ��ڵ� ���帻��
		string FullPath = path.generic_string();

		json ObjectJson;
		ObjectJson["ModelName"] = ModelName;
		ObjectJson["Path"] = FullPath; // �Ǵ� Path

		FavoritJsonArray.push_back(ObjectJson);

	}

	// ���Ͽ� JSON ����
	Favorite << FavoritJsonArray.dump(4);
	Favorite.close();

	//MSG_BOX("��� ã�� ���� ����");

	return S_OK;
}

HRESULT CMapTool::Load_Favorite()
{
	//������ ������ ��
	//���̽��� ����� �̸��� �𵨵��� ��� �ε��ϰ�
	//��� ã�� ��Ͽ� ���� ��Ų��.

	ifstream inFile("../Bin/Save/MapTool/Favorite.json");
	if (!inFile.is_open())
	{
		MSG_BOX("Favorite.json ������ �� �� �����ϴ�.");
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
		MessageBoxA(nullptr, e.what(), "JSON �Ľ� ����", MB_OK);
		return E_FAIL;
	}

	// JSON ������ Ȯ��
	for (const auto& element : FavoriteJson)
	{
		string ModelName = element.value("ModelName", "");
		string Path = element.value("Path", "");

		//�� ������ Ÿ�� ����
		wstring PrototypeTag = L"Prototype_Component_Model_" + StringToWString(ModelName);

		const _char* pModelFilePath = Path.c_str();

		if (FAILED(Load_Mesh(PrototypeTag, pModelFilePath)))
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

	//Render_File();

	//�巡�� �簢�� �׸���
	if (m_bDragging && ImGui::GetIO().WantCaptureMouse == false)
	{
		_float2 vDragEnd = Get_MousePos();

		ImDrawList* draw_list = ImGui::GetBackgroundDrawList(); // ��濡 �׷���
		ImVec2 start(m_vMouseDragStart.x, m_vMouseDragStart.y);
		ImVec2 end(vDragEnd.x, vDragEnd.y);

		draw_list->AddRectFilled(start, end, IM_COL32(100, 150, 255, 40));  // ������ �Ķ���
		draw_list->AddRect(start, end, IM_COL32(100, 150, 255, 255));       // �ܰ���
	}

	return S_OK;
}

void CMapTool::Render_Hierarchy()
{
#pragma region ���̾��Ű
	ImGui::Begin("Hierarchy", nullptr);

	if (ImGui::BeginListBox("##HierarchyList", ImVec2(-FLT_MIN, 300)))
	{
		_bool bHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

		if (bHovered)
		{
			_uint i = 0; // ��ü Hierarchy �׸� �ε����� ���� ī���� (���� �ε���)
			for (auto& group : m_ModelGroups) // �� �̸����� �׷�ȭ�� GameObject ����� �ݺ�
			{
				const string& ModelName = group.first; // ���� �׷��� �� �̸�
				// Ʈ�� ��� ���� (�����ְ� �⺻ ����) - �� �̸��� �������� �׷�ȭ�� �׸�
				_bool bOpen = ImGui::TreeNodeEx(ModelName.c_str());

				for (auto pGameObject : group.second)
				{
					// ���� ������Ʈ�� ���� �̸� ���� (ID �� ���� ���߿�)
					string strHierarchyName = "(ID:" + to_string(static_cast<CMapToolObject*>(pGameObject)->Get_ID()) + ')' + ModelName;

					// ���� �ε����� ���õ� �������� Ȯ��
					_bool isSelected = (m_SelectedIndexies.count(i) > 0);

					if (bOpen) // Ʈ�� ��尡 ���� ���� ���� Selectable �׸��� �׸���
					{
						// �ش� �׸��� Ŭ���Ǹ� �ε����� ����Ͽ� ���� ���·� �����
						if (ImGui::Selectable(strHierarchyName.c_str(), isSelected))
						{
							if (ImGui::GetIO().KeyCtrl)
							{
								// Ctrl �������� ���� ���
								if (isSelected)
								{
									m_SelectedObjects.erase(static_cast<CMapToolObject*>(pGameObject));
									//Safe_Release(pGameObject);

									m_SelectedIndexies.erase(i);
								}
								else
								{
									m_SelectedObjects.insert(static_cast<CMapToolObject*>(pGameObject));
									//Safe_AddRef(pGameObject);

									m_SelectedIndexies.insert(i);
								}
							}
							else
							{
								//for (CMapToolObject* pObj : m_SelectedObjects)
									//Safe_Release(pObj);
								m_SelectedObjects.clear();

								m_SelectedObjects.insert(static_cast<CMapToolObject*>(pGameObject));
								//Safe_AddRef(pGameObject);

								// Ctrl �� �������� ���� ����
								m_SelectedIndexies.clear();
								m_SelectedIndexies.insert(i);
							}

							//������ Ŭ���� �׸�
							m_iFocusIndex = i;

							//Safe_Release(m_pFocusObject);
							m_pFocusObject = static_cast<CMapToolObject*>(Get_Focused_Object());
							//Safe_AddRef(m_pFocusObject);
						}
					}

					// ��Ŀ���� ������ �׸�
					if (m_iFocusIndex == i)
						ImGui::SetItemDefaultFocus();

					++i; // ��ü �ε��� ���� (Ʈ�� ��� �����ֵ� ���� �������Ѿ� ��)
				}

				if (bOpen)
					ImGui::TreePop(); // Ʈ�� ��� �ݱ� (Ʈ�� UI�� �ݾ���)
			}


		}
		else
		{
			for (auto& group : m_ModelGroups)
			{
				const string& ModelName = group.first;
				ImGui::TreeNodeEx(ModelName.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
			}
		}

		ImGui::EndListBox(); // ����Ʈ�ڽ� ��

	}
#pragma endregion

	if (ImGui::Button("Delete"))
	{
		DeleteMapToolObject();
	}

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
			if (FAILED(Add_Favorite(m_ModelNames[m_iSelectedModelIndex], true)))
				MSG_BOX("���ã�� �߰� ����");
		}
	}

	ImGui::SameLine();
	// ������ ��ư ��Ÿ�� ����
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); // ��ư ����
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // ȣ�� ��
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f)); // Ŭ�� ��
	if (ImGui::Button("Clear List"))
	{
		m_iSelectedModelIndex = -1;
		m_ModelNames.clear();
	}

	ImGui::PopStyleColor(3); // ������ ���� 3�� �ǵ�����


	// â ��Ŀ�� ����
	const _bool bWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootAndChildWindows);

	if (ImGui::BeginListBox("##Model List", ImVec2(-FLT_MIN, 100)))
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

						//Lod ����
						if (strPrototypeTag.find("Lod1") != string::npos || strPrototypeTag.find("Lod2") != string::npos)
							continue;

						string ModelFilePath = ModelPath.string();
						const _char* pModelFilePath = ModelFilePath.c_str();

						if (FAILED(Load_Mesh(PrototypeTag, pModelFilePath)))
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

				if (FAILED(Save_Favorite()))
				{
					MSG_BOX("��� ã�� ���� ����");
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

	Detail_Name();

	ImGui::Separator();

	Detail_Transform();

	ImGui::Separator();

	Detail_Collider();

	ImGui::Separator();

	Detail_Tile();

	ImGui::Separator();
	
	Detail_LightShape();

	ImGui::Separator();

	Detail_NoInstancing();

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
	else
	{
		// â�� ���� ������ ��
		m_bPreviewHovered = false;
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

	MapToolObjDesc.iID = ++m_iID;

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

	// ������Ʈ ���� ��Ŀ� ����
	XMStoreFloat4x4(&MapToolObjDesc.WorldMatrix, SpawnWorldMatrix);
#pragma endregion

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
		ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
		return E_FAIL;

	//��� �߰��Ѽ��� �� �׷쿡 �з��ؼ� ����
	Add_ModelGroup(WStringToString(ModelName), m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), LayerTag));

	//��Ŀ�� ����
	m_iFocusIndex = Find_HierarchyIndex_By_ID(MapToolObjDesc.iID);

	//Safe_Release(m_pFocusObject);
	m_pFocusObject = Find_Object_By_Index(m_iFocusIndex);
	//Safe_AddRef(m_pFocusObject);

	//���õȰ͵� ��� �ʱ�ȭ �� ���� �߰��Ѱ� �ֱ�
	m_SelectedIndexies.clear();

	/*for (CMapToolObject* pObj : m_SelectedObjects)
		Safe_Release(pObj);*/
	m_SelectedObjects.clear();

	m_SelectedIndexies.insert(m_iFocusIndex);
	m_SelectedObjects.insert(m_pFocusObject);
	//Safe_AddRef(m_pFocusObject);

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

	MapToolObjDesc.iID = ++m_iID;

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

	// ������Ʈ ���� ��Ŀ� ����
	XMStoreFloat4x4(&MapToolObjDesc.WorldMatrix, SpawnWorldMatrix);
#pragma endregion

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
		ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
		return E_FAIL;

	//��� �߰��Ѽ��� �� �׷쿡 �з��ؼ� ����
	Add_ModelGroup(ModelName, m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), LayerTag));

	//��Ŀ�� ����
	m_iFocusIndex = Find_HierarchyIndex_By_ID(MapToolObjDesc.iID);

	//Safe_Release(m_pFocusObject);
	m_pFocusObject = Find_Object_By_Index(m_iFocusIndex);
	//Safe_AddRef(m_pFocusObject);

	//���õȰ͵� ��� �ʱ�ȭ �� ���� �߰��Ѱ� �ֱ�
	m_SelectedIndexies.clear();

	for (CMapToolObject* pObj : m_SelectedObjects)
		//Safe_Release(pObj);
	m_SelectedObjects.clear();

	m_SelectedIndexies.insert(m_iFocusIndex);
	m_SelectedObjects.insert(m_pFocusObject);
	//Safe_AddRef(m_pFocusObject);

	return S_OK;
}

HRESULT CMapTool::Duplicate_Selected_Object()
{
	//���� ���õ� ������Ʈ�� �ʿ�����Ʈ�� ����
	/*CMapToolObject* pMapToolObject = static_cast<CMapToolObject*>(Get_Focused_Object());
	if (pMapToolObject == nullptr)
		return S_OK;*/

	if (m_SelectedObjects.empty())
		return E_FAIL;

	//�ӽ� ����
	set<CMapToolObject*> pTempSelectObjects = m_SelectedObjects;

	//���õȰ͵� ��� �ʱ�ȭ �� �����Ѱ͸� �ֱ�
	m_SelectedIndexies.clear();

	/*for (CMapToolObject* pObj : m_SelectedObjects)
		Safe_Release(pObj);*/
	m_SelectedObjects.clear();

	for (CMapToolObject* pObj : pTempSelectObjects)
	{
		CMapToolObject::MAPTOOLOBJ_DESC MapToolObjDesc = {};

		//TEXT("Prototype_Component_Model_���̸�"),
		string ModelName = pObj->Get_ModelName();

		lstrcpy(MapToolObjDesc.szModelPrototypeTag, pObj->Get_ModelPrototypeTag().c_str());
		lstrcpy(MapToolObjDesc.szModelName, StringToWString(ModelName).c_str());
		/*
		������ ������ �� � ���̾ ��Ҵ��� �����ϴ� ���̾� ����Ʈ�� �ʿ���
		������ �� ���̾ �����ϴ� �� Ȯ��(���̾ ������ �������� �ʴٴ� ��)
		�� ���̾ ������� ����Ʈ���� ���� �ؾ���
		*/
		wstring LayerTag = TEXT("Layer_MapToolObject_");
		LayerTag += StringToWString(ModelName);

		MapToolObjDesc.iID = ++m_iID;

#pragma region �ش� ������Ʈ �����ٰ� ��ȯ
		_matrix SpawnWorldMatrix = pObj->Get_TransfomCom()->Get_WorldMatrix();

		//// x������ 3.f ��ŭ �̵��ϴ� ��ȯ ���
		//_matrix matOffset = XMMatrixTranslation(3.f, 0.f, 0.f);

		//// ��ȯ ����: ���� ��Ŀ� offset�� �����ش�
		//_matrix matResult = matOffset * SpawnWorldMatrix;

		// ��� ����
		XMStoreFloat4x4(&MapToolObjDesc.WorldMatrix, SpawnWorldMatrix);
#pragma endregion

		if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_MapToolObject"),
			ENUM_CLASS(LEVEL::YW), LayerTag, &MapToolObjDesc)))
			return E_FAIL;

		//��� �߰��� ���� �� �׷쿡 �з��ؼ� ����
		CGameObject* pLastObject = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::YW), LayerTag);

		Add_ModelGroup(ModelName, pLastObject);
		m_iFocusIndex = Find_HierarchyIndex_By_ID(m_iID);

		//��Ŀ�� �Ǵ� ������Ʈ ����
		//Safe_Release(m_pFocusObject);
		m_pFocusObject = static_cast<CMapToolObject*>(pLastObject);
		//Safe_AddRef(m_pFocusObject);

		//������ �� �ֱ�
		m_SelectedIndexies.insert(m_iFocusIndex);
		m_SelectedObjects.insert(m_pFocusObject);
		//Safe_AddRef(m_pFocusObject);
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
		//m_SelectedObjects ������
		//Safe_Release(pObj);
		//�׷쿡�� ����
		Delete_ModelGroup(pObj);
		//������ ����
		pObj->Set_bDead();
	}

	m_SelectedIndexies.clear();
	m_SelectedObjects.clear();

	m_iFocusIndex = -1;

	//��Ŀ�� ������
	//Safe_Release(m_pFocusObject);
	m_pFocusObject = nullptr;
}

void CMapTool::Clear_Map()
{
	for (auto Group : m_ModelGroups)
	{
		for (auto pObj : Group.second)
		{
			pObj->Set_bDead();
			//Safe_Release(pObj);
		}
			
		Group.second.clear();
	}
	m_ModelGroups.clear();

	//Safe_Release(m_pFocusObject);
	m_pFocusObject = nullptr;

	/*for (CMapToolObject* pObj : m_SelectedObjects)
		Safe_Release(pObj);*/
	m_SelectedObjects.clear();


}
HRESULT CMapTool::Load_Mesh(const wstring& strPrototypeTag, const _char* pModelFilePath)
{
	_matrix PreTransformMatrix = XMMatrixScaling(PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE);

	// ���� ��ο��� ���丮, ���ϸ�, Ȯ���� �и�
	wstring wsBasePath = filesystem::path(pModelFilePath).parent_path();
	wstring wsFilename = filesystem::path(pModelFilePath).stem();	// Ȯ���� �� ���ϸ�
	wstring wsExtension = filesystem::path(pModelFilePath).extension();

	// LOD0 �� ��� ����
	string strModelPathLod0 = (filesystem::path(wsBasePath) / filesystem::path(wsFilename + wsExtension)).string();

	// LOD1, LOD2 ���ϸ� ���� (_Lod1, _Lod2 ����)
	wstring wsLod1Name = wsFilename + L"_Lod1" + wsExtension;
	wstring wsLod2Name = wsFilename + L"_Lod2" + wsExtension;

	// LOD1, LOD2 ��� ����
	filesystem::path Lod1Path = filesystem::path(wsBasePath) / wsLod1Name;
	filesystem::path Lod2Path = filesystem::path(wsBasePath) / wsLod2Name;
	string strModelPathLod1 = Lod1Path.string();
	string strModelPathLod2 = Lod2Path.string();

	// LOD0 ������Ÿ�� ���� ���� Ȯ�� �� ������ �ε�
	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::YW), strPrototypeTag) == nullptr)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), strPrototypeTag,
			CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelPathLod0.c_str(), PreTransformMatrix))))
			return E_FAIL;
	}

	// LOD1 ������Ÿ�� �±� ���� �� �ε� (���� �����ϸ�)
	wstring wsPrototypeTagLod1 = strPrototypeTag + L"_Lod1";
	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::YW), wsPrototypeTagLod1) == nullptr)
	{
		if (filesystem::exists(strModelPathLod1))
		{
			if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), wsPrototypeTagLod1,
				CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelPathLod1.c_str(), PreTransformMatrix))))
				return S_OK; // �����ص� ����
		}
	}

	// LOD2 ������Ÿ�� �±� ���� �� �ε� (���� �����ϸ�)
	wstring wsPrototypeTagLod2 = strPrototypeTag + L"_Lod2";
	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::YW), wsPrototypeTagLod2) == nullptr)
	{
		if (filesystem::exists(strModelPathLod2))
		{
			if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::YW), wsPrototypeTagLod2,
				CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelPathLod2.c_str(), PreTransformMatrix))))
				return S_OK; // �����ص� ����
		}
	}

	return S_OK;
}



void CMapTool::Add_ModelGroup(string ModelName, CGameObject* pMapToolObject)
{
	//Safe_AddRef(pMapToolObject);
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

	//Safe_Release(pMapToolObject);
	// ����Ʈ���� �ش� ������Ʈ ����
	objList.remove(pMapToolObject);

	// ���� �ش� �׷��� ����ٸ� map���� �׷� ��ü�� ����
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

	return nullptr; // �ε��� �ʰ� �� null
}

void CMapTool::SelectByClick()
{
	_int iID = -1;
	if (m_pGameInstance->PickByClick(&iID))
	{
		printf("ID: %d\n", iID);
		m_iFocusIndex = Find_HierarchyIndex_By_ID(iID);

		//Safe_Release(m_pFocusObject);
		m_pFocusObject = static_cast<CMapToolObject*>(Get_Focused_Object());
		//Safe_AddRef(m_pFocusObject);

		if (m_pFocusObject == nullptr) //���� �޽� �߸� Ŭ������ �� null����
		{
			m_iFocusIndex = -1;
			return;
		}

		//���߼��� ����
		if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) == false)
		{
			//���� ���õ� �͵� Ŭ����
			m_SelectedIndexies.clear();
			/*for (CMapToolObject* pObj : m_SelectedObjects)
				Safe_Release(pObj);*/
			m_SelectedObjects.clear();
		}

		//���� �߰�
		m_SelectedIndexies.insert(Find_HierarchyIndex_By_ID(m_pFocusObject->Get_ID()));
		m_SelectedObjects.insert(m_pFocusObject);
		//Safe_AddRef(m_pFocusObject);
	}
}

void CMapTool::SelectByDrag(const _float2& vMouseDragEnd)
{
	set<_int> IDs = {};

	if (m_pGameInstance->PickInRect(m_vMouseDragStart, vMouseDragEnd, &IDs))
	{
		//���� ���õ� �͵� Ŭ����
		m_SelectedIndexies.clear();
		/*for (CMapToolObject* pObj : m_SelectedObjects)
			Safe_Release(pObj);*/
		m_SelectedObjects.clear();

		_bool bSetFocusObject = { false };

		for (_int iID : IDs)
		{
			//���̵� 0�� ����Ʈ ��, ������ ������
			if (iID <= 0)
				continue;

			//���� �߰�
			_int iIndex = Find_HierarchyIndex_By_ID(iID);

			m_SelectedIndexies.insert(iIndex);

			CMapToolObject* pObj = Find_Object_By_Index(iIndex);
			m_SelectedObjects.insert(pObj);
			//Safe_AddRef(pObj);

			if (bSetFocusObject == false)
			{
				bSetFocusObject = true;
				m_iFocusIndex = iIndex;

				//Safe_Release(m_pFocusObject);
				m_pFocusObject = pObj;
				//Safe_AddRef(m_pFocusObject);
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

		//��� ���� ��ư
		ImGui::SameLine();
		if (ImGui::Button("Copy"))
		{
			XMStoreFloat4x4(&m_CopyWorldMatrix, m_pFocusObject->Get_TransfomCom()->Get_WorldMatrix());
		}

		//��� ���̱� ��ư
		ImGui::SameLine();
		if (ImGui::Button("Paste"))
		{
			m_pFocusObject->Get_TransfomCom()->Set_WorldMatrix(XMLoadFloat4x4(&m_CopyWorldMatrix));
		}

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

		//���� �ȴ��Ȱ� ���� �������� ����
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
			// ImGuizmo�� ���۵� matrix �״�� ����
			memcpy(&worldMat, matrix, sizeof(_float) * 16);

			// ��ǥ ������Ʈ�� ���� ��İ� �� ��� ��
			_matrix matPrevMain = m_pFocusObject->Get_TransfomCom()->Get_WorldMatrix();
			_matrix matNewMain = XMLoadFloat4x4(&worldMat);

			// ��ǥ ������Ʈ�� �̵�/ȸ��/������ ��ȭ ��� ���
			_matrix matOffset = matNewMain * XMMatrixInverse(nullptr, matPrevMain);

			pTransform->Set_WorldMatrix(worldMat);

			//���õ� ��������Ʈ�� ���� �����̰�
			for (CGameObject* pObj : m_SelectedObjects)
			{
				if (pObj == m_pFocusObject)
					continue;

				_matrix matOld = pObj->Get_TransfomCom()->Get_WorldMatrix();
				_matrix matNew = matOffset * matOld;

				pObj->Get_TransfomCom()->Set_WorldMatrix(matNew);
				static_cast<CMapToolObject*>(pObj)->Update_ColliderPos();
			}

		}
		else if (bPositionChanged || bRotationChanged || bScaleChanged)
		{
			m_pFocusObject->Set_UndoWorldMatrix(pTransform->Get_WorldMatrix());

			// ���� �Է����� �ٲ� �� �� matrix �籸�� �� ����
			ImGuizmo::RecomposeMatrixFromComponents(position, rotation, scale, matrix);
			memcpy(&worldMat, matrix, sizeof(_float) * 16);
			pTransform->Set_WorldMatrix(worldMat);
			m_pFocusObject->Update_ColliderPos();
		}

#pragma endregion
	}
}

void CMapTool::Detail_Tile()
{
	ImGui::Text("Tile");
	if (m_pFocusObject)
	{
		// Ÿ�ϸ� ���� üũ�ڽ�
		ImGui::Checkbox("Enable Tiling", &m_pFocusObject->m_bUseTiling);

		// Ÿ�ϸ� �� �����̴� 
		if (m_pFocusObject->m_bUseTiling)
		{
			ImGui::DragFloat2("Tiling (U,V)", m_pFocusObject->m_TileDensity, 0.01f, 0.01f, 32.0f, "%.2f");
		}
	}
}

void CMapTool::Detail_Collider()
{
	ImGui::Text("Collider");
	ImGui::SameLine();

	if (m_bRenderAllCollider)
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));  // ������
	else
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));  // �ʷϻ�

	if (ImGui::Button(m_bRenderAllCollider ? "Show All" : "Focus Only"))
	{
		m_bRenderAllCollider = !m_bRenderAllCollider;
	}
	ImGui::PopStyleColor();

	if (m_pFocusObject)
	{
		// �ݶ��̴� Ÿ�� ���� �޺��ڽ�
		const _char* ColliderTypes[] = { "None", "Convex", "Triangle" };
		_int CurrentCollider = static_cast<_int>(m_pFocusObject->m_eColliderType);

		if (ImGui::Combo("Collider Type", &CurrentCollider, ColliderTypes, IM_ARRAYSIZE(ColliderTypes)))
		{
			//m_pFocusObject->Set_Collider(static_cast<COLLIDER_TYPE>(CurrentCollider));

			//���õ� �ֵ� ��� ����
			for (CMapToolObject* pObj : m_SelectedObjects)
			{
				pObj->Set_Collider(static_cast<COLLIDER_TYPE>(CurrentCollider));
			}
		}

	}
}

void CMapTool::Detail_LightShape()
{
	ImGui::Text("Light Shape");
	if (m_pFocusObject)
	{
		if (ImGui::InputInt("Light Shape", &m_pFocusObject->m_iLightShape))
		{
			//üũ �ڽ��� ������ ��
			//��Ŀ���� ������Ʈ�� ���� �𵨵��� ���� �Ӽ� �ο� ��Ų��.
			auto iter = m_ModelGroups.find(m_pFocusObject->Get_ModelName());
			if (iter != m_ModelGroups.end())
			{
				// it->second �� �ش� �� �̸��� ����Ʈ
				list<CGameObject*>& ObjectList = iter->second;

				for (auto* pObj : ObjectList)
				{
					static_cast<CMapToolObject*>(pObj)->m_iLightShape = m_pFocusObject->m_iLightShape;
				}
			}
			else
			{
				//ġ���� ����
				MSG_BOX("Detail_No Instancing Error : �� �׷��� ã�� �� ����");
			}
		}
	}
}

void CMapTool::Detail_NoInstancing()
{
	ImGui::Text("No Instancing");
	if (m_pFocusObject)
	{
		if (ImGui::Checkbox("No Instancing", &m_pFocusObject->m_bNoInstancing))
		{
			//üũ �ڽ��� ������ ��
			//��Ŀ���� ������Ʈ�� ���� �𵨵��� ���� �Ӽ� �ο� ��Ų��.
			auto iter = m_ModelGroups.find(m_pFocusObject->Get_ModelName());
			if (iter != m_ModelGroups.end())
			{
				// it->second �� �ش� �� �̸��� ����Ʈ
				list<CGameObject*>& ObjectList = iter->second;

				for (auto* pObj : ObjectList)
				{
					static_cast<CMapToolObject*>(pObj)->m_bNoInstancing = m_pFocusObject->m_bNoInstancing;
				}
			}
			else
			{
				//ġ���� ����
				MSG_BOX("Detail_No Instancing Error : �� �׷��� ã�� �� ����");
			}
			
		}
	}

}

HRESULT CMapTool::Add_Favorite(const string& ModelName, _bool bSave)
{
	// �̹� �̸��� ��Ͽ� �����ϸ� �ߺ� �߰����� ����
	if (find(m_FavoriteModelNames.begin(), m_FavoriteModelNames.end(), ModelName) == m_FavoriteModelNames.end())
	{
		m_FavoriteModelNames.push_back(ModelName); // �̸� �߰�

		if (bSave)
		{
			if (FAILED(Save_Favorite()))
			{
				return E_FAIL;
				MSG_BOX("��� ã�� ���� ����");
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
		/*for (auto pObj : Group.second )
			Safe_Release(pObj);*/
		Group.second.clear();
	}
	m_ModelGroups.clear();

	//Safe_Release(m_pFocusObject);

	/*for (CMapToolObject* pObj : m_SelectedObjects)
		Safe_Release(pObj);*/

	m_SelectedObjects.clear();
}
