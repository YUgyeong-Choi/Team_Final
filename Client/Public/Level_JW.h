#pragma once

#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CLevel_JW final : public CLevel
{
private:
	CLevel_JW(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_JW() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
public:
	static CLevel_JW* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END