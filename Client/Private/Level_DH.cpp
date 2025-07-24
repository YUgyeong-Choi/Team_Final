#include "Level_DH.h"
#include "GameInstance.h"
#include "DHTool.h"

CLevel_DH::CLevel_DH(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
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

	return S_OK;
}

void CLevel_DH::Update(_float fTimeDelta)
{
	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::OBJECT)]->Update(fTimeDelta);

	__super::Update(fTimeDelta);
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

	ImGui_Render();
	//������ 
	ImGui::ShowDemoWindow(); // Show demo window! :)

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	//ImGui::UpdatePlatformWindows();
	//ImGui::RenderPlatformWindowsDefault();
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


	return S_OK;
}

HRESULT CLevel_DH::Ready_ImGuiTools()
{
	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::OBJECT)] = CDHTool::Create(m_pDevice, m_pContext);
	if (nullptr == m_ImGuiTools[ENUM_CLASS(IMGUITOOL::OBJECT)])
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_DH::Ready_ImGui()
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

HRESULT CLevel_DH::ImGui_Render()
{
	if (FAILED(ImGui_Docking_Settings()))
		return E_FAIL;

	if (FAILED(m_ImGuiTools[ENUM_CLASS(IMGUITOOL::OBJECT)]->Render()))
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

	Safe_Release(m_ImGuiTools[ENUM_CLASS(IMGUITOOL::OBJECT)]);
	
	__super::Free();

}
