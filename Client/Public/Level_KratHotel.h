#pragma once

#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Engine)
class CSound_Core;
NS_END

#define PATH_NONANIM "../Bin/Resources/Models/Bin_NonAnim"
#define PRE_TRANSFORMMATRIX_SCALE 0.01f

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
	HRESULT Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath);
	HRESULT Ready_MapModel();
	HRESULT LoadMap();

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