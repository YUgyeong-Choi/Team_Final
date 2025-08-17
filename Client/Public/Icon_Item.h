#pragma once

#include "Client_Defines.h"
#include "Dynamic_UI.h"



NS_BEGIN(Client)

class CIcon_Item : public CDynamic_UI
{
private:
	CIcon_Item(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CIcon_Item(const CIcon_Item& Prototype);
	virtual ~CIcon_Item() = default;

public:
	_bool Get_isSelect() { return m_isSelect; }
	void  Set_isSelect(_bool isSelect) { m_isSelect = isSelect; }
	void  Set_isInput(_bool isInput) { m_isInput = isInput; }
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	// 벨트 칸에 맞게끔 조절하기?
	void Update_ICon(ITEM_DESC* pDesc);

	HRESULT Bind_ShaderResources();
	HRESULT Ready_Component(_wstring& strTextureTag);

private:
	_bool     m_isSelect = { false };
	_bool     m_isInput = { false };
	_bool     m_isUsable = {true};
	_wstring  m_strCaption = {};
	CTexture* m_pItemTextureCom = { nullptr };
	CTexture* m_pEffectTextureCom = { nullptr };
	CTexture* m_pInputTextureCom = { nullptr };
	_int	  m_iItemIndex = { 0 };

	_float	  m_fDuration = {0.3f};
	_float    m_fInputTime = {};

public:
	static CIcon_Item* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END