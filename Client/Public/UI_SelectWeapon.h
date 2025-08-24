#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"

NS_BEGIN(Client)

// 파일에 있는 거만 가져오도록
// 파일을 따로 툴로 만들어서 저장하면 될듯

class CUI_SelectWeapon : public CUI_Container
{
public:
	typedef struct tagSelectWeaponUIDesc : public UI_CONTAINER_DESC
	{
		CGameObject* pTarget;
		
	}SELECT_WEAPON_UI_DESC;
protected:
	CUI_SelectWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_SelectWeapon(const CUI_SelectWeapon& Prototype);
	virtual ~CUI_SelectWeapon() = default;

public:




public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void Update_Script(const string strName, const string strText, _bool isAuto);

	virtual void Active_Update(_bool isActive);


private:
	CUI_Container* m_pButtons = { nullptr };
	CUI_Container* m_pIcons = { nullptr };
	CUI_Container* m_pBackgrounds = { nullptr };
	CUI_Container* m_pButton_Select = { nullptr };
	CUI_Container* m_pText = { nullptr };
	CUI_Container* m_pSelectEffect = { nullptr };

	CGameObject* m_pTarget = { nullptr };

	_bool		m_isSelectWeapon = { false };
	_int		m_iSelectIndex = {};
	


public:
	static CUI_SelectWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END