#include "Level_KratCentralStation.h"
#include "GameInstance.h"
#include "Camera_Manager.h"

#include "StaticMesh.h"
#include "PBRMesh.h"
#include "Level_Loading.h"
#include "UI_Container.h"

#include "Player.h"

CLevel_KratCentralStation::CLevel_KratCentralStation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{

}

HRESULT CLevel_KratCentralStation::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	if (FAILED(Ready_Shadow()))
		return E_FAIL;
	if (FAILED(Ready_Camera()))
		return E_FAIL;
	if (FAILED(Ready_Layer_StaticMesh(TEXT("Layer_StaticMesh"))))
		return E_FAIL;
	if (FAILED(Ready_Layer_Sky(TEXT("Layer_Sky"))))
		return E_FAIL;

	// 플레이어 생성 전에 ui부터 일단
	if (FAILED(Ready_UI()))
		return E_FAIL;

	//애니메이션 오브젝트
	if (FAILED(Ready_Player()))
		return E_FAIL;
	/*if (FAILED(Ready_MapModel()))
		return E_FAIL;
	if (FAILED(LoadMap()))
		return E_FAIL;*/

	

	/* [ 사운드 ] */
	m_pBGM = m_pGameInstance->Get_Single_Sound("LiesOfP");
	m_pBGM->Set_Volume(1.f);
	m_pBGM->Play();

	m_pCamera_Manager->SetOrbitalCam();
	m_pGameInstance->SetCurrentLevelIndex(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION));

	m_pGameInstance->Set_IsChangeLevel(false);

	CCamera_Manager::Get_Instance()->Play_CutScene(CUTSCENE_TYPE::TWO);
	return S_OK;
}

void CLevel_KratCentralStation::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_F1))
	{
		m_pGameInstance->Set_IsChangeLevel(true);
		CCamera_Manager::Get_Instance()->SetPlayer(nullptr);
		m_pGameInstance->ClearRenderObjects();
		m_pGameInstance->RemoveAll_Light(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION));
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOGO))))
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

	m_pCamera_Manager->Update(fTimeDelta);
}

HRESULT CLevel_KratCentralStation::Render()
{
	SetWindowText(g_hWnd, TEXT("게임플레이 레벨입니다."));

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath)
{
	//이미 프로토타입이존재하는 지확인

	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), strPrototypeTag) != nullptr)
	{
		MSG_BOX("이미 프로토타입이 존재함");
		return S_OK;
	}

	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE);

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), strPrototypeTag,
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_MapModel()
{
	ifstream inFile("../Bin/Save/MapTool/ReadyModel.json");
	if (!inFile.is_open())
	{
		MSG_BOX("ReadyModel.json 파일을 열 수 없습니다.");
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

		//모델 프로토 타입 생성
		wstring PrototypeTag = L"Prototype_Component_Model_" + StringToWString(ModelName);

		const _char* pModelFilePath = Path.c_str();

		if (FAILED(Load_Model(PrototypeTag, pModelFilePath)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLevel_KratCentralStation::LoadMap()
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
		_uint iObjectCount = Models[i]["ObjectCount"];
		const json& objects = Models[i]["Objects"];

		for (_uint j = 0; j < iObjectCount; ++j)
		{
			const json& WorldMatrixJson = objects[j]["WorldMatrix"];
			_float4x4 WorldMatrix = {};

			for (_int row = 0; row < 4; ++row)
				for (_int col = 0; col < 4; ++col)
					WorldMatrix.m[row][col] = WorldMatrixJson[row][col];

			//오브젝트 생성, 배치

			wstring LayerTag = TEXT("Layer_MapToolObject_");
			LayerTag += StringToWString(ModelName);

			CStaticMesh::STATICMESH_DESC StaticMeshDesc = {};

			StaticMeshDesc.iRender = 0;
			StaticMeshDesc.m_eLevelID = LEVEL::KRAT_CENTERAL_STATION;
			//lstrcpy(StaticMeshDesc.szName, TEXT("SM_TEST_FLOOR"));

			wstring wstrModelName = StringToWString(ModelName);
			wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
			ModelPrototypeTag += wstrModelName;

			lstrcpy(StaticMeshDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());
			StaticMeshDesc.WorldMatrix = WorldMatrix;

			if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_StaticMesh"),
				ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), LayerTag, &StaticMeshDesc)))
				return E_FAIL;

		}
	}

	return S_OK;
}

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
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Player"), &pDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

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

	return S_OK;
}

HRESULT CLevel_KratCentralStation::Ready_Shadow()
{
	CShadow::SHADOW_DESC		Desc{};
	Desc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;

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

	eDesc.strFilePath = TEXT("../Bin/Save/UI/Panel_Player.json");

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"),
		static_cast<_uint>(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player_Panel"), &eDesc)))
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
}
