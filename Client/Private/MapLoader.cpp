#include "MapLoader.h"
#include "GameInstance.h"

#include "StaticMesh.h"
#include "StaticMesh_Instance.h"
#include "Static_Decal.h"
#include "BreakableMesh.h"
#include "Nav.h"

#include "Unit.h"
#include "ErgoItem.h"
#include "Stargazer.h"

CMapLoader::CMapLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
}

HRESULT CMapLoader::Initialize()
{
	return S_OK;
}

HRESULT CMapLoader::Ready_Map_Async()
{

	if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
		return E_FAIL;
	if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "STATION")))
		return E_FAIL;




	if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "HOTEL")))
		return E_FAIL;
	if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "HOTEL")))
		return E_FAIL;

	if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "OUTER")))
		return E_FAIL;
	if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "OUTER")))
		return E_FAIL;

	if (FAILED(Load_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "FIRE_EATER")))
		return E_FAIL;
	if (FAILED(Ready_Map(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), "FIRE_EATER")))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapLoader::Ready_Etc()
{
	return S_OK;
}

HRESULT CMapLoader::Load_Map(_uint iLevelIndex, const _char* Map)
{
	//맵
	if (FAILED(Loading_Meshs(iLevelIndex, Map)))
	{
		MSG_BOX("메쉬 로딩 실패!");
		return E_FAIL;
	}

	//네비
	if (FAILED(Loading_Navigation(iLevelIndex, Map)))
	{
		MSG_BOX("네비게이션 로딩 실패!");
		return E_FAIL;
	}

	//데칼
	if (FAILED(Loading_Decal_Textures(iLevelIndex, Map)))
	{
		MSG_BOX("데칼 텍스처 로딩 실패!");
		return E_FAIL;
	}

	//부서질 수 있는 메쉬 조각들, 또는 본메쉬 로드
	if (FAILED(Loading_Breakable(iLevelIndex, Map)))
	{
		MSG_BOX("브레이커블 메쉬 로딩 실패!");
		return E_FAIL;
	}

	return S_OK;

}

HRESULT CMapLoader::Load_Mesh(const wstring& strPrototypeTag, const _char* pModelFilePath, _bool bInstance, _uint iLevelIndex)
{
	_matrix PreTransformMatrix = XMMatrixScaling(PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE);

	// 파일 경로에서 디렉토리, 파일명, 확장자 분리
	wstring wsBasePath = filesystem::path(pModelFilePath).parent_path();
	wstring wsFilename = filesystem::path(pModelFilePath).stem();	// 확장자 뺀 파일명
	wstring wsExtension = filesystem::path(pModelFilePath).extension();

	// LOD0 모델 경로 생성
	string strModelPathLod0 = (filesystem::path(wsBasePath) / filesystem::path(wsFilename + wsExtension)).string();

	if (bInstance == false)
	{
		// LOD1, LOD2 파일명 생성 (_Lod1, _Lod2 붙임)
		wstring wsLod1Name = wsFilename + L"_Lod1" + wsExtension;
		wstring wsLod2Name = wsFilename + L"_Lod2" + wsExtension;

		// LOD1, LOD2 경로 생성
		filesystem::path Lod1Path = filesystem::path(wsBasePath) / wsLod1Name;
		filesystem::path Lod2Path = filesystem::path(wsBasePath) / wsLod2Name;
		string strModelPathLod1 = Lod1Path.string();
		string strModelPathLod2 = Lod2Path.string();

		// LOD0 프로토타입 존재 여부 확인 후 없으면 로드
		if (m_pGameInstance->Find_Prototype(iLevelIndex, strPrototypeTag) == nullptr)
		{
			if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, strPrototypeTag,
				CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelPathLod0.c_str(), PreTransformMatrix))))
				return E_FAIL;
		}

		// LOD1 프로토타입 태그 생성 및 로드 (파일 존재하면)
		wstring wsPrototypeTagLod1 = strPrototypeTag + L"_Lod1";
		if (m_pGameInstance->Find_Prototype(iLevelIndex, wsPrototypeTagLod1) == nullptr)
		{
			if (filesystem::exists(strModelPathLod1))
			{
				if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, wsPrototypeTagLod1,
					CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelPathLod1.c_str(), PreTransformMatrix))))
					return S_OK; // 실패해도 무시
			}
		}

		// LOD2 프로토타입 태그 생성 및 로드 (파일 존재하면)
		wstring wsPrototypeTagLod2 = strPrototypeTag + L"_Lod2";
		if (m_pGameInstance->Find_Prototype(iLevelIndex, wsPrototypeTagLod2) == nullptr)
		{
			if (filesystem::exists(strModelPathLod2))
			{
				if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, wsPrototypeTagLod2,
					CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelPathLod2.c_str(), PreTransformMatrix))))
					return S_OK; // 실패해도 무시
			}
		}

	}
	else
	{
		if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, strPrototypeTag,
			CModel_Instance::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelFilePath, PreTransformMatrix))))
			return E_FAIL;
	}


	return S_OK;
}

HRESULT CMapLoader::Loading_Meshs(_uint iLevelIndex, const _char* Map)
{

	string ResourcePath = string("../Bin/Save/MapTool/Resource_") + Map + ".json";

	ifstream inFile(ResourcePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Resource_" + StringToWString(Map) + L".json 파일을 열 수 없습니다: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);

		return S_OK;
	}

	json ResourceJson;
	try
	{
		inFile >> ResourceJson;
		inFile.close();
	}
	catch (const exception& e)
	{
		inFile.close();
		MessageBoxA(nullptr, e.what(), "JSON 파싱 실패", MB_OK);
		return E_FAIL;
	}

	// JSON 데이터 확인
	for (const auto& element : ResourceJson)
	{
		string ModelName = element.value("ModelName", "");
		string Path = element.value("Path", "");

		//갯수도 저장해서 인스턴스용 모델 프로토타입을 만들지 결정해야할듯(충돌여부로 판단하자)
		_uint iObjectCount = element["ObjectCount"];

		_bool bCollision = element["Collision"];

		_bool bNoInstancing = element["NoInstancing"];

		wstring PrototypeTag = {};
		_bool bInstance = false;
		if (bCollision == false && iObjectCount > INSTANCE_THRESHOLD && bNoInstancing == false)
		{
			//인스턴싱용 모델 프로토 타입 생성
			PrototypeTag = L"Prototype_Component_Model_Instance_" + StringToWString(ModelName);
			bInstance = true;

		}
		else
		{
			//모델 프로토 타입 생성
			PrototypeTag = L"Prototype_Component_Model_" + StringToWString(ModelName);
			bInstance = false;
		}


		const _char* pModelFilePath = Path.c_str();

		if (FAILED(Load_Mesh(PrototypeTag, pModelFilePath, bInstance, iLevelIndex)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CMapLoader::Loading_Navigation(_uint iLevelIndex, const _char* Map/*, _bool bForTool*/)
{
	//bForTool = true;//bForTool 없어질 예정

	wstring wsResourcePath = L"../Bin/Save/NavTool/Nav_" + StringToWString(Map) + L".json";

	// 파일 열기
	ifstream inFile(wsResourcePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Nav_" + StringToWString(Map) + L".json 파일을 열 수 없습니다.";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);
		return S_OK;
	}

	wstring wsPrototypeTag = {};

	//if(bForTool)
	wsPrototypeTag = L"Prototype_Component_Navigation_" + StringToWString(Map);
	//else
	//	wsPrototypeTag = L"Prototype_Component_Navigation";//실제 맵을 위한 로딩(툴과 똑같이 적용할 예정)

	/* Prototype_Component_Navigation */
	if (FAILED(m_pGameInstance->Add_Prototype(
		iLevelIndex,
		wsPrototypeTag.c_str(),
		CNavigation::Create(m_pDevice, m_pContext, wsResourcePath.c_str())
	)))
		return E_FAIL;


	return S_OK;
}

HRESULT CMapLoader::Loading_Decal_Textures(_uint iLevelIndex, const _char* Map)
{
	string ResourcePath = string("../Bin/Save/DecalTool/Resource_") + Map + ".json";

	ifstream inFile(ResourcePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Resource_" + StringToWString(Map) + L".json 파일을 열 수 없습니다: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);

		return S_OK;
	}

	json ReadyTextureJson;
	try
	{
		inFile >> ReadyTextureJson;
		inFile.close();
	}
	catch (const exception& e)
	{
		inFile.close();
		MessageBoxA(nullptr, e.what(), "JSON 파싱 실패", MB_OK);
		return E_FAIL;
	}

	//텍스쳐 프로토타입 생성
	for (auto& [PrototypeTag, Path] : ReadyTextureJson.items())
	{
		//이미 있으면 넘기고
		if (m_pGameInstance->Find_Prototype(iLevelIndex, StringToWString(PrototypeTag)) != nullptr)
			continue;

		if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, StringToWString(PrototypeTag),
			CTexture::Create(m_pDevice, m_pContext, StringToWString(Path).c_str()))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMapLoader::Loading_Breakable(_uint iLevelIndex, const _char* Map)
{
	_matrix PreTransformMatrix = XMMatrixScaling(
		PRE_TRANSFORMMATRIX_SCALE,
		PRE_TRANSFORMMATRIX_SCALE,
		PRE_TRANSFORMMATRIX_SCALE);

	// JSON 파일 경로
	wstring wsPath = L"../Bin/Save/MapTool/Breakable_" + StringToWString(Map) + L".json";

	ifstream ifs(wsPath);
	if (!ifs.is_open())
		return S_OK;

	json j;
	ifs >> j;

	// JSON 최상위 바로 ModelName 확인
	for (auto& [ModelName, ModelData] : j.items())
	{
		// 푸오코 기둥 무시
		if (ModelName == "SM_Factory_BasePipe_07")
			continue;

		// FragmentCount 읽기
		if (!ModelData.contains("FragmentCount"))
			continue;

		_uint finalDenom = ModelData["FragmentCount"].get<_uint>();

		// 1 ~ finalDenom 범위의 조각 등록
		for (_uint num = 1; num <= finalDenom; ++num)
		{
			wstring wsPrototypeTag =
				L"Prototype_Component_Model_" + StringToWString(ModelName) +
				L"_" + to_wstring(num) + L"of" + to_wstring(finalDenom);

			if (m_pGameInstance->Find_Prototype(iLevelIndex, wsPrototypeTag) == nullptr)
			{
				filesystem::path modelPath = filesystem::path(PATH_NONANIM) /
					(ModelName + "_" + std::to_string(num) + "of" + std::to_string(finalDenom) + ".bin");

				if (filesystem::exists(modelPath))
				{
					if (FAILED(m_pGameInstance->Add_Prototype(
						iLevelIndex, wsPrototypeTag,
						CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM,
							modelPath.string().c_str(),
							PreTransformMatrix))))
					{
						return S_OK; // 실패해도 무시
					}
				}
			}
		}
	}

	return S_OK;
}


HRESULT CMapLoader::Ready_Map(_uint iLevelIndex, const _char* Map)
{
	//어떤 맵을 소환 시킬 것인지?
	if (FAILED(Ready_Meshs(iLevelIndex, Map))) //TEST, STAION
		return E_FAIL;

	//네비 소환
	if (FAILED(Ready_Nav(TEXT("Layer_Nav"), iLevelIndex, Map)))
		return E_FAIL;

	//어떤 데칼을 소환 시킬 것인지?
	if (FAILED(Ready_Static_Decal(iLevelIndex, Map))) //TEST, STATION
		return E_FAIL;

	//부서질 수 있는 오브젝트 소환
	if (FAILED(Ready_Breakable(iLevelIndex, Map)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapLoader::Ready_Meshs(_uint iLevelIndex, const _char* Map)
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

		if (iObjectCount == 0)
			continue;

		const json& objects = Models[i]["Objects"];

		_bool bCollision = Models[i]["Collision"];
		_bool bNoInstancing = Models[i]["NoInstancing"];

		//일정 갯수 이상이면 인스턴싱오브젝트로 로드(충돌이 없는 모델이면 인스턴싱)
		if (bCollision == false && iObjectCount > INSTANCE_THRESHOLD && bNoInstancing == false)
		{
			if (FAILED(Ready_StaticMesh_Instance(iObjectCount, objects, ModelName, iLevelIndex, Map)))
				return E_FAIL;
		}
		else
		{
			if (FAILED(Ready_StaticMesh(iObjectCount, objects, ModelName, iLevelIndex, Map)))
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CMapLoader::Ready_StaticMesh(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex, const _char* Map)
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

#pragma region 바닥여부
		if (objects[j].contains("IsFloor"))
		{
			StaticMeshDesc.bIsFloor = objects[j].value("IsFloor", false);
		}
		else
		{
			StaticMeshDesc.bIsFloor = false;
		}
#pragma endregion

#pragma region 컬링 여부
		if (objects[j].contains("CullNone"))
			StaticMeshDesc.bCullNone = true;
		else
			StaticMeshDesc.bCullNone = false;
#pragma endregion

		StaticMeshDesc.iRender = 0;
		StaticMeshDesc.iLevelID = iLevelIndex;

		wstring wstrModelName = StringToWString(ModelName);
		wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_");
		ModelPrototypeTag += wstrModelName;

		lstrcpy(StaticMeshDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

		wstring wsLayerTag = TEXT("Layer_StaticMesh_") + StringToWString(Map); //Layer_StaticMesh_STATION, Layer_StaticMesh_HOTEL

		// 락 걸기
		//lock_guard<mutex> lock(m_mtx);


		if (FAILED(m_pGameInstance->Add_GameObject_And_PushOctoTree(iLevelIndex, TEXT("Prototype_GameObject_StaticMesh"),
			iLevelIndex, wsLayerTag, &StaticMeshDesc)))
			return E_FAIL;

		/*	CGameObject* pGameObject = nullptr;
			if (FAILED(m_pGameInstance->Add_GameObjectReturn(iLevelIndex, TEXT("Prototype_GameObject_StaticMesh"),
				iLevelIndex, wsLayerTag, &pGameObject, &StaticMeshDesc)))
				return E_FAIL;

			m_pGameInstance->PushOctoTreeObjects(pGameObject);*/
	}

	return S_OK;
}

HRESULT CMapLoader::Ready_StaticMesh_Instance(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex, const _char* Map)
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
	//LayerTag += StringToWString(ModelName);

	CStaticMesh_Instance::STATICMESHINSTANCE_DESC StaticMeshInstanceDesc = {};
	StaticMeshInstanceDesc.iNumInstance = iObjectCount;//인스턴스 갯수랑
	StaticMeshInstanceDesc.pInstanceMatrixs = &InstanceMatixs;//월드행렬들을 넘겨줘야한다.

	StaticMeshInstanceDesc.iRender = 0;
	StaticMeshInstanceDesc.iLevelID = iLevelIndex;
	//lstrcpy(StaticMeshInstanceDesc.szName, TEXT("SM_TEST_FLOOR"));

	StaticMeshInstanceDesc.iLightShape = objects[0].value("LightShape", 0); //오브젝트중 하나 가져와서 라이트모양을 넣어주자.
	//cout << StaticMeshInstanceDesc.iLightShape << endl;

	wstring wstrModelName = StringToWString(ModelName);
	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_Instance_"); //인스턴스 용 모델을 준비해야겠는디?
	ModelPrototypeTag += wstrModelName;

	lstrcpy(StaticMeshInstanceDesc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	wstring wsLayerTag = TEXT("Layer_StaticMesh_Instance_") + StringToWString(Map); //Layer_StaticMesh_Instance_STATION, Layer_StaticMesh_Instance_HOTEL
	StaticMeshInstanceDesc.wsMap = StringToWString(Map);

	if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_StaticMesh_Instance"),
		iLevelIndex, wsLayerTag, &StaticMeshInstanceDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapLoader::Ready_Nav(const _wstring strLayerTag, _uint iLevelIndex, const _char* Map)
{
	CNav::NAV_DESC NavDesc = {};
	NavDesc.iLevelIndex = iLevelIndex;
	NavDesc.wsNavComName = StringToWString(Map);

	if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_Nav"),
		iLevelIndex, strLayerTag, &NavDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapLoader::Ready_Static_Decal(_uint iLevelIndex, const _char* Map)
{
	//현재 맵에 필요한 데칼 텍스쳐를 로드한다.
	string DecalDataPath = string("../Bin/Save/DecalTool/Decal_") + Map + ".json";
	//string ResourcePath = string("../Bin/Save/MapTool/Resource_") + Map + ".json"; //나중에 쓸듯 맵 바꿀때

	ifstream inFile(DecalDataPath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Decal_" + StringToWString(Map) + L".json 파일을 열 수 없습니다: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"에러", MB_OK);
		return S_OK;
	}

	// JSON 파싱
	json JSON;
	inFile >> JSON;

	// 데이터 만큼 Decal 소환
	for (auto& item : JSON)
	{
		// 4x4 행렬 읽기
		_float4x4 WorldMatrix;
		for (_int iRow = 0; iRow < 4; ++iRow)
		{
			for (_int iCol = 0; iCol < 4; ++iCol)
			{
				WorldMatrix.m[iRow][iCol] = item["WorldMatrix"][iRow][iCol].get<_float>();
			}
		}

		//텍스쳐 프로토타입 이름 전달
		CStatic_Decal::DECAL_DESC Desc = {};
		Desc.WorldMatrix = WorldMatrix;
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] = StringToWString(item["ARMT"].get<string>());
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] = StringToWString(item["N"].get<string>());
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] = StringToWString(item["BC"].get<string>());
		Desc.iLevelID = iLevelIndex;

		// Decal 객체 생성
		if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_Static_Decal"),
			iLevelIndex, TEXT("Layer_Decal"), &Desc)))
			return E_FAIL;

	}

	return S_OK;

}

HRESULT CMapLoader::Add_MapActor(const _char* Map)
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

HRESULT CMapLoader::Ready_Breakable(_uint iLevelIndex, const _char* Map)
{
	// JSON 경로
	string FilePath = string("../Bin/Save/MapTool/Breakable_") + Map + ".json";
	ifstream inFile(FilePath);
	if (!inFile.is_open())
		return S_OK;

	json Json;
	inFile >> Json;
	inFile.close();

	// JSON에서 ModelName 바로 접근
	for (auto& [ModelNameStr, ModelData] : Json.items())
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

			CBreakableMesh::BREAKABLEMESH_DESC Desc{};
			Desc.iLevelID = iLevelIndex;
			Desc.WorldMatrix = WorldMatrix;
			Desc.wsNavName = StringToWString(Map);

			if (ModelNameStr == "SM_Factory_BasePipe_07")
			{
				// 푸오코 기둥 예외 처리
				Desc.bFireEaterBossPipe = true;
				Desc.iPartModelCount = 3;
				Desc.ModelName = TEXT("Main");
				Desc.vOffsets = {
					_float3(4.09f, -8.75f, 1.21f),
					_float3(4.09f, -5.82f, 1.21f),
					_float3(4.09f, -2.89f, 1.21f)
				};
				Desc.PartModelNames = { TEXT("Part2"), TEXT("Part1"), TEXT("Part1") };
			}
			else
			{
				// 일반적인 부서짐
				Desc.bFireEaterBossPipe = false;
				Desc.iPartModelCount = FragmentCount;
				Desc.ModelName = ModelName;

				for (_uint i = 0; i < Desc.iPartModelCount; ++i)
				{
					wstring PartName = ModelName + L"_" +
						to_wstring(i + 1) + L"of" + to_wstring(Desc.iPartModelCount);

					Desc.vOffsets.push_back(_float3(0.f, 0.f, 0.f));
					Desc.PartModelNames.push_back(PartName);
				}
			}

			if (FAILED(m_pGameInstance->Add_GameObject(
				Desc.iLevelID, TEXT("Prototype_GameObject_BreakableMesh"),
				Desc.iLevelID, TEXT("Layer_BreakableMesh"), &Desc)))
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CMapLoader::Ready_Monster()
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

HRESULT CMapLoader::Ready_Stargazer()
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

HRESULT CMapLoader::Ready_Stargazer(const _char* Map)
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

HRESULT CMapLoader::Ready_ErgoItem()
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

HRESULT CMapLoader::Ready_ErgoItem(const _char* Map)
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

HRESULT CMapLoader::Ready_Breakable()
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

HRESULT CMapLoader::Ready_Breakable(const _char* Map)
{
	return S_OK;
}
HRESULT CMapLoader::Ready_Monster(const _char* Map)
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



CMapLoader* CMapLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapLoader* pInstance = new CMapLoader(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CMapLoader");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapLoader::Free()
{
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
}
