#pragma once

#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(Client)

class CMainApp final : public CBase
{
private:
	CMainApp();
	virtual ~CMainApp() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();

private:
	CGameInstance*				m_pGameInstance = { };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

//#ifdef _DEBUG
private:
	_tchar						m_szFPS[MAX_PATH] = {};
	_uint						m_iRenderCount = {};
	_float						m_fTimeAcc = {};

//#endif

public:
	HRESULT Ready_Fonts();
	HRESULT Ready_Prototype_Component();
	HRESULT Ready_Static();
	HRESULT Ready_Sound();
	HRESULT Start_Level(LEVEL eStartLevel);

	// 너무 많아지면 줄이거나 없애기
	HRESULT Ready_Loading();

public:
	static CMainApp* Create();
	virtual void Free() override;
};

NS_END