#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"



NS_BEGIN(Client)

class CPanel_Player_RD : public CUI_Container
{

private:
	CPanel_Player_RD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPanel_Player_RD(const CPanel_Player_RD& Prototype);
	virtual ~CPanel_Player_RD() = default;

public:
	virtual void Set_isReverse(_bool isReverse);

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void Update_Info();

private:
	// �⺻ �̹��� + �� �� ui part��

	// ���� ���� �ؽ���
	CUI_Container* m_pWeaponTexture = { nullptr };

	_wstring	   m_strWeaponTextureTag = {};
	

	// ��ų ���� ui
	CUI_Container* m_pSkillType = { nullptr };
	CUI_Container* m_pManaCost = { nullptr };
	CUI_Container* m_pKeyIcon = { nullptr };

public:
	static CPanel_Player_RD* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END