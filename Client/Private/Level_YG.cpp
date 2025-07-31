#include "Level_YG.h"
#include "GameInstance.h"
#include "YGTool.h"
#include "Camera_Manager.h"
#include "Level_Loading.h"

#include "YGDynamicGib.h"
#include "YGDynamicObj.h"
#include "PBRMesh.h"
#include "TestAnimObject.h"

CLevel_YG::CLevel_YG(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{

}

HRESULT CLevel_YG::Initialize()
{
	m_pGameInstance->Set_IsChangeLevel(false);
	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Sky(TEXT("Layer_Sky"))))
		return E_FAIL;

	if (FAILED(Ready_Camera()))
		return E_FAIL;

	if (FAILED(Ready_ImGui()))
		return E_FAIL;

	if (FAILED(Ready_ImGuiTools()))
		return E_FAIL;

	//if (FAILED(Ready_Layer_Object(TEXT("Layer_YG"))))
	//	return E_FAIL;

	if (FAILED(Ready_Layer_Station(TEXT("Layer_StaticMesh"))))
		return E_FAIL;

	m_pGameInstance->SetCurrentLevelIndex(ENUM_CLASS(LEVEL::YG));
	return S_OK;
}

void CLevel_YG::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_F1))
	{
		m_pGameInstance->Set_IsChangeLevel(true);
		CCamera_Manager::Get_Instance()->SetPlayer(nullptr);
		m_pGameInstance->ClearRenderObjects();
		m_pGameInstance->Remove_NoLevelLight();
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOGO))))
		{
			return;
		}
	}


	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::CAMERA)]->Priority_Update(fTimeDelta);
	m_pCamera_Manager->Update(fTimeDelta);
	__super::Update(fTimeDelta);
	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::CAMERA)]->Update(fTimeDelta);
	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::CAMERA)]->Late_Update(fTimeDelta); 
}

HRESULT CLevel_YG::Render()
{
	SetWindowText(g_hWnd, TEXT("유경 레벨입니다."));

	_wstring text = L"제일 왼쪽: Capsule";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 30.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));

	text = L"왼쪽 : TriangleMesh";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 60.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));

	text = L"가운곳 : ConvexMesh <- 지 맘대로 콜라이더 만들어서 간단하게 생긴 오브젝트한테만 해야함";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 90.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));

	text = L"오른쪽 : Box";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 120.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));

	text = L"제일 오른쪽 : Shere";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 150.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));

	

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

HRESULT CLevel_YG::Ready_Layer_Station(const _wstring strLayerTag)
{
	CPBRMesh::STATICMESH_DESC Desc{};
	Desc.iRender = 0;
	Desc.m_eLevelID = LEVEL::YG;
	Desc.szMeshID = TEXT("Train");
	lstrcpy(Desc.szName, TEXT("Train"));

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::YG), strLayerTag, &Desc)))
		return E_FAIL;

	Desc.szMeshID = TEXT("Station");
	lstrcpy(Desc.szName, TEXT("Station"));
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::YG), strLayerTag, &Desc)))
		return E_FAIL;

	CTestAnimObject::GAMEOBJECT_DESC playerDesc{};
	playerDesc.fSpeedPerSec = 3.f;
	playerDesc.fRotationPerSec = XMConvertToRadians(600.0f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_TestAnimObject"),
		ENUM_CLASS(LEVEL::YG), TEXT("TestAnimObject"), &playerDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_YG::Ready_Camera()
{
	m_pCamera_Manager->Initialize(LEVEL::STATIC);
	m_pCamera_Manager->SetFreeCam();

	return S_OK;
}

HRESULT CLevel_YG::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(0.6f, 0.6f, 0.6f, 1.f);
	LightDesc.fAmbient = 0.2f;
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.fAmbient = 0.2f;
	LightDesc.fIntensity = 0.5f;
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_LevelLightData(_uint(LEVEL::YG), LightDesc)))
		return E_FAIL;


	return S_OK;
}

HRESULT CLevel_YG::Ready_Layer_Object(const _wstring strLayerTag)
{
	//if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGObject"),
	//	ENUM_CLASS(LEVEL::YG), strLayerTag)))
	//	return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGController"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGCapsule"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGTriangleMesh"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGConvexMesh"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGBox"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGShpere"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGTrrigerWithoutModel"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	/*for (int i = 1; i <= 11; ++i) {
		wstring protoTag = L"Prototype_Component_Model_Part" + to_wstring(i);
		wstring name = L"DynamicGib" + to_wstring(i);

		CYGDynamicGib::GIBDATA_DESC _desc{};
		_desc.protoTag = protoTag;
		wcscmp(_desc.szName, name.c_str());

		if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGDynamicGib"),
			ENUM_CLASS(LEVEL::YG), strLayerTag, &_desc)))
			return E_FAIL;
	}*/

	CYGDynamicObj::DYNAMICDATA_DESC _dynamicDesc{};
	_dynamicDesc.colliderType = L"Box";
	lstrcpy(_dynamicDesc.szName, TEXT("YGDynamicBox"));

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGDynamicObj"),
		ENUM_CLASS(LEVEL::YG), strLayerTag, &_dynamicDesc)))
		return E_FAIL;

	_dynamicDesc.colliderType = L"Shpere";
	lstrcpy(_dynamicDesc.szName, TEXT("YGDynamicShpere"));

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGDynamicObj"),
		ENUM_CLASS(LEVEL::YG), strLayerTag, &_dynamicDesc)))
		return E_FAIL;

	_dynamicDesc.colliderType = L"Convex";
	lstrcpy(_dynamicDesc.szName, TEXT("YGDynamicConvex"));

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGDynamicObj"),
		ENUM_CLASS(LEVEL::YG), strLayerTag, &_dynamicDesc)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGFloor"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	return S_OK;
}


HRESULT CLevel_YG::Ready_ImGuiTools()
{
	m_ImGuiTools[ENUM_CLASS(IMGUITOOL::CAMERA)] = CYGTool::Create(m_pDevice, m_pContext);
	if (nullptr == m_ImGuiTools[ENUM_CLASS(IMGUITOOL::CAMERA)])
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_YG::Ready_ImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
	io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Impact.ttf", 14.0f, nullptr, io.Fonts->GetGlyphRangesKorean());

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

HRESULT CLevel_YG::ImGui_Render()
{
	if (FAILED(ImGui_Docking_Settings()))
		return E_FAIL;

	if (FAILED(m_ImGuiTools[ENUM_CLASS(IMGUITOOL::CAMERA)]->Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT CLevel_YG::ImGui_Docking_Settings()
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


HRESULT CLevel_YG::Ready_Layer_Sky(const _wstring strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Sky"),
		ENUM_CLASS(LEVEL::STATIC), strLayerTag)))
		return E_FAIL;

	return S_OK;
}


CLevel_YG* CLevel_YG::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_YG* pInstance = new CLevel_YG(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_YG");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_YG::Free()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Safe_Release(m_ImGuiTools[ENUM_CLASS(IMGUITOOL::CAMERA)]);

	__super::Free();
}
