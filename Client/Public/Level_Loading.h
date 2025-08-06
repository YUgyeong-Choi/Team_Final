#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "UI_Bar_Loading.h"

NS_BEGIN(Client)

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Loading() = default;

public:
	virtual HRESULT Initialize(LEVEL eNextLevelID);
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT	Ready_Loading();

	void Update_Loding_Bar();
	

private:
	LEVEL				m_eNextLevelID = { LEVEL::END };
	class CLoader*		m_pLoader = { nullptr };

	atomic<_float>      m_fRatio = {0.f};

	CUI_Bar_Loading*	m_pLoadingBar = { nullptr };

	_float				m_fDelay = { 0.5f };

public:
	static CLevel_Loading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID);
	virtual void Free() override;
};

NS_END