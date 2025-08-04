#include "Level_KratHotel.h"
#include "GameInstance.h"
#include "Camera_Manager.h"

#include "Level_Loading.h"

//#include "StaticMesh.h"
#include "StaticMesh_Instance.h"

#include "SpriteEffect.h"

#include "Player.h"

CLevel_KratHotel::CLevel_KratHotel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{

}

HRESULT CLevel_KratHotel::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	if (FAILED(Ready_Camera()))
		return E_FAIL;
	if (FAILED(Ready_Layer_Sky(TEXT("Layer_Sky"))))
		return E_FAIL;

	//맵을 생성하기위한 모델 프로토타입을 준비한다.
	if (FAILED(Ready_MapModel(ENUM_CLASS(LEVEL::KRAT_HOTEL))))
		return E_FAIL;

	//제이슨으로 저장된 맵을 로드한다.
	if (FAILED(LoadMap(ENUM_CLASS(LEVEL::KRAT_HOTEL))))
		return E_FAIL;

	if (FAILED(Ready_Player()))
		return E_FAIL;
	


	// 이펙트 파싱 테스트입니다요
	//if (FAILED(Ready_TestEffect(TEXT("Layer_Effect"))))
	//	return E_FAIL;



	//제이슨으로 로드하지않고 따로 스태틱매쉬 소환하려면 모델프로토타입을 넣어줘야함
	//if (FAILED(Ready_Layer_StaticMesh(TEXT("Layer_StaticMesh"))))
	//	return E_FAIL;

	/* [ 사운드 ] */
	m_pBGM = m_pGameInstance->Get_Single_Sound("LiesOfP");
	m_pBGM->Set_Volume(1.f);
	m_pBGM->Play();

	return S_OK;
}

void CLevel_KratHotel::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_F1))
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOGO))))
			return;
	}

	m_pCamera_Manager->Update(fTimeDelta);
	__super::Update(fTimeDelta);
}

HRESULT CLevel_KratHotel::Render()
{
	SetWindowText(g_hWnd, TEXT("게임플레이 레벨입니다."));

	return S_OK;
}

HRESULT CLevel_KratHotel::Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath, _bool bInstance, _uint iLevelIndex)
{
	//이미 프로토타입이존재하는 지확인

	if (m_pGameInstance->Find_Prototype(iLevelIndex, strPrototypeTag) != nullptr)
	{
		//MSG_BOX("이미 프로토타입이 존재함");
		return S_OK;
	}

	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE);

	if (bInstance == false)
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_HOTEL), strPrototypeTag,
			CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_HOTEL), strPrototypeTag,
			CModel_Instance::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
			return E_FAIL;
	}


	return S_OK;
}

HRESULT CLevel_KratHotel::Ready_MapModel(_uint iLevelIndex)
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

		//갯수도 저장해서 인스턴스용 모델 프로토타입을 만들지 결정해야할듯(충돌여부로 판단하자)
		_uint iObjectCount = element["ObjectCount"];

		_bool bCollision = element["Collision"];

		wstring PrototypeTag = {};
		_bool bInstance = false;
		if (bCollision == false /*iObjectCount > INSTANCE_THRESHOLD*/)
		{
			//인스턴싱용 모델 프로토 타입 생성
			PrototypeTag = L"Prototype_Component_Model_Instance" + StringToWString(ModelName);
			bInstance = true;

		}
		else
		{
			//모델 프로토 타입 생성
			PrototypeTag = L"Prototype_Component_Model_" + StringToWString(ModelName);
			bInstance = false;
		}


		const _char* pModelFilePath = Path.c_str();

		if (FAILED(Load_Model(PrototypeTag, pModelFilePath, bInstance, iLevelIndex)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLevel_KratHotel::LoadMap(_uint iLevelIndex)
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

HRESULT CLevel_KratHotel::Load_StaticMesh(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex)
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

HRESULT CLevel_KratHotel::Load_StaticMesh_Instance(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex)
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
	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_Instance"); //인스턴스 용 모델을 준비해야겠는디?
	ModelPrototypeTag += wstrModelName;

	lstrcpy(StaticMeshInstanceDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_StaticMesh_Instance"),
		iLevelIndex, LayerTag, &StaticMeshInstanceDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratHotel::Ready_Player()
{
	CPlayer::PLAYER_DESC pDesc{};
	//pDesc.fSpeedPerSec = 1.f;
	pDesc.fSpeedPerSec = 5.f;
	pDesc.fRotationPerSec = XMConvertToRadians(600.0f);
	pDesc.eLevelID = LEVEL::KRAT_HOTEL;
	pDesc.InitPos = _float3(0.f, 0.978f, 1.f);
	pDesc.InitScale = _float3(1.f, 1.f, 1.f);
	lstrcpy(pDesc.szName, TEXT("Player"));
	pDesc.szMeshID = TEXT("Player");
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_GameObject_Player"),
		ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Player"), &pDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratHotel::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);	
	LightDesc.vDiffuse = _float4(0.6f, 0.6f, 0.6f, 1.f);
	LightDesc.fAmbient = 0.2f;
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	
	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;


	/*LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = _float4(10.f, 5.0f, 10.f, 1.f);
	LightDesc.fRange = 10.f;
	LightDesc.vDiffuse = _float4(1.f, 0.f, 0.f, 1.f);
	LightDesc.fAmbient = 0.4f;
	LightDesc.vSpecular = _float4(1.f, 0.f, 0.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = _float4(15.f, 5.0f, 10.f, 1.f);
	LightDesc.fRange = 10.f;
	LightDesc.vDiffuse = _float4(0.f, 1.f, 0.f, 1.f);
	LightDesc.fAmbient = 0.4f;
	LightDesc.vSpecular = _float4(0.f, 1.f, 0.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;*/


	CShadow::SHADOW_DESC		Desc{};
	Desc.vEye = _float4(0.f, 20.f, -15.f, 1.f);
	Desc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;
	
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWA)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratHotel::Ready_Camera()
{
	m_pCamera_Manager->Initialize(LEVEL::STATIC);
	m_pCamera_Manager->SetFreeCam();

	return S_OK;
}


HRESULT CLevel_KratHotel::Ready_Layer_Sky(const _wstring strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Sky"),
		ENUM_CLASS(LEVEL::KRAT_HOTEL), strLayerTag)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratHotel::Ready_Layer_StaticMesh(const _wstring strLayerTag)
{
	CStaticMesh::STATICMESH_DESC Desc{};
	Desc.iRender = 0;
	Desc.m_eLevelID = LEVEL::KRAT_HOTEL;

	Desc.szMeshID = TEXT("SM_BuildingA_Lift_01");
	lstrcpy(Desc.szName, TEXT("SM_BuildingA_Lift_01"));
	 lstrcpy(Desc.szModelPrototypeTag, TEXT("Prototype_Component_Model_SM_BuildingA_Lift_01"));
	
	//if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_GameObject_StaticMesh"),
	//	ENUM_CLASS(LEVEL::KRAT_HOTEL), strLayerTag, &Desc)))
	//	return E_FAIL;

	CStaticMesh_Instance::STATICMESHINSTANCE_DESC InstanceDesc{};
	InstanceDesc.iRender = 0;
	InstanceDesc.m_eLevelID = LEVEL::KRAT_HOTEL;

	InstanceDesc.szMeshID = TEXT("SM_BuildingA_Lift_02");
	lstrcpy(InstanceDesc.szName, TEXT("SM_BuildingA_Lift_02"));
	lstrcpy(InstanceDesc.szModelPrototypeTag, TEXT("Prototype_Component_Model_Instance_SM_BuildingA_Lift_02")); //인스턴스용 모델

	//인스턴스 월드 매트릭스들
	vector<_float4x4> InstanceMatrixs(2);
	XMStoreFloat4x4(&InstanceMatrixs[0], XMMatrixIdentity());
	XMStoreFloat4x4(&InstanceMatrixs[1], XMMatrixTranslationFromVector(XMVectorSet(5.f, 0.f, 0.f, 1.f)));
	InstanceDesc.pInstanceMatrixs = &InstanceMatrixs;
	//인스턴스 갯수
	InstanceDesc.iNumInstance = static_cast<_uint>(InstanceMatrixs.size());

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_GameObject_StaticMesh_Instance"),
		ENUM_CLASS(LEVEL::KRAT_HOTEL), strLayerTag, &InstanceDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratHotel::Ready_TestEffect(const _wstring strLayerTag)
{
	CSpriteEffect::DESC SEDesc = {};
	lstrcpy(SEDesc.pJsonFilePath, TEXT("../Bin/Save/Effect/SE_Test.json"));

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_SpriteEffect"),
		ENUM_CLASS(LEVEL::KRAT_HOTEL), strLayerTag, &SEDesc)))
		return E_FAIL;

	return S_OK;
}

CLevel_KratHotel* CLevel_KratHotel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_KratHotel* pInstance = new CLevel_KratHotel(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_KratHotel");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_KratHotel::Free()
{
	__super::Free();

	if (m_pBGM)
	{
		m_pBGM->Stop();
		Safe_Release(m_pBGM);
	}
}
