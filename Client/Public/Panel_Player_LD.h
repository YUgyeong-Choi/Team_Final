#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"


NS_BEGIN(Client)

class CPanel_Player_LD : public CUI_Container
{

private:
	CPanel_Player_LD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPanel_Player_LD(const CPanel_Player_LD& Prototype);
	virtual ~CPanel_Player_LD() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;


private:
	CUI_Container* m_pBelt_Up = { nullptr };
	CUI_Container* m_pBelt_Down = { nullptr };

	vector<ITEM_DESC> m_BeltUpDescs;
	vector<ITEM_DESC> m_BeltDownDescs;

public:
	static CPanel_Player_LD* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END