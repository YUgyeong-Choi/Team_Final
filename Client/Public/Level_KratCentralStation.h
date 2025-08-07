#pragma once

#include "Client_Defines.h"
#include "Level.h"

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
	virtual HRESULT Render() override;

private:
	//HRESULT Load_Model(const wstring& strPrototypeTag, const _char* pModelFilePath);
	//HRESULT Ready_MapModel();
	//HRESULT LoadMap();
	HRESULT LoadMap(_uint iLevelIndex);
	HRESULT Load_StaticMesh(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex);
	HRESULT Load_StaticMesh_Instance(_uint iObjectCount, const json& objects, string ModelName, _uint iLevelIndex);


private:
	HRESULT Ready_Player();
	HRESULT Ready_Npc();
	HRESULT Ready_Lights();
	HRESULT Ready_Shadow();
	HRESULT Ready_Camera();
	HRESULT Ready_Layer_StaticMesh(const _wstring strLayerTag);
	HRESULT Ready_Layer_Sky(const _wstring strLayerTag);
	HRESULT Ready_UI();
	HRESULT Ready_Video();
	HRESULT Ready_Monster();
	HRESULT Ready_Effect();

private:
	void ToggleHoldMouse() { m_bHold = !m_bHold; }

private:
	class CCamera_Manager* m_pCamera_Manager = { nullptr };
	CSound_Core* m_pBGM = { nullptr };

	class CUI_Video* m_pStartVideo = {nullptr};

private:
	_bool m_bHold = { true };

public:
	static CLevel_KratCentralStation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END