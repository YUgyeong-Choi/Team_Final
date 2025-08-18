#pragma once


#include "Client_Defines.h"

#include "Level.h"
#include "DHTool.h"

NS_BEGIN(Engine)
class CSound_Core;
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
	HRESULT Ready_Player();
	HRESULT Ready_Camera();
	HRESULT Ready_Lights();
	HRESULT Ready_UI();
	HRESULT Ready_Layer_Sky(const _wstring strLayerTag);
	HRESULT Ready_OctoTree();
	HRESULT Ready_Interact();
	HRESULT Add_MapActor(); //맵 액터 추가(콜라이더 활성화)
private:
	HRESULT Load_Shader();
	HRESULT Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg = nullptr);


	HRESULT Add_Light(CDHTool::LIGHT_TYPE eType, CDHTool::LEVEL_TYPE eLType);
	HRESULT Add_RenderGroup_OctoTree();
private:
	HRESULT Separate_Area();
	void ToggleHoldMouse() { m_bHold = !m_bHold; }

private:
	HRESULT Ready_Layer_Object(const _wstring strLayerTag);
private:

	HRESULT Ready_ImGuiTools();
	HRESULT Ready_ImGui();
	HRESULT ImGui_Render();
	HRESULT ImGui_Docking_Settings();
private:
	CShader* m_pShaderComPBR = { nullptr };
	CShader* m_pShaderComANIM = { nullptr };
	CShader* m_pShaderComInstance = { nullptr };

	CSound_Core* m_pBGM = { nullptr };

	class CUI_Video* m_pStartVideo = { nullptr };
	_bool m_bHold = { true };
	vector<class CDH_ToolMesh*> m_vecLights;

	class CPlayer* m_pPlayer = { nullptr };
private:
	class CCamera_Manager* m_pCamera_Manager = { nullptr };
	class CGameObject* m_ImGuiTools[ENUM_CLASS(IMGUITOOL::END)];
public:
	static CLevel_YG* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END