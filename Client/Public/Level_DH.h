#pragma once

#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI

#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
class CSound_Core;
class CShader;
class CComponent;
NS_END

NS_BEGIN(Client)

class CLevel_DH final : public CLevel
{
public:
	enum class IMGUITOOL {
		DONGHA,
		END
	};
private:
	CLevel_DH(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_DH() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;



private:
	HRESULT Ready_Camera();
	HRESULT Ready_Lights();
	HRESULT Ready_ImGuiTools();
	HRESULT Ready_Layer_StaticMesh(const _wstring strLayerTag);
	HRESULT Ready_Layer_Sky(const _wstring strLayerTag);
private:
	HRESULT Ready_ImGui();
	HRESULT ImGui_Render();
	HRESULT ImGui_Docking_Settings();
private:
	class CGameObject* m_ImGuiTools[ENUM_CLASS(IMGUITOOL::END)];

private:
	class CCamera_Manager* m_pCamera_Manager = { nullptr };

public:
	static CLevel_DH* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END