#pragma once

#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CLevel_KratCentralStation final : public CLevel
{
private:
	CLevel_KratCentralStation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_KratCentralStation() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
public:
	static CLevel_KratCentralStation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END