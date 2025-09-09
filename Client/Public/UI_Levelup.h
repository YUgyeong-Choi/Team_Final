
#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"
#include "Player.h"

NS_BEGIN(Client)

// 파일에 있는 거만 가져오도록
// 파일을 따로 툴로 만들어서 저장하면 될듯

class CUI_Levelup : public CUI_Container
{
public:
	typedef struct tagLevelupUIStruct : public CUI_Container::UI_CONTAINER_DESC
	{

		CGameObject* pTarget;

	}UI_LEVELUP_DESC;
protected:
	CUI_Levelup(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Levelup(const CUI_Levelup& Prototype);
	virtual ~CUI_Levelup() = default;

public:




public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	// 같은거 여러개 만들고 스탯 가져와서 넣어주고... 위치 맞춰주고
	HRESULT Ready_Stat_UI();
	HRESULT Ready_Weapon_UI();

	void Update_Button();

	void Interation_Button();

	void Update_Stat(_bool isPlus);

	// 돈이 되면 true, 안되면 false
	_bool Check_Ergo();

	_float Compute_MaxErgo();

protected:

	// 여기서 끝나면 다시 스크립트로 이동하도록
	CGameObject* m_pTarget = { nullptr };

	// 플레이어 정보를 가져오고 그거 기반으로 스탯을 조절한다
	CPlayer* m_pPlayer = { nullptr };
	
	// 스탯 보여줄 버튼
	vector<CUI_Container*> m_pStatButtons = {};
	// 옆에 불
	class CDynamic_UI* m_pSelectUI = { nullptr };

	// 에르고
	CUI_Container* m_pErgoInfo = { nullptr };

	// 레벨 업 버튼
	CUI_Container* m_pLevelUpButton = { nullptr };

	// 무기 정보, 리전 암 보정 수치
	vector<CUI_Container*> m_pWeaponStat = {};
	vector<CUI_Container*> m_pLegionStat = {};

	// 체력, 스태미나 이런 거
	CUI_Container* m_pAbilInfo = { nullptr };

	CUI_Container* m_pWeaponInfo = { nullptr };
	CUI_Container* m_pLegionInfo = { nullptr };

	// 방어력 / 피감 / 저항
	CUI_Container* m_pArmorInfo = { nullptr };

	// 맨 아래
	CUI_Container* m_pBehindButtons = { nullptr };

	_int m_iButtonIndex = { 0 };

	_bool m_bInit = {};

	
	STAT_DESC m_eStat = {};
	_int	  m_iLevel = {};
	_float    m_fErgo = {};
	// 계산할 때 쓸 거
	_float    m_fCurrentErgo = {};

	_int	  m_iCount = {};
	_int      m_iStat[6] = {};

public:
	static CUI_Levelup* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END