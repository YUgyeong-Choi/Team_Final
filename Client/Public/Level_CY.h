#pragma once

#include "Client_Defines.h"

#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class CLevel_CY final : public CLevel
{
public:
	enum class IMGUITOOL {
		MAP,
		END
	};
private:
	CLevel_CY(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_CY() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();
	HRESULT Ready_ImGuiTools();
	HRESULT Ready_Sky(const _wstring strLayerTag);

	HRESULT Ready_Layer_StaticMesh(const _wstring strLayerTag);

private:
	HRESULT Ready_ImGui();
	HRESULT ImGui_Render();
	HRESULT ImGui_Docking_Settings();
private:
	class CGameObject* m_ImGuiTools[ENUM_CLASS(IMGUITOOL::END)];

private:
	class CCamera_Manager* m_pCamera_Manager = { nullptr };

public:
	static CLevel_CY* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END