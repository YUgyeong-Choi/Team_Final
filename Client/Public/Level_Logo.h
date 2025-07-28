#pragma once

#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CLevel_Logo final : public CLevel
{
private:
	CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Logo() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT Ready_Video();

	HRESULT Ready_Menu();


private:
	class CUI_Video* m_pMainUI = {};

	_bool			 m_isReady = { false };
	LEVEL			 m_eNextLevel = {LEVEL::END};

	_float           m_fDelay = 0.3f;
	

	_int			 m_iButtonIndex = {-1};

	vector <class CUI_Button* > m_pButtons = {};

public:
	static CLevel_Logo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END