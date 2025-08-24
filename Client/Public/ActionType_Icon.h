

#pragma once

#include "Client_Defines.h"
#include "Dynamic_UI.h"



NS_BEGIN(Client)

class CActionType_Icon : public CDynamic_UI
{
private:
	CActionType_Icon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CActionType_Icon(const CActionType_Icon& Prototype);
	virtual ~CActionType_Icon() = default;

public:
	_bool Get_isSelect() { return m_isSelect; }
	void  Set_isSelect(_bool isSelect) { m_isSelect = isSelect; }
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	HRESULT Bind_ShaderResources();
	
	_bool Check_Hover();

private:
	_bool     m_isSelect = { false };
	_bool	  m_isHover = {};
	_wstring  m_strCaption = {};
	CTexture* m_pEffectTextureCom = { nullptr };
	

public:
	static CActionType_Icon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END