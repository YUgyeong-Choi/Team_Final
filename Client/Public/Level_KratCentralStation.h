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

#pragma region YW
private:
	//맵 소환(true면 테스트 맵 소환)
	HRESULT Ready_Map(_uint iLevelIndex, _bool bTest = false);
	HRESULT Ready_StaticMesh(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex);
	HRESULT Ready_StaticMesh_Instance(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex);

	//네비게이션 소환
	HRESULT Ready_Nav(const _wstring strLayerTag);

	//스태틱 데칼을 소환한다. (true면 테스트 데칼 소환)
	HRESULT Ready_Static_Decal(_uint iLevelIndex, _bool bTest = false);
#pragma endregion

private:
	HRESULT Ready_Player();
	HRESULT Ready_Npc();
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();
	HRESULT Ready_Layer_Sky(const _wstring strLayerTag);
	HRESULT Ready_UI();
	HRESULT Ready_Video();
	HRESULT Ready_Monster();
	HRESULT Ready_Effect();
	HRESULT Ready_OctoTree();

private:
	HRESULT Load_Shader();
	HRESULT Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg = nullptr);


	HRESULT Add_Light(CDHTool::LIGHT_TYPE eType, CDHTool::LEVEL_TYPE eLType);
	HRESULT Add_RenderGroup_OctoTree();

private:
	void ToggleHoldMouse() { m_bHold = !m_bHold; }

private:
	CShader* m_pShaderComPBR = { nullptr };
	CShader* m_pShaderComANIM = { nullptr };
	CShader* m_pShaderComInstance = { nullptr };

private:
	class CCamera_Manager* m_pCamera_Manager = { nullptr };
	CSound_Core* m_pBGM = { nullptr };

	class CUI_Video* m_pStartVideo = {nullptr};

private:
	vector<class CStaticMesh*> m_vecOctoTreeObjects;

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