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

	vector<CItem*>& Get_ViewItems() { return m_ViewItems; }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Add_Item(CItem* pItem, _int iIndex);
	void Use_SelectItem(_bool isActive);
	void DeselectItem();
	void Change_Next_Item();

	_bool Find_Item(const _wstring& strProtoTag);

private:
	vector<CItem*> m_Items;
	vector<CItem*> m_ViewItems;
	CItem*		   m_pSelectItem = { nullptr };
	_int		   m_iSelectIndex = {};

public:
	static CBelt* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END