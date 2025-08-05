#pragma once

#include "Client_Defines.h"
#include "Item.h"
#include "GameObject.h"

NS_BEGIN(Client)

class CBelt final :public CGameObject
{
private:
	CBelt(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBelt(const CBelt& Prototype);
	virtual ~CBelt() = default;

public:
	CItem* Get_Current_Item() { return m_Items[m_iSelectIndex]; }
	vector<CItem*>& Get_Items() { return m_Items; }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Add_Item(CItem* pItem, _int iIndex);
	void Use_SelectItem();
	void DeselectItem();
	void Change_Next_Item();

private:
	vector<CItem*> m_Items;
	CItem*		   m_pSelectItem = { nullptr };
	_int		   m_iSelectIndex = { 0 };

public:
	static CBelt* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END