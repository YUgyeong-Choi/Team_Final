

#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"

NS_BEGIN(Client)

// 파일에 있는 거만 가져오도록
// 파일을 따로 툴로 만들어서 저장하면 될듯

class CUI_Pickup_Item : public CUI_Container
{
public:

protected:
	CUI_Pickup_Item(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Pickup_Item(const CUI_Pickup_Item& Prototype);
	virtual ~CUI_Pickup_Item() = default;

public:




public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void Active_Update(_bool isActive);
	
	void Update_Description(string itemName, _int itemType);

protected:

private:
	CUI_Container* m_pDescription = { nullptr };



public:
	static CUI_Pickup_Item* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END