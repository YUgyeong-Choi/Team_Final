#include "Level_KratCentralStation.h"
#include "GameInstance.h"
#include "Camera_Manager.h"
#include "Effect_Manager.h"
#include "EffectContainer.h"

#include "StaticMesh.h"
#include "StaticMesh_Instance.h"

#include "PBRMesh.h"
#include "Level_Loading.h"
#include "UI_Container.h"
#include "UI_Video.h"

#include "Player.h"
#include "Wego.h"
CLevel_KratCentralStation::CLevel_KratCentralStation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{

}

HRESULT CLevel_KratCentralStation::Initialize()
{
	if(FAILED(Ready_Video()))
		return E_FAIL;

	if (FAILED(Ready_Lights()))
		return E_FAIL;
	if (FAILED(Ready_Shadow()))
		return E_FAIL;
	/*if (FAILED(Ready_Layer_StaticMesh(TEXT("Layer_StaticMesh"))))
		return E_FAIL;*/
	if (FAILED(Ready_Camera()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Sky(TEXT("Layer_Sky"))))
		return E_FAIL;

	//제이슨으로 저장된 맵을 로드한다. (왜 안되지 모델을 왜 못찾지)
	if (FAILED(LoadMap(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION))))
		return E_FAIL;

	// 값 sync 맞추려고 플레이어 생성 전에 미리 생성해서 옵저버에 콜백 등록하기 위해
	if (FAILED(Ready_UI()))
		return E_FAIL;

	//애니메이션 오브젝트
	if (FAILED(Ready_Player()))
		return E_FAIL;

	if (FAILED(Ready_Monster()))
		return E_FAIL;

	if (FAILED(Ready_Npc()))
		return E_FAIL;

	m_pBGM = m_pGameInstance->Get_Single_Sound("LiesOfP");
	m_pBGM->Set_Volume(1.f);

	m_pCamera_Manager->SetCutSceneCam();

	
	m_pGameInstance->SetCurrentLevelIndex(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION));

	m_pGameInstance->Set_IsChangeLevel(false);


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
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOGO))))
			return;
	}


}

void CLevel_KratCentralStation::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_SPACE))
	{

		if (nullptr == m_pStartVideo)
			return;

		m_pStartVideo->Set_bDead();



	}

 	if (nullptr != m_pStartVideo)
	{
		if (m_pStartVideo->Get_bDead())
		{
			/* [ 사운드 ] */
			m_pBGM->Play();

			m_pStartVideo = nullptr;
	


			CCamera_Manager::Get_Instance()->Play_CutScene(CUTSCENE_TYPE::TWO);
		}

		return;
	}
	

	if (KEY_DOWN(DIK_U))
		m_pGameInstance->Set_GameTimeScale(1.f);
	if (KEY_DOWN(DIK_I))
		m_pGameInstance->Set_GameTimeScale(0.5f);

	if(KEY_DOWN(DIK_H))
		ToggleHoldMouse();
	if(m_bHold)
		HoldMouse();

	if (KEY_PRESSING(DIK_LSHIFT))
	{
		if (KEY_DOWN(DIK_Z))
		{
			auto PL = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Player"));
			CEffectContainer::DESC ECDesc = {};

			ECDesc.vPresetPosition = { PL->Get_TransfomCom()->Get_State(STATE::POSITION).m128_f32[0] ,
			PL->Get_TransfomCom()->Get_State(STATE::POSITION).m128_f32[1] + 0.1f ,
			PL->Get_TransfomCom()->Get_State(STATE::POSITION).m128_f32[2] };

			if (FAILED(EFFECT_MANAGER->Make_EffectContainer(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_ErgoItem_M3P1"), &ECDesc)))
				MSG_BOX("조짐");
			//if (FAILED(EFFECT_MANAGER->Make_EffectContainer(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("EC_ErgoItem_M3P1"))))
			//	MSG_BOX("조짐");
		}

		if (KEY_DOWN(DIK_X))
		{
			auto PL = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Player"));
			cout << "Player Position : " << PL->Get_TransfomCom()->Get_State(STATE::POSITION).m128_f32[0] << ", "
				<< PL->Get_TransfomCom()->Get_State(STATE::POSITION).m128_f32[1] << ", "
				<< PL->Get_TransfomCom()->Get_State(STATE::POSITION).m128_f32[2] << endl;
		}
	}



	m_pCamera_Manager->Update(fTimeDelta);
}

HRESULT CLevel_KratCentralStation::Render()
{
	SetWindowText(g_hWnd, TEXT("게임플레이 레벨입니다."));

	return S_OK;
}

HRESULT CLevel_KratCentralStation::LoadMap(_uint iLevelIndex)
{
	ifstream inFile("../Bin/Save/MapTool/MapData.json");
	if (!inFile.is_open())
	{
		MSG_BOX("MapData.json 파일을 열 수 없습니다.");
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

HRESULT CLevel_KratCentralStation::Load_StaticMesh(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex)
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

		wstring LayerTag = TEXT("Layer_MapToolObject_");
		LayerTag += StringToWString(ModelName);

		StaticMeshDesc.iRender = 0;
		StaticMeshDesc.m_eLevelID = static_cast<LEVEL>(iLevelIndex);
		//lstrcpy(StaticMeshDesc.szName, TEXT("SM_TEST_FLOOR"));

		wstring wstrModelName = StringToWString(ModelName);
		wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
		ModelPrototypeTag += wstrModelName;

		lstrcpy(StaticMeshDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());


		if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_StaticMesh"),
			iLevelIndex, LayerTag, &StaticMeshDesc)))
			return E_FAIL;

	}

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Load_StaticMesh_Instance(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex)
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
	StaticMeshInstanceDesc.m_eLevelID = static_cast<LEVEL>(iLevelIndex);
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


//HRESULT CLevel_KratCentralStation::Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath)
//{
//	//이미 프로토타입이존재하는 지확인
//
//	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), strPrototypeTag) != nullptr)
//	{
//		MSG_BOX("이미 프로토타입이 존재함");
//		return S_OK;
//	}
//
//	_matrix		PreTransformMatrix = XMMatrixIdentity();
//	PreTransformMatrix = XMMatrixIdentity();
//	PreTransformMatrix = XMMatrixScaling(PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE);
//
//	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), strPrototypeTag,
//		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
//		return E_FAIL;
//
//	return S_OK;
//}
//
//HRESULT CLevel_KratCentralStation::Ready_MapModel()
//{
//	ifstream inFile("../Bin/Save/MapTool/ReadyModel.json");
//	if (!inFile.is_open())
//	{
//		MSG_BOX("ReadyModel.json 파일을 열 수 없습니다.");
//		return S_OK;
//	}
//
//	json ReadyModelJson;
//	try
//	{
//		inFile >> ReadyModelJson;
//		inFile.close();
//	}
//	catch (const exception& e)
//	{
//		inFile.close();
//		MessageBoxA(nullptr, e.what(), "JSON 파싱 실패", MB_OK);
//		return E_FAIL;
//	}
//
//	// JSON 데이터 확인
//	for (const auto& element : ReadyModelJson)
//	{
//		string ModelName = element.value("ModelName", "");
//		string Path = element.value("Path", "");
//
//		//모델 프로토 타입 생성
//		wstring PrototypeTag = L"Prototype_Component_Model_" + StringToWString(ModelName);
//
//		const _char* pModelFilePath = Path.c_str();
//
//		if (FAILED(Load_Model(PrototypeTag, pModelFilePath)))
//		{
//			return E_FAIL;
//		}
//	}
//
//	return S_OK;
//}
//
//HRESULT CLevel_KratCentralStation::LoadMap()
//{
//	ifstream inFile("../Bin/Save/MapTool/MapData.json");
//	if (!inFile.is_open())
//	{
//		MSG_BOX("MapData.json 파일을 열 수 없습니다.");
//		return S_OK;
//	}
//
//	json MapDataJson;
//	inFile >> MapDataJson;
//	inFile.close();
//
//	_uint iModelCount = MapDataJson["ModelCount"];
//	const json& Models = MapDataJson["Models"];
//
//	for (_uint i = 0; i < iModelCount; ++i)
//	{
//		string ModelName = Models[i]["ModelName"];
//		_uint iObjectCount = Models[i]["ObjectCount"];
//		const json& objects = Models[i]["Objects"];
//
//		for (_uint j = 0; j < iObjectCount; ++j)
//		{
//			const json& WorldMatrixJson = objects[j]["WorldMatrix"];
//			_float4x4 WorldMatrix = {};
//
//			for (_int row = 0; row < 4; ++row)
//				for (_int col = 0; col < 4; ++col)
//					WorldMatrix.m[row][col] = WorldMatrixJson[row][col];
//
//			//오브젝트 생성, 배치
//
//			wstring LayerTag = TEXT("Layer_MapToolObject_");
//			LayerTag += StringToWString(ModelName);
//
//			CStaticMesh::STATICMESH_DESC StaticMeshDesc = {};
//
//			StaticMeshDesc.iRender = 0;
//			StaticMeshDesc.m_eLevelID = LEVEL::KRAT_CENTERAL_STATION;
//			//lstrcpy(StaticMeshDesc.szName, TEXT("SM_TEST_FLOOR"));
//
//			wstring wstrModelName = StringToWString(ModelName);
//			wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
//			ModelPrototypeTag += wstrModelName;
//
//			lstrcpy(StaticMeshDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());
//			StaticMeshDesc.WorldMatrix = WorldMatrix;
//
//			if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_StaticMesh"),
//				ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), LayerTag, &StaticMeshDesc)))
//				return E_FAIL;
//
//		}
//	}
//
//	return S_OK;
//}

HRESULT CLevel_KratCentralStation::Ready_Player()
{
	CPlayer::PLAYER_DESC pDesc{};
	//pDesc.fSpeedPerSec = 1.f;
	pDesc.fSpeedPerSec = 5.f;
	pDesc.fRotationPerSec = XMConvertToRadians(600.0f);
	pDesc.eLevelID = LEVEL::STATIC;
	pDesc.InitPos = _float3(0.f, 0.978f, 1.f);
	pDesc.InitScale = _float3(1.f, 1.f, 1.f);
	lstrcpy(pDesc.szName, TEXT("Player"));
	pDesc.szMeshID = TEXT("Player");
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Player"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"), &pDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Npc()
{
	CWego::WEGO_DESC pWegoDesc{};
	//pDesc.fSpeedPerSec = 1.f;
	pWegoDesc.fSpeedPerSec = 5.f;
	pWegoDesc.fRotationPerSec = XMConvertToRadians(600.0f);
	pWegoDesc.eLevelID = LEVEL::STATIC;
	pWegoDesc.InitPos = _float3(103.47f, 1.55f, -4.14f);
	pWegoDesc.InitScale = _float3(1.f, 1.f, 1.f);
	lstrcpy(pWegoDesc.szName, TEXT("Wego"));
	pWegoDesc.szMeshID = TEXT("Wego");
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Wego"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Wego"), &pWegoDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Lights()
{
#pragma region 호텔에 있던 라이트 가져옴
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
	Desc.fFar = 500.f;

	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWA)))
		return E_FAIL;

#pragma endregion

#pragma region 원래 여기 있던 라이트
	//LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.fAmbient = 0.2f;
	LightDesc.fIntensity = 1.f;
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.fFogDensity = 0.f;

	if (FAILED(m_pGameInstance->Add_LevelLightData(_uint(LEVEL::KRAT_CENTERAL_STATION), LightDesc)))
		return E_FAIL;

	LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.fAmbient = 0.2f;
	LightDesc.fIntensity = 1.f;
	LightDesc.fRange = 100.f;
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vPosition = _float4(10.f, 5.0f, 10.f, 1.f);
	LightDesc.fFogDensity = 0.f;

	if (FAILED(m_pGameInstance->Add_LevelLightData(_uint(LEVEL::KRAT_CENTERAL_STATION), LightDesc)))
		return E_FAIL;
#pragma endregion

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Shadow()
{
	CShadow::SHADOW_DESC		Desc{};
	Desc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 1000.f;

	Desc.vEye = _float4(76.f, 57.f, -21.f, 1.f);
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

HRESULT CLevel_KratCentralStation::Ready_Camera()
{
	m_pCamera_Manager->Initialize(LEVEL::STATIC);
	m_pCamera_Manager->SetFreeCam();

	return S_OK;
}


HRESULT CLevel_KratCentralStation::Ready_Layer_StaticMesh(const _wstring strLayerTag)
{
	CPBRMesh::STATICMESH_DESC Desc{};
	Desc.iRender = 0;
	Desc.m_eLevelID = LEVEL::KRAT_CENTERAL_STATION;
	Desc.szMeshID = TEXT("Train");
	lstrcpy(Desc.szName, TEXT("Train"));

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), strLayerTag, &Desc)))
		return E_FAIL;

	Desc.szMeshID = TEXT("Station");
	lstrcpy(Desc.szName, TEXT("Station"));
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PBRMesh"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), strLayerTag, &Desc)))
		return E_FAIL;

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


	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Video()
{
	
	CUI_Video::VIDEO_UI_DESC eDesc = {};
	eDesc.fOffset = 0.0f;
	eDesc.fInterval = 1.f;
	eDesc.fSpeedPerSec = 60.f;
	eDesc.strVideoPath = TEXT("../Bin/Resources/Video/Startscene.mp4");
	eDesc.fX = g_iWinSizeX * 0.5f;
	eDesc.fY = g_iWinSizeY * 0.5f;
	eDesc.fSizeX = g_iWinSizeX;
	eDesc.fSizeY = g_iWinSizeY;
	eDesc.fAlpha = 1.f;
	eDesc.isLoop = false;
	eDesc.isCull = true;

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Video"),
		static_cast<_uint>(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Background_Video"), &eDesc)))
		return E_FAIL;


	m_pStartVideo = static_cast<CUI_Video*>(m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Background_Video")));

	

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Monster()
{
	CUnit::UNIT_DESC pDesc{};
	//pDesc.fSpeedPerSec = 1.f;
	pDesc.fSpeedPerSec = 5.f;
	pDesc.fRotationPerSec = XMConvertToRadians(600.0f);
	pDesc.eLevelID = LEVEL::KRAT_CENTERAL_STATION;
	pDesc.InitPos = _float3(100.f, 0.5f, -10.f);
	pDesc.InitScale = _float3(1.5f, 1.5f, 1.5f);
	lstrcpy(pDesc.szName, TEXT("Elite_Police"));
	pDesc.szMeshID = TEXT("Elite_Police");
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Monster_Test"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster"), &pDesc)))
		return E_FAIL;

	pDesc.InitPos = _float3(80.f, 0.5f, -5.f);

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Monster_Test"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster"), &pDesc)))
		return E_FAIL;

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

	Safe_Release(m_pStartVideo);
}
