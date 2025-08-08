#pragma once


#include "Client_Defines.h"

#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class CLevel_YG final : public CLevel
{
public:
	enum class IMGUITOOL {
		CAMERA,
		END
	};
private:
	CLevel_YG(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_YG() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Camera();
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Object(const _wstring strLayerTag);
	HRESULT Ready_Layer_Sky(const _wstring strLayerTag);
	HRESULT Ready_Layer_Station(const _wstring strLayerTag);
	HRESULT Ready_Player();

	HRESULT LoadMap(_uint iLevelIndex);
	HRESULT Load_StaticMesh(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex);
	HRESULT Load_StaticMesh_Instance(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex);
private:

	HRESULT Ready_ImGuiTools();
	HRESULT Ready_ImGui();
	HRESULT ImGui_Render();
	HRESULT ImGui_Docking_Settings();

private:
	class CCamera_Manager* m_pCamera_Manager = { nullptr };
	class CLockOn_Manager* m_pLockOn_Manager = { nullptr };
	class CGameObject* m_ImGuiTools[ENUM_CLASS(IMGUITOOL::END)];
public:
	static CLevel_YG* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END