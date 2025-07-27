#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CButton_UI abstract : public CUIObject
{
public:
	typedef struct tagButtonUIDesc : public CUIObject::UIOBJECT_DESC
	{

		_int iTextureIndex = { 0 };
		wstring strTextureTag;
		wstring strCaption;
	}BUTTON_UI_DESC;


	_wstring& Get_StrTextureTag() { return m_strTextureTag; }

public:
	void Set_Click(_bool isClick) { m_isClick = isClick; }

	

protected:
	CButton_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CButton_UI(const CButton_UI& Prototype);
	virtual ~CButton_UI() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();


	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

	_bool   Check_Click();

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

private:

	_wstring m_strTextureTag = {};

	_bool    m_isClick = {false};

public:

	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;

};

NS_END