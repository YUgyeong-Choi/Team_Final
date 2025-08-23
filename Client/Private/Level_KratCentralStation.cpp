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
#pragma endregion

#include "DoorMesh.h"
#include "TriggerSound.h"
#include "TriggerTalk.h"

#include "PBRMesh.h"
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
		: CLevel { pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{

}

HRESULT CLevel_KratCentralStation::Initialize()
{
	/* [ 레벨 셋팅 ] */
	m_pGameInstance->SetCurrentLevelIndex(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION));
	m_pGameInstance->Set_IsChangeLevel(false);

	/* [ 사운드 ] */
	m_pBGM = m_pGameInstance->Get_Single_Sound("AMB_SS_CentralstationB_Inside");
	m_pBGM->Set_Volume(1.f * g_fBGMSoundVolume);
	m_pBGM->Play();

	if(FAILED(Ready_Video()))
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

	return S_OK;
}

void CLevel_KratCentralStation::Priority_Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_F1))
	{
		m_pGameInstance->Set_IsChangeLevel(true);
		CCamera_Manager::Get_Instance()->SetPlayer(nullptr);
		m_pGameInstance->ClearRenderObjects();
		m_pGameInstance->RemoveAll_Light(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION));
		m_pGameInstance->Reset_All();
		CLockOn_Manager::Get_Instance()->Set_Off(nullptr);
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOGO))))
			return;

		
	}

	if (m_pGameInstance->Key_Down(DIK_SPACE) && !m_bEndVideo)
	{
		if(m_pStartVideo)
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

	if (KEY_DOWN(DIK_U))
		m_pGameInstance->Set_GameTimeScale(1.f);
	if (KEY_DOWN(DIK_I))
		m_pGameInstance->Set_GameTimeScale(0.5f);

	if(KEY_DOWN(DIK_H))
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

	//if (KEY_PRESSING(DIK_LCONTROL))
	//{
	//	if (KEY_DOWN(DIK_Z))
	//	{
	//		if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_TestGrinder_VStretch_wls_P2"),
	//			m_pGameInstance->Compute_Random(-1.f, 1.f),
	//			m_pGameInstance->Compute_Random(-1.f, 1.f),
	//			m_pGameInstance->Compute_Random(-1.f, 1.f)))
	//			MSG_BOX("조짐");
	//	}
	//	if (KEY_DOWN(DIK_X))
	//	{
	//		_float3 pos = {};
	//		if (m_pPlayer)
	//			XMStoreFloat3(&pos, m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION));
	//		if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_AttackHit_Thrust_Spiral_2"),
	//			pos.x,
	//			pos.y + 1.f,
	//			pos.z))
	//			MSG_BOX("조짐");
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

HRESULT CLevel_KratCentralStation::Ready_Level()
{
	/* [ 해야할 준비들 ] */
	if (FAILED(Ready_Dummy()))
		return E_FAIL;
	if (FAILED(Add_MapActor()))//맵 액터(콜라이더) 추가
		return E_FAIL;
	if (FAILED(Ready_Lights()))
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
	Desc.szMeshID = TEXT("Hotel");
	lstrcpy(Desc.szName, TEXT("Hotel"));
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
		_int m_iVolumetricMode = jLight["Volumetric"].get<int>();

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
		pNewLight->SetbVolumetric(m_iVolumetricMode);

		pNewLight->SetDebug(false);
	}

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

	// --- 입력으로 받은 3개 구역의 두 점(마지막 1.00은 무시) ---
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
	_float3 a3p0 = _float3{ 119.81f, -5.63f,  32.20f };
	_float3 a3p1 = _float3{ -40.69f, 52.55f, -61.73f };
	_float3 a3Min, a3Max; 
	FnToAABB(a3p0, a3p1, a3Min, a3Max);

	// Area 4
	_float3 a4p0 = _float3{ 120.04f, - 5.32f, 4.66f };
	_float3 a4p1 = _float3{ 95.67f, 15.49f, -21.39f };
	_float3 a4Min, a4Max;
	FnToAABB(a4p0, a4p1, a4Min, a4Max);

	// Area 5
	_float3 a5p0 = _float3{ 110.f, -5.63f,  32.20f };
	_float3 a5p1 = _float3{ 26.53f,  49.64f, -52.41f };
	_float3 a5Min, a5Max;
	FnToAABB(a5p0, a5p1, a5Min, a5Max);

	// Area 6
	_float3 a6p0 = _float3{ 186.83f, -0.18f, 24.92f };
	_float3 a6p1 = _float3{ 113.46f, 48.18f, -75.18f };
	_float3 a6Min, a6Max;
	FnToAABB(a6p0, a6p1, a6Min, a6Max);

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
		const vector<_uint> vecAdj3 = { 1, 2 };
		if (!m_pGameInstance->AddArea_AABB(
			3, a3Min, a3Max, vecAdj3, AREA::EAreaType::OUTDOOR, ENUM_CLASS(AREA::EAreaType::OUTDOOR)))
			return E_FAIL;
	}
	{
		/* [ 4번 구역 ] */
		const vector<_uint> vecAdj4 = { 5, 2, 4 };
		if (!m_pGameInstance->AddArea_AABB(
			4, a4Min, a4Max, vecAdj4, AREA::EAreaType::INDOOR, ENUM_CLASS(AREA::EAreaType::INDOOR)))
			return E_FAIL;
	} 
	{
		/* [ 5번 구역 ] */
		const vector<_uint> vecAdj5 = { 3 , 4 };
		if (!m_pGameInstance->AddArea_AABB(
			5, a5Min, a5Max, vecAdj5, AREA::EAreaType::INDOOR, ENUM_CLASS(AREA::EAreaType::INDOOR)))
			return E_FAIL;
	}
	{
		/* [ 6번 구역 ] */
		const vector<_uint> vecAdj6 = { 4 };
		if (!m_pGameInstance->AddArea_AABB(
			6, a6Min, a6Max, vecAdj6, AREA::EAreaType::LOBBY, ENUM_CLASS(AREA::EAreaType::LOBBY)))
			return E_FAIL;
	}


	// 3) 파티션 확정 (ID→Index 매핑/검증)
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
	
	eLockonDesc.fSizeX = 50.f;
	eLockonDesc.fSizeY = 50.f;
	

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_LockOn_Icon"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Lockon_Icon"), &eLockonDesc)))
		return E_FAIL;


	eDesc.strFilePath = TEXT("../Bin/Save/UI/Popup/Popup.json");

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Popup"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player_Popup"), &eDesc)))
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
	if (FAILED(Ready_Monster("STATION")))
		return E_FAIL;
	if (FAILED(Ready_Monster("HOTEL")))
		return E_FAIL;
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

	//Desc.InitScale = _float3(1.f, 1.f, 1.f);
	//lstrcpy(Desc.szName, TEXT("Buttler_Train"));
	//Desc.szMeshID = TEXT("Buttler_Train");

	//if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Monster_Buttler_Train"),
	//	ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster_Normal"), &Desc)))
	//	return E_FAIL;

	//Desc.InitPos = _float3(80.5f, 0.f, -7.f); //스테이션 위치
	//if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Monster_Buttler_Train"),
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

			//만약 보스몹(푸오코라면 다른 방식으로 소환
			if (wstrMonsterName == TEXT("FireEater"))
			{
				CUnit::UNIT_DESC UnitDesc{};
				UnitDesc.eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
				UnitDesc.wsNavName = StringToWString(Map);
				UnitDesc.WorldMatrix = WorldMatrix;

				//푸오쿠는 따로 생성
				if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Fuoco"),
					ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster"), &UnitDesc)))
					return E_FAIL;

				continue;
			}

			// 오브젝트 생성 Desc 채우기
			CMonster_Base::MONSTER_BASE_DESC Desc{};

			Desc.fSpeedPerSec = 5.f;
			Desc.fRotationPerSec = XMConvertToRadians(180.0f);
			Desc.fHeight = 1.f;
			Desc.vExtent = { 0.5f,1.f,0.5f };
			Desc.eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
			Desc.szMeshID = wstrMonsterName.c_str();
			Desc.wsNavName = StringToWString(Map);
			Desc.WorldMatrix = WorldMatrix;

			wstring wsPrototypeTag = TEXT("Prototype_GameObject_Monster_") + wstrMonsterName;

			if (FAILED(m_pGameInstance->Add_GameObject(
				ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION),
				wsPrototypeTag.c_str(),
				ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION),
				TEXT("Layer_Monster_Normal"),
				&Desc)))
			{
				return E_FAIL;
			}
		}
	}

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Effect()
{
	//_matrix presetmat = XMMatrixIdentity();
	//CEffectContainer::DESC ECDesc = {};

	//presetmat = XMMatrixTranslation(52.83f, 0.09f, 1.57f);
	//XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	//if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_ErgoItem_M3P1_WB_FRAMELOOPTEST"), &ECDesc))
	//	MSG_BOX("이펙트 생성 실패");

	//presetmat = XMMatrixTranslation(69.25f, -0.22f, -8.17f);
	//XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	//if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_ErgoItem_M3P1_WB"), &ECDesc))
	//	MSG_BOX("이펙트 생성 실패");

	//presetmat = XMMatrixTranslation(99.86f, 0.64f, -13.69f);
	//XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	//if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_ErgoItem_M3P1_WB"), &ECDesc))
	//	MSG_BOX("이펙트 생성 실패");

	//presetmat = XMMatrixTranslation(0.f, 0.f, 0.f);
	//XMStoreFloat4x4(&ECDesc.PresetMatrix, presetmat);
	//if (nullptr == MAKE_EFFECT(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_ErgoItem_M3P1_WB_FRAMELOOPTEST"), &ECDesc))
	//	MSG_BOX("이펙트 생성 실패");

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
	CDoorMesh::DOORMESH_DESC Desc{};
	Desc.m_eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
	Desc.szMeshID = TEXT("SM_Station_TrainDoor");
	lstrcpy(Desc.szName, TEXT("SM_Station_TrainDoor"));

	/* 문자열 받는 곳 */
	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_SM_Station_TrainDoor");
	lstrcpy(Desc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	_float3 vPosition = _float3(52.6f, 0.02f, -2.4f);
	_matrix matWorld = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
	_float4x4 matWorldFloat;
	XMStoreFloat4x4(&matWorldFloat, matWorld);
	Desc.WorldMatrix = matWorldFloat;

	Desc.eInteractType = INTERACT_TYPE::TUTORIALDOOR;
	Desc.vTriggerOffset = _vector({ 0.f, 0.f, 0.3f, 0.f });
	Desc.vTriggerSize = _vector({ 1.f, 0.2f, 0.5f, 0.f });
	Desc.pBGM = m_pBGM;
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_DoorMesh"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("TrainDoor"), &Desc)))
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
			const _int type = j.value("UseMesh", 0);
			// 공통 파싱
			const auto vPosArr = j.value("vPos", vector<float>{});
			const auto rotDegArr = j.value("rotationDeg", vector<float>{});
			const auto offsetArr = j.value("triggerOffset", vector<float>{});
			const auto sizeArr = j.value("triggerSize", vector<float>{});
			const int  triggerType = j.value("TriggerType", 0); // 필요 시 enum으로 캐스팅

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

					if(type == 0)
						data.strSpeaker = "";
					else
						data.strSpeaker = snd["Speaker"].get<string>();

					vecSoundData.push_back(data);
				}
			}

			if (type == 0) {
				CTriggerSound::TRIGGERNOMESH_DESC Desc{};
				Desc.vPos = VecSetW(vPosArr, 1.f);
				Desc.Rotation = VecToFloat3(rotDegArr);       
				Desc.vTriggerOffset = VecSetW(offsetArr, 0.f);
				Desc.vTriggerSize = VecSetW(sizeArr, 0.f);
				Desc.eTriggerBoxType = static_cast<TRIGGERBOX_TYPE>(triggerType);
				Desc.m_vecSoundData = vecSoundData;
				if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerSound"),
					ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_TriggerSound"), &Desc)))
					return E_FAIL;
			}
			else if (type == 1) {
				CTriggerTalk::TRIGGERTALK_DESC Desc{};
				string objectTag = j["ObjectTag"].get<string>();
				Desc.vPos = VecSetW(vPosArr, 1.f);
				Desc.Rotation = VecToFloat3(rotDegArr);      
				Desc.vTriggerOffset = VecSetW(offsetArr, 0.f);
				Desc.vTriggerSize = VecSetW(sizeArr, 0.f);
				Desc.eTriggerBoxType = static_cast<TRIGGERBOX_TYPE>(triggerType);
				Desc.m_vecSoundData = vecSoundData;
				Desc.gameObjectTag = objectTag;
				Desc.bCanCancel = j.value("CanCancel", 0);
				if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_TriggerTalk"),
					ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_TriggerTalk"), &Desc)))
					return E_FAIL;
			}
		}
	}
	return S_OK;
}

HRESULT CLevel_KratCentralStation::Add_MapActor()
{
	//여기서 액터를 추가해준다. 메인스레드에서 액터를 추가하는 것이 안전하다고 한다.

	list<CGameObject*> ObjList = m_pGameInstance->Get_ObjectList(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_StaticMesh"));

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
	Safe_Release(m_pShaderComPBR);
	Safe_Release(m_pShaderComANIM);
	Safe_Release(m_pShaderComInstance);
//	Safe_Release(m_pStartVideo);
}
