#include "Level_KratHotel.h"
#include "GameInstance.h"
#include "Camera_Manager.h"

#include "Level_Loading.h"

//#include "StaticMesh.h"
#include "StaticMesh_Instance.h"

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
	if (FAILED(Ready_MapModel()))
		return E_FAIL;

	//제이슨으로 저장된 맵을 로드한다.
	if (FAILED(LoadMap()))
		return E_FAIL;

	//제이슨으로 로드하지않고 따로 스태틱매쉬 소환하려면 모델프로토타입을 넣어줘야함
	if (FAILED(Ready_Layer_StaticMesh(TEXT("Layer_StaticMesh"))))
		return E_FAIL;

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

HRESULT CLevel_KratHotel::Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath)
{
	//이미 프로토타입이존재하는 지확인

	if (m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::KRAT_HOTEL), strPrototypeTag) != nullptr)
	{
		MSG_BOX("이미 프로토타입이 존재함");
		return S_OK;
	}

	_matrix		PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixIdentity();
	PreTransformMatrix = XMMatrixScaling(PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE);

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_HOTEL), strPrototypeTag,
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_KratHotel::Ready_MapModel()
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

HRESULT CLevel_KratHotel::LoadMap()
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
		_uint iObjectCount = Models[i]["ObjectCount"]; //오브젝트 갯수를보고 인스턴싱을 쓸지 말지 결정해야겠다.
		const json& objects = Models[i]["Objects"];

		Load_StaticMesh(iObjectCount, objects, ModelName);

		////일정 갯수 이상이면 인스턴싱오브젝트로 로드
		//if (iObjectCount > 0)
		//{
		//	Load_StaticMesh_Instance(iObjectCount, objects, ModelName);
		//}
		//else
		//{
		//	Load_StaticMesh(iObjectCount, objects, ModelName);
		//}
	}

	return S_OK;
}

HRESULT CLevel_KratHotel::Load_StaticMesh(_uint iObjectCount, const json& objects, string ModelName)
{
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
		StaticMeshDesc.m_eLevelID = LEVEL::KRAT_HOTEL;
		//lstrcpy(StaticMeshDesc.szName, TEXT("SM_TEST_FLOOR"));

		wstring wstrModelName = StringToWString(ModelName);
		wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
		ModelPrototypeTag += wstrModelName;

		lstrcpy(StaticMeshDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());
		StaticMeshDesc.WorldMatrix = WorldMatrix;

		if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_GameObject_StaticMesh"),
			ENUM_CLASS(LEVEL::KRAT_HOTEL), LayerTag, &StaticMeshDesc)))
			return E_FAIL;

	}

	return S_OK;
}

HRESULT CLevel_KratHotel::Load_StaticMesh_Instance(_uint iObjectCount, const json& objects, string ModelName)
{
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

		CStaticMesh_Instance::STATICMESHINSTANCE_DESC StaticMeshInstanceDesc = {};
		StaticMeshInstanceDesc.iNumInstance = iObjectCount;//인스턴스 갯수랑, 월드행렬들을 넘겨줘야한다.
		StaticMeshInstanceDesc.iRender = 0;
		StaticMeshInstanceDesc.m_eLevelID = LEVEL::KRAT_HOTEL;
		//lstrcpy(StaticMeshInstanceDesc.szName, TEXT("SM_TEST_FLOOR"));

		wstring wstrModelName = StringToWString(ModelName);
		wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
		ModelPrototypeTag += wstrModelName;

		lstrcpy(StaticMeshInstanceDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());
		StaticMeshInstanceDesc.WorldMatrix = WorldMatrix;

		if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_GameObject_StaticMesh_Instance"),
			ENUM_CLASS(LEVEL::KRAT_HOTEL), LayerTag, &StaticMeshInstanceDesc)))
			return E_FAIL;

	}

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
	
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_GameObject_StaticMesh"),
		ENUM_CLASS(LEVEL::KRAT_HOTEL), strLayerTag, &Desc)))
		return E_FAIL;

	Desc.szMeshID = TEXT("SM_BuildingA_Lift_02");
	lstrcpy(Desc.szName, TEXT("SM_BuildingA_Lift_02"));
	lstrcpy(Desc.szModelPrototypeTag, TEXT("Prototype_Component_Model_SM_BuildingA_Lift_02"));
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_HOTEL), TEXT("Prototype_GameObject_StaticMesh"),
		ENUM_CLASS(LEVEL::KRAT_HOTEL), strLayerTag, &Desc)))
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
