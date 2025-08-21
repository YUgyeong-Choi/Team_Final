#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "DHTool.h"

NS_BEGIN(Engine)
class CSound_Core;
NS_END

NS_BEGIN(Client)

class CLevel_KratCentralStation final : public CLevel
{
private:
	CLevel_KratCentralStation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_KratCentralStation() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT Ready_Level();

private:
	HRESULT Add_MapActor(); //맵 액터 추가(콜라이더 활성화)
	HRESULT Ready_Player();
	HRESULT Ready_Npc();
	HRESULT Ready_Dummy();
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();
	HRESULT Ready_Layer_Sky(const _wstring strLayerTag);
	HRESULT Ready_UI();
	HRESULT Ready_Video();
	HRESULT Ready_Monster();
	//특정 맵의 몬스터를 소환한다. 그 맵의 네비게이션을 장착시킨다.
	HRESULT Ready_Monster(const _char* Map);
	HRESULT Ready_Effect();
	HRESULT Ready_OctoTree();
	HRESULT Ready_Interact();
	HRESULT Ready_Trigger();



private:
	HRESULT Load_Shader();
	HRESULT Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg = nullptr);


	HRESULT Add_Light(CDHTool::LIGHT_TYPE eType, CDHTool::LEVEL_TYPE eLType);
	HRESULT Add_RenderGroup_OctoTree();

private:
	HRESULT Separate_Area();

private:
	void ToggleHoldMouse() { m_bHold = !m_bHold; }

private:
	CShader* m_pShaderComPBR = { nullptr };
	CShader* m_pShaderComANIM = { nullptr };
	CShader* m_pShaderComInstance = { nullptr };

private:
	class CCamera_Manager* m_pCamera_Manager = { nullptr };
	CSound_Core* m_pBGM = { nullptr };

	class CUI_Video* m_pStartVideo = { nullptr };
	_bool m_bEndVideo = {};

private:
	_bool m_bHold = { true };
	vector<class CDH_ToolMesh*> m_vecLights;

private:
	class CPlayer* m_pPlayer = { nullptr };

public:
	static CLevel_KratCentralStation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END