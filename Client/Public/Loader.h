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
	CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
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

public:
	HRESULT Loading_For_Logo();
	HRESULT Loading_For_Static(); // 모두 사용하는 것들 Logo에서 처음에만 생성 
	HRESULT Loading_For_KRAT_CENTERAL_STATION();
	HRESULT Loading_For_KRAT_HOTEL();
	HRESULT Loading_For_DH();
	HRESULT Loading_For_JW();
	HRESULT Loading_For_GL();
	HRESULT Loading_For_YW();
	HRESULT Loading_For_CY();
	HRESULT Loading_For_YG();


public:
	static CLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID);
	virtual void Free() override;
};

NS_END