#pragma once

#include "Client_Defines.h"
#include "Base.h"

#define TESTMAP //테스트맵 하기 싫으면 주석하시오 (영웅)

#ifdef TESTMAP

//#define TEST_TEST_MAP // 이거 키면 테스트맵
//#define TEST_STATION_MAP //이거 키면 스테이션 맵 생성됨
//#define TEST_HOTEL_MAP //이거 키면   호텔 맵 생성됨
#define TEST_OUTER_MAP //이거 키면  외부 맵 생성됨
//#define TEST_FIRE_EATER_MAP //이거 키면 푸오코 맵 생성됨

#endif _TESTMAP

#define START_MAP "STATION"


NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(Client)
class CMapLoader final :public CBase
{
private:
	CMapLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMapLoader() = default;

public:
	HRESULT Initialize();
	_bool Check_MapLoadComplete(_uint iLevelIndex);

public:
	/*
	 스테틱 메쉬 로딩, 배치
	 맵 액터 등록
	 네비, 데칼, 부서지는 메쉬, 아이템, 스타게이저 등등...
	*/
	HRESULT Ready_Map_Async();

	HRESULT Load_Ready_Nav_All(_uint iLevelIndex);

	HRESULT Load_Ready_All_Etc(_uint iLevelIndex);

	HRESULT Ready_Etc(_uint iLevelIndex, const _char* Map);

public:
	//여기서 맵에 필요한것들 모두 로드(맵, 데칼, 네비 등등...)
	HRESULT Load_Map(_uint iLevelIndex, const _char* Map);

	//<맵>에 필요한 메쉬들을 로딩한다.
	HRESULT Loading_Meshs(_uint iLevelIndex, const _char* Map);
	HRESULT Load_Mesh(const wstring& strPrototypeTag, const _char* pModelFilePath, _bool bInstance, _uint iLevelIndex);

	//네비게이션을 로딩한다.
	HRESULT Loading_Navigation(_uint iLevelIndex, const _char* Map/*, _bool bForTool = false*/);

	//필요한 데칼 텍스쳐를 로딩한다.
	HRESULT Loading_Decal_Textures(_uint iLevelIndex, const _char* Map);

	//부서질 수 있는 메쉬를 로딩한다.
	HRESULT Loading_Breakable(_uint iLevelIndex, const _char* Map);

public:
	//맵 소환(메쉬, 네비, 데칼 등...)
	HRESULT Ready_Map(_uint iLevelIndex, const _char* Map);

	//메쉬 소환
	HRESULT Ready_Meshs(_uint iLevelIndex, const _char* Map);
	HRESULT Ready_StaticMesh(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex, const _char* Map);
	HRESULT Ready_StaticMesh_Instance(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex, const _char* Map);

	//네비게이션 소환
	HRESULT Ready_Nav(const _wstring strLayerTag, _uint iLevelIndex, const _char* Map);

	//스태틱 데칼을 소환한다. (true면 테스트 데칼 소환)
	HRESULT Ready_Static_Decal(_uint iLevelIndex, const _char* Map);

	//부서질 수 있는 메쉬를 소환한다.
	HRESULT Ready_Breakable(_uint iLevelIndex, const _char* Map);

public:
	HRESULT Add_MapActor(const _char* Map); //맵 액터 추가(콜라이더 활성화)


public:
	HRESULT Ready_Monster();
	HRESULT Ready_Monster(const _char* Map);//특정 맵의 몬스터를 소환한다. 그 맵의 네비게이션을 장착시킨다.

	HRESULT Ready_Stargazer();
	HRESULT Ready_Stargazer(const _char* Map);

	HRESULT Ready_ErgoItem();
	HRESULT Ready_ErgoItem(const _char* Map);

	HRESULT Ready_Breakable();
	HRESULT Ready_Breakable(const _char* Map);

private:
	vector<const _char*> m_Maps = { "HOTEL", "OUTER" ,"FIRE_EATER" };

private:
	// 완료된 맵 이름을 저장하는 스레드 안전 큐
	queue<const char*> m_ReadyQueue;
	mutex m_QueueMutex;

	//맵 개수 관리용 멤버 추가
	size_t m_TotalMapCount = 0;
	atomic<size_t> m_LoadedCount = 0;

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance* m_pGameInstance = { nullptr };

public:
	static CMapLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};
NS_END