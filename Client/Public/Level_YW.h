#pragma once

#include "Client_Defines.h"

#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class CLevel_YW final : public CLevel
{
public:
	enum class IMGUITOOL {
		MAP,
		DECAL,
		END
	};
private:
	CLevel_YW(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_YW() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();
	HRESULT Ready_ImGuiTools();
private:
	HRESULT Ready_ImGui();
	HRESULT ImGui_Render();
	HRESULT ImGui_Docking_Settings();

private:
	HRESULT Ready_Layer_DummyMap(const _wstring strLayerTag);
	HRESULT Ready_Layer_PreviewObject(const _wstring strLayerTag);
	HRESULT Ready_Layer_Sky(const _wstring strLayerTag);

private:
	class CGameObject* m_ImGuiTools[ENUM_CLASS(IMGUITOOL::END)] = {};
	IMGUITOOL	m_eActiveTool = { IMGUITOOL::MAP };

private:
	class CCamera_Manager* m_pCamera_Manager = { nullptr };

public:
	static CLevel_YW* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END