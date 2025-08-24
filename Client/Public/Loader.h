#pragma once

#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(Client)

class CLoader final : public CBase
{
private:
	CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, atomic<float>& fRatio);
	virtual ~CLoader() = default;

public:
	_bool isFinished() const {
		return m_isFinished;
	}

public:
	HRESULT Initialize(LEVEL eNextLevelID);
	HRESULT Loading();

	void Output_LoadingText() {
		SetWindowText(g_hWnd, m_szLoadingText);
	}
	

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	LEVEL						m_eNextLevelID = { LEVEL::END };
	_bool						m_isFinished = { false };
	CGameInstance*				m_pGameInstance = { nullptr };

private:
	HANDLE						m_hThread = {};
	CRITICAL_SECTION			m_CriticalSection = {};
	_tchar						m_szLoadingText[MAX_PATH] = {};

	atomic<float>&             m_fRatio;
public:
	HRESULT Loading_For_Logo();
	HRESULT Loading_For_Static(); // 모두 사용하는 것들 Logo에서 처음에만 생성 
	HRESULT Loading_For_KRAT_CENTERAL_STATION();
	HRESULT Loading_For_KRAT_HOTEL();
	HRESULT Loading_For_DH();
	HRESULT Loading_For_JW();
	HRESULT Loading_For_GL();

#pragma region YW
private:
	HRESULT Loading_For_YW();

	//여기서 맵에 필요한것들 모두 로드(맵, 데칼, 네비 등등...)
	HRESULT Load_Map(_uint iLevelIndex, const _char* Map);

	//<맵>에 필요한 메쉬들을 로딩한다.
	HRESULT Loading_Meshs(_uint iLevelIndex, const _char* Map);
	HRESULT Load_Mesh(const wstring& strPrototypeTag, const _char* pModelFilePath, _bool bInstance, _uint iLevelIndex);

	//네비게이션을 로딩한다.
	HRESULT Loading_Navigation(_uint iLevelIndex, const _char* Map/*, _bool bForTool = false*/);

	//필요한 데칼 텍스쳐를 로딩한다.
	HRESULT Loading_Decal_Textures(_uint iLevelIndex, const _char* Map);

private:
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

public:
#pragma endregion
	
#pragma region GL
	HRESULT Loading_For_UI_Texture();
#pragma endregion

	HRESULT Loading_For_CY();
	HRESULT Loading_For_YG();

	static _bool m_bLoadStatic;
public:
	static CLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID, atomic<float>& fRatio);
	virtual void Free() override;
};

NS_END