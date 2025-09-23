#include "Level_KratCentralStation.h"
#include "GameInstance.h"
#include "Camera_Manager.h"
#include "Effect_Manager.h"
#include "EffectContainer.h"
#include "Client_Function.h"

#pragma region YW
//#include "StaticMesh.h"
//#include "StaticMesh_Instance.h"
//#include "Nav.h"
#include "Static_Decal.h"
//#include "Stargazer.h"
//#include "ErgoItem.h"
//#include "BreakableMesh.h"
#include "MapLoader.h"
#pragma endregion

#include "DefaultDoor.h"
#include "BossDoor.h"
#include "TriggerSound.h"
#include "TriggerTalk.h"
#include "TriggerUI.h"
#include "TriggerBGM.h"
#include "TriggerRain.h"
#include "Trigger_Effect.h"

#include "PBRMesh.h"
#include "WaterPuddle.h"

#include "DH_ToolMesh.h"     
#include "Level_Loading.h"
#include "UI_Container.h"
#include "UI_Video.h"

#include "Monster_Base.h"
#include "Player.h"
#include "Wego.h"

#include "LockOn_Manager.h"
#include "UI_Manager.h"
#include "Dynamic_UI.h"

#include "AnimatedProp.h"
#include "Client_Calculation.h"
#include "AreaSoundBox.h"

CLevel_KratCentralStation::CLevel_KratCentralStation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{

}

HRESULT CLevel_KratCentralStation::Initialize()
{
	m_pMapLoader = CMapLoader::Create(m_pDevice, m_pContext);
	
#ifndef TESTMAP
	//첫 맵(스테이션은 로딩완료), 게임중에 다른 맵 로딩 시작
	m_pMapLoader->Ready_Map_Async();
#endif

	/* [ 레벨 셋팅 ] */
	m_pGameInstance->SetCurrentLevelIndex(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION));
	m_pGameInstance->Set_IsChangeLevel(false);

	/* [ 사운드 ] */
	Start_BGM("AMB_SS_Train_In_03");

	if (FAILED(Ready_Sound()))
		return E_FAIL;

	if (FAILED(Ready_Video()))
		return E_FAIL;

	/* [ 셰이더 값 세팅 ] */
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
		TEXT("Com_ShaderPBR"), reinterpret_cast<CComponent**>(&m_pShaderComPBR))))
		return E_FAIL;
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxAnimMesh")),
		TEXT("Com_ShaderANIM"), reinterpret_cast<CComponent**>(&m_pShaderComANIM))))
		return E_FAIL;
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxMesh_Instance")),
		TEXT("Com_ShaderPBR"), reinterpret_cast<CComponent**>(&m_pShaderComInstance))))
		return E_FAIL;
	if (FAILED(Load_Shader()))
		return E_FAIL;

	if (FAILED(Ready_Level()))
		return E_FAIL;

	if (FAILED(Ready_Trigger()))
		return E_FAIL;	
	
	if (FAILED(Ready_TriggerEffect()))
		return E_FAIL;

	if (FAILED(Ready_TriggerBGM()))
		return E_FAIL;
	
	//if (FAILED(Ready_WaterPuddle()))
	//	return E_FAIL;

	Reset();
	return S_OK;
}

void CLevel_KratCentralStation::Priority_Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_F1))
	{
		CCamera_Manager::Get_Instance()->SetPlayer(nullptr);
		m_pGameInstance->Call_BeforeChangeLevel();

		CLockOn_Manager::Get_Instance()->Set_Off(nullptr);
		CEffect_Manager::Get_Instance()->Remove_AllStoredECs();
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOGO))))
			return;
	}

	if (m_pGameInstance->Key_Down(DIK_SPACE) && !m_bEndVideo)
	{
		if (m_pStartVideo)
			m_pStartVideo->Set_bDead();
		//	m_pStartVideo = nullptr;
		m_bEndVideo = true;

		/* [ 플레이어 제어 ] */
		m_pPlayer->GetCurrentAnimContrller()->SetState("Sit_End");
		CCamera_Manager::Get_Instance()->Play_CutScene(CUTSCENE_TYPE::WAKEUP);
	}

	if (m_pStartVideo->Get_bDead() && !m_bEndVideo)
	{
		//	m_pStartVideo = nullptr;

		m_pPlayer->GetCurrentAnimContrller()->SetState("Sit_End");
		CCamera_Manager::Get_Instance()->Play_CutScene(CUTSCENE_TYPE::WAKEUP);

		m_bEndVideo = true;
	}

	Update_ChangeBGM(fTimeDelta);
}

void CLevel_KratCentralStation::Update(_float fTimeDelta)
{
#ifndef TESTMAP
	if (m_pMapLoader->Check_MapLoadComplete(m_pGameInstance->GetCurrentLevelIndex()))
		Ready_OctoTree();
#endif

	if (!m_bEndVideo)
		return;

	if (KEY_DOWN(DIK_H))
		ToggleHoldMouse();
	if (m_bHold)
	{
		// 화면 중앙으로 마우스 위치 고정
		RECT rcClient;
		GetClientRect(g_hWnd, &rcClient);

		POINT ptCenter;
		ptCenter.x = (rcClient.right - rcClient.left) / 2;
		ptCenter.y = (rcClient.bottom - rcClient.top) / 2;

		// 클라이언트 좌표 -> 스크린 좌표로 변환
		ClientToScreen(g_hWnd, &ptCenter);

		// 마우스 커서 이동
		SetCursorPos(ptCenter.x, ptCenter.y);

		// 커서 강제로 숨기기
		while (ShowCursor(FALSE) >= 0);
		//HoldMouse();
	}
	else
	{
		// 커서 강제로 보이기
		while (ShowCursor(TRUE) < 0);
	}


	if (KEY_DOWN(DIK_F7))
		m_pGameInstance->ToggleDebugOctoTree();
	if (KEY_DOWN(DIK_F8))
		m_pGameInstance->ToggleDebugArea();

	if (KEY_DOWN(DIK_Y))
	{
	}



	m_pCamera_Manager->Update(fTimeDelta);
	CLockOn_Manager::Get_Instance()->Update(fTimeDelta);
	
	__super::Update(fTimeDelta);
}

void CLevel_KratCentralStation::Late_Update(_float fTimeDelta)
{
	CLockOn_Manager::Get_Instance()->Late_Update(fTimeDelta);

	Add_RenderGroup_OctoTree();

	/* [ 번개 라이트 설정 ] */
	UpdateThunder(fTimeDelta);
}

HRESULT CLevel_KratCentralStation::Render()
{
	SetWindowText(g_hWnd, TEXT("Lies Of P .. GamePlay"));

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Reset()
{
	list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), L"Layer_FireEater");
	for (auto& obj : objList)
	{
		if (auto pBoss = dynamic_cast<CEliteUnit*>(obj))
		{
			if (pBoss->GetCurrentState() != CEliteUnit::EEliteState::CUTSCENE
				&& pBoss->GetCurrentState() != CEliteUnit::EEliteState::DEAD)
				m_pGameInstance->Return_PoolObject(L"Layer_FireEater", obj, true);
		}
	}

	objList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), L"Layer_FestivalLeader");
	for (auto& obj : objList)
	{
		if (auto pBoss = dynamic_cast<CEliteUnit*>(obj))
		{
			if (pBoss->GetCurrentState() != CEliteUnit::EEliteState::CUTSCENE
				&& pBoss->GetCurrentState() != CEliteUnit::EEliteState::DEAD)
				m_pGameInstance->Return_PoolObject(L"Layer_FestivalLeader", obj, true);
		}
	}

	objList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), L"Layer_Monster_Normal");
	for (auto& obj : objList)
		m_pGameInstance->Return_PoolObject(L"Layer_Monster_Normal", obj, false);

	m_pGameInstance->UseAll_PoolObjects(L"Layer_FireEater", false);
	m_pGameInstance->UseAll_PoolObjects(L"Layer_FestivalLeader", false);
	m_pGameInstance->UseAll_PoolObjects(L"Layer_Monster_Normal", true);

	//부서지는 메쉬 리셋
	objList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), L"Layer_BreakableMesh");
	for (auto& obj : objList)
		obj->Reset();

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Level()
{
	/* [ 해야할 준비들 ] */
	/*if (FAILED(Ready_Dummy()))
		return E_FAIL;*/

#ifndef TESTMAP

	if (FAILED(m_pMapLoader->Add_MapActor(START_MAP)))//맵 액터(콜라이더) 추가
		return E_FAIL;
#endif // !TESTMAP

#ifdef TEST_TEST_MAP
	if (FAILED(m_pMapLoader->Add_MapActor("TEST")))//맵 액터(콜라이더) 추가
		return E_FAIL;
#endif // TEST_TEST_MAP

#ifdef TEST_STATION_MAP	
	if (FAILED(m_pMapLoader->Add_MapActor("STATION")))//맵 액터(콜라이더) 추가
		return E_FAIL;
#endif //TEST_STATION_MAP

#ifdef TEST_FIRE_EATER_MAP
	if (FAILED(m_pMapLoader->Add_MapActor("FIRE_EATER")))//맵 액터(콜라이더) 추가
		return E_FAIL;
#endif // TEST_FIRE_EATER_MAP

#ifdef TEST_HOTEL_MAP
	if (FAILED(m_pMapLoader->Add_MapActor("HOTEL")))//맵 액터(콜라이더) 추가
		return E_FAIL;
#endif // TEST_HOTEL

#ifdef TEST_OUTER_MAP
	if (FAILED(m_pMapLoader->Add_MapActor("OUTER")))//맵 액터(콜라이더) 추가
		return E_FAIL;
#endif // TEST_OUTER_MAP


	//고사양 모드
	if (FAILED(Ready_Lights()))
		return E_FAIL;

	//저사양 모드
	//if (FAILED(Ready_Lights_LowQuality()))
	//	return E_FAIL;
	
	if (FAILED(Ready_OctoTree()))
		return E_FAIL;
	if (FAILED(Separate_Area()))
		return E_FAIL;
	if (FAILED(Ready_Effect()))
		return E_FAIL;
	if (FAILED(Ready_Camera()))
		return E_FAIL;
	if (FAILED(Ready_Layer_Sky(TEXT("Layer_Sky"))))
		return E_FAIL;
	if (FAILED(Ready_Npc()))
		return E_FAIL;
	if (FAILED(Ready_UI()))
		return E_FAIL;
	if (FAILED(Ready_Player()))
		return E_FAIL;
	if (FAILED(Ready_Thunder()))
		return E_FAIL;


#ifndef TESTMAP
	//모든 몬스터, 스타게이저, 부수는 오브젝트, 별바라기 소환하자.
	if (FAILED(m_pMapLoader->Load_Ready_All_Etc(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION))))
		return E_FAIL;
	cout << "[MAP] Load_Ready_All_Etc 완료" << endl;
#endif

	if (FAILED(m_pMapLoader->Ready_Monster()))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_ErgoItem()))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_Breakable()))
		return E_FAIL;
	if (FAILED(m_pMapLoader->Ready_Stargazer()))
		return E_FAIL;

	// 문 같이 상호작용 하는 것들
	if (FAILED(Ready_Interact()))
		return E_FAIL;

	if (FAILED(Ready_AnimatedProp()))
		return E_FAIL;

	return S_OK;
}

void CLevel_KratCentralStation::Apply_AreaBGM()
{
	AREAMGR areaMgr = m_pGameInstance->GetCurrentAreaMgr();
	switch (areaMgr)
	{
	case Engine::AREAMGR::STATION:
		m_pGameInstance->Change_BGM("AMB_SS_Train_Out_Rain");
		break;
	case Engine::AREAMGR::HOTEL:
		m_pGameInstance->Change_BGM("AMB_SS_Cathedral_Hall");
		break;
	case Engine::AREAMGR::FUOCO:
		m_pGameInstance->Change_BGM("");
		break;
	case Engine::AREAMGR::OUTER:
	case Engine::AREAMGR::FESTIVAL:
		m_pGameInstance->Change_BGM("AMB_SS_Rain_02");
		break;
	default:
		break;
	}
}

HRESULT CLevel_KratCentralStation::Ready_Player()
{
	CPlayer::PLAYER_DESC pDesc{};
	//pDesc.fSpeedPerSec = 1.f;
	pDesc.fSpeedPerSec = 5.f;
	pDesc.fRotationPerSec = XMConvertToRadians(600.0f);
	pDesc.eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
	pDesc.InitPos = _float3(-1.35f, 0.978f, 0.96f);
	pDesc.InitScale = _float3(1.f, 1.f, 1.f);
	lstrcpy(pDesc.szName, TEXT("Player"));
	pDesc.szMeshID = TEXT("Player");
	pDesc.iLevelID = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Player"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"), &pGameObject, &pDesc)))
		return E_FAIL;

	m_pPlayer = dynamic_cast<CPlayer*>(pGameObject);

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Npc()
{
	CWego::WEGO_DESC pWegoDesc{};
	//pDesc.fSpeedPerSec = 1.f;
	pWegoDesc.fSpeedPerSec = 5.f;
	pWegoDesc.fRotationPerSec = XMConvertToRadians(600.0f);
	pWegoDesc.eMeshLevelID = LEVEL::STATIC;
	pWegoDesc.InitPos = _float3(54.638927f, -0.221457f, -10.478647f);
	pWegoDesc.InitScale = _float3(1.f, 1.f, 1.f);
	lstrcpy(pWegoDesc.szName, TEXT("Wego"));
	pWegoDesc.szMeshID = TEXT("Wego");
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Wego"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Wego"), &pWegoDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Dummy()
{
	CPBRMesh::STATICMESH_DESC Desc{};
	Desc.iRender = 0;
	Desc.m_eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
	Desc.szMeshID = TEXT("OutDoor");
	lstrcpy(Desc.szName, TEXT("OutDoor"));
	//lstrcpy(Desc.szModelPrototypeTag, TEXT("Prototype_Component_Model_Hotel"));

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Dummy"), &pGameObject, &Desc)))
		return E_FAIL;

	//m_pGameInstance->PushOctoTreeObjects(pGameObject);

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Lights()
{
	wstring basePath = L"../Bin/Save/LightInfomation/";
	wstring fileName = L"Light_Information.json";

	ifstream ifs(basePath + fileName);
	if (!ifs.is_open())
		return E_FAIL;

	nlohmann::json jLights;
	ifs >> jLights;
	ifs.close();

	for (const auto& jLight : jLights)
	{
		// 1. 기본 정보 추출
		vector<vector<float>> matrixArray = jLight["WorldMatrix"];
		_float fIntensity = jLight["Intensity"];
		_float fRange = jLight["Range"];
		vector<float> colorVec = jLight["Color"];

		_float fInnerCosAngle = jLight["InnerCosAngle"];
		_float fOuterCosAngle = jLight["OuterCosAngle"];
		_float fFalloff = jLight["Falloff"];
		_float fFogDensity = jLight["FogDensity"];
		_float fFogCutOff = jLight["FogCutOff"];
		_bool  bVolumetricMode = jLight["Volumetric"];
		_int   iLightCustom = jLight["LightCustom"];

		CDHTool::LIGHT_TYPE eLightType = static_cast<CDHTool::LIGHT_TYPE>(jLight["LightType"].get<int>());
		CDHTool::LEVEL_TYPE eLevelType = static_cast<CDHTool::LEVEL_TYPE>(jLight["LevelType"].get<int>());

		// 2. 행렬 복원
		XMFLOAT4X4 mat;
		mat._11 = matrixArray[0][0]; mat._12 = matrixArray[0][1]; mat._13 = matrixArray[0][2]; mat._14 = matrixArray[0][3];
		mat._21 = matrixArray[1][0]; mat._22 = matrixArray[1][1]; mat._23 = matrixArray[1][2]; mat._24 = matrixArray[1][3];
		mat._31 = matrixArray[2][0]; mat._32 = matrixArray[2][1]; mat._33 = matrixArray[2][2]; mat._34 = matrixArray[2][3];
		mat._41 = matrixArray[3][0]; mat._42 = matrixArray[3][1]; mat._43 = matrixArray[3][2]; mat._44 = matrixArray[3][3];
		_matrix matWorld = XMLoadFloat4x4(&mat);

		// 3. 라이트 생성
		if (FAILED(Add_Light(eLightType, eLevelType)))
			continue;

		// 4. 생성된 라이트 설정
		CDH_ToolMesh* pNewLight = m_vecLights.back();
		if (!pNewLight)
			continue;

		pNewLight->Get_TransfomCom()->Set_WorldMatrix(matWorld);
		pNewLight->SetIntensity(fIntensity);
		pNewLight->SetRange(fRange);
		pNewLight->SetColor(_float4(colorVec[0], colorVec[1], colorVec[2], colorVec[3]));
		pNewLight->SetfInnerCosAngle(fInnerCosAngle);
		pNewLight->SetfOuterCosAngle(fOuterCosAngle);
		pNewLight->SetfFalloff(fFalloff);
		pNewLight->SetfFogDensity(fFogDensity);
		pNewLight->SetfFogCutOff(fFogCutOff);
		pNewLight->SetbVolumetric(bVolumetricMode);
		pNewLight->SetLightCustom(iLightCustom);

		if (iLightCustom == 1)
			m_pGameInstance->AddCustomLight(TEXT("Festival_Light"), pNewLight);
		if (iLightCustom == 2)
			m_pGameInstance->AddCustomLight(TEXT("Lamp_Light"), pNewLight);
	}

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Lights_LowQuality()
{
	m_pGameInstance->RemoveAll_Light(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION));

	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.fAmbient = 0.6f;
	LightDesc.fIntensity = 0.8f;
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(1.f, -0.5f, 1.f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.fFogDensity = 0.f;

	if (FAILED(m_pGameInstance->Add_LevelLightData(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), LightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Add_Light(CDHTool::LIGHT_TYPE eType, CDHTool::LEVEL_TYPE eLType)
{
	CDH_ToolMesh::DHTOOL_DESC Desc{};
	if (eType == CDHTool::LIGHT_TYPE::POINT)
	{
		Desc.szMeshID = TEXT("PointLight");
		lstrcpy(Desc.szName, TEXT("PointLight"));
	}
	if (eType == CDHTool::LIGHT_TYPE::SPOT)
	{
		Desc.szMeshID = TEXT("SpotLight");
		lstrcpy(Desc.szName, TEXT("SpotLight"));
	}
	if (eType == CDHTool::LIGHT_TYPE::DIRECTIONAL)
	{
		Desc.szMeshID = TEXT("DirrectionalLight");
		lstrcpy(Desc.szName, TEXT("DirrectionalLight"));
	}

	if (eLType == CDHTool::LEVEL_TYPE::KRAT_CENTERAL_STATION)
		Desc.eLEVEL = LEVEL::KRAT_CENTERAL_STATION;
	if (eLType == CDHTool::LEVEL_TYPE::KRAT_HOTEL)
		Desc.eLEVEL = LEVEL::KRAT_HOTEL;


	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.m_vInitPos = _float3(0.f, 10.f, 10.f);
	Desc.iID = 0;

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_ToolMesh"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), L"Layer_ToolMesh", &pGameObject, &Desc)))
		return E_FAIL;

	m_vecLights.push_back(dynamic_cast<CDH_ToolMesh*>(pGameObject));

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Add_RenderGroup_OctoTree()
{
	_matrix matView = XMLoadFloat4x4(m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW));
	_matrix matProj = XMLoadFloat4x4(m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ));

	m_pGameInstance->BeginQueryFrame(matView, matProj);
	m_pGameInstance->QueryVisible();

	vector<class CGameObject*> AllStaticMesh = m_pGameInstance->GetIndexToObj();
	vector<_uint> VisitCell = m_pGameInstance->GetCulledStaticObjects();
	vector<OCTOTREEOBJECTTYPE> vTypeTable = m_pGameInstance->GetObjectType();

	for (_uint i = 0; i < static_cast<_uint>(AllStaticMesh.size()); ++i)
		AllStaticMesh[i]->Set_bLightOnOff(false);


	for (_uint iIdx : VisitCell)
	{
		CGameObject* pObj = AllStaticMesh[iIdx];

		if (nullptr == pObj || pObj->Get_isActive() == false)
			continue;

		if (vTypeTable[iIdx] == OCTOTREEOBJECTTYPE::MESH)
		{
			m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, pObj);
			pObj->Set_IsLOD(true);
		}
		else
		{
			//만약 조명오브젝트라면? 
			pObj->Set_bLightOnOff(true);
		}
	}

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Separate_Area()
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
	_float3 a19p0 = _float3{ 160.44f, 42.46f, -266.51f };
	_float3 a19p1 = _float3{ -42.91f, 0.20f, -80.25f };
	_float3 a19Min, a19Max;
	FnToAABB(a19p0, a19p1, a19Min, a19Max);

	// Area 20 
	_float3 a20p0 = _float3{ 162.2489f, 0.f, 4.0193f };
	_float3 a20p1 = _float3{ 186.83f, 48.18f, -20.2639f };
	_float3 a20Min, a20Max;
	FnToAABB(a20p0, a20p1, a20Min, a20Max);

	// Area 21
	_float3 a21p0 = _float3{ 176.838f, 0.f, -2.1049f };
	_float3 a21p1 = _float3{ 184.8192f, 48.18f, -12.4495f };
	_float3 a21Min, a21Max;
	FnToAABB(a21p0, a21p1, a21Min, a21Max);

	// Area 22
	_float3 a22p0 = _float3{ 190.0822f, 0.f, -4.3780f };
	_float3 a22p1 = _float3{ 181.0464f, 48.1307f, -12.5612f };
	_float3 a22Min, a22Max;
	FnToAABB(a22p0, a22p1, a22Min, a22Max);
	
	// ------------- Area 50 --------------
	_float3 a50p0 = _float3{ 168.3228f, 61.1988f, 62.4843f };
	_float3 a50p1 = _float3{ 274.4402f, -25.574f, -92.6368f };
	_float3 a50Min, a50Max;
	FnToAABB(a50p0, a50p1, a50Min, a50Max);

	// ------------- Area 51 --------------
	_float3 a51p0 = _float3{ 222.4114f, 49.5397f, -31.8351f };
	_float3 a51p1 = _float3{ 278.6766f, 3.0048f, -92.0069f };
	_float3 a51Min, a51Max;
	FnToAABB(a51p0, a51p1, a51Min, a51Max);

	// ------------- Area 52 --------------
	_float3 a52p0 = _float3{ 259.0224f, -3.9186f, 0.5415f };
	_float3 a52p1 = _float3{ 306.6759f, 55.6382f, -43.3137f };
	_float3 a52Min, a52Max;
	FnToAABB(a52p0, a52p1, a52Min, a52Max);

	// ------------- Area 53 --------------
	_float3 a53p0 = _float3{ 274.1295f, 33.1423f, -4.9048f };
	_float3 a53p1 = _float3{ 310.5031f, -5.5138f, 20.2391f };
	_float3 a53Min, a53Max;
	FnToAABB(a53p0, a53p1, a53Min, a53Max);

	// ------------- Area 54 --------------
	_float3 a54p0 = _float3{ 303.3356f, 35.4927f, -54.3758f };
	_float3 a54p1 = _float3{ 277.5223f, 3.7437f, -30.f };
	_float3 a54Min, a54Max;
	FnToAABB(a54p0, a54p1, a54Min, a54Max);

	// ------------- Area 55 --------------
	_float3 a55p0 = _float3{ 300.7349f, 34.1267f, -27.0379f };
	_float3 a55p1 = _float3{ 345.4542f, -10.8288f, -47.1570f };
	_float3 a55Min, a55Max;
	FnToAABB(a55p0, a55p1, a55Min, a55Max);

	// ------------- Area 56 --------------
	_float3 a56p0 = _float3{ 343.6360f, -9.9221f, -35.5008f };
	_float3 a56p1 = _float3{ 303.3795f, 32.7578f, 8.8136f };
	_float3 a56Min, a56Max;
	FnToAABB(a56p0, a56p1, a56Min, a56Max);

	// ------------- Area 57 --------------
	_float3 a57p0 = _float3{ 343.6360f, -9.9221f, -35.5008f };
	_float3 a57p1 = _float3{ 301.3311f, 69.7224f, -129.5032f };
	_float3 a57Min, a57Max;
	FnToAABB(a57p0, a57p1, a57Min, a57Max);

	// ------------- Area 58 --------------
	_float3 a58p0 = _float3{ 342.1822f, 1.2864f, -11.8887f };
	_float3 a58p1 = _float3{ 373.2293f, 45.0072f, -45.6612f };
	_float3 a58Min, a58Max;
	FnToAABB(a58p0, a58p1, a58Min, a58Max);

	// ------------- Area 59 --------------
	_float3 a59p0 = _float3{ 373.2293f, 45.0072f, -45.6612f };
	_float3 a59p1 = _float3{ 350.6409f, 5.9597f, -66.2621f };
	_float3 a59Min, a59Max;
	FnToAABB(a59p0, a59p1, a59Min, a59Max);

	// ------------- Area 60 --------------
	_float3 a60p0 = _float3{ 370.8979f, 52.3032f, -84.6963f };
	_float3 a60p1 = _float3{ 439.8365f, 5.1572f, -10.2087f };
	_float3 a60Min, a60Max;
	FnToAABB(a60p0, a60p1, a60Min, a60Max);

	// ------------- Area 61 --------------
	_float3 a61p0 = _float3{ 168.32f, 61.1988f, 62.4843f };
	_float3 a61p1 = _float3{ 203.69f, -25.574f, -92.6368f };
	_float3 a61Min, a61Max;
	FnToAABB(a61p0, a61p1, a61Min, a61Max);
	// ------------- Area 62 --------------
	_float3 a62p0 = _float3{ 203.69f, 61.1988f, 62.4843f };
	_float3 a62p1 = _float3{ 239.06f, -25.574f, -92.6368f };
	_float3 a62Min, a62Max;
	FnToAABB(a62p0, a62p1, a62Min, a62Max);
	// ------------- Area 63 --------------
	_float3 a63p0 = _float3{ 239.06f, 61.1988f, 62.4843f };
	_float3 a63p1 = _float3{ 274.44f, -25.574f, -92.6368f };
	_float3 a63Min, a63Max;
	FnToAABB(a63p0, a63p1, a63Min, a63Max);

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
		const vector<_uint> vecAdj7 = { 8, 9, 14, 12, 18, 20 };
		if (!m_pGameInstance->AddArea_AABB(
			7, a7Min, a7Max, vecAdj7, AREA::EAreaType::INDOOR, ENUM_CLASS(AREA::EAreaType::INDOOR)))
			return E_FAIL;
	}
	{
		/* [ 8번 구역 ] */
		const vector<_uint> vecAdj8 = { 9, 10, 11, 20 };
		if (!m_pGameInstance->AddArea_AABB(
			8, a8Min, a8Max, vecAdj8, AREA::EAreaType::ROOM, ENUM_CLASS(AREA::EAreaType::ROOM)))
			return E_FAIL;
	}
	{
		/* [ 9번 구역 ] */
		const vector<_uint> vecAdj9 = { 7, 8, 10, 11 };
		if (!m_pGameInstance->AddArea_AABB(
			9, a9Min, a9Max, vecAdj9, AREA::EAreaType::HOUSE, ENUM_CLASS(AREA::EAreaType::HOUSE)))
			return E_FAIL;
	}
	{
		/* [ 10번 구역 ] */
		const vector<_uint> vecAdj10 = { 9 };
		if (!m_pGameInstance->AddArea_AABB(
			10, a10Min, a10Max, vecAdj10, AREA::EAreaType::LOBBY, ENUM_CLASS(AREA::EAreaType::LOBBY)))
			return E_FAIL;
	}
	{
		/* [ 11번 구역 ] */
		const vector<_uint> vecAdj11 = { 9, 12 };
		if (!m_pGameInstance->AddArea_AABB(
			11, a11Min, a11Max, vecAdj11, AREA::EAreaType::LOBBY, ENUM_CLASS(AREA::EAreaType::LOBBY)))
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
		/* [ 푸오코 구역 ] */
		const vector<_uint> vecAdj19 = {  };
		if (!m_pGameInstance->AddArea_AABB(
			19, a19Min, a19Max, vecAdj19, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 20번 구역 ] */
		const vector<_uint> vecAdj20 = { 7, 8, 9, 11, 21, 22, 61 };
		if (!m_pGameInstance->AddArea_AABB(
			20, a20Min, a20Max, vecAdj20, AREA::EAreaType::LOBBY, ENUM_CLASS(AREA::EAreaType::LOBBY)))
			return E_FAIL;
	}
	{
		/* [ 21번 구역 ] */
		const vector<_uint> vecAdj21 = { 7, 20, 22, 61, 62 };
		if (!m_pGameInstance->AddArea_AABB(
			21, a21Min, a21Max, vecAdj21, AREA::EAreaType::ROOM, ENUM_CLASS(AREA::EAreaType::ROOM)))
			return E_FAIL;
	}
	{
		/* [ 22번 구역 ] */
		const vector<_uint> vecAdj22 = { 7, 20, 50, 52 };
		if (!m_pGameInstance->AddArea_AABB(
			22, a22Min, a22Max, vecAdj22, AREA::EAreaType::ROOM, ENUM_CLASS(AREA::EAreaType::ROOM)))
			return E_FAIL;
	}



	/* ---------- 야외 ------------ */
	{
		/* [ 50번 구역 ] */
		const vector<_uint> vecAdj50 = { 20, 51, 52, 61, 62, 63 };
		if (!m_pGameInstance->AddArea_AABB(
			50, a50Min, a50Max, vecAdj50, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 51번 구역 ] */
		const vector<_uint> vecAdj51 = { 50, 52, 53 };
		if (!m_pGameInstance->AddArea_AABB(
			51, a51Min, a51Max, vecAdj51, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 52번 구역 ] */
		const vector<_uint> vecAdj52 = { 50, 53, 54 };
		if (!m_pGameInstance->AddArea_AABB(
			52, a52Min, a52Max, vecAdj52, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 53번 구역 ] */
		const vector<_uint> vecAdj53 = { 52, 54 };
		if (!m_pGameInstance->AddArea_AABB(
			53, a53Min, a53Max, vecAdj53, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 54번 구역 ] */
		const vector<_uint> vecAdj54 = { 52, 53, 55, 58, 59 };
		if (!m_pGameInstance->AddArea_AABB(
			54, a54Min, a54Max, vecAdj54, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 55번 구역 ] */
		const vector<_uint> vecAdj55 = { 54, 56, 57, 58, 59 };
		if (!m_pGameInstance->AddArea_AABB(
			55, a55Min, a55Max, vecAdj55, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 56번 구역 ] */
		const vector<_uint> vecAdj56 = { 55, 57, 58, 59 };
		if (!m_pGameInstance->AddArea_AABB(
			56, a56Min, a56Max, vecAdj56, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 57번 구역 ] */
		const vector<_uint> vecAdj57 = { 55, 56 };
		if (!m_pGameInstance->AddArea_AABB(
			57, a57Min, a57Max, vecAdj57, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 58번 구역 ] */
		const vector<_uint> vecAdj58 = { 54, 55, 56, 57, 59, 60 };
		if (!m_pGameInstance->AddArea_AABB(
			58, a58Min, a58Max, vecAdj58, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 59번 구역 ] */
		const vector<_uint> vecAdj59 = { 55, 56, 58, 60 };
		if (!m_pGameInstance->AddArea_AABB(
			59, a59Min, a59Max, vecAdj59, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 60번 구역 ] */
		const vector<_uint> vecAdj60 = { 59, 58 };
		if (!m_pGameInstance->AddArea_AABB(
			60, a60Min, a60Max, vecAdj60, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 61번 구역 ] */
		const vector<_uint> vecAdj61 = { 20, 50, 52, 62, 63 };
		if (!m_pGameInstance->AddArea_AABB(
			61, a61Min, a61Max, vecAdj61, AREA::EAreaType::INDOOR, ENUM_CLASS(AREA::EAreaType::INDOOR)))
			return E_FAIL;
	}
	{
		/* [ 62번 구역 ] */
		const vector<_uint> vecAdj62 = { 20, 50, 51, 52, 61, 63 };
		if (!m_pGameInstance->AddArea_AABB(
			62, a62Min, a62Max, vecAdj62, AREA::EAreaType::INDOOR, ENUM_CLASS(AREA::EAreaType::INDOOR)))
			return E_FAIL;
	}
	{
		/* [ 63번 구역 ] */
		const vector<_uint> vecAdj63 = { 50, 51, 52, 53, 61, 62 };
		if (!m_pGameInstance->AddArea_AABB(
			63, a63Min, a63Max, vecAdj63, AREA::EAreaType::INDOOR, ENUM_CLASS(AREA::EAreaType::INDOOR)))
			return E_FAIL;
	}
	if (FAILED(m_pGameInstance->FinalizePartition()))
		return E_FAIL;


	return S_OK;
}

void CLevel_KratCentralStation::InitThunder()
{
	m_tThunder.fNextStrike = 1.f + (static_cast<_float>(rand()) / RAND_MAX) * 3.f;
}

void CLevel_KratCentralStation::UpdateThunder(_float fTimeDelta)
{
	_int iNumArea = m_pGameInstance->GetCurAreaIds();
	if (iNumArea == 3 || iNumArea == 4 || iNumArea == 5
		|| m_pGameInstance->GetCurrentAreaMgr() == AREAMGR::OUTER
		|| m_pGameInstance->GetCurrentAreaMgr() == AREAMGR::FESTIVAL)
	{
		if (m_pGameInstance->GetCurrentAreaMgr() == AREAMGR::OUTER)
			m_vecThunder[0]->Get_TransfomCom()->Set_State(STATE::POSITION, XMVectorSet(231.6f, 50.f, 1.35f, 1.f));
		else if (m_pGameInstance->GetCurrentAreaMgr() == AREAMGR::FESTIVAL)
			m_vecThunder[0]->Get_TransfomCom()->Set_State(STATE::POSITION, XMVectorSet(359.f, 50.f, -40.f, 1.f));

		m_vecThunder[0]->SetbLightUse(true);

		m_tThunder.fTimer += fTimeDelta;
		_float fIntensity = 0.f;

		if (!m_tThunder.bStriking)
		{
			// 대기 상태
			if (m_tThunder.fTimer >= m_tThunder.fNextStrike)
			{
				m_tThunder.bStriking = true;
				m_tThunder.iBlinkIndex = 0;
				m_tThunder.fTimer = 0.f;
				m_tThunder.fBlinkDuration = 0.1f + (static_cast<_float>(rand()) / RAND_MAX) * 0.05f;
			}
		}
		else
		{
			// 깜빡임 중
			if (m_tThunder.iBlinkIndex == 0)
			{
				_float fNoise = (static_cast<_float>(rand()) / RAND_MAX) * 0.3f;
				fIntensity = 1.7f + fNoise;

				if (m_tThunder.fTimer >= m_tThunder.fBlinkDuration)
				{
					// 두 번째 깜빡임 준비
					m_tThunder.iBlinkIndex = 1;
					m_tThunder.fTimer = 0.f;
					m_tThunder.fBlinkDuration = 0.05f + (static_cast<_float>(rand()) / RAND_MAX) * 0.1f;
				}
			}
			else if (m_tThunder.iBlinkIndex == 1)
			{
				_float fNoise = (static_cast<_float>(rand()) / RAND_MAX) * 0.2f;
				fIntensity = 1.2f + fNoise;

				if (m_tThunder.fTimer >= m_tThunder.fBlinkDuration)
				{
					PlayThunderSound();
					m_tThunder.bStriking = false;
					m_tThunder.fTimer = 0.f;
					m_tThunder.fNextStrike = 4.5f + (static_cast<_float>(rand()) / RAND_MAX) * 6.f;
				}
			}
		}

		if (!m_vecThunder.empty() && m_vecThunder[0] != nullptr)
			m_vecThunder[0]->SetIntensity(fIntensity);
	}
	else
	{
		m_vecThunder[0]->SetIntensity(0.f);
	}
}

void CLevel_KratCentralStation::PlayThunderSound()
{
	/* [ 사운드 크기 설정 ] */
	m_pThunderSoundCom->SetVolume("AMB_SS_Thunder_Rumble_01", 1.f);
	m_pThunderSoundCom->SetVolume("AMB_SS_Thunder_Rumble_02", 1.f);
	m_pThunderSoundCom->SetVolume("AMB_SS_Thunder_Rumble_03", 1.f);
	m_pThunderSoundCom->SetVolume("AMB_SS_Thunder_Rumble_04", 1.f);
	m_pThunderSoundCom->SetVolume("AMB_SS_Thunder_Rumble_05", 1.f);
	m_pThunderSoundCom->SetVolume("AMB_SS_Thunder_Rumble_06", 1.f);

	_int iRand = GetRandomInt(1, 6);
	switch (iRand)
	{
	case 1:
		m_pThunderSoundCom->Play("AMB_SS_Thunder_Rumble_01");
		break;

	case 2:
		m_pThunderSoundCom->Play("AMB_SS_Thunder_Rumble_02");
		break;

	case 3:
		m_pThunderSoundCom->Play("AMB_SS_Thunder_Rumble_03");
		break;

	case 4:
		m_pThunderSoundCom->Play("AMB_SS_Thunder_Rumble_04");
		break;

	case 5:
		m_pThunderSoundCom->Play("AMB_SS_Thunder_Rumble_05");
		break;

	case 6:
		m_pThunderSoundCom->Play("AMB_SS_Thunder_Rumble_06");
		break;
	
	default:
		break;
	}
}

HRESULT CLevel_KratCentralStation::Ready_Camera()
{
	m_pCamera_Manager->SetFreeCam();

	return S_OK;
}


HRESULT CLevel_KratCentralStation::Ready_Layer_Sky(const _wstring strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Sky"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), strLayerTag)))
		return E_FAIL;

	return S_OK;
}


HRESULT CLevel_KratCentralStation::Ready_UI()
{

	CUI_Container::UI_CONTAINER_DESC eDesc = {};

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Panel_Player_LU.json");

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player_Panel"), &eDesc)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Panel_Player_LD"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player_Panel"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Panel_Player_RD"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player_Panel"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Panel_Player_RU"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player_Panel"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Panel_Player_Arm"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player_Panel"))))
		return E_FAIL;


	CUIObject::UIOBJECT_DESC eLockonDesc = {};

	eLockonDesc.fSizeX = 64.f;
	eLockonDesc.fSizeY = 64.f;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Fatal_Icon"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Lockon_Icon"), &eLockonDesc)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_LockOn_Icon"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Lockon_Icon"), &eLockonDesc)))
		return E_FAIL;



	eDesc.strFilePath = TEXT("../Bin/Save/UI/Popup/Popup.json");

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Popup"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_UI_Popup"), &eDesc)))
		return E_FAIL;


	eDesc.strFilePath = TEXT("../Bin/Save/UI/Script/Script_Text.json");

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Script_Text"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Script_Text"), &eDesc)))
		return E_FAIL;

	CGameObject* pTextScript = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Script_Text"));

	CUI_Manager::Get_Instance()->Emplace_UI(dynamic_cast<CUIObject*>(pTextScript), L"TextScript");

	CUI_Manager::Get_Instance()->Activate_TextScript(false);

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Script/Script_Talk_Background.json");

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Script_Talk"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Script_Text"), &eDesc)))
		return E_FAIL;

	pTextScript = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Script_Text"));

	CUI_Manager::Get_Instance()->Emplace_UI(dynamic_cast<CUIObject*>(pTextScript), L"TalkScript");
	CUI_Manager::Get_Instance()->Activate_TalkScript(false);

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Pickup_Item"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Pickup_Item"))))
		return E_FAIL;

	/*
	CUI_Container::UI_CONTAINER_DESC eDesc = {};

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Death.json");

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player_UI_Death"), &eDesc)))
		return E_FAIL;*/

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container_DeBuff"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player_DeBuff_UI"))))
		return E_FAIL;


	//
	CDynamic_UI::DYNAMIC_UI_DESC eDynamicDesc = {};
	eDynamicDesc.fAlpha = 0.f;
	eDynamicDesc.fSizeX = g_iWinSizeX;
	eDynamicDesc.fSizeY = g_iWinSizeY;
	eDynamicDesc.fX = g_iWinSizeX * 0.5f;
	eDynamicDesc.fY = g_iWinSizeY * 0.5f;
	eDynamicDesc.vColor = { 0.f,0.f,0.f,1.f };
	eDynamicDesc.strTextureTag = TEXT("Prototype_Component_Texture_BackGround_Loading_Desk");
	eDynamicDesc.iPassIndex = 2;


	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dynamic_UI"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_BackGround_Base"), &eDynamicDesc)))
		return E_FAIL;

	CGameObject* pBackGround = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_BackGround_Base"));

	CUI_Manager::Get_Instance()->Emplace_UI(dynamic_cast<CUIObject*>(pBackGround), L"BackGround");

	CUI_Manager::Get_Instance()->Off_Panel();

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Video()
{

	CUI_Video::VIDEO_UI_DESC eDesc = {};
	eDesc.eType = CUI_Video::VIDEO_TYPE::INTRO;
	eDesc.fOffset = 0.0f;
	eDesc.fInterval = 0.016f;
	eDesc.fSpeedPerSec = 1.f;
	eDesc.strVideoPath = TEXT("../Bin/Resources/Video/Startscene.mp4");
	eDesc.fX = g_iWinSizeX * 0.5f;
	eDesc.fY = g_iWinSizeY * 0.5f;
	eDesc.fSizeX = g_iWinSizeX;
	eDesc.fSizeY = g_iWinSizeY;
	eDesc.fAlpha = 1.f;
	eDesc.isLoop = false;


	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Video"),
		static_cast<_uint>(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Background_Video"), &eDesc)))
		return E_FAIL;


	m_pStartVideo = static_cast<CUI_Video*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Background_Video")));
	//	Safe_AddRef(m_pStartVideo);



	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Effect()
{
	_matrix presetmat = XMMatrixIdentity();
	CEffectContainer::DESC ECDesc = {};

	CGameObject* pEC = { nullptr };

	presetmat = XMMatrixTranslation(62.f, 10.f, -7.2f);
	XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	pEC = MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_Rain_Changed"), &ECDesc);
	if (pEC == nullptr)
		MSG_BOX("이펙트 생성 실패");
	EFFECT_MANAGER->Store_EffectContainer(TEXT("StationRain_1"), static_cast<CEffectContainer*>(pEC));

	pEC = nullptr;
	presetmat = XMMatrixTranslation(86.8f, 10.f, -7.3f);
	XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	pEC = MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_Rain_Changed"), &ECDesc);
	if (pEC == nullptr)
		MSG_BOX("이펙트 생성 실패");

	EFFECT_MANAGER->Store_EffectContainer(TEXT("StationRain_2"), static_cast<CEffectContainer*>(pEC));

	pEC = nullptr;
	presetmat = XMMatrixTranslation(194.0f, 18.f, -8.0f);
	XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	pEC = MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_Rain_NewOuterWelcomeRain"), &ECDesc);
	if (pEC == nullptr)
		MSG_BOX("이펙트 생성 실패");

	EFFECT_MANAGER->Store_EffectContainer(TEXT("OuterRain_1"), static_cast<CEffectContainer*>(pEC));

	pEC = nullptr;
	presetmat = XMMatrixTranslation(403.f, 31.f, -49.0f);
	XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	pEC = MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_Rain_BossArea_Normal"), &ECDesc);
	if (pEC == nullptr)
		MSG_BOX("이펙트 생성 실패");

	EFFECT_MANAGER->Store_EffectContainer(TEXT("BossroomRain_1"), static_cast<CEffectContainer*>(pEC));

	pEC = nullptr;
	presetmat = XMMatrixTranslation(366.2f, 20.0f, -48.0f);
	XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	pEC = MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_Rain_BossDoor"), &ECDesc);
	if (pEC == nullptr)
		MSG_BOX("이펙트 생성 실패");

	EFFECT_MANAGER->Store_EffectContainer(TEXT("BossroomRain_1"), static_cast<CEffectContainer*>(pEC));


	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_OctoTree()
{
	m_pGameInstance->ClearIndexToObj();

	vector<AABBBOX> staticBounds;
	vector<OCTOTREEOBJECTTYPE>  vObjectType;
	map<Handle, _uint> handleToIndex;

	//용량 확보: 메쉬 + 라이트
	vector<class CGameObject*> OctoObject = m_pGameInstance->GetOctoTreeObjects();
	const _uint iReserve = static_cast<_uint>(OctoObject.size() + m_vecLights.size());
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

	for (CDH_ToolMesh* pLightMesh : m_vecLights)
	{
		_vector vPosVec = pLightMesh->Get_TransfomCom()->Get_State(STATE::POSITION);
		_float3 vCenter{};
		XMStoreFloat3(&vCenter, vPosVec);

		_float  fRange = pLightMesh->GetRange();
		AABBBOX tLightBox = MakeLightAABB_Point(vCenter, fRange);

		const _uint iIdx = static_cast<_uint>(staticBounds.size());
		staticBounds.push_back(tLightBox);
		vObjectType.push_back(OCTOTREEOBJECTTYPE::LIGHT);

		handleToIndex[Handle{ nextHandleId++ }] = iIdx;
		m_pGameInstance->PushBackIndexToObj(pLightMesh);
	}


	if (FAILED(m_pGameInstance->Ready_OctoTree(staticBounds, handleToIndex)))
		return E_FAIL;

	m_pGameInstance->SetObjectType(vObjectType);

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Interact()
{
	/*  [ 기차역 슬라이딩 문 ] */
	CDefaultDoor::DEFAULTDOOR_DESC Desc{};
	Desc.m_eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
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
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_SlideDoor"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("TrainDoor"), &Desc)))
		return E_FAIL;


	/* [ 푸쿠오 보스 문 ] */
	CBossDoor::BOSSDOORMESH_DESC BossDoorDesc{};
	BossDoorDesc.m_eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
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
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_BossDoor"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("BossDoor"), &BossDoorDesc)))
		return E_FAIL;

	/* [ 축제의 인도자 문 ] */
	BossDoorDesc = {};
	BossDoorDesc.m_eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
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
 	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_BossDoor"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("BossDoor"), &BossDoorDesc)))
		return E_FAIL;

	/* [ 야외 나가는 문 ] */
	Desc = {};
	Desc.m_eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
	Desc.szMeshID = TEXT("StationDoubleDoor");
	lstrcpy(Desc.szName, TEXT("StationDoubleDoor"));

	ModelPrototypeTag = TEXT("Prototype_Component_Model_StationDoubleDoor");
	lstrcpy(Desc.szModelPrototypeTag, ModelPrototypeTag.c_str());
	vPosition = _float3(184.04f, 8.90f, -8.f);
	trans = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
	world = trans;

	XMStoreFloat4x4(&matWorldFloat, world);
	Desc.WorldMatrix = matWorldFloat;
	Desc.vColliderOffSet = _vector({ 0.f, 1.5f, 0.f, 0.f });
	Desc.vColliderSize = _vector({ 0.2f, 2.f, 2.f, 0.f });

	Desc.eInteractType = INTERACT_TYPE::OUTDOOR;
	Desc.vTriggerOffset = _vector({ 0.f, 0.f, 0.f, 0.f });
	Desc.vTriggerSize = _vector({ 0.5f, 0.2f, 1.0f, 0.f });
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_KeyDoor"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("KeyDoor"), &Desc)))
		return E_FAIL;

	/* [ 기차 내부 문 ] */
	Desc.m_eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
	Desc.szMeshID = TEXT("StationInnerDoor");
	lstrcpy(Desc.szName, TEXT("StationInnerDoor"));

	ModelPrototypeTag = TEXT("Prototype_Component_Model_StationInnerDoor");
	lstrcpy(Desc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	vPosition = _float3(34.4f, 0.0822f, 0.57f);
	matWorld = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
	XMStoreFloat4x4(&matWorldFloat, matWorld);
	Desc.WorldMatrix = matWorldFloat;
	Desc.vColliderOffSet = _vector({ 0.f, 0.5f, 0.f, 0.f });
	Desc.vColliderSize = _vector({ 0.2f, 1.f, 1.0f, 0.f });

	Desc.eInteractType = INTERACT_TYPE::INNERDOOR;
	Desc.vTriggerOffset = _vector({ 0.f, 0.5f, 0.f, 0.f });
	Desc.vTriggerSize = _vector({ 0.5f, 0.7f, 0.5f, 0.f });
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_KeyDoor"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("KeyDoor"), &Desc)))
		return E_FAIL;

	/* [ 숏컷 문 ] */
	Desc = {};
	Desc.m_eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
	Desc.szMeshID = TEXT("ShortCutDoor");
	lstrcpy(Desc.szName, TEXT("ShortCutDoor"));

	ModelPrototypeTag = TEXT("Prototype_Component_Model_ShortCutDoor");
	lstrcpy(Desc.szModelPrototypeTag, ModelPrototypeTag.c_str());
	vPosition = _float3(147.46f, 2.66f, -25.17f);
	trans = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
	world = trans;

	XMStoreFloat4x4(&matWorldFloat, world);
	Desc.WorldMatrix = matWorldFloat;
	Desc.vColliderOffSet = _vector({ 0.f, 1.5f, 0.f, 0.f });
	Desc.vColliderSize = _vector({ 2.0f, 2.f, 0.2f, 0.f });

	Desc.eInteractType = INTERACT_TYPE::SHORTCUT;
	Desc.vTriggerOffset = _vector({ 0.f, 0.f, 0.f, 0.f });
	Desc.vTriggerSize = _vector({ 0.5f, 0.2f, 1.0f, 0.f });
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_ShortCutDoor"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("ShortCutDoor"), &Desc)))
		return E_FAIL;


	/* [ 마지막 문 ] */
	CGameObject::GAMEOBJECT_DESC ObjDesc = {};
	ObjDesc.iLevelID = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);
	lstrcpy(Desc.szName, TEXT("FinalDoor"));
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_FinalDoor"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("FinalDoor"), &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Trigger()
{
	ifstream inFile("../Bin/Save/Trigger/Trigger.json");
	if (inFile.is_open())
	{
		json root;
		inFile >> root;
		inFile.close();

		for (const auto& j : root["triggers"]) {
			const _int type = j.value("ClassType", 0);
			// 공통 파싱
			const auto vPosArr = j.value("vPos", vector<float>{});
			const auto rotDegArr = j.value("rotationDeg", vector<float>{});
			const auto offsetArr = j.value("triggerOffset", vector<float>{});
			const auto sizeArr = j.value("triggerSize", vector<float>{});

			// SoundDatas 파싱
			vector<CTriggerBox::SOUNDDATA> vecSoundData;
			if (j.contains("SoundDatas") && j["SoundDatas"].is_array()) {
				for (const auto& snd : j["SoundDatas"]) {
					CTriggerBox::SOUNDDATA data{};

					if (snd.contains("SoundName")) {
						data.strSoundTag = snd["SoundName"].get<string>();
					}
					if (snd.contains("Text")) {
						data.strSoundText = snd["Text"].get<string>();
					}

					if (type == 0)
						data.strSpeaker = "";
					else
						data.strSpeaker = snd["Speaker"].get<string>();

					vecSoundData.push_back(data);
				}
			}

			if (type == 0) {
				const int  triggerType = j.value("TriggerType", 0);

				CTriggerSound::TRIGGERNOMESH_DESC Desc{};
				Desc.vPos = VecSetW(vPosArr, 1.f);
				Desc.Rotation = VecToFloat3(rotDegArr);
				Desc.vTriggerOffset = VecSetW(offsetArr, 0.f);
				Desc.vTriggerSize = VecSetW(sizeArr, 0.f);
				Desc.eTriggerBoxType = static_cast<TRIGGERSOUND_TYPE>(triggerType);
				Desc.m_vecSoundData = vecSoundData;
				if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerSound"),
					ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_TriggerSound"), &Desc)))
					return E_FAIL;
			}
			else if (type == 1) {
				const int  triggerType = j.value("TriggerType", 0);

				CTriggerTalk::TRIGGERTALK_DESC Desc{};
				string objectTag = j["ObjectTag"].get<string>();
				Desc.vPos = VecSetW(vPosArr, 1.f);
				Desc.Rotation = VecToFloat3(rotDegArr);
				Desc.vTriggerOffset = VecSetW(offsetArr, 0.f);
				Desc.vTriggerSize = VecSetW(sizeArr, 0.f);
				Desc.eTriggerBoxType = static_cast<TRIGGERSOUND_TYPE>(triggerType);
				Desc.m_vecSoundData = vecSoundData;
				Desc.gameObjectTag = objectTag;
				if (objectTag != "")
				{
					const auto offSetObj = j.value("offSetObj", vector<float>{});
					Desc.vOffSetObj = VecSetW(offSetObj, 0.f);

					const auto scaleObj = j.value("scaleObj", vector<float>{});
					Desc.vScaleObj = VecSetW(scaleObj, 0.f);
				}
				Desc.bCanCancel = j.value("CanCancel", 0);
				if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerTalk"),
					ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_TriggerTalk"), &Desc)))
					return E_FAIL;
			}
			else if (type == 2) {
				const int  triggerType = j.value("TriggerType", 0);

				CTriggerUI::TRIGGERUI_DESC Desc{};
				Desc.vPos = VecSetW(vPosArr, 1.f);
				Desc.Rotation = VecToFloat3(rotDegArr);
				Desc.vTriggerOffset = VecSetW(offsetArr, 0.f);
				Desc.vTriggerSize = VecSetW(sizeArr, 0.f);

				Desc.strProtoName = StringToWStringU8(j["PrototypeName"].get<string>());

				for (auto& filePath : j["FilePaths"])
				{
					string path = filePath.get<string>();

					Desc.strFilePaths.push_back(StringToWStringU8(path));
				}
				
				if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerUI"),
					ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_TriggerUI"), &Desc)))
					return E_FAIL;
			}
		}
	}
	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_TriggerBGM()
{
	ifstream inFile("../Bin/Save/Trigger/TriggerBGM.json");
	if (inFile.is_open())
	{
		json root;
		inFile >> root;
		inFile.close();

		for (const auto& j : root["triggers"]) {
			const auto vPosArr = j.value("vPos", vector<float>{});
			const auto rotDegArr = j.value("rotationDeg", vector<float>{});
			const auto offsetArr = j.value("triggerOffset", vector<float>{});
			const auto sizeArr = j.value("triggerSize", vector<float>{});

			const auto strInBGM = j.value("strInBGM", string{});
			const auto strOutBGM = j.value("strOutBGM", string{});
			const auto strInBGM2 = j.value("strInBGM2", string{});
			const auto strOutBGM2 = j.value("strOutBGM2", string{});

			CTriggerBGM::tagTriggerBGMDesc Desc{};
			Desc.vPos = VecSetW(vPosArr, 1.f);
			Desc.Rotation = VecToFloat3(rotDegArr);
			Desc.vTriggerOffset = VecSetW(offsetArr, 0.f);
			Desc.vTriggerSize = VecSetW(sizeArr, 0.f);
			Desc.strInBGM = strInBGM;
			Desc.strOutBGM = strOutBGM;

			if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerBGM"),
				ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_TriggerBGM"), &Desc)))
				return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_TriggerEffect()
{
	CTriggerRain::TRIGGERNOMESH_DESC Desc{};
	Desc.vPos = _vector({ 191.78f, 8.5f, -8.3f});
	Desc.Rotation = _float3(0.f,0.f,0.f);
	Desc.vTriggerOffset = _vector({});
	Desc.vTriggerSize = _vector({ 0.2f, 0.2f, 8.f, 0.f });
	Desc.m_vecSoundData = {};
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerRain"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_TriggerRain"), &Desc)))
		return E_FAIL;


	CTriggerRain::TRIGGERNOMESH_DESC BossdoorDesc{};
	BossdoorDesc.vPos = _vector({ 366.f, 13.5f, -48.3f });
	BossdoorDesc.Rotation = _float3(0.f, 0.f, 0.f);
	BossdoorDesc.vTriggerOffset = _vector({});
	BossdoorDesc.vTriggerSize = _vector({ 0.2f, 0.2f, 8.f, 0.f });
	BossdoorDesc.m_vecSoundData = {};
	BossdoorDesc.bBossDoor = true;
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerRain"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_TriggerRain"), &BossdoorDesc)))
		return E_FAIL;

	// 이펙트 트리거 미리 생성, 간판 스파크 용으로 일단 생성
	CTrigger_Effect::TRIGGER_EFFECT_DESC sparkDesc = {};
	_vector vPos = { 366.f, 13.5f, -49.f,1.f };
	sparkDesc.vPos = vPos;
	sparkDesc.Rotation = _float3(0.f, 0.f, 0.f);
	sparkDesc.vTriggerOffset = _vector({});
	sparkDesc.vTriggerSize = _vector({ 1.f, 0.2f, 8.f, 0.f });
	sparkDesc.m_vecSoundData = {};
	sparkDesc.strEffectTag = { TEXT("EC_OldSparkDrop_Big"),  TEXT("EC_OldSparkDrop_Big_1"),  TEXT("EC_OldSparkDrop_Big"), TEXT("EC_OldSparkDrop_Big_3"), TEXT("EC_OldSparkDrop_Big_2") };
	sparkDesc.vMakePos = { _float4(400.f, 25.f, -48.25f,1.f), _float4(400.f, 25.5f, -44.5f,1.f), _float4(400.f, 25.f, -53.f,1.f), _float4(400.f, 24.5f, -46.f,1.f), _float4(400.f, 25.f, -52.f,1.f) };

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerEffect"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_TriggerSpark"), &sparkDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_WaterPuddle()
{
	CWaterPuddle::WATER_DESC Desc{};

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_WaterPuddle"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_WaterPuddle"), &Desc)))
		return E_FAIL;


	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Thunder()
{
	CDH_ToolMesh::DHTOOL_DESC Desc{};
	Desc.szMeshID = TEXT("PointLight");
	lstrcpy(Desc.szName, TEXT("PointLight"));
	
	Desc.eLEVEL = LEVEL::KRAT_CENTERAL_STATION;


	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.m_vInitPos = _float3(51.f, 30.f, -21.f);
	Desc.iID = 9999;

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_ToolMesh"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), L"Layer_Thunder", &pGameObject, &Desc)))
		return E_FAIL;
	
	CDH_ToolMesh* pThunder = dynamic_cast<CDH_ToolMesh*>(pGameObject);
	pThunder->SetRange(150.f);
	pThunder->SetIntensity(2.f);
	pThunder->SetColor(_float4(0.6f, 0.75f, 1.0f, 1.0f));
	m_vecThunder.push_back(dynamic_cast<CDH_ToolMesh*>(pGameObject));
	
	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Sound()
{
	/* For.Com_Sound */
	if (FAILED(Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_Thunder"),
		TEXT("Com_ThunderSound"), reinterpret_cast<CComponent**>(&m_pThunderSoundCom))))
		return E_FAIL;

	/* [ 에어리어 박스를 설치해보자 ] */
	CAreaSoundBox::AREASOUNDBOX_DESC eDesc = {};
	eDesc.eTriggerBoxType = TRIGGERSOUND_TYPE::NONE;
	eDesc.fMinMax = { 1.f , 30.f };
	eDesc.fVolume = 1.f;
	eDesc.szSoundID = TEXT("Station");
	eDesc.strSoundName = "AMB_SS_Trainstation_Raindrop";
	eDesc.vPosition = _float3(62.32f, -0.27f, -7.84f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;


	//0. 기차통로
	eDesc.strSoundName = "AMB_SS_Arcade_WineStoreroom_02";
	eDesc.vPosition = _float3(15.02f, 0.084f, 0.69f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Arcade_WineStoreroom_02";
	eDesc.vPosition = _float3(37.02f, 0.084f, 0.69f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//1. 기차역 빗소리
	eDesc.strSoundName = "AMB_SS_Trainstation_Raindrop";
	eDesc.vPosition = _float3(90.05f, -0.27f, -7.12f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//2. 기차역 빗소리
	eDesc.strSoundName = "AMB_SS_Arcade_WineStoreroom_01";
	eDesc.vPosition = _float3(127.08f, 1.61f, -7.82f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//3. 호텔 입구소리
	eDesc.strSoundName = "AMB_SS_Arcade_WineStoreroom_02";
	eDesc.vPosition = _float3(127.08f, 1.61f, -7.82f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//4. 호텔 로비소리
	eDesc.strSoundName = "AMB_SS_Cathedral_Mine_Drone";
	eDesc.vPosition = _float3(143.56f, 2.66f, -8.16f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Factory_Basement_Loop_02";
	eDesc.vPosition = _float3(143.56f, 2.66f, -8.16f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//4. 호텔 20구역
	eDesc.strSoundName = "AMB_SS_Arcade_WineStoreroom_02";
	eDesc.vPosition = _float3(167.50f, 4.95f, -8.28f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Rain_Drip_02";
	eDesc.vPosition = _float3(183.39f, 8.89f, -8.12f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//4. 호텔 8구역
	eDesc.strSoundName = "AMB_SS_Cathedral_Underground_Drone";
	eDesc.vPosition = _float3(171.46f, 4.95f, -21.44f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//4. 호텔 9구역
	eDesc.strSoundName = "AMB_SS_Cathedral_Underground_Drone";
	eDesc.vPosition = _float3(171.63f, 4.95f, -35.54f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Factory_Basement_Loop_02";
	eDesc.vPosition = _float3(179.32f, 6.65f, -8.037f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;


	//4. 호텔 10구역
	eDesc.strSoundName = "AMB_SS_Factory_Drip_02_Loop";
	eDesc.vPosition = _float3(157.62f, 5.07f, -55.54f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//5. 호텔 11구역
	eDesc.strSoundName = "AMB_SS_Factory_Basement_Loop_01";
	eDesc.vPosition = _float3(167.46f, 7.69f, -64.39f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	eDesc.strSoundName = "AMB_SS_Factory_Drip_02_Loop";
	eDesc.vPosition = _float3(145.71f, 13.41f, -66.92f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//6. 호텔 12구역
	eDesc.strSoundName = "AMB_SS_Exhibition_Inside_02";
	eDesc.vPosition = _float3(119.85f, 13.41f, -61.43f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	eDesc.strSoundName = "AMB_SS_Exhibition_Inside_02";
	eDesc.vPosition = _float3(118.56f, 13.41f, -39.73f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;


	//7. 호텔 13구역
	eDesc.strSoundName = "AMB_SS_Factory_PropRoom_Loop_01";
	eDesc.vPosition = _float3(118.76f, 13.43f, -19.81f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//8. 호텔 14구역
	eDesc.strSoundName = "AMB_SS_Exhibition_Inside_02";
	eDesc.vPosition = _float3(137.06f, 13.41f, -28.14f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Factory_Basement_Loop_01";
	eDesc.vPosition = _float3(151.65f, 13.41f, -29.32f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//9. 호텔 15구역
	eDesc.strSoundName = "AMB_SS_Factory_PropRoom_Loop_01";
	eDesc.vPosition = _float3(162.70f, 13.44f, -24.99f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//10. 호텔 18구역
	eDesc.strSoundName = "AMB_SS_Exhibition_Inside_02";
	eDesc.vPosition = _float3(144.99f, 2.66f, -36.04f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//11. 엘리트 몬스터
	eDesc.strSoundName = "AMB_SS_Grave_Valley_Crane";
	eDesc.vPosition = _float3(148.51f, 5.38f, -42.02f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	
	//12. 숏컷
	eDesc.strSoundName = "AMB_SS_Lab_Fan_M_DLC";
	eDesc.vPosition = _float3(147.43f, 2.65f, -26.4f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//13. 대문 앞
	eDesc.strSoundName = "AMB_SS_OnTheBrige";
	eDesc.vPosition = _float3(193.39f, 8.89f, -8.12f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Rain_02";
	eDesc.vPosition = _float3(197.39f, 8.89f, -8.12f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;


	//50. 대문 앞
	eDesc.strSoundName = "AMB_SS_Rain_07_01";
	eDesc.vPosition = _float3(224.89f, 7.41f, -13.49f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	eDesc.strSoundName = "AMB_SS_Rain_07_02";
	eDesc.vPosition = _float3(214.93f, 7.41f, -33.58f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	eDesc.strSoundName = "AMB_SS_Rain_07_03";
	eDesc.vPosition = _float3(243.24f, 7.41f, -9.54f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	eDesc.strSoundName = "AMB_SS_Rain_07_01";
	eDesc.vPosition = _float3(267.20f, 7.36f, -18.76f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//51. 샛길
	eDesc.strSoundName = "AMB_SS_Rain_07_01";
	eDesc.vPosition = _float3(261.76f, 11.38f, -52.22f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Rain_07_02";
	eDesc.vPosition = _float3(265.265717f, 11.720646f, -69.103844f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//52. 중간
	eDesc.strSoundName = "AMB_SS_Rain_07_02";
	eDesc.vPosition = _float3(289.411530f, 7.374067f, -23.546703f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Rain_07_01";
	eDesc.vPosition = _float3(288.239685f, 1.255161f, -3.136308f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//53. 통로
	eDesc.strSoundName = "AMB_SS_Rain_07_04";
	eDesc.vPosition = _float3(292.029144f, 7.424714f, -38.567604f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Monastery_Wind_High";
	eDesc.vPosition = _float3(290.029144f, 7.424714f, -38.567604f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//54. 다리입구
	eDesc.strSoundName = "AMB_SS_Rain_07_01";
	eDesc.vPosition = _float3(314.179718f, 7.424715f, -39.300175f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//55. 1층
	eDesc.strSoundName = "AMB_SS_Rain_07_02";
	eDesc.vPosition = _float3(318.018982f, 0.456333f, -26.187710f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Rain_07_03";
	eDesc.vPosition = _float3(323.116608f, 0.456333f, -9.479903f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Rain_07_04";
	eDesc.vPosition = _float3(334.849762f, -4.499187f, -25.113424f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//56. 다리끝
	//eDesc.strSoundName = "AMB_SS_OnTheBrige";
	//eDesc.vPosition = _float3(328.757233f, 7.424715f, -36.812428f);
	//if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
	//	ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
	//	return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Rain_07_01";
	eDesc.fMinMax = { 1.f , 30.f };
	eDesc.vPosition = _float3(345.374847f, 7.424715f, -36.440987f);
	eDesc.fMinMax = { 1.f , 30.f };
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//57. 보스샛길
	eDesc.strSoundName = "AMB_SS_Rain_07_02";
	eDesc.vPosition = _float3(364.335480f, 7.467113f, -30.350395f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//58. 보스 중간계단
	eDesc.strSoundName = "AMB_SS_Rain_07_03";
	eDesc.vPosition = _float3(357.521851f, 10.838378f, -48.273304f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Monastery_Wind_High";
	eDesc.vPosition = _float3(351.521851f, 10.838378f, -48.273304f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;

	//59. 보스맵
	eDesc.strSoundName = "AMB_SS_Rain_07_01";
	eDesc.vPosition = _float3(385.177612f, 15.845862f, -47.998962f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;
	eDesc.strSoundName = "AMB_SS_Rain_07_02";
	eDesc.vPosition = _float3(403.043243f, 15.713462f, -48.864983f);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AreaSound"), &eDesc)))
		return E_FAIL;


	// 축제의 인도자 입구 (컷씬카메라에서 특정 프레임에 사운드 종료하려고 layer따로 씀)
	eDesc.strSoundName = "MU_MS_Boss_FestivalLeader_Entrance";
	eDesc.vPosition = _float3(374.98f, 14.95f, -48.74f);
	eDesc.fMinMax = { 1.f , 60.f };
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AreaSoundBox"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_FestivalEntranceSound"), &eDesc)))
		return E_FAIL;
	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_AnimatedProp()
{
	CAnimatedProp::ANIMTEDPROP_DESC Desc{};
	Desc.eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
	_float4x4 WorldMatrix = {};
	Desc.WorldMatrix = _float4x4(
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		232.05f, 12.12f, -7.89f, 1.0f
	);
	Desc.bCullNone = true;
	Desc.szMeshID = TEXT("SquareStatue");
	Desc.bUseSecondMesh = false;
	Desc.iLevelID = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AnimatedProp"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AnimPropLoop"), &Desc)))
		return E_FAIL;

	Desc.bUseSecondMesh = true;
	Desc.bCullNone = false;
	Desc.szMeshID = TEXT("ClownPanel");
	Desc.szSecondMeshID = TEXT("ClownStationPanel");
	Desc.WorldMatrix = _float4x4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		403.78f, 15.5f, -49.5f, 1.0f
	);

	//천막
	Desc.vSecondWorldMatrix = _float4x4(
		1.35f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.35f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.35f, 0.0f,
		405.11f, 15.5f, -44.5f, 1.0f
	);
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_AnimatedProp"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AnimPropPanel"), &Desc)))
		return E_FAIL;

	return S_OK;
}


HRESULT CLevel_KratCentralStation::Load_Shader()
{
	// [1] 경로 준비
	const string strPath = "../Bin/Save/ShaderParameters/ShaderData.json";

	// [2] 파일 존재 여부 확인
	if (!filesystem::exists(strPath))
	{
		MSG_BOX("셰이더 불러오기 실패: 저장된 파일이 없습니다.");
		return E_FAIL;
	}

	// [3] JSON 파일 열기
	ifstream ShaderDataFile(strPath);
	if (!ShaderDataFile.is_open())
	{
		MSG_BOX("셰이더 불러오기 실패: 파일 열기 실패.");
		return E_FAIL;
	}

	// [4] JSON 파싱
	json ShaderDataJson;
	ShaderDataFile >> ShaderDataJson;

	// [5] 각 값이 존재할 경우에만 안전하게 불러오기	
	if (ShaderDataJson.contains("DiffuseIntensity"))
	{
		_float fDiffuse = ShaderDataJson["DiffuseIntensity"];
		if (FAILED(m_pShaderComPBR->Bind_RawValue("g_fDiffuseIntensity", &fDiffuse, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComANIM->Bind_RawValue("g_fDiffuseIntensity", &fDiffuse, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fDiffuseIntensity", &fDiffuse, sizeof(_float))))
			return E_FAIL;
	}
	if (ShaderDataJson.contains("NormalIntensity"))
	{
		_float fNormal = ShaderDataJson["NormalIntensity"];
		if (FAILED(m_pShaderComPBR->Bind_RawValue("g_fNormalIntensity", &fNormal, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComANIM->Bind_RawValue("g_fNormalIntensity", &fNormal, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fNormalIntensity", &fNormal, sizeof(_float))))
			return E_FAIL;
	}
	if (ShaderDataJson.contains("AOIntensity"))
	{
		_float fAO = ShaderDataJson["AOIntensity"];
		if (FAILED(m_pShaderComPBR->Bind_RawValue("g_fAOIntensity", &fAO, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComANIM->Bind_RawValue("g_fAOIntensity", &fAO, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fAOIntensity", &fAO, sizeof(_float))))
			return E_FAIL;
	}
	if (ShaderDataJson.contains("AOPower"))
	{
		_float fAOPower = ShaderDataJson["AOPower"];
		if (FAILED(m_pShaderComPBR->Bind_RawValue("g_fAOPower", &fAOPower, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComANIM->Bind_RawValue("g_fAOPower", &fAOPower, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fAOPower", &fAOPower, sizeof(_float))))
			return E_FAIL;
	}
	if (ShaderDataJson.contains("RoughnessIntensity"))
	{
		_float fRoughness = ShaderDataJson["RoughnessIntensity"];
		if (FAILED(m_pShaderComPBR->Bind_RawValue("g_fRoughnessIntensity", &fRoughness, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComANIM->Bind_RawValue("g_fRoughnessIntensity", &fRoughness, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fRoughnessIntensity", &fRoughness, sizeof(_float))))
			return E_FAIL;
	}
	if (ShaderDataJson.contains("MetallicIntensity"))
	{
		_float fMetallic = ShaderDataJson["MetallicIntensity"];
		if (FAILED(m_pShaderComPBR->Bind_RawValue("g_fMetallicIntensity", &fMetallic, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComANIM->Bind_RawValue("g_fMetallicIntensity", &fMetallic, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fMetallicIntensity", &fMetallic, sizeof(_float))))
			return E_FAIL;
	}
	if (ShaderDataJson.contains("ReflectionIntensity"))
	{
		_float fReflection = ShaderDataJson["ReflectionIntensity"];
		if (FAILED(m_pShaderComPBR->Bind_RawValue("g_fReflectionIntensity", &fReflection, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComANIM->Bind_RawValue("g_fReflectionIntensity", &fReflection, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fReflectionIntensity", &fReflection, sizeof(_float))))
			return E_FAIL;
	}
	if (ShaderDataJson.contains("SpecularIntensity"))
	{
		_float fSpecular = ShaderDataJson["SpecularIntensity"];
		if (FAILED(m_pShaderComPBR->Bind_RawValue("g_fSpecularIntensity", &fSpecular, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComANIM->Bind_RawValue("g_fSpecularIntensity", &fSpecular, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fSpecularIntensity", &fSpecular, sizeof(_float))))
			return E_FAIL;
	}

	if (ShaderDataJson.contains("AlbedoTint"))
	{
		const auto& TintArray = ShaderDataJson["AlbedoTint"];
		if (TintArray.is_array() && TintArray.size() == 4)
		{
			_float4 vTint = _float4(
				TintArray[0].get<_float>(),
				TintArray[1].get<_float>(),
				TintArray[2].get<_float>(),
				TintArray[3].get<_float>()
			);

			if (FAILED(m_pShaderComPBR->Bind_RawValue("g_vDiffuseTint", &vTint, sizeof(_float4))))
				return E_FAIL;
			if (FAILED(m_pShaderComANIM->Bind_RawValue("g_vDiffuseTint", &vTint, sizeof(_float4))))
				return E_FAIL;
			if (FAILED(m_pShaderComInstance->Bind_RawValue("g_vDiffuseTint", &vTint, sizeof(_float4))))
				return E_FAIL;
		}
	}

	ShaderDataFile.close();

	return S_OK;
}
HRESULT CLevel_KratCentralStation::Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg)
{
	CComponent* pComponent = static_cast<CComponent*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_COMPONENT, iPrototypeLevelIndex, strPrototypeTag, pArg));
	if (nullptr == pComponent)
		return E_FAIL;

	*ppOut = pComponent;
	return S_OK;
}


CLevel_KratCentralStation* CLevel_KratCentralStation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_KratCentralStation* pInstance = new CLevel_KratCentralStation(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_KratCentralStation");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_KratCentralStation::Free()
{
	__super::Free();

	Safe_Release(m_pShaderComPBR);
	Safe_Release(m_pShaderComANIM);
	Safe_Release(m_pShaderComInstance);

	Safe_Release(m_pMapLoader);

	Safe_Release(m_pThunderSoundCom);
	//	Safe_Release(m_pStartVideo);
}
