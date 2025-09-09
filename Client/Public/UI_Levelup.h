
#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"
#include "Player.h"

NS_BEGIN(Client)

// ���Ͽ� �ִ� �Ÿ� ����������
// ������ ���� ���� ���� �����ϸ� �ɵ�

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
	// ������ ������ ����� ���� �����ͼ� �־��ְ�... ��ġ �����ְ�
	HRESULT Ready_Stat_UI();
	HRESULT Ready_Weapon_UI();

	void Update_Button();

	void Interation_Button();

	void Update_Stat(_bool isPlus);

	// ���� �Ǹ� true, �ȵǸ� false
	_bool Check_Ergo();

	_float Compute_MaxErgo();

protected:

	// ���⼭ ������ �ٽ� ��ũ��Ʈ�� �̵��ϵ���
	CGameObject* m_pTarget = { nullptr };

	// �÷��̾� ������ �������� �װ� ������� ������ �����Ѵ�
	CPlayer* m_pPlayer = { nullptr };
	
	// ���� ������ ��ư
	vector<CUI_Container*> m_pStatButtons = {};
	// ���� ��
	class CDynamic_UI* m_pSelectUI = { nullptr };

	// ������
	CUI_Container* m_pErgoInfo = { nullptr };

	// ���� �� ��ư
	CUI_Container* m_pLevelUpButton = { nullptr };

	// ���� ����, ���� �� ���� ��ġ
	vector<CUI_Container*> m_pWeaponStat = {};
	vector<CUI_Container*> m_pLegionStat = {};

	// ü��, ���¹̳� �̷� ��
	CUI_Container* m_pAbilInfo = { nullptr };

	CUI_Container* m_pWeaponInfo = { nullptr };
	CUI_Container* m_pLegionInfo = { nullptr };

	// ���� / �ǰ� / ����
	CUI_Container* m_pArmorInfo = { nullptr };

	// �� �Ʒ�
	CUI_Container* m_pBehindButtons = { nullptr };

	_int m_iButtonIndex = { 0 };

	_bool m_bInit = {};

	
	STAT_DESC m_eStat = {};
	_int	  m_iLevel = {};
	_float    m_fErgo = {};
	// ����� �� �� ��
	_float    m_fCurrentErgo = {};

	_int	  m_iCount = {};
	_int      m_iStat[6] = {};

public:
	static CUI_Levelup* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END