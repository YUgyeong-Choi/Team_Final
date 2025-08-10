#include "Level_DH.h"
#include "GameInstance.h"
#include "DHTool.h"
#include "Camera_Manager.h"
#include "Level_Loading.h"
#include "PBRMesh.h"

#include "StaticMesh.h"
#include "StaticMesh_Instance.h"

CLevel_DH::CLevel_DH(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{

}

HRESULT CLevel_DH::Initialize()
{
	if (FAILED(Ready_ImGui()))
		return E_FAIL;

	if (FAILED(Ready_ImGuiTools()))
		return E_FAIL;

	if (FAILED(Ready_Lights()))
		return E_FAIL;
	
	if (FAILED(Ready_Shadow()))
		return E_FAIL;


#pragma region �� �ε�
	//���� �����ϱ����� �� ������Ÿ���� �غ��Ѵ�.
	if (FAILED(Ready_MapModel()))
		return E_FAIL;

	//���̽����� ����� ���� �ε��Ѵ�.
	if (FAILED(LoadMap()))
		return E_FAIL;
#pragma endregion



	if (FAILED(Ready_Layer_Sky(TEXT("Layer_Sky"))))
		return E_FAIL;

	if (FAILED(Ready_Camera()))
		return E_FAIL;

	//if (FAILED(Ready_Layer_StaticMesh(TEXT("Layer_StaticMesh"))))
	//	return E_FAIL;

	m_pGameInstance->SetCurrentLevelIndex(ENUM_CLASS(LEVEL::DH));
	return S_OK;
}

void CLevel_DH::Priority_Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_F1))
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOGO))))
			return;
	}
}

void CLevel_DH::Update(_float fTimeDelta)
{

	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::DONGHA)]->Update(fTimeDelta);
	m_pCamera_Manager->Update(fTimeDelta);
	__super::Update(fTimeDelta);
}

void CLevel_DH::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

}

HRESULT CLevel_DH::Render()
{
	SetWindowText(g_hWnd, TEXT("���� �����Դϴ�."));

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGuiIO& io = ImGui::GetIO();


	RECT rect;
	GetClientRect(g_hWnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	io.DisplaySize = ImVec2((float)width, (float)height);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	ImGui_Render();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return S_OK;
}



#pragma region �� �ε�

HRESULT CLevel_DH::Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath, _bool bInstance)
{
	//�̹� ������Ÿ���������ϴ� ��Ȯ��

	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::DH), strPrototypeTag) != nullptr)
	{
		//MSG_BOX("�̹� ������Ÿ���� ������");
		return S_OK;
	}

	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE);

	if (bInstance == false)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), strPrototypeTag,
			CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::DH), strPrototypeTag,
			CModel_Instance::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
			return E_FAIL;
	}


	return S_OK;
}

HRESULT CLevel_DH::Ready_MapModel()
{
	ifstream inFile("../Bin/Save/MapTool/Resource_STATION.json");
	if (!inFile.is_open())
	{
		MSG_BOX("Resource_STATION.json ������ �� �� �����ϴ�.");
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

		//������ �����ؼ� �ν��Ͻ��� �� ������Ÿ���� ������ �����ؾ��ҵ�
		_uint iObjectCount = element["ObjectCount"];

		wstring PrototypeTag = {};
		_bool bInstance = false;
		if (iObjectCount > INSTANCE_THRESHOLD)
		{
			//�ν��Ͻ̿� �� ������ Ÿ�� ����
			PrototypeTag = L"Prototype_Component_Model_Instance" + StringToWString(ModelName);
			bInstance = true;

		}
		else
		{
			//�� ������ Ÿ�� ����
			PrototypeTag = L"Prototype_ComponentModel" + StringToWString(ModelName);
			bInstance = false;
		}


		const _char* pModelFilePath = Path.c_str();

		if (FAILED(Load_Model(PrototypeTag, pModelFilePath, bInstance)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}
HRESULT CLevel_DH::LoadMap()
{
	ifstream inFile("../Bin/Save/MapTool/Map_STATION.json");
	if (!inFile.is_open())
	{
		MSG_BOX("Map_STATION.json ������ �� �� �����ϴ�.");
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
		_uint iObjectCount = Models[i]["ObjectCount"]; //������Ʈ ���������� �ν��Ͻ��� ���� ���� �����ؾ߰ڴ�.
		const json& objects = Models[i]["Objects"];

		//���� ���� �̻��̸� �ν��Ͻ̿�����Ʈ�� �ε�
		if (iObjectCount > INSTANCE_THRESHOLD)
		{
			Load_StaticMesh_Instance(iObjectCount, objects, ModelName);
		}
		else
		{
			Load_StaticMesh(iObjectCount, objects, ModelName);
		}
	}

	return S_OK;
}

HRESULT CLevel_DH::Load_StaticMesh(_uint iObjectCount, const json& objects, string ModelName)
{
	for (_uint j = 0; j < iObjectCount; ++j)
	{
#pragma region �������
		CStaticMesh::STATICMESH_DESC StaticMeshDesc = {};

		const json& WorldMatrixJson = objects[j]["WorldMatrix"];
		_float4x4 WorldMatrix = {};

		for (_int row = 0; row < 4; ++row)
			for (_int col = 0; col < 4; ++col)
				WorldMatrix.m[row][col] = WorldMatrixJson[row][col];

		StaticMeshDesc.WorldMatrix = WorldMatrix;
#pragma endregion

#pragma region Ÿ�ϸ�
		//Ÿ�ϸ�
		if (objects[j].contains("TileDensity"))
		{
			StaticMeshDesc.bUseTiling = true;

			const json& TileDensityJson = objects[j]["TileDensity"];
			StaticMeshDesc.vTileDensity = {
				TileDensityJson[0].get<_float>(),
				TileDensityJson[1].get<_float>()
			};
		}
#pragma endregion

#pragma region �ݶ��̴�
		//�ݶ��̴�
		if (objects[j].contains("ColliderType") && objects[j]["ColliderType"].is_number_integer())
		{
			StaticMeshDesc.eColliderType = static_cast<COLLIDER_TYPE>(objects[j]["ColliderType"].get<_int>());
		}
		else
			return E_FAIL;
#pragma endregion

		wstring LayerTag = TEXT("Layer_MapToolObject_");
		LayerTag += StringToWString(ModelName);

		StaticMeshDesc.iRender = 0;
		StaticMeshDesc.m_eLevelID = LEVEL::DH;
		//lstrcpy(StaticMeshDesc.szName, TEXT("SM_TEST_FLOOR"));

		wstring wstrModelName = StringToWString(ModelName);
		wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
		ModelPrototypeTag += wstrModelName;

		lstrcpy(StaticMeshDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());


		if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_StaticMesh"),
			ENUM_CLASS(LEVEL::DH), LayerTag, &StaticMeshDesc)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevel_DH::Load_StaticMesh_Instance(_uint iObjectCount, const json& objects, string ModelName)
{
	vector<_float4x4> InstanceMatixs(iObjectCount);

	for (_uint i = 0; i < iObjectCount; ++i)
	{
		const json& WorldMatrixJson = objects[i]["WorldMatrix"];

		for (_int row = 0; row < 4; ++row)
			for (_int col = 0; col < 4; ++col)
				InstanceMatixs[i].m[row][col] = WorldMatrixJson[row][col];
	}


	//������Ʈ ����, ��ġ

	wstring LayerTag = TEXT("Layer_MapToolObject_");
	LayerTag += StringToWString(ModelName);

	CStaticMesh_Instance::STATICMESHINSTANCE_DESC StaticMeshInstanceDesc = {};
	StaticMeshInstanceDesc.iNumInstance = iObjectCount;//�ν��Ͻ� ������
	StaticMeshInstanceDesc.pInstanceMatrixs = &InstanceMatixs;//������ĵ��� �Ѱ�����Ѵ�.

	StaticMeshInstanceDesc.iRender = 0;
	StaticMeshInstanceDesc.m_eLevelID = LEVEL::DH;
	//lstrcpy(StaticMeshInstanceDesc.szName, TEXT("SM_TEST_FLOOR"));

	wstring wstrModelName = StringToWString(ModelName);
	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_Instance"); //�ν��Ͻ� �� ���� �غ��ؾ߰ڴµ�?
	ModelPrototypeTag += wstrModelName;

	lstrcpy(StaticMeshInstanceDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_StaticMesh_Instance"),
		ENUM_CLASS(LEVEL::DH), LayerTag, &StaticMeshInstanceDesc)))
		return E_FAIL;

	return S_OK;
}

#pragma endregion


HRESULT CLevel_DH::Ready_Camera()
{
	m_pCamera_Manager->Initialize(LEVEL::STATIC);
	m_pCamera_Manager->SetFreeCam();

	return S_OK;
}

HRESULT CLevel_DH::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(0.6f, 0.6f, 0.6f, 1.f);
	LightDesc.fAmbient = 0.2f;
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	CShadow::SHADOW_DESC		Desc{};
	Desc.vEye = _float4(0.f, 20.f, -15.f, 1.f);
	Desc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 1000.f;

	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWA)))
		return E_FAIL;

	return S_OK;

	return S_OK;
}

HRESULT CLevel_DH::Ready_Shadow()
{
	CShadow::SHADOW_DESC		Desc{};
	Desc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 1000.f;

	Desc.vEye = _float4(0.f, 100.f, -20.f, 1.f);
	Desc.fFovy = XMConvertToRadians(40.0f);
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWA)))
		return E_FAIL;

	Desc.fFovy = XMConvertToRadians(80.0f);
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWB)))
		return E_FAIL;

	Desc.fFovy = XMConvertToRadians(120.0f);
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWC)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_DH::Ready_ImGuiTools()
{
	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::DONGHA)] = CDHTool::Create(m_pDevice, m_pContext);
	if (nullptr == m_ImGuiTools[ENUM_CLASS(IMGUITOOL::DONGHA)])
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_DH::Ready_Layer_StaticMesh(const _wstring strLayerTag)
{
	CPBRMesh::STATICMESH_DESC Desc{};
	Desc.iRender = 0;
	Desc.m_eLevelID = LEVEL::DH;
	Desc.szMeshID = TEXT("SM_BuildingA_Lift_01");
	lstrcpy(Desc.szName, TEXT("SM_BuildingA_Lift_01"));

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::DH), strLayerTag, &Desc)))
		return E_FAIL;

	Desc.szMeshID = TEXT("SM_BuildingA_Lift_02");
	lstrcpy(Desc.szName, TEXT("SM_BuildingA_Lift_02"));
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::DH), strLayerTag, &Desc)))
		return E_FAIL;

	Desc.szMeshID = TEXT("SM_BuildingC_Sewer_01");
	lstrcpy(Desc.szName, TEXT("SM_BuildingC_Sewer_01"));
	Desc.InitPos = _float3(10.f, 0.f, 0.f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::DH), strLayerTag, &Desc)))
		return E_FAIL;

	Desc.szMeshID = TEXT("SM_Station_Floor_01");
	lstrcpy(Desc.szName, TEXT("SM_Station_Floor_01"));
	Desc.InitPos = _float3(0.f, -1.f, 0.f);
	Desc.InitScale = _float3(1.f, 1.f, 1.f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::DH), strLayerTag, &Desc)))
		return E_FAIL;
	
	Desc.szMeshID = TEXT("TestMap");
	lstrcpy(Desc.szName, TEXT("TestMap"));
	Desc.InitPos = _float3(0.f, 0.f, 0.f);
	Desc.InitScale = _float3(10.f, 10.f, 10.f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::DH), strLayerTag, &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_DH::Ready_Layer_Sky(const _wstring strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Sky"),
		ENUM_CLASS(LEVEL::DH), strLayerTag)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_DH::Ready_ImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 14.0f, nullptr, io.Fonts->GetGlyphRangesKorean());

	ImGui::StyleColorsDark();  // ��ο� �׸� ���

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 8.0f;
	style.FrameRounding = 5.0f;
	style.GrabRounding = 5.0f;
	style.ScrollbarRounding = 6.0f;

	style.WindowPadding = ImVec2(20, 20);   // �˳��� â ���� ����
	style.FramePadding = ImVec2(10, 8);    // ��ư/�Է�â ���� ����
	style.ItemSpacing = ImVec2(12, 10);   // ���� �� ����

	// ������ ��� ����
	style.Colors[ImGuiCol_WindowBg].w = 0.92f;  // 1.0�� ���� ������

	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // ��ư �Ķ�
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f); // ȣ�� �� ���� �Ķ�
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f); // Ŭ�� �� ���� �Ķ�
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f); // �Է�â ���

	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);

	return S_OK;
}

HRESULT CLevel_DH::ImGui_Render()
{
	if (FAILED(ImGui_Docking_Settings()))
		return E_FAIL;

	if (FAILED(m_ImGuiTools[ENUM_CLASS(IMGUITOOL::DONGHA)]->Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT CLevel_DH::ImGui_Docking_Settings()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_MenuBar;


	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", nullptr, window_flags);
	ImGui::PopStyleVar(3);


	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	ImGui::End();

	return S_OK;
}

CLevel_DH* CLevel_DH::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_DH* pInstance = new CLevel_DH(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_DH");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_DH::Free()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	//ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable; // ��Ƽ ����Ʈ ��Ȱ��ȭ
	//ImGui::GetIO().Fonts->Clear(); // ��Ʈ ĳ�� ����
	ImGui::DestroyContext();

	Safe_Release(m_ImGuiTools[ENUM_CLASS(IMGUITOOL::DONGHA)]);
	
	__super::Free();

}
