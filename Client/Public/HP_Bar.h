#pragma once

#include "Client_Defines.h"
#include "Dynamic_UI.h"




NS_BEGIN(Client)

class CHP_Bar : public CDynamic_UI
{
private:
	CHP_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHP_Bar(const CHP_Bar& Prototype);
	virtual ~CHP_Bar() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	_int m_iCurrentHP = {};
	_int m_iMaxHP = {};
	_float m_fRatio = {};

public:
	static CHP_Bar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END