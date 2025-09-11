#include "Level_DH.h"
#include "GameInstance.h"
#include "DHTool.h"
#include "Camera_Manager.h"
#include "Level_Loading.h"
#include "PBRMesh.h"
#include "DH_ToolMesh.h"

#include "StaticMesh.h"
#include "StaticMesh_Instance.h"

// 상호작용 문
#include "SlideDoor.h"
#include "KeyDoor.h"
#include "BossDoor.h"
#include "ShortCutDoor.h"

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

	if (FAILED(Ready_Dummy()))
		return E_FAIL;
	
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	
	if (FAILED(Ready_Shadow()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Sky(TEXT("Layer_Sky"))))
		return E_FAIL;

	if (FAILED(Ready_Camera()))
		return E_FAIL;
	
	if (FAILED(Ready_OctoTree()))
		return E_FAIL;
	
	if (FAILED(Separate_Area()))
		return E_FAIL;

	if (FAILED(Ready_Interact()))
		return E_FAIL;

	/* [ 플레이어가 속한 구역탐색 ] */
	//6번구역
	m_pGameInstance->SetPlayerPosition(_fvector{ 188.27f, 10.18f, -8.23f, 1.f });
	//1번구역
	//m_pGameInstance->SetPlayerPosition(_fvector{ -0.2f, 1.f, 1.01f, 1.f });

	m_pGameInstance->FindAreaContainingPoint();

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
	ShowCursor(true);
	//__super::Update(fTimeDelta);
}

void CLevel_DH::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
	
	Add_RenderGroup_OctoTree();
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

HRESULT CLevel_DH::Ready_OctoTree()
{
	m_pGameInstance->ClearIndexToObj();

	vector<AABBBOX> staticBounds;
	vector<OCTOTREEOBJECTTYPE>  vObjectType;
	map<Handle, _uint> handleToIndex;

	//용량 확보: 메쉬 + 라이트
	vector<class CGameObject*> OctoObject = m_pGameInstance->GetOctoTreeObjects();
	const _uint iReserve = static_cast<_uint>(OctoObject.size());
	staticBounds.reserve(iReserve);
	vObjectType.reserve(iReserve);

	_uint nextHandleId = 1000; // 핸들 ID 인데 1000부터 시작임

	for (auto* OctoTreeObjects : OctoObject)
	{
		AABBBOX worldBox = OctoTreeObjects->GetWorldAABB();
		_uint idx = static_cast<_uint>(staticBounds.size());
		staticBounds.push_back(worldBox);
		vObjectType.push_back(OCTOTREEOBJECTTYPE::MESH);

		Handle h{ nextHandleId++ };
		handleToIndex[h] = idx;
		m_pGameInstance->PushBackIndexToObj(OctoTreeObjects);
	}

	if (FAILED(m_pGameInstance->Ready_OctoTree(staticBounds, handleToIndex)))
		return E_FAIL;

	m_pGameInstance->SetObjectType(vObjectType);

	return S_OK;
}

HRESULT CLevel_DH::Add_RenderGroup_OctoTree()
{
	_matrix matView = XMLoadFloat4x4(m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW));
	_matrix matProj = XMLoadFloat4x4(m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ));

	m_pGameInstance->BeginQueryFrame(matView, matProj);
	m_pGameInstance->QueryVisible();

	vector<class CGameObject*> AllStaticMesh = m_pGameInstance->GetIndexToObj();
	vector<_uint> VisitCell = m_pGameInstance->GetCulledStaticObjects();
	vector<OCTOTREEOBJECTTYPE> vTypeTable = m_pGameInstance->GetObjectType();


	for (_uint iIdx : VisitCell)
	{
		CGameObject* pObj = AllStaticMesh[iIdx];

		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, pObj);
	}

	return S_OK;
}

HRESULT CLevel_DH::Separate_Area()
{
	m_pGameInstance->Reset_Parm();

	/* [ 레벨의 구역을 나눠준다. ] */
	auto FnToAABB = [](_float3 a, _float3 b, _float3& outMin, _float3& outMax)
		{
			outMin = _float3{
				static_cast<_float>(min(a.x, b.x)),
				static_cast<_float>(min(a.y, b.y)),
				static_cast<_float>(min(a.z, b.z))
			};
			outMax = _float3{
				static_cast<_float>(max(a.x, b.x)),
				static_cast<_float>(max(a.y, b.y)),
				static_cast<_float>(max(a.z, b.z))
			};
		};

	// Area 1
	_float3 a1p0 = _float3{ 35.73f, -2.87f,  4.97f };
	_float3 a1p1 = _float3{ -10.57f,  9.92f, -4.62f };
	_float3 a1Min, a1Max;
	FnToAABB(a1p0, a1p1, a1Min, a1Max);

	// Area 2
	_float3 a2p0 = _float3{ 61.82f, -5.39f,  8.25f };
	_float3 a2p1 = _float3{ 32.16f,  8.91f, -4.62f };
	_float3 a2Min, a2Max;
	FnToAABB(a2p0, a2p1, a2Min, a2Max);

	// Area 3
	_float3 a3p0 = _float3{ 110.f, -5.63f,  32.20f };
	_float3 a3p1 = _float3{ -40.69f, 52.55f, -61.73f };
	_float3 a3Min, a3Max;
	FnToAABB(a3p0, a3p1, a3Min, a3Max);

	// Area 4
	_float3 a4p0 = _float3{ 120.04f, -5.32f, 4.66f };
	_float3 a4p1 = _float3{ 95.67f, 15.49f, -21.39f };
	_float3 a4Min, a4Max;
	FnToAABB(a4p0, a4p1, a4Min, a4Max);

	// Area 5
	_float3 a5p0 = _float3{ 110.35f, -5.63f,  32.20f };
	_float3 a5p1 = _float3{ 26.53f,  49.64f, -52.41f };
	_float3 a5Min, a5Max;
	FnToAABB(a5p0, a5p1, a5Min, a5Max);

	// Area 6
	_float3 a6p0 = _float3{ 135.35f, 1.29f, 3.18f };
	_float3 a6p1 = _float3{ 117.31f, 19.65f, -17.30f };
	_float3 a6Min, a6Max;
	FnToAABB(a6p0, a6p1, a6Min, a6Max);

	// Area 7
	_float3 a7p0 = _float3{ 186.83f, -0.18f, 24.92f };
	_float3 a7p1 = _float3{ 113.46f, 48.18f, -30.18f };
	_float3 a7Min, a7Max;
	FnToAABB(a7p0, a7p1, a7Min, a7Max);

	// Area 8
	_float3 a8p0 = _float3{ 178.65f, 1.57f, -16.40f };
	_float3 a8p1 = _float3{ 163.46f, 21.62f, -28.31f };
	_float3 a8Min, a8Max;
	FnToAABB(a8p0, a8p1, a8Min, a8Max);

	// Area 9
	_float3 a9p0 = _float3{ 180.31f, -0.99f, -24.41f };
	_float3 a9p1 = _float3{ 164.76f, 28.10f, -58.69f };
	_float3 a9Min, a9Max;
	FnToAABB(a9p0, a9p1, a9Min, a9Max);

	// Area 10
	_float3 a10p0 = _float3{ 165.54f, -0.73f, -35.35f };
	_float3 a10p1 = _float3{ 153.13f, 15.00f, -86.29f };
	_float3 a10Min, a10Max;
	FnToAABB(a10p0, a10p1, a10Min, a10Max);

	// Area 11
	_float3 a11p0 = _float3{ 174.33f, 3.05f, -58.39f };
	_float3 a11p1 = _float3{ 141.42f, 25.38f, -72.59f };
	_float3 a11Min, a11Max;
	FnToAABB(a11p0, a11p1, a11Min, a11Max);

	// Area 12
	_float3 a12p0 = _float3{ 150.09f, 0.33f, -71.50f };
	_float3 a12p1 = _float3{ 110.35f, 36.75f, -26.58f };
	_float3 a12Min, a12Max;
	FnToAABB(a12p0, a12p1, a12Min, a12Max);

	// Area 13
	_float3 a13p0 = _float3{ 114.46f, 36.75f, -26.58f };
	_float3 a13p1 = _float3{ 123.11f, 8.05f, -15.80f };
	_float3 a13Min, a13Max;
	FnToAABB(a13p0, a13p1, a13Min, a13Max);

	// Area 14
	_float3 a14p0 = _float3{ 139.64f, -3.73f, -34.23f };
	_float3 a14p1 = _float3{ 164.14f, 27.16f, -15.25f };
	_float3 a14Min, a14Max;
	FnToAABB(a14p0, a14p1, a14Min, a14Max);

	// Area 15
	_float3 a15p0 = _float3{ 164.14f, 27.16f, -15.25f };
	_float3 a15p1 = _float3{ 166.82f, 11.23f, -33.21f };
	_float3 a15Min, a15Max;
	FnToAABB(a15p0, a15p1, a15Min, a15Max);

	// Area 16
	_float3 a16p0 = _float3{ 166.82f, 11.23f, -33.21f };
	_float3 a16p1 = _float3{ 149.83f, 28.02f, -44.12f };
	_float3 a16Min, a16Max;
	FnToAABB(a16p0, a16p1, a16Min, a16Max);

	// Area 17
	_float3 a17p0 = _float3{ 161.07f, 25.85f, -32.40f };
	_float3 a17p1 = _float3{ 144.76f, -1.09f, -48.68f };
	_float3 a17Min, a17Max;
	FnToAABB(a17p0, a17p1, a17Min, a17Max);

	// Area 18
	_float3 a18p0 = _float3{ 153.11f, -1.26f, -24.49f };
	_float3 a18p1 = _float3{ 141.22f, 22.73f, -45.47f };
	_float3 a18Min, a18Max;
	FnToAABB(a18p0, a18p1, a18Min, a18Max);

	// Area 19 
	_float3 a19p0 = _float3{ 51.44f, 42.46f, -266.51f };
	_float3 a19p1 = _float3{ -42.91f, 0.20f, -139.25f };
	_float3 a19Min, a19Max;
	FnToAABB(a19p0, a19p1, a19Min, a19Max);

	// ------------- Area 50 --------------
	_float3 a50p0 = _float3{ 180.65f, -47.92f, 63.37f };
	_float3 a50p1 = _float3{ 457.66f, 133.33f, -116.79f };
	_float3 a50Min, a50Max;
	FnToAABB(a50p0, a50p1, a50Min, a50Max);

	{
		/* [ 1번 구역 ] */
		const vector<_uint> vecAdj1 = { 2 };
		if (!m_pGameInstance->AddArea_AABB(
			1, a1Min, a1Max, vecAdj1, AREA::EAreaType::LOBBY, ENUM_CLASS(AREA::EAreaType::LOBBY)))
			return E_FAIL;
	}
	{
		/* [ 2번 구역 ] */
		const vector<_uint> vecAdj2 = { 1, 3 };
		if (!m_pGameInstance->AddArea_AABB(
			2, a2Min, a2Max, vecAdj2, AREA::EAreaType::ROOM, ENUM_CLASS(AREA::EAreaType::ROOM)))
			return E_FAIL;
	}
	{
		/* [ 3번 구역 ] */
		const vector<_uint> vecAdj3 = { 1, 2, 5 };
		if (!m_pGameInstance->AddArea_AABB(
			3, a3Min, a3Max, vecAdj3, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 4번 구역 ] */
		const vector<_uint> vecAdj4 = { 5, 6, 7 };
		if (!m_pGameInstance->AddArea_AABB(
			4, a4Min, a4Max, vecAdj4, AREA::EAreaType::LOBBY, ENUM_CLASS(AREA::EAreaType::LOBBY)))
			return E_FAIL;
	}
	{
		/* [ 5번 구역 ] */
		const vector<_uint> vecAdj5 = { 4, 6 };
		if (!m_pGameInstance->AddArea_AABB(
			5, a5Min, a5Max, vecAdj5, AREA::EAreaType::INDOOR, ENUM_CLASS(AREA::EAreaType::INDOOR)))
			return E_FAIL;
	}
	{
		/* [ 6번 구역 ] */
		const vector<_uint> vecAdj6 = { 4, 7, 12 };
		if (!m_pGameInstance->AddArea_AABB(
			6, a6Min, a6Max, vecAdj6, AREA::EAreaType::LOBBY, ENUM_CLASS(AREA::EAreaType::LOBBY)))
			return E_FAIL;
	}
	{
		/* [ 7번 구역 ] */
		const vector<_uint> vecAdj7 = { 8, 9, 14, 18, 12 };
		if (!m_pGameInstance->AddArea_AABB(
			7, a7Min, a7Max, vecAdj7, AREA::EAreaType::INDOOR, ENUM_CLASS(AREA::EAreaType::INDOOR)))
			return E_FAIL;
	}
	{
		/* [ 8번 구역 ] */
		const vector<_uint> vecAdj8 = { 7, 9, 10, 11 };
		if (!m_pGameInstance->AddArea_AABB(
			8, a8Min, a8Max, vecAdj8, AREA::EAreaType::ROOM, ENUM_CLASS(AREA::EAreaType::ROOM)))
			return E_FAIL;
	}
	{
		/* [ 9번 구역 ] */
		const vector<_uint> vecAdj9 = { 7, 8, 10, 11 };
		if (!m_pGameInstance->AddArea_AABB(
			9, a9Min, a9Max, vecAdj9, AREA::EAreaType::LOBBY, ENUM_CLASS(AREA::EAreaType::LOBBY)))
			return E_FAIL;
	}
	{
		/* [ 10번 구역 ] */
		const vector<_uint> vecAdj10 = { 9, 12, 14, 16, 17, 18 };
		if (!m_pGameInstance->AddArea_AABB(
			10, a10Min, a10Max, vecAdj10, AREA::EAreaType::ROOM, ENUM_CLASS(AREA::EAreaType::ROOM)))
			return E_FAIL;
	}
	{
		/* [ 11번 구역 ] */
		const vector<_uint> vecAdj11 = { 9, 12 };
		if (!m_pGameInstance->AddArea_AABB(
			11, a11Min, a11Max, vecAdj11, AREA::EAreaType::ROOM, ENUM_CLASS(AREA::EAreaType::ROOM)))
			return E_FAIL;
	}
	{
		/* [ 12번 구역 ] */
		const vector<_uint> vecAdj12 = { 7, 11, 13, 14, 15, 16, 18 };
		if (!m_pGameInstance->AddArea_AABB(
			12, a12Min, a12Max, vecAdj12, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 13번 구역 ] */
		const vector<_uint> vecAdj13 = { 12 };
		if (!m_pGameInstance->AddArea_AABB(
			13, a13Min, a13Max, vecAdj13, AREA::EAreaType::ROOM, ENUM_CLASS(AREA::EAreaType::ROOM)))
			return E_FAIL;
	}
	{
		/* [ 14번 구역 ] */
		const vector<_uint> vecAdj14 = { 12, 15, 16, 17, 18, 7 };
		if (!m_pGameInstance->AddArea_AABB(
			14, a14Min, a14Max, vecAdj14, AREA::EAreaType::LOBBY, ENUM_CLASS(AREA::EAreaType::LOBBY)))
			return E_FAIL;
	}
	{
		/* [ 15번 구역 ] */
		const vector<_uint> vecAdj15 = { 14 , 12, 16, 18, 7 };
		if (!m_pGameInstance->AddArea_AABB(
			15, a15Min, a15Max, vecAdj15, AREA::EAreaType::ROOM, ENUM_CLASS(AREA::EAreaType::ROOM)))
			return E_FAIL;
	}
	{
		/* [ 16번 구역 ] */
		const vector<_uint> vecAdj16 = { 14, 15, 17, 18, 12, 7 };
		if (!m_pGameInstance->AddArea_AABB(
			16, a16Min, a16Max, vecAdj16, AREA::EAreaType::ROOM, ENUM_CLASS(AREA::EAreaType::ROOM)))
			return E_FAIL;
	}
	{
		/* [ 17번 구역 ] */
		const vector<_uint> vecAdj17 = { 7, 12, 14, 16, 18 };
		if (!m_pGameInstance->AddArea_AABB(
			17, a17Min, a17Max, vecAdj17, AREA::EAreaType::LOBBY, ENUM_CLASS(AREA::EAreaType::LOBBY)))
			return E_FAIL;
	}
	{
		/* [ 18번 구역 ] */
		const vector<_uint> vecAdj18 = { 7, 12, 14, 16, 17 };
		if (!m_pGameInstance->AddArea_AABB(
			18, a18Min, a18Max, vecAdj18, AREA::EAreaType::ROOM, ENUM_CLASS(AREA::EAreaType::ROOM)))
			return E_FAIL;
	}
	{
		/* [ 19번 구역 ] */
		const vector<_uint> vecAdj19 = {  };
		if (!m_pGameInstance->AddArea_AABB(
			19, a19Min, a19Max, vecAdj19, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}




	{
		/* -------- [ 50번 구역 ] ---------- */
		const vector<_uint> vecAdj50 = {  };
		if (!m_pGameInstance->AddArea_AABB(
			50, a50Min, a50Max, vecAdj50, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	if (FAILED(m_pGameInstance->FinalizePartition()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_DH::Ready_Dummy()
{
	CPBRMesh::STATICMESH_DESC Desc{};
	Desc.iRender = 0;
	Desc.m_eMeshLevelID = LEVEL::DH;
	Desc.szMeshID = TEXT("OutDoor");
	lstrcpy(Desc.szName, TEXT("OutDoor"));

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::DH), TEXT("Layer_Dummy"), &pGameObject, &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_DH::Ready_Interact()
{
	/*  [ 기차역 슬라이딩 문 ] */
	CSlideDoor::DOORMESH_DESC Desc{};
	Desc.m_eMeshLevelID = LEVEL::DH;
	Desc.szMeshID = TEXT("SM_Station_TrainDoor");
	lstrcpy(Desc.szName, TEXT("SM_Station_TrainDoor"));

	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_SM_Station_TrainDoor");
	lstrcpy(Desc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	_float3 vPosition = _float3(52.6f, 0.02f, -2.4f);
	_matrix matWorld = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
	_float4x4 matWorldFloat;
	XMStoreFloat4x4(&matWorldFloat, matWorld);
	Desc.WorldMatrix = matWorldFloat;
	Desc.vColliderOffSet = _vector({ 0.f, 1.3f, 0.f, 0.f });
	Desc.vColliderSize = _vector({ 1.5f, 2.f, 0.2f, 0.f });

	Desc.eInteractType = INTERACT_TYPE::TUTORIALDOOR;
	Desc.vTriggerOffset = _vector({ 0.f, 0.f, 0.3f, 0.f });
	Desc.vTriggerSize = _vector({ 1.f, 0.2f, 0.5f, 0.f });
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_SlideDoor"),
		ENUM_CLASS(LEVEL::DH), TEXT("TrainDoor"), &Desc)))
		return E_FAIL;

	/* [ 푸쿠오 보스 문 ] */
	CBossDoor::BOSSDOORMESH_DESC BossDoorDesc{};
	BossDoorDesc.m_eMeshLevelID = LEVEL::DH;
	BossDoorDesc.szMeshID = TEXT("FacotoryDoor");
	lstrcpy(BossDoorDesc.szName, TEXT("FacotoryDoor"));

	ModelPrototypeTag = TEXT("Prototype_Component_Model_FacotoryDoor");
	lstrcpy(BossDoorDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	vPosition = _float3(-1.4f, 0.31f, -235.f);
	XMMATRIX trans = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
	XMMATRIX rotY = XMMatrixRotationY(XM_PIDIV2); // = 90도
	XMMATRIX world = rotY * trans;

	XMStoreFloat4x4(&matWorldFloat, world);
	BossDoorDesc.WorldMatrix = matWorldFloat;
	BossDoorDesc.vColliderOffSet = _vector({ 0.f, 1.5f, 0.f, 0.f });
	BossDoorDesc.vColliderSize = _vector({ 0.2f, 2.f, 1.5f, 0.f });

	BossDoorDesc.eInteractType = INTERACT_TYPE::FUOCO;
	BossDoorDesc.vTriggerOffset = _vector({ 0.f, 0.5f, 0.f, 0.f });
	BossDoorDesc.vTriggerSize = _vector({ 0.5f, 0.2f, 1.5f, 0.f });
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_BossDoor"),
		ENUM_CLASS(LEVEL::DH), TEXT("BossDoor"), &BossDoorDesc)))
		return E_FAIL;

	/* [ 축제의 인도자 문 ] */
	BossDoorDesc = {};
	BossDoorDesc.m_eMeshLevelID = LEVEL::DH;
	BossDoorDesc.szMeshID = TEXT("FestivalDoor");
	lstrcpy(BossDoorDesc.szName, TEXT("FestivalDoor"));

	ModelPrototypeTag = TEXT("Prototype_Component_Model_FestivalDoor");
	lstrcpy(BossDoorDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	BossDoorDesc.bNeedSecondDoor = true;
	ModelPrototypeTag = TEXT("Prototype_Component_Model_FestivalCrashDoor");
	lstrcpy(BossDoorDesc.szSecondModelPrototypeTag, ModelPrototypeTag.c_str());

	vPosition = _float3(375.63f, 15.00f, -48.67f);
	trans = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
	world = trans;

	XMStoreFloat4x4(&matWorldFloat, world);
	BossDoorDesc.WorldMatrix = matWorldFloat;
	BossDoorDesc.vColliderOffSet = _vector({ 0.f, 1.5f, 0.f, 0.f });
	BossDoorDesc.vColliderSize = _vector({ 0.2f, 2.f, 2.f, 0.f });

	BossDoorDesc.eInteractType = INTERACT_TYPE::FESTIVALDOOR;
	BossDoorDesc.vTriggerOffset = _vector({ 0.f, 0.f, 0.f, 0.f });
	BossDoorDesc.vTriggerSize = _vector({ 0.5f, 0.2f, 1.0f, 0.f });
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_BossDoor"),
		ENUM_CLASS(LEVEL::DH), TEXT("BossDoor"), &BossDoorDesc)))
		return E_FAIL;

	/* [ 야외 나가는 문 ] */
	CKeyDoor::KEYDOORMESH_DESC KeyDoorDesc{};
	KeyDoorDesc.m_eMeshLevelID = LEVEL::DH;
	KeyDoorDesc.szMeshID = TEXT("StationDoubleDoor");
	lstrcpy(KeyDoorDesc.szName, TEXT("StationDoubleDoor"));

	ModelPrototypeTag = TEXT("Prototype_Component_Model_StationDoubleDoor");
	lstrcpy(KeyDoorDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());
	vPosition = _float3(184.04f, 8.90f, -8.f);
	trans = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
	world = trans;

	XMStoreFloat4x4(&matWorldFloat, world);
	KeyDoorDesc.WorldMatrix = matWorldFloat;
	KeyDoorDesc.vColliderOffSet = _vector({ 0.f, 1.5f, 0.f, 0.f });
	KeyDoorDesc.vColliderSize = _vector({ 0.2f, 2.f, 2.f, 0.f });

	KeyDoorDesc.eInteractType = INTERACT_TYPE::OUTDOOR;
	KeyDoorDesc.vTriggerOffset = _vector({ 0.f, 0.f, 0.f, 0.f });
	KeyDoorDesc.vTriggerSize = _vector({ 0.5f, 0.2f, 1.0f, 0.f });
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_KeyDoor"),
		ENUM_CLASS(LEVEL::DH), TEXT("KeyDoor"), &KeyDoorDesc)))
		return E_FAIL;

	/* [ 숏컷 문 ] */
	CShortCutDoor::SHORTCUTDOORMESH_DESC ShortCutDoorDesc{};
	KeyDoorDesc.m_eMeshLevelID = LEVEL::DH;
	KeyDoorDesc.szMeshID = TEXT("ShortCutDoor");
	lstrcpy(KeyDoorDesc.szName, TEXT("ShortCutDoor"));

	ModelPrototypeTag = TEXT("Prototype_Component_Model_ShortCutDoor");
	lstrcpy(KeyDoorDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());
	vPosition = _float3(147.46f, 2.66f, -25.17f);
	trans = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
	world = trans;

	XMStoreFloat4x4(&matWorldFloat, world);
	KeyDoorDesc.WorldMatrix = matWorldFloat;
	KeyDoorDesc.vColliderOffSet = _vector({ 0.f, 1.5f, 0.f, 0.f });
	KeyDoorDesc.vColliderSize = _vector({ 2.0f, 2.f, 0.2f, 0.f });

	KeyDoorDesc.eInteractType = INTERACT_TYPE::SHORTCUT;
	KeyDoorDesc.vTriggerOffset = _vector({ 0.f, 0.f, 0.f, 0.f });
	KeyDoorDesc.vTriggerSize = _vector({ 0.5f, 0.2f, 1.0f, 0.f });
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::DH), TEXT("Prototype_GameObject_ShortCutDoor"),
		ENUM_CLASS(LEVEL::DH), TEXT("ShortCutDoor"), &KeyDoorDesc)))
		return E_FAIL;

	return S_OK;
}


#pragma region 맵 로드

HRESULT CLevel_DH::Load_Mesh(const wstring& strPrototypeTag, const _char* pModelFilePath, _bool bInstance)
{
	//이미 프로토타입이존재하는 지확인

	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::DH), strPrototypeTag) != nullptr)
	{
		//MSG_BOX("이미 프로토타입이 존재함");
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
		MSG_BOX("Resource_STATION.json 파일을 열 수 없습니다.");
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
		MessageBoxA(nullptr, e.what(), "JSON 파싱 실패", MB_OK);
		return E_FAIL;
	}

	// JSON 데이터 확인
	for (const auto& element : ReadyModelJson)
	{
		string ModelName = element.value("ModelName", "");
		string Path = element.value("Path", "");

		//갯수도 저장해서 인스턴스용 모델 프로토타입을 만들지 결정해야할듯
		_uint iObjectCount = element["ObjectCount"];

		wstring PrototypeTag = {};
		_bool bInstance = false;
		if (iObjectCount > INSTANCE_THRESHOLD)
		{
			//인스턴싱용 모델 프로토 타입 생성
			PrototypeTag = L"Prototype_Component_Model_Instance" + StringToWString(ModelName);
			bInstance = true;

		}
		else
		{
			//모델 프로토 타입 생성
			PrototypeTag = L"Prototype_ComponentModel" + StringToWString(ModelName);
			bInstance = false;
		}


		const _char* pModelFilePath = Path.c_str();

		if (FAILED(Load_Mesh(PrototypeTag, pModelFilePath, bInstance)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}


HRESULT CLevel_DH::LoadMap(_uint iLevelIndex, const _char* Map)
{
	string MapPath = string("../Bin/Save/MapTool/Map_") + Map + ".json";

	ifstream inFile(MapPath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Map_" + StringToWString(Map) + L".json 파일을 열 수 없습니다: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);
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
		_uint iObjectCount = Models[i]["ObjectCount"]; //오브젝트 갯수를보고 인스턴싱을 쓸지 말지 결정해야겠다.(아니 충돌여부로 인스턴싱 해야겠다.)
		const json& objects = Models[i]["Objects"];

		_bool bCollision = Models[i]["Collision"];
		//일정 갯수 이상이면 인스턴싱오브젝트로 로드(충돌이 없는 모델이면 인스턴싱)
		if (bCollision == false /*iObjectCount > INSTANCE_THRESHOLD*/)
		{
			Load_StaticMesh_Instance(iObjectCount, objects, ModelName, iLevelIndex);
		}
		else
		{
			Load_StaticMesh(iObjectCount, objects, ModelName, iLevelIndex);
		}
	}

	return S_OK;
}

HRESULT CLevel_DH::Load_StaticMesh(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex)
{
	for (_uint j = 0; j < iObjectCount; ++j)
	{
#pragma region 월드행렬
		CStaticMesh::STATICMESH_DESC StaticMeshDesc = {};

		const json& WorldMatrixJson = objects[j]["WorldMatrix"];
		_float4x4 WorldMatrix = {};

		for (_int row = 0; row < 4; ++row)
			for (_int col = 0; col < 4; ++col)
				WorldMatrix.m[row][col] = WorldMatrixJson[row][col];

		StaticMeshDesc.WorldMatrix = WorldMatrix;
#pragma endregion

#pragma region 타일링
		//타일링
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

#pragma region 콜라이더
		//콜라이더
		if (objects[j].contains("ColliderType") && objects[j]["ColliderType"].is_number_integer())
		{
			StaticMeshDesc.eColliderType = static_cast<COLLIDER_TYPE>(objects[j]["ColliderType"].get<_int>());
		}
		else
			return E_FAIL;
#pragma endregion

#pragma region 라이트모양
		StaticMeshDesc.iLightShape = objects[j].value("LightShape", 0);
#pragma endregion

		wstring LayerTag = TEXT("Layer_MapToolObject_");
		LayerTag += StringToWString(ModelName);

		StaticMeshDesc.iRender = 0;
		StaticMeshDesc.iLevelID = iLevelIndex;
		//lstrcpy(StaticMeshDesc.szName, TEXT("SM_TEST_FLOOR"));

		wstring wstrModelName = StringToWString(ModelName);
		wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
		ModelPrototypeTag += wstrModelName;

		lstrcpy(StaticMeshDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

		CGameObject* pGameObject = nullptr;
		if (FAILED(m_pGameInstance->Add_GameObjectReturn(iLevelIndex, TEXT("Prototype_GameObject_StaticMesh"),
			iLevelIndex, LayerTag, &pGameObject, &StaticMeshDesc)))
			return E_FAIL;

		CStaticMesh* pStaticMesh = dynamic_cast<CStaticMesh*>(pGameObject);
		//m_vecOctoTreeObjects.push_back(pStaticMesh);
	}

	return S_OK;
}

HRESULT CLevel_DH::Load_StaticMesh_Instance(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex)
{
	vector<_float4x4> InstanceMatixs(iObjectCount);

	for (_uint i = 0; i < iObjectCount; ++i)
	{
		const json& WorldMatrixJson = objects[i]["WorldMatrix"];

		for (_int row = 0; row < 4; ++row)
			for (_int col = 0; col < 4; ++col)
				InstanceMatixs[i].m[row][col] = WorldMatrixJson[row][col];
	}


	//오브젝트 생성, 배치

	wstring LayerTag = TEXT("Layer_MapToolObject_");
	LayerTag += StringToWString(ModelName);

	CStaticMesh_Instance::STATICMESHINSTANCE_DESC StaticMeshInstanceDesc = {};
	StaticMeshInstanceDesc.iNumInstance = iObjectCount;//인스턴스 갯수랑
	StaticMeshInstanceDesc.pInstanceMatrixs = &InstanceMatixs;//월드행렬들을 넘겨줘야한다.

	StaticMeshInstanceDesc.iRender = 0;
	StaticMeshInstanceDesc.iLevelID = iLevelIndex;
	//lstrcpy(StaticMeshInstanceDesc.szName, TEXT("SM_TEST_FLOOR"));

	wstring wstrModelName = StringToWString(ModelName);
	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_Instance_"); //인스턴스 용 모델을 준비해야겠는디?
	ModelPrototypeTag += wstrModelName;

	lstrcpy(StaticMeshInstanceDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_StaticMesh_Instance"),
		iLevelIndex, LayerTag, &StaticMeshInstanceDesc)))
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
	Desc.m_eMeshLevelID = LEVEL::DH;
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
