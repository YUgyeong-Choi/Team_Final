#pragma once

#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI

#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class CLevel_GL final : public CLevel
{
public:
	enum class IMGUITOOL {
		OBJECT,
		END
	};
private:
	CLevel_GL(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_GL() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_ImGuiTools();
private:
	HRESULT Ready_ImGui();
	HRESULT ImGui_Render();
	HRESULT ImGui_Docking_Settings();
private:
	class CGameObject* m_ImGuiTools[ENUM_CLASS(IMGUITOOL::END)];
public:
	static CLevel_GL* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END