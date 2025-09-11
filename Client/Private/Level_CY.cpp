#include "Level_CY.h"
#include "GameInstance.h"
#include "CYTool.h"
#include "Camera_Manager.h"
#include "Effect_Manager.h"
#include "EffectContainer.h"

#include "Level_Loading.h"
#include "PBRMesh.h"
#include "Player.h"

static CGameObject* pMap = { nullptr };

CLevel_CY::CLevel_CY(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{

}

HRESULT CLevel_CY::Initialize()
{
	/* [ 레벨 셋팅 ] */
	m_pGameInstance->SetCurrentLevelIndex(ENUM_CLASS(LEVEL::CY));
	m_pGameInstance->Set_IsChangeLevel(false);


	if (FAILED(Ready_ImGui()))
		return E_FAIL;

	if (FAILED(Ready_ImGuiTools()))
		return E_FAIL;
	 
	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_Camera()))
		return E_FAIL;

	if (FAILED(Ready_Sky(TEXT("Layer_Sky"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_StaticMesh(TEXT("Layer_StaticMesh"))))
		return E_FAIL;
	
	if (FAILED(Ready_Layer_DummyMap(TEXT("Layer_DummyMap"))))
		return E_FAIL;

	if (FAILED(Ready_Player()))
		return E_FAIL;

	return S_OK;
}

void CLevel_CY::Priority_Update(_float fTimeDelta)
{
	ShowCursor(TRUE);
	//if (m_pGameInstance->Key_Down(DIK_F1))
	//{
	//	m_pGameInstance->Set_IsChangeLevel(true);
	//	CCamera_Manager::Get_Instance()->SetPlayer(nullptr);
	//	m_pGameInstance->ClearRenderObjects();
	//	m_pGameInstance->RemoveAll_Light(ENUM_CLASS(LEVEL::CY));
	//	m_pGameInstance->Reset_All();
	//	if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOGO))))
	//		return;
	//}

	if (KEY_DOWN(DIK_BACKSLASH))
	{
		static _bool Active = { true };
		Active = !Active;
		CCamera_Manager::Get_Instance()->SetbMoveable(Active);
	}
}

void CLevel_CY::Update(_float fTimeDelta)
{
	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::MAP)]->Priority_Update(fTimeDelta);
	m_pCamera_Manager->Update(fTimeDelta);
	//__super::Update(fTimeDelta);
	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::MAP)]->Update(fTimeDelta);
	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::MAP)]->Late_Update(fTimeDelta);
}

HRESULT CLevel_CY::Render()
{
	SetWindowText(g_hWnd, TEXT("채영 레벨입니다."));

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

	if (FAILED(ImGui_Render()))
		int a = 0;
	//렌더링 

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		///* 뷰포트 관련*/
		ID3D11RenderTargetView* mainRTV = nullptr;
		ID3D11DepthStencilView* mainDSV = nullptr;
		m_pContext->OMGetRenderTargets(1, &mainRTV, &mainDSV);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		m_pContext->OMSetRenderTargets(1, &mainRTV, mainDSV);

		if (mainRTV) mainRTV->Release();
		if (mainDSV) mainDSV->Release();
	}

	return S_OK;
}

HRESULT CLevel_CY::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.fAmbient = 0.3f;
	LightDesc.fIntensity = 0.2f;
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(0.f, 1.f, 0.f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.fFogDensity = 0.f;
		
	if (FAILED(m_pGameInstance->Add_LevelLightData(ENUM_CLASS(LEVEL::CY), LightDesc)))
		return E_FAIL;


	return S_OK;
}

HRESULT CLevel_CY::Ready_Camera()
{
	m_pCamera_Manager->Initialize(LEVEL::STATIC);
	m_pCamera_Manager->SetFreeCam();

	return S_OK;
}

HRESULT CLevel_CY::Ready_ImGuiTools()
{

	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::MAP)] = CCYTool::Create(m_pDevice, m_pContext);
	if (nullptr == m_ImGuiTools[ENUM_CLASS(IMGUITOOL::MAP)])
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_CY::Ready_Sky(const _wstring strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Sky"),
		ENUM_CLASS(LEVEL::CY), strLayerTag)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_CY::Ready_Layer_StaticMesh(const _wstring strLayerTag)
{
	CPBRMesh::STATICMESH_DESC Desc{};
	Desc.iRender = 0;
	Desc.m_eMeshLevelID = LEVEL::CY;
//	Desc.szMeshID = TEXT("SM_BuildingA_Lift_01");
//	Desc.InitPos = { 10.f, 5.f, 10.f };
//	lstrcpy(Desc.szName, TEXT("SM_BuildingA_Lift_01"));

//	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
//		ENUM_CLASS(LEVEL::CY), strLayerTag, &Desc)))
//		return E_FAIL;
//
//	Desc.szMeshID = TEXT("SM_BuildingA_Lift_02");
//	lstrcpy(Desc.szName, TEXT("SM_BuildingA_Lift_02"));
//	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
//		ENUM_CLASS(LEVEL::CY), strLayerTag, &Desc)))
//		return E_FAIL;
//
//	Desc.szMeshID = TEXT("SM_BuildingC_Sewer_01");
//	lstrcpy(Desc.szName, TEXT("SM_BuildingC_Sewer_01"));
//	Desc.InitPos = _float3(20.f, 0.f, 0.f);
//	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
//		ENUM_CLASS(LEVEL::CY), strLayerTag, &Desc)))
//		return E_FAIL;
//
//	Desc.szMeshID = TEXT("SM_Cathedral_FloorBR_03");
//	lstrcpy(Desc.szName, TEXT("SM_Cathedral_FloorBR_03"));
//	Desc.InitPos = _float3(10.f, 5.f, 15.f);
//	Desc.InitScale = _float3(3.f, 3.f, 3.f);
//	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
//		ENUM_CLASS(LEVEL::CY), strLayerTag, &Desc)))
//		return E_FAIL;

	Desc.szMeshID = TEXT("TestMap");
	lstrcpy(Desc.szName, TEXT("TestMap"));
	Desc.InitPos = _float3(0.f, 0.f, 0.f);
	Desc.InitScale = _float3(12.f, 12.f, 12.f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::CY), strLayerTag, &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_CY::Ready_Layer_DummyMap(const _wstring strLayerTag)
{
	CPBRMesh::STATICMESH_DESC Desc{};
	Desc.iRender = 0;
	Desc.m_eMeshLevelID = LEVEL::CY;
	Desc.szMeshID = TEXT("Train");
	lstrcpy(Desc.szName, TEXT("Train"));

	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::CY), strLayerTag, &pMap, &Desc)))
		return E_FAIL;

	//Desc.szMeshID = TEXT("Station");
	//lstrcpy(Desc.szName, TEXT("Station"));
	//if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
	//	ENUM_CLASS(LEVEL::CY), strLayerTag, &Desc)))
	//	return E_FAIL;

	return S_OK;
}


HRESULT CLevel_CY::Ready_Player()
{
	CPlayer::PLAYER_DESC pDesc{};
	//pDesc.fSpeedPerSec = 1.f;
	pDesc.fSpeedPerSec = 5.f;
	pDesc.fRotationPerSec = XMConvertToRadians(600.0f);
	pDesc.eMeshLevelID = LEVEL::CY;
	pDesc.InitPos = _float3(-1.3f, 0.978f, 1.f);
	pDesc.InitScale = _float3(1.f, 1.f, 1.f);
	lstrcpy(pDesc.szName, TEXT("Player"));
	pDesc.szMeshID = TEXT("Player");
	pDesc.iLevelID = ENUM_CLASS(LEVEL::CY);

	CGameObject* pGameObject = nullptr;
 	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_Player"),
		ENUM_CLASS(LEVEL::CY), TEXT("Layer_Player"), &pGameObject, &pDesc)))
		return E_FAIL;

	//m_pPlayer = dynamic_cast<CPlayer*>(pGameObject);

	return S_OK;
}


HRESULT CLevel_CY::Ready_ImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // multi-viewport?

	ImGui::StyleColorsDark();
	io.Fonts->AddFontFromFileTTF("C://Windows//Fonts//gulim.ttc", 14.0f, nullptr, io.Fonts->GetGlyphRangesKorean());
	io.IniFilename = "../Bin/Save/Effect/ImGui.ini"; // ImGui 설정 파일 경로
	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);

	return S_OK;
}

HRESULT CLevel_CY::ImGui_Render()
{
	if (FAILED(ImGui_Docking_Settings()))
		return E_FAIL;

	if (FAILED(m_ImGuiTools[ENUM_CLASS(IMGUITOOL::MAP)]->Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT CLevel_CY::ImGui_Docking_Settings()
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
		ImGuiWindowFlags_NoDecoration /*|
		ImGuiWindowFlags_MenuBar*/;


	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	// 툴바메뉴 여기

	//_bool test;
	//if (ImGui::BeginMenuBar()) {
	//	if (ImGui::BeginMenu("File")) {
	//		if (ImGui::MenuItem("Open", "Ctrl+O", &test)) { /* 파일 열기 로직 */ }
	//		if (ImGui::MenuItem("Save", "Ctrl+S", &test)) { /* 파일 저장 로직 */ }
	//		ImGui::EndMenu();
	//	}
	//}
	//ImGui::EndMenuBar();
	
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	ImGui::End();

	return S_OK;
}

CLevel_CY* CLevel_CY::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_CY* pInstance = new CLevel_CY(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_CY");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_CY::Free()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	//ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable; // 멀티 뷰포트 비활성화
	//ImGui::GetIO().Fonts->Clear(); // 폰트 캐시 정리
	ImGui::DestroyContext();

	Safe_Release(m_ImGuiTools[ENUM_CLASS(IMGUITOOL::MAP)]);
	
	__super::Free();

}
