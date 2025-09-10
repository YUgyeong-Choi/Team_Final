#include "Level_KratCentralStation.h"
#include "GameInstance.h"
#include "Camera_Manager.h"
#include "Effect_Manager.h"
#include "EffectContainer.h"
#include "Client_Function.h"

#pragma region YW
#include "StaticMesh.h"
#include "StaticMesh_Instance.h"
#include "Nav.h"
#include "Static_Decal.h"
#include "Stargazer.h"
#include "ErgoItem.h"
#include "BreakableMesh.h"
#pragma endregion

#include "SlideDoor.h"
#include "KeyDoor.h"
#include "BossDoor.h"
#include "TriggerSound.h"
#include "TriggerTalk.h"
#include "TriggerUI.h"
#include "TriggerBGM.h"

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

CLevel_KratCentralStation::CLevel_KratCentralStation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{

}

HRESULT CLevel_KratCentralStation::Initialize()
{
	/* [ 레벨 셋팅 ] */
	m_pGameInstance->SetCurrentLevelIndex(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION));
	m_pGameInstance->Set_IsChangeLevel(false);

	/* [ 사운드 ] */
	m_pBGM = m_pGameInstance->Get_Single_Sound("AMB_SS_Train_In_03");
	m_pBGM->Set_Volume(1.f * g_fBGMSoundVolume);
	m_pBGM->Play();

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
}

void CLevel_KratCentralStation::Update(_float fTimeDelta)
{
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

	//	if (KEY_DOWN(DIK_X))
	//	{
		//_float3 pos = {};
		//if (m_pPlayer)
		//	XMStoreFloat3(&pos, m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION));

		//_matrix presetmat = XMMatrixIdentity();
		//CEffectContainer::DESC ECDesc = {};
		//presetmat = XMMatrixTranslation(pos.x + m_pGameInstance->Compute_Random(-1.f, 1.f),
		//	pos.y + m_pGameInstance->Compute_Random(0.f, 3.f),
		//	pos.z + m_pGameInstance->Compute_Random(-1.f, 1.f));
		//XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
		//if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_Rain"), &ECDesc))
		//	MSG_BOX("이펙트 생성 실패");
		////if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_ErgoItem_M3P1_WB_FRAMELOOPTEST"), &ECDesc))
		////	MSG_BOX("이펙트 생성 실패");
	//	}
	//}



	m_pCamera_Manager->Update(fTimeDelta);
	CLockOn_Manager::Get_Instance()->Update(fTimeDelta);
}

void CLevel_KratCentralStation::Late_Update(_float fTimeDelta)
{

	CLockOn_Manager::Get_Instance()->Late_Update(fTimeDelta);
	__super::Late_Update(fTimeDelta);

	Add_RenderGroup_OctoTree();
}

HRESULT CLevel_KratCentralStation::Render()
{
	SetWindowText(g_hWnd, TEXT("게임플레이 레벨입니다."));

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Reset()
{
	list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), L"Layer_FireEater");
	for (auto& obj : objList)
		m_pGameInstance->Return_PoolObject(L"Layer_FireEater", obj);

	objList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), L"Layer_FestivalLeader");
	for (auto& obj : objList)
		m_pGameInstance->Return_PoolObject(L"Layer_FestivalLeader", obj);

	objList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), L"Layer_Monster_Normal");
	for (auto& obj : objList)
		m_pGameInstance->Return_PoolObject(L"Layer_Monster_Normal", obj);

	m_pGameInstance->UseAll_PoolObjects(L"Layer_FireEater");
	m_pGameInstance->UseAll_PoolObjects(L"Layer_FestivalLeader");
	m_pGameInstance->UseAll_PoolObjects(L"Layer_Monster_Normal");

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
	if (FAILED(Add_MapActor("TEST")))//맵 액터(콜라이더) 추가
		return E_FAIL;
	if (FAILED(Add_MapActor("STATION")))//맵 액터(콜라이더) 추가
		return E_FAIL;
	if (FAILED(Add_MapActor("HOTEL")))//맵 액터(콜라이더) 추가
		return E_FAIL;
	if (FAILED(Add_MapActor("OUTER")))//맵 액터(콜라이더) 추가
		return E_FAIL;
	if (FAILED(Add_MapActor("FIRE_EATER")))//맵 액터(콜라이더) 추가
		return E_FAIL;

	//고사양 모드
	//if (FAILED(Ready_Lights()))
	//	return E_FAIL;

	//저사양 모드
	if (FAILED(Ready_Lights_LowQuality()))
		return E_FAIL;
	
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

	if (FAILED(Ready_Monster()))
		return E_FAIL;
	if (FAILED(Ready_ErgoItem()))
		return E_FAIL;
	if (FAILED(Ready_Breakable()))
		return E_FAIL;
	if (FAILED(Ready_Stargazer()))
		return E_FAIL;

	// 문 같이 상호작용 하는 것들
	if (FAILED(Ready_Interact()))
		return E_FAIL;

	return S_OK;
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

		//pNewLight->SetDebug(false);
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

	// Area 1 (우선순위 최상)
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
	
	// ------------- Area 99 --------------
	_float3 a99p0 = _float3{ 180.65f, -47.92f, 63.37f };
	_float3 a99p1 = _float3{ 457.66f, 133.33f, -116.79f };
	_float3 a99Min, a99Max;
	FnToAABB(a99p0, a99p1, a99Min, a99Max);

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
		/* -------- [ 99번 구역 ] ---------- */
		const vector<_uint> vecAdj99 = {  };
		if (!m_pGameInstance->AddArea_AABB(
			99, a99Min, a99Max, vecAdj99, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	if (FAILED(m_pGameInstance->FinalizePartition()))
		return E_FAIL;


	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Camera()
{
	m_pCamera_Manager->Initialize(LEVEL::STATIC);
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

HRESULT CLevel_KratCentralStation::Ready_Monster()
{
#pragma region 몬스터 JSON 파일 받아서 소환

#ifdef TESTMAP
	if (FAILED(Ready_Monster("TEST")))
		return E_FAIL;

#ifdef TEST_STATION_MAP
	if (FAILED(Ready_Monster("STATION")))
		return E_FAIL;
#endif // TEST_STATION_MAP

#ifdef TEST_HOTEL_MAP
	if (FAILED(Ready_Monster("HOTEL")))
		return E_FAIL;
#endif // TEST_HOTEL_MAP

#ifdef TEST_OUTER_MAP
	if (FAILED(Ready_Monster("OUTER")))
		return E_FAIL;
#endif // TEST_OUTER_MAP

#ifdef TEST_FIRE_EATER_MAP
	if (FAILED(Ready_Monster("FIRE_EATER")))
		return E_FAIL;
#endif // TEST_FIRE_EATER_MAP

#endif // TESTMAP

#ifndef TESTMAP
	if (FAILED(Ready_Monster("STATION")))
		return E_FAIL;
	if (FAILED(Ready_Monster("HOTEL")))
		return E_FAIL;
	if (FAILED(Ready_Monster("OUTER")))
		return E_FAIL;
	if (FAILED(Ready_Monster("FIRE_EATER")))
		return E_FAIL;
#endif // !TESTMAP

#pragma endregion


#pragma region 전통적인 하드코드 방식으로 소환(지금 월드행렬로 몬스터 소환중이라 이걸로하면 항등행렬로 소환됨!!! 근데 항등행렬로 소환하니까 플레이어 충돌 문제 생김)

	//CMonster_Base::MONSTER_BASE_DESC Desc{};
	//Desc.fSpeedPerSec = 5.f;
	//Desc.fRotationPerSec = XMConvertToRadians(180.0f);
	//Desc.fHeight = 1.f;
	//Desc.vExtent = { 0.5f,1.f,0.5f };
	//Desc.eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;

	////시작 위치에 네비게이션이 없으면 터진다 지금
	//Desc.wsNavName = TEXT("STATION");
	//Desc.InitPos =
	//	//_float3(148.f, 2.47f, -7.38f); //호텔위치
	//	_float3(85.5f, 0.f, -7.5f); //스테이션 위치
	//Desc.szMeshID = TEXT("WatchDog");
	//

	////Desc.InitPos = _float3(80.5f, 0.f, -7.f); //스테이션 위치
	//if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_WatchDog"),
	//	ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster_Normal"), &Desc)))
	//	return E_FAIL;

	//Desc.wsNavName = TEXT("HOTEL");
	//Desc.InitPos =
	//	_float3(148.f, 2.47f, -7.38f); //호텔위치
	//if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Monster_Buttler_Train"),
	//	ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster_Normal"), &Desc)))
	//	return E_FAIL;

	//if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Fuoco"),
	//	ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster"))))
	//	return E_FAIL;
#pragma endregion

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Stargazer()
{
#ifdef TESTMAP
	if (FAILED(Ready_Stargazer("TEST")))
		return E_FAIL;

#ifdef TEST_STATION_MAP
	if (FAILED(Ready_Stargazer("STATION")))
		return E_FAIL;
#endif // TEST_STATION_MAP

#ifdef TEST_HOTEL_MAP
	if (FAILED(Ready_Stargazer("HOTEL")))
		return E_FAIL;
#endif // TEST_HOTEL_MAP

#ifdef TEST_OUTER_MAP
	if (FAILED(Ready_Stargazer("OUTER")))
		return E_FAIL;
#endif // TEST_OUTER_MAP

#ifdef TEST_FIRE_EATER_MAP
	if (FAILED(Ready_Stargazer("FIRE_EATER")))
		return E_FAIL;
#endif // TEST_FIRE_EATER_MAP

#endif // TESTMAP

#ifndef TESTMAP
	if (FAILED(Ready_Stargazer("STATION")))
		return E_FAIL;
	if (FAILED(Ready_Stargazer("HOTEL")))
		return E_FAIL;
	if (FAILED(Ready_Stargazer("OUTER")))
		return E_FAIL;
	if (FAILED(Ready_Stargazer("FIRE_EATER")))
		return E_FAIL;
#endif // !TESTMAP

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Stargazer(const _char* Map)
{
	string StargazerFilePath = string("../Bin/Save/MapTool/Stargazer_") + Map + ".json";
	ifstream inFile(StargazerFilePath);
	if (!inFile.is_open())
	{
		//wstring ErrorMessage = L"Stargazer_" + StringToWString(Map) + L".json 파일을 열 수 없습니다.";
		//MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);
		return S_OK;
	}

	json StargazerJson;
	inFile >> StargazerJson;
	inFile.close();

	// 배열 순회
	for (auto& StargazerData : StargazerJson)
	{
		// 월드 행렬
		const json& WorldMatrixJson = StargazerData["WorldMatrix"];
		_float4x4 WorldMatrix = {};
		for (_int row = 0; row < 4; ++row)
			for (_int col = 0; col < 4; ++col)
				WorldMatrix.m[row][col] = WorldMatrixJson[row][col];

		CStargazer::STARGAZER_DESC Desc{};
		Desc.iLevelID = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);
		Desc.WorldMatrix = WorldMatrix;
		if (StargazerData.contains("Tag") && StargazerData["Tag"].is_number_unsigned())
		{
			Desc.eStargazerTag = static_cast<STARGAZER_TAG>(StargazerData["Tag"].get<_uint>());
		}
		else
		{
			MSG_BOX("별바라기 태그를 안달아줌!!!!");

			return E_FAIL;
		}

		if (FAILED(m_pGameInstance->Add_GameObject(Desc.iLevelID, TEXT("Prototype_GameObject_Stargazer"),
			Desc.iLevelID, TEXT("Layer_Stargazer"), &Desc)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_ErgoItem()
{
#ifdef TESTMAP
	if (FAILED(Ready_ErgoItem("TEST")))
		return E_FAIL;

#ifdef TEST_STATION_MAP
	if (FAILED(Ready_ErgoItem("STATION")))
		return E_FAIL;
#endif // TEST_STATION_MAP

#ifdef TEST_HOTEL_MAP
	if (FAILED(Ready_ErgoItem("HOTEL")))
		return E_FAIL;
#endif // TEST_HOTEL_MAP

#ifdef TEST_OUTER_MAP
	if (FAILED(Ready_ErgoItem("OUTER")))
		return E_FAIL;
#endif // TEST_OUTER_MAP

#ifdef TEST_FIRE_EATER_MAP
	if (FAILED(Ready_ErgoItem("FIRE_EATER")))
		return E_FAIL;
#endif // TEST_FIRE_EATER_MAP


#endif // TESTMAP

#ifndef TESTMAP
	if (FAILED(Ready_ErgoItem("STATION")))
		return E_FAIL;
	if (FAILED(Ready_ErgoItem("HOTEL")))
		return E_FAIL;
	if (FAILED(Ready_ErgoItem("OUTER")))
		return E_FAIL;
	if (FAILED(Ready_ErgoItem("FIRE_EATER")))
		return E_FAIL;
#endif // !TESTMAP

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_ErgoItem(const _char* Map)
{
	string ErgoItemFilePath = string("../Bin/Save/MapTool/ErgoItem_") + Map + ".json";
	ifstream inFile(ErgoItemFilePath);
	if (!inFile.is_open())
	{
		//wstring ErrorMessage = L"Stargazer_" + StringToWString(Map) + L".json 파일을 열 수 없습니다.";
		//MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);
		return S_OK;
	}

	json ErgoItemJson;
	inFile >> ErgoItemJson;
	inFile.close();

	// 배열 순회
	for (auto& ErgoItemData : ErgoItemJson)
	{
		// 월드 행렬
		const json& WorldMatrixJson = ErgoItemData["WorldMatrix"];
		_float4x4 WorldMatrix = {};
		for (_int row = 0; row < 4; ++row)
			for (_int col = 0; col < 4; ++col)
				WorldMatrix.m[row][col] = WorldMatrixJson[row][col];

		CErgoItem::ERGOITEM_DESC Desc{};
		Desc.iLevelID = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);
		Desc.WorldMatrix = WorldMatrix;

		if (ErgoItemData.contains("Tag") && ErgoItemData["Tag"].is_number_unsigned())
		{
			Desc.eItemTag = static_cast<ITEM_TAG>(ErgoItemData["Tag"].get<_uint>());
		}
		else
		{
			//태그 안달아주면 희미한 에르고로 하자
			Desc.eItemTag = ITEM_TAG::ERGO_SHARD;
			//MSG_BOX("아이템 태그를 안달아줌!!!!");

			//return E_FAIL;
		}


		if (FAILED(m_pGameInstance->Add_GameObject(Desc.iLevelID, TEXT("Prototype_GameObject_ErgoItem"),
			Desc.iLevelID, TEXT("Layer_ErgoItem"), &Desc)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Breakable()
{
	//아직 푸오코 기둥을 위한 것으로만

#ifdef TESTMAP
	if (FAILED(Ready_Breakable("TEST")))
		return E_FAIL;

#ifdef TEST_STATION_MAP
	if (FAILED(Ready_Breakable("STATION")))
		return E_FAIL;
#endif // TEST_STATION_MAP

#ifdef TEST_HOTEL_MAP
	if (FAILED(Ready_Breakable("HOTEL")))
		return E_FAIL;
#endif // TEST_HOTEL_MAP

#ifdef TEST_OUTER_MAP
	if (FAILED(Ready_Breakable("OUTER")))
		return E_FAIL;
#endif // TEST_OUTER_MAP

#ifdef TEST_FIRE_EATER_MAP
	if (FAILED(Ready_Breakable("FIRE_EATER")))
		return E_FAIL;
#endif // TEST_FIRE_EATER_MAP

#endif // TESTMAP

#ifndef TESTMAP
	if (FAILED(Ready_Breakable("STATION")))
		return E_FAIL;
	if (FAILED(Ready_Breakable("HOTEL")))
		return E_FAIL;
	if (FAILED(Ready_Breakable("OUTER")))
		return E_FAIL;
	if (FAILED(Ready_Breakable("FIRE_EATER")))
		return E_FAIL;
#endif // !TESTMAP

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Breakable(const _char* Map)
{
	// JSON 경로
	string FilePath = string("../Bin/Save/MapTool/Breakable_") + Map + ".json";
	ifstream inFile(FilePath);
	if (!inFile.is_open())
		return S_OK;

	json Json;
	inFile >> Json;
	inFile.close();

	// "Breakables" 오브젝트 접근
	if (!Json.contains("Breakables") || !Json["Breakables"].is_object())
		return E_FAIL;

	auto& Breakables = Json["Breakables"];

	// 모델명(key) - 데이터(value) 순회
	for (auto& [ModelNameStr, ModelData] : Breakables.items())
	{
		wstring ModelName = StringToWString(ModelNameStr);

		// FragmentCount 읽기
		_int FragmentCount = 0;
		if (ModelData.contains("FragmentCount") && ModelData["FragmentCount"].is_number_integer())
			FragmentCount = ModelData["FragmentCount"];

		// Instances 배열 확인
		if (!ModelData.contains("Instances") || !ModelData["Instances"].is_array())
			continue;

		const json& Instances = ModelData["Instances"];

		for (const auto& Obj : Instances)
		{
			if (!Obj.contains("WorldMatrix"))
				continue;

			// 월드 행렬 읽기
			const json& WorldMatrixJson = Obj["WorldMatrix"];
			_float4x4 WorldMatrix = {};
			for (_int row = 0; row < 4; ++row)
				for (_int col = 0; col < 4; ++col)
					WorldMatrix.m[row][col] = WorldMatrixJson[row][col];

			// 푸오코 기둥 예외 처리
			if (ModelNameStr == "SM_Factory_BasePipe_07")
			{
				CBreakableMesh::BREAKABLEMESH_DESC Desc{};
				Desc.bFireEaterBossPipe = true;
				Desc.iLevelID = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);
				Desc.iPartModelCount = 3;
				Desc.ModelName = TEXT("Main");
				Desc.vOffsets = {
					_float3(4.09f, -8.75f, 1.21f),
					_float3(4.09f, -5.82f, 1.21f),
					_float3(4.09f, -2.89f, 1.21f)
				};
				Desc.PartModelNames = { TEXT("Part2"), TEXT("Part1"), TEXT("Part1") };
				Desc.WorldMatrix = WorldMatrix;
				Desc.wsNavName = StringToWString(Map);

				if (FAILED(m_pGameInstance->Add_GameObject(
					Desc.iLevelID, TEXT("Prototype_GameObject_BreakableMesh"),
					Desc.iLevelID, TEXT("Layer_BreakableMesh"), &Desc)))
					return E_FAIL;
			}
			else
			{
				// 일반적인 부서짐
				CBreakableMesh::BREAKABLEMESH_DESC Desc{};
				Desc.bFireEaterBossPipe = false;
				Desc.iLevelID = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);
				Desc.iPartModelCount = FragmentCount;
				Desc.ModelName = ModelName;

				for (_uint i = 0; i < Desc.iPartModelCount; ++i)
				{
					wstring PartName = ModelName + L'_' +
						to_wstring(i + 1) + L"of" + to_wstring(Desc.iPartModelCount);

					Desc.vOffsets.push_back(_float3(0.f, 0.f, 0.f));
					Desc.PartModelNames.push_back(PartName);
				}

				Desc.WorldMatrix = WorldMatrix;
				Desc.wsNavName = StringToWString(Map);

				if (FAILED(m_pGameInstance->Add_GameObject(
					Desc.iLevelID, TEXT("Prototype_GameObject_BreakableMesh"),
					Desc.iLevelID, TEXT("Layer_BreakableMesh"), &Desc)))
					return E_FAIL;
			}
		}
	}

	return S_OK;
}


HRESULT CLevel_KratCentralStation::Ready_Monster(const _char* Map)
{
	string MonsterFilePath = string("../Bin/Save/MonsterTool/Monster_") + Map + ".json";
	ifstream inFile(MonsterFilePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Monster_" + StringToWString(Map) + L".json 파일을 열 수 없습니다.";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);
		return S_OK;
	}

	json MonsterJson;
	inFile >> MonsterJson;
	inFile.close();

	// 몬스터 종류별로 반복
	for (auto& [MonsterName, MonsterArray] : MonsterJson.items())
	{
		wstring wstrMonsterName = StringToWString(MonsterName);

		for (auto& MonsterData : MonsterArray)
		{
			// 월드 행렬 로드
			const json& WorldMatrixJson = MonsterData["WorldMatrix"];
			_float4x4 WorldMatrix = {};
			for (_int row = 0; row < 4; ++row)
				for (_int col = 0; col < 4; ++col)
					WorldMatrix.m[row][col] = WorldMatrixJson[row][col];

			wstring wsLayer = {};

			if (wstrMonsterName == TEXT("FireEater"))
			{
				wsLayer = TEXT("Layer_FireEater");
			}
			else if (wstrMonsterName == TEXT("FestivalLeader"))
			{
				wsLayer = TEXT("Layer_FestivalLeader");
			}
			else
			{
				wsLayer = TEXT("Layer_Monster_Normal");
			}

			// 오브젝트 생성 Desc 채우기
			CUnit::UNIT_DESC UnitDesc{};
			UnitDesc.eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
			UnitDesc.wsNavName = StringToWString(Map);
			UnitDesc.WorldMatrix = WorldMatrix;
			UnitDesc.iLevelID = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);
			UnitDesc.szMeshID = wstrMonsterName.c_str();

			if (MonsterData.contains("SpawnType"))
				UnitDesc.eSpawnType = static_cast<SPAWN_TYPE>(MonsterData["SpawnType"].get<_int>());
			else
				UnitDesc.eSpawnType = SPAWN_TYPE::IDLE;
			

			wstring wsPrototypeTag = TEXT("Prototype_GameObject_") + wstrMonsterName;

			CGameObject* pObj = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), wsPrototypeTag, &UnitDesc));
			m_pGameInstance->Add_PoolObject(wsLayer, pObj);

			if (pObj == nullptr)
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Effect()
{
	_matrix presetmat = XMMatrixIdentity();
	CEffectContainer::DESC ECDesc = {};

	//presetmat = XMMatrixTranslation(52.83f, 0.09f, 1.57f);
	//XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	//if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_ErgoItem_M3P1_WB_FRAMELOOPTEST"), &ECDesc))
	//	MSG_BOX("이펙트 생성 실패");

	//presetmat = XMMatrixTranslation(69.25f, -0.22f, -8.17f);
	//XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	//if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_ErgoItem_M3P1_WB"), &ECDesc))
	//	MSG_BOX("이펙트 생성 실패");

	presetmat = XMMatrixTranslation(99.86f, 0.64f, -13.69f);
	XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_ErgoItem_M3P1_WB"), &ECDesc))
		MSG_BOX("이펙트 생성 실패");

	//presetmat = XMMatrixTranslation(0.f, 0.f, 0.f);
	//XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	//if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_ErgoItem_M3P1_WB_FRAMELOOPTEST"), &ECDesc))
	//	MSG_BOX("이펙트 생성 실패");

	CGameObject* pEC = { nullptr };

	presetmat = XMMatrixTranslation(62.f, 10.f, -7.2f);
	XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	pEC = MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_Rain"), &ECDesc);
	if (pEC == nullptr)
		MSG_BOX("이펙트 생성 실패");
	EFFECT_MANAGER->Store_EffectContainer(TEXT("StationRain_1"), static_cast<CEffectContainer*>(pEC));

	pEC = nullptr;
	presetmat = XMMatrixTranslation(88.8f, 10.f, -7.3f);
	XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	pEC = MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_Rain"), &ECDesc);
	if (pEC == nullptr)
		MSG_BOX("이펙트 생성 실패");

	EFFECT_MANAGER->Store_EffectContainer(TEXT("StationRain_2"), static_cast<CEffectContainer*>(pEC));



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
	CSlideDoor::DOORMESH_DESC Desc{};
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

	///* [ 축제의 인도자 문 ] */
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

	CKeyDoor::KEYDOORMESH_DESC KeyDoorDesc{};
	KeyDoorDesc.m_eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
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
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_KeyDoor"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("KeyDoor"), &KeyDoorDesc)))
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
			Desc.pBGM = m_pBGM;
			Desc.pBGM2 = m_pBGM2;
			Desc.strInBGM = strInBGM;
			Desc.strOutBGM = strOutBGM;
			Desc.strInBGM2 = strInBGM2;
			Desc.strOutBGM2 = strOutBGM2;

			if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerBGM"),
				ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_TriggerBGM"), &Desc)))
				return E_FAIL;
		}
	}
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

HRESULT CLevel_KratCentralStation::Add_MapActor(const _char* Map)
{
	//여기서 액터를 추가해준다. 메인스레드에서 액터를 추가하는 것이 안전하다고 한다.

	wstring wsLayerTag = TEXT("Layer_StaticMesh_") + StringToWString(Map);

	list<CGameObject*> ObjList = m_pGameInstance->Get_ObjectList(m_pGameInstance->GetCurrentLevelIndex(), wsLayerTag);

	for (CGameObject* pObj : ObjList)
	{
		if (FAILED(static_cast<CStaticMesh*>(pObj)->Add_Actor()))
			return E_FAIL;
	}

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

	if (m_pBGM)
	{
		m_pBGM->Stop();
		Safe_Release(m_pBGM);
	}

	if (m_pBGM2)
	{
		m_pBGM2->Stop();
		Safe_Release(m_pBGM2);
	}
	Safe_Release(m_pShaderComPBR);
	Safe_Release(m_pShaderComANIM);
	Safe_Release(m_pShaderComInstance);
	//	Safe_Release(m_pStartVideo);
}
