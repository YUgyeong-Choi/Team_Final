#include "Level_DH.h"
#include "GameInstance.h"
#include "DHTool.h"
#include "Camera_Manager.h"
#include "Level_Loading.h"
#include "PBRMesh.h"

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

	//if (FAILED(Ready_Layer_Sky(TEXT("Layer_Sky"))))
	//	return E_FAIL;

	if (FAILED(Ready_Camera()))
		return E_FAIL;

	if (FAILED(Ready_Layer_StaticMesh(TEXT("Layer_StaticMesh"))))
		return E_FAIL;

	m_pGameInstance->SetCurrentLevelIndex(ENUM_CLASS(LEVEL::DH));
	return S_OK;
}

void CLevel_DH::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_F1))
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOGO))))
			return;
	}

	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::DONGHA)]->Update(fTimeDelta);
	m_pCamera_Manager->Update(fTimeDelta);
	__super::Update(fTimeDelta);
}

HRESULT CLevel_DH::Render()
{
	SetWindowText(g_hWnd, TEXT("동하 레벨입니다."));

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

HRESULT CLevel_DH::Ready_Camera()
{
	m_pCamera_Manager->Initialize(LEVEL::STATIC);
	m_pCamera_Manager->SetFreeCam();

	return S_OK;
}

HRESULT CLevel_DH::Ready_Lights()
{
	return S_OK;
}

HRESULT CLevel_DH::Ready_Shadow()
{
	CShadow::SHADOW_DESC		Desc{};
	Desc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;

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
	
	Desc.szMeshID = TEXT("SM_Cathedral_FloorBR_03");
	lstrcpy(Desc.szName, TEXT("SM_Cathedral_FloorBR_03"));
	Desc.InitPos = _float3(0.f, 0.f, 5.f);
	Desc.InitScale = _float3(3.f, 3.f, 3.f);
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

	ImGui::StyleColorsDark();  // 어두운 테마 기반

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 8.0f;
	style.FrameRounding = 5.0f;
	style.GrabRounding = 5.0f;
	style.ScrollbarRounding = 6.0f;

	style.WindowPadding = ImVec2(20, 20);   // 넉넉한 창 내부 여백
	style.FramePadding = ImVec2(10, 8);    // 버튼/입력창 안쪽 여백
	style.ItemSpacing = ImVec2(12, 10);   // 위젯 간 간격

	// 투명한 배경 느낌
	style.Colors[ImGuiCol_WindowBg].w = 0.92f;  // 1.0은 완전 불투명

	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // 버튼 파랑
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f); // 호버 시 연한 파랑
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f); // 클릭 시 진한 파랑
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f); // 입력창 배경

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
	//ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable; // 멀티 뷰포트 비활성화
	//ImGui::GetIO().Fonts->Clear(); // 폰트 캐시 정리
	ImGui::DestroyContext();

	Safe_Release(m_ImGuiTools[ENUM_CLASS(IMGUITOOL::DONGHA)]);
	
	__super::Free();

}
