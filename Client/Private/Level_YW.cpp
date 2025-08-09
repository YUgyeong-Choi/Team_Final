#include "Level_YW.h"
#include "GameInstance.h"

#include "Camera_Manager.h"
#include "Level_Loading.h"

#include "MapTool.h"
#include "NavTool.h"
#include "DecalTool.h"

#pragma region 다른 사람 거
#include "PBRMesh.h"
#pragma endregion



CLevel_YW::CLevel_YW(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{
	Safe_AddRef(m_pCamera_Manager);
}

HRESULT CLevel_YW::Initialize()
{
	/*if (FAILED(Ready_Layer_TestDecal(TEXT("Layer_TestDecal"))))
		return E_FAIL;*/

	if (FAILED(Ready_ImGui()))
		return E_FAIL;

	if (FAILED(Ready_Camera()))
		return E_FAIL;

	if (FAILED(Ready_ImGuiTools()))
		return E_FAIL;

	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Sky(TEXT("Layer_Sky"))))
		return E_FAIL;	

	/*if (FAILED(Ready_Layer_DummyMap(TEXT("Layer_DummyMap"))))
		return E_FAIL;*/	

	m_pGameInstance->SetCurrentLevelIndex(ENUM_CLASS(LEVEL::YW));
	return S_OK;
}

void CLevel_YW::Priority_Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_F1))
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOGO))))
			return;
	}

	m_ImGuiTools[ENUM_CLASS(m_eActiveTool)]->Priority_Update(fTimeDelta);
}

void CLevel_YW::Update(_float fTimeDelta)
{
	m_ImGuiTools[ENUM_CLASS(m_eActiveTool)]->Update(fTimeDelta);

	m_pCamera_Manager->Update(fTimeDelta);
	//__super::Update(fTimeDelta);
}

void CLevel_YW::Late_Update(_float fTimeDelta)
{
	m_ImGuiTools[ENUM_CLASS(m_eActiveTool)]->Late_Update(fTimeDelta);
}

HRESULT CLevel_YW::Render()
{
	SetWindowText(g_hWnd, TEXT("영웅 레벨입니다."));

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

	ImGui_Render();
	//렌더링 
	//ImGui::ShowDemoWindow(); // Show demo window! :)

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	//ImGui::UpdatePlatformWindows();
	//ImGui::RenderPlatformWindowsDefault();

	return S_OK;
}

HRESULT CLevel_YW::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(0.6f, 0.6f, 0.6f, 1.f);
	LightDesc.fAmbient = 0.8f;
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	//LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.fAmbient = 0.2f;
	LightDesc.fIntensity = 0.5f;
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_LevelLightData(_uint(LEVEL::YW), LightDesc)))
		return E_FAIL;


	return S_OK;
}

HRESULT CLevel_YW::Ready_Camera()
{
	m_pCamera_Manager->Initialize(LEVEL::STATIC);
	m_pCamera_Manager->SetFreeCam();

	return S_OK;
}

HRESULT CLevel_YW::Ready_Layer_Sky(const _wstring strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Sky"),
		ENUM_CLASS(LEVEL::YW), strLayerTag)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_YW::Ready_ImGuiTools()
{
	if (FAILED(Ready_Layer_PreviewObject(TEXT("Layer_PreviewObject"))))
		return E_FAIL;

	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::MAP)] = reinterpret_cast<CYWTool*>(CMapTool::Create(m_pDevice, m_pContext));
	if (nullptr == m_ImGuiTools[ENUM_CLASS(IMGUITOOL::MAP)])
		return E_FAIL;

	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::NAV)] = reinterpret_cast<CYWTool*>(CNavTool::Create(m_pDevice, m_pContext));
	if (nullptr == m_ImGuiTools[ENUM_CLASS(IMGUITOOL::NAV)])
		return E_FAIL;

	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::DECAL)] = reinterpret_cast<CYWTool*>(CDecalTool::Create(m_pDevice, m_pContext));
	if (nullptr == m_ImGuiTools[ENUM_CLASS(IMGUITOOL::DECAL)])
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_YW::Ready_ImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // multi-viewport?

	ImGui::StyleColorsDark();
	io.Fonts->AddFontFromFileTTF("C://Windows//Fonts//gulim.ttc", 14.0f, nullptr, io.Fonts->GetGlyphRangesKorean());

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);

	return S_OK;
}

HRESULT CLevel_YW::ImGui_Render()
{
	if (FAILED(ImGui_Docking_Settings()))
		return E_FAIL;

	if (ImGui::Begin("Tool Selector"), nullptr, ImGuiWindowFlags_NoCollapse)
	{
		if (ImGui::BeginTabBar("##ToolTabs"))
		{
			if (ImGui::BeginTabItem("Map Tool"))
			{
				m_eActiveTool = IMGUITOOL::MAP;

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Nav Tool"))
			{
				m_eActiveTool = IMGUITOOL::NAV;

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Decal Tool"))
			{
				m_eActiveTool = IMGUITOOL::DECAL;

				ImGui::EndTabItem();
			}

			// ... 다른 탭도 추가 가능

			ImGui::EndTabBar();
		}
		ImGui::End();
	}

	if (FAILED(m_ImGuiTools[ENUM_CLASS(m_eActiveTool)]->Render_ImGui()))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CLevel_YW::ImGui_Docking_Settings()
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

HRESULT CLevel_YW::Ready_Layer_TestDecal(const _wstring strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_DecalToolObject"),
		ENUM_CLASS(LEVEL::YW), strLayerTag)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_YW::Ready_Layer_DummyMap(const _wstring strLayerTag)
{
	CPBRMesh::STATICMESH_DESC Desc{};
	Desc.iRender = 0;
	Desc.m_eLevelID = LEVEL::YW;
	Desc.szMeshID = TEXT("Train");
	lstrcpy(Desc.szName, TEXT("Train"));

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::YW), strLayerTag, &Desc)))
		return E_FAIL;

	Desc.szMeshID = TEXT("Station");
	lstrcpy(Desc.szName, TEXT("Station"));
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::YW), strLayerTag, &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_YW::Ready_Layer_PreviewObject(const _wstring strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_GameObject_PreviewObject"),
		ENUM_CLASS(LEVEL::YW), strLayerTag)))
		return E_FAIL;

	return S_OK;
}

CLevel_YW* CLevel_YW::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_YW* pInstance = new CLevel_YW(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_YW");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_YW::Free()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	//ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable; // 멀티 뷰포트 비활성화
	//ImGui::GetIO().Fonts->Clear(); // 폰트 캐시 정리
	ImGui::DestroyContext();

	//Safe_Release(m_ImGuiTools[ENUM_CLASS(IMGUITOOL::MAP)]);

	for (CGameObject* pTool : m_ImGuiTools)
	{
		Safe_Release(pTool);
	}

	Safe_Release(m_pCamera_Manager);

	__super::Free();

}
