#pragma once

#include "Client_Defines.h"
#include "Dynamic_UI.h"



NS_BEGIN(Client)

class CIcon_Weapon : public CDynamic_UI
{
private:
	CIcon_Weapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CIcon_Weapon(const CIcon_Weapon& Prototype);
	virtual ~CIcon_Weapon() = default;


public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Update_ICon(_wstring& strTextureTag);

	
	

private:
	

public:
	static CIcon_Weapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END