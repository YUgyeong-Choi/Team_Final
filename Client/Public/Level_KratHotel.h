#pragma once

#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Engine)
class CSound_Core;
NS_END

NS_BEGIN(Client)

class CLevel_KratHotel final : public CLevel
{
private:
	CLevel_KratHotel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_KratHotel() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();
	HRESULT Ready_Layer_Sky(const _wstring strLayerTag);

private:
	class CCamera_Manager* m_pCamera_Manager = { nullptr };
	CSound_Core* m_pBGM = { nullptr };

public:
	static CLevel_KratHotel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END