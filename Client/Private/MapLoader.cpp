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
	//��
	if (FAILED(Loading_Meshs(iLevelIndex, Map)))
	{
		MSG_BOX("�޽� �ε� ����!");
		return E_FAIL;
	}

	//�׺�
	if (FAILED(Loading_Navigation(iLevelIndex, Map)))
	{
		MSG_BOX("�׺���̼� �ε� ����!");
		return E_FAIL;
	}

	//��Į
	if (FAILED(Loading_Decal_Textures(iLevelIndex, Map)))
	{
		MSG_BOX("��Į �ؽ�ó �ε� ����!");
		return E_FAIL;
	}

	//�μ��� �� �ִ� �޽� ������, �Ǵ� ���޽� �ε�
	if (FAILED(Loading_Breakable(iLevelIndex, Map)))
	{
		MSG_BOX("�극��Ŀ�� �޽� �ε� ����!");
		return E_FAIL;
	}

	return S_OK;

}

HRESULT CMapLoader::Load_Mesh(const wstring& strPrototypeTag, const _char* pModelFilePath, _bool bInstance, _uint iLevelIndex)
{
	_matrix PreTransformMatrix = XMMatrixScaling(PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE, PRE_TRANSFORMMATRIX_SCALE);

	// ���� ��ο��� ���丮, ���ϸ�, Ȯ���� �и�
	wstring wsBasePath = filesystem::path(pModelFilePath).parent_path();
	wstring wsFilename = filesystem::path(pModelFilePath).stem();	// Ȯ���� �� ���ϸ�
	wstring wsExtension = filesystem::path(pModelFilePath).extension();

	// LOD0 �� ��� ����
	string strModelPathLod0 = (filesystem::path(wsBasePath) / filesystem::path(wsFilename + wsExtension)).string();

	if (bInstance == false)
	{
		// LOD1, LOD2 ���ϸ� ���� (_Lod1, _Lod2 ����)
		wstring wsLod1Name = wsFilename + L"_Lod1" + wsExtension;
		wstring wsLod2Name = wsFilename + L"_Lod2" + wsExtension;

		// LOD1, LOD2 ��� ����
		filesystem::path Lod1Path = filesystem::path(wsBasePath) / wsLod1Name;
		filesystem::path Lod2Path = filesystem::path(wsBasePath) / wsLod2Name;
		string strModelPathLod1 = Lod1Path.string();
		string strModelPathLod2 = Lod2Path.string();

		// LOD0 ������Ÿ�� ���� ���� Ȯ�� �� ������ �ε�
		if (m_pGameInstance->Find_Prototype(iLevelIndex, strPrototypeTag) == nullptr)
		{
			if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, strPrototypeTag,
				CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelPathLod0.c_str(), PreTransformMatrix))))
				return E_FAIL;
		}

		// LOD1 ������Ÿ�� �±� ���� �� �ε� (���� �����ϸ�)
		wstring wsPrototypeTagLod1 = strPrototypeTag + L"_Lod1";
		if (m_pGameInstance->Find_Prototype(iLevelIndex, wsPrototypeTagLod1) == nullptr)
		{
			if (filesystem::exists(strModelPathLod1))
			{
				if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, wsPrototypeTagLod1,
					CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelPathLod1.c_str(), PreTransformMatrix))))
					return S_OK; // �����ص� ����
			}
		}

		// LOD2 ������Ÿ�� �±� ���� �� �ε� (���� �����ϸ�)
		wstring wsPrototypeTagLod2 = strPrototypeTag + L"_Lod2";
		if (m_pGameInstance->Find_Prototype(iLevelIndex, wsPrototypeTagLod2) == nullptr)
		{
			if (filesystem::exists(strModelPathLod2))
			{
				if (FAILED(m_pGameInstance->Add_Prototype(iLevelIndex, wsPrototypeTagLod2,
					CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelPathLod2.c_str(), PreTransformMatrix))))
					return S_OK; // �����ص� ����
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
		wstring ErrorMessage = L"Resource_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);

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
		MessageBoxA(nullptr, e.what(), "JSON �Ľ� ����", MB_OK);
		return E_FAIL;
	}

	// JSON ������ Ȯ��
	for (const auto& element : ResourceJson)
	{
		string ModelName = element.value("ModelName", "");
		string Path = element.value("Path", "");

		//������ �����ؼ� �ν��Ͻ��� �� ������Ÿ���� ������ �����ؾ��ҵ�(�浹���η� �Ǵ�����)
		_uint iObjectCount = element["ObjectCount"];

		_bool bCollision = element["Collision"];

		_bool bNoInstancing = element["NoInstancing"];

		wstring PrototypeTag = {};
		_bool bInstance = false;
		if (bCollision == false && iObjectCount > INSTANCE_THRESHOLD && bNoInstancing == false)
		{
			//�ν��Ͻ̿� �� ������ Ÿ�� ����
			PrototypeTag = L"Prototype_Component_Model_Instance_" + StringToWString(ModelName);
			bInstance = true;

		}
		else
		{
			//�� ������ Ÿ�� ����
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
	//bForTool = true;//bForTool ������ ����

	wstring wsResourcePath = L"../Bin/Save/NavTool/Nav_" + StringToWString(Map) + L".json";

	// ���� ����
	ifstream inFile(wsResourcePath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Nav_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�.";
		MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);
		return S_OK;
	}

	wstring wsPrototypeTag = {};

	//if(bForTool)
	wsPrototypeTag = L"Prototype_Component_Navigation_" + StringToWString(Map);
	//else
	//	wsPrototypeTag = L"Prototype_Component_Navigation";//���� ���� ���� �ε�(���� �Ȱ��� ������ ����)

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
		wstring ErrorMessage = L"Resource_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);

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
		MessageBoxA(nullptr, e.what(), "JSON �Ľ� ����", MB_OK);
		return E_FAIL;
	}

	//�ؽ��� ������Ÿ�� ����
	for (auto& [PrototypeTag, Path] : ReadyTextureJson.items())
	{
		//�̹� ������ �ѱ��
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

	// JSON ���� ���
	wstring wsPath = L"../Bin/Save/MapTool/Breakable_" + StringToWString(Map) + L".json";

	ifstream ifs(wsPath);
	if (!ifs.is_open())
		return S_OK;

	json j;
	ifs >> j;

	// JSON �ֻ��� �ٷ� ModelName Ȯ��
	for (auto& [ModelName, ModelData] : j.items())
	{
		// Ǫ���� ��� ����
		if (ModelName == "SM_Factory_BasePipe_07")
			continue;

		// FragmentCount �б�
		if (!ModelData.contains("FragmentCount"))
			continue;

		_uint finalDenom = ModelData["FragmentCount"].get<_uint>();

		// 1 ~ finalDenom ������ ���� ���
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
						return S_OK; // �����ص� ����
					}
				}
			}
		}
	}

	return S_OK;
}


HRESULT CMapLoader::Ready_Map(_uint iLevelIndex, const _char* Map)
{
	//� ���� ��ȯ ��ų ������?
	if (FAILED(Ready_Meshs(iLevelIndex, Map))) //TEST, STAION
		return E_FAIL;

	//�׺� ��ȯ
	if (FAILED(Ready_Nav(TEXT("Layer_Nav"), iLevelIndex, Map)))
		return E_FAIL;

	//� ��Į�� ��ȯ ��ų ������?
	if (FAILED(Ready_Static_Decal(iLevelIndex, Map))) //TEST, STATION
		return E_FAIL;

	//�μ��� �� �ִ� ������Ʈ ��ȯ
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
		wstring ErrorMessage = L"Map_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);
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
		_uint iObjectCount = Models[i]["ObjectCount"]; //������Ʈ ���������� �ν��Ͻ��� ���� ���� �����ؾ߰ڴ�.(�ƴ� �浹���η� �ν��Ͻ� �ؾ߰ڴ�.)

		if (iObjectCount == 0)
			continue;

		const json& objects = Models[i]["Objects"];

		_bool bCollision = Models[i]["Collision"];
		_bool bNoInstancing = Models[i]["NoInstancing"];

		//���� ���� �̻��̸� �ν��Ͻ̿�����Ʈ�� �ε�(�浹�� ���� ���̸� �ν��Ͻ�)
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
#pragma region �������
		CStaticMesh::STATICMESH_DESC StaticMeshDesc = {};

		const json& WorldMatrixJson = objects[j]["WorldMatrix"];
		_float4x4 WorldMatrix = {};

		for (_int row = 0; row < 4; ++row)
			for (_int col = 0; col < 4; ++col)
				WorldMatrix.m[row][col] = WorldMatrixJson[row][col];

		StaticMeshDesc.WorldMatrix = WorldMatrix;
#pragma endregion

#pragma region Ÿ�ϸ�
		//Ÿ�ϸ�
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

#pragma region �ݶ��̴�
		//�ݶ��̴�
		if (objects[j].contains("ColliderType") && objects[j]["ColliderType"].is_number_integer())
		{
			StaticMeshDesc.eColliderType = static_cast<COLLIDER_TYPE>(objects[j]["ColliderType"].get<_int>());
		}
		else
			return E_FAIL;
#pragma endregion

#pragma region ����Ʈ���
		StaticMeshDesc.iLightShape = objects[j].value("LightShape", 0);
#pragma endregion

#pragma region �ٴڿ���
		if (objects[j].contains("IsFloor"))
		{
			StaticMeshDesc.bIsFloor = objects[j].value("IsFloor", false);
		}
		else
		{
			StaticMeshDesc.bIsFloor = false;
		}
#pragma endregion

#pragma region �ø� ����
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

		// �� �ɱ�
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


	//������Ʈ ����, ��ġ
	//LayerTag += StringToWString(ModelName);

	CStaticMesh_Instance::STATICMESHINSTANCE_DESC StaticMeshInstanceDesc = {};
	StaticMeshInstanceDesc.iNumInstance = iObjectCount;//�ν��Ͻ� ������
	StaticMeshInstanceDesc.pInstanceMatrixs = &InstanceMatixs;//������ĵ��� �Ѱ�����Ѵ�.

	StaticMeshInstanceDesc.iRender = 0;
	StaticMeshInstanceDesc.iLevelID = iLevelIndex;
	//lstrcpy(StaticMeshInstanceDesc.szName, TEXT("SM_TEST_FLOOR"));

	StaticMeshInstanceDesc.iLightShape = objects[0].value("LightShape", 0); //������Ʈ�� �ϳ� �����ͼ� ����Ʈ����� �־�����.
	//cout << StaticMeshInstanceDesc.iLightShape << endl;

	wstring wstrModelName = StringToWString(ModelName);
	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_Instance_"); //�ν��Ͻ� �� ���� �غ��ؾ߰ڴµ�?
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
	//���� �ʿ� �ʿ��� ��Į �ؽ��ĸ� �ε��Ѵ�.
	string DecalDataPath = string("../Bin/Save/DecalTool/Decal_") + Map + ".json";
	//string ResourcePath = string("../Bin/Save/MapTool/Resource_") + Map + ".json"; //���߿� ���� �� �ٲܶ�

	ifstream inFile(DecalDataPath);
	if (!inFile.is_open())
	{
		wstring ErrorMessage = L"Decal_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�: ";
		MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);
		return S_OK;
	}

	// JSON �Ľ�
	json JSON;
	inFile >> JSON;

	// ������ ��ŭ Decal ��ȯ
	for (auto& item : JSON)
	{
		// 4x4 ��� �б�
		_float4x4 WorldMatrix;
		for (_int iRow = 0; iRow < 4; ++iRow)
		{
			for (_int iCol = 0; iCol < 4; ++iCol)
			{
				WorldMatrix.m[iRow][iCol] = item["WorldMatrix"][iRow][iCol].get<_float>();
			}
		}

		//�ؽ��� ������Ÿ�� �̸� ����
		CStatic_Decal::DECAL_DESC Desc = {};
		Desc.WorldMatrix = WorldMatrix;
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::ARMT)] = StringToWString(item["ARMT"].get<string>());
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::N)] = StringToWString(item["N"].get<string>());
		Desc.PrototypeTag[ENUM_CLASS(CDecal::TEXTURE_TYPE::BC)] = StringToWString(item["BC"].get<string>());
		Desc.iLevelID = iLevelIndex;

		// Decal ��ü ����
		if (FAILED(m_pGameInstance->Add_GameObject(iLevelIndex, TEXT("Prototype_GameObject_Static_Decal"),
			iLevelIndex, TEXT("Layer_Decal"), &Desc)))
			return E_FAIL;

	}

	return S_OK;

}

HRESULT CMapLoader::Add_MapActor(const _char* Map)
{
	//���⼭ ���͸� �߰����ش�. ���ν����忡�� ���͸� �߰��ϴ� ���� �����ϴٰ� �Ѵ�.

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
	// JSON ���
	string FilePath = string("../Bin/Save/MapTool/Breakable_") + Map + ".json";
	ifstream inFile(FilePath);
	if (!inFile.is_open())
		return S_OK;

	json Json;
	inFile >> Json;
	inFile.close();

	// JSON���� ModelName �ٷ� ����
	for (auto& [ModelNameStr, ModelData] : Json.items())
	{
		wstring ModelName = StringToWString(ModelNameStr);

		// FragmentCount �б�
		_int FragmentCount = 0;
		if (ModelData.contains("FragmentCount") && ModelData["FragmentCount"].is_number_integer())
			FragmentCount = ModelData["FragmentCount"];

		// Instances �迭 Ȯ��
		if (!ModelData.contains("Instances") || !ModelData["Instances"].is_array())
			continue;

		const json& Instances = ModelData["Instances"];

		for (const auto& Obj : Instances)
		{
			if (!Obj.contains("WorldMatrix"))
				continue;

			// ���� ��� �б�
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
				// Ǫ���� ��� ���� ó��
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
				// �Ϲ����� �μ���
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
#pragma region ���� JSON ���� �޾Ƽ� ��ȯ

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


#pragma region �������� �ϵ��ڵ� ������� ��ȯ(���� ������ķ� ���� ��ȯ���̶� �̰ɷ��ϸ� �׵���ķ� ��ȯ��!!! �ٵ� �׵���ķ� ��ȯ�ϴϱ� �÷��̾� �浹 ���� ����)

	//CMonster_Base::MONSTER_BASE_DESC Desc{};
	//Desc.fSpeedPerSec = 5.f;
	//Desc.fRotationPerSec = XMConvertToRadians(180.0f);
	//Desc.fHeight = 1.f;
	//Desc.vExtent = { 0.5f,1.f,0.5f };
	//Desc.eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;

	////���� ��ġ�� �׺���̼��� ������ ������ ����
	//Desc.wsNavName = TEXT("STATION");
	//Desc.InitPos =
	//	//_float3(148.f, 2.47f, -7.38f); //ȣ����ġ
	//	_float3(85.5f, 0.f, -7.5f); //�����̼� ��ġ
	//Desc.szMeshID = TEXT("WatchDog");
	//

	////Desc.InitPos = _float3(80.5f, 0.f, -7.f); //�����̼� ��ġ
	//if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_WatchDog"),
	//	ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster_Normal"), &Desc)))
	//	return E_FAIL;

	//Desc.wsNavName = TEXT("HOTEL");
	//Desc.InitPos =
	//	_float3(148.f, 2.47f, -7.38f); //ȣ����ġ
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
		//wstring ErrorMessage = L"Stargazer_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�.";
		//MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);
		return S_OK;
	}

	json StargazerJson;
	inFile >> StargazerJson;
	inFile.close();

	// �迭 ��ȸ
	for (auto& StargazerData : StargazerJson)
	{
		// ���� ���
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
			MSG_BOX("���ٶ�� �±׸� �ȴ޾���!!!!");

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
		//wstring ErrorMessage = L"Stargazer_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�.";
		//MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);
		return S_OK;
	}

	json ErgoItemJson;
	inFile >> ErgoItemJson;
	inFile.close();

	// �迭 ��ȸ
	for (auto& ErgoItemData : ErgoItemJson)
	{
		// ���� ���
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
			//�±� �ȴ޾��ָ� ����� ������� ����
			Desc.eItemTag = ITEM_TAG::ERGO_SHARD;
			//MSG_BOX("������ �±׸� �ȴ޾���!!!!");

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
	//���� Ǫ���� ����� ���� �����θ�

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
		wstring ErrorMessage = L"Monster_" + StringToWString(Map) + L".json ������ �� �� �����ϴ�.";
		MessageBox(nullptr, ErrorMessage.c_str(), L"����", MB_OK);
		return S_OK;
	}

	json MonsterJson;
	inFile >> MonsterJson;
	inFile.close();

	// ���� �������� �ݺ�
	for (auto& [MonsterName, MonsterArray] : MonsterJson.items())
	{
		wstring wstrMonsterName = StringToWString(MonsterName);

		for (auto& MonsterData : MonsterArray)
		{
			// ���� ��� �ε�
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

			// ������Ʈ ���� Desc ä���
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
