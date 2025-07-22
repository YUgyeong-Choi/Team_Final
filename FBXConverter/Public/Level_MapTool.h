#pragma once

#include "Level.h"

#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class CLevel_MapTool final : public CLevel
{
public:
	enum class IMGUITOOL {
		TERRAIN,
		OBJECT,
		CONVERT,
		CAMERA,
		END
	};
private:
	CLevel_MapTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_MapTool() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_ImGui();

private:
	HRESULT ImGui_Render();
	HRESULT ImGui_Docking_Settings();
	HRESULT Window_ObjectList();
	HRESULT Window_ObjectInspector();
	HRESULT Window_ResourceList();

private: /* 상단 툴 바 */
	HRESULT Toolbar_Menus();
	HRESULT Terrain_Tools(_bool* p_open);
	HRESULT Show_ExternWindows();

private:
	CGameObject*		m_pCurSelectedObj;
	IMGUIWINDATA		m_tWindowData;

private:
	class CImGuiTool* m_ImGuiTools[ENUM_CLASS(IMGUITOOL::END)];

public:
	HRESULT Ready_Lights();
	HRESULT Ready_ImGuiTools();
	HRESULT Ready_Layer_Camera(const _wstring strLayerTag);


public:
	static CLevel_MapTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END