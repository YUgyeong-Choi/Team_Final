
#pragma once

#include "Client_Defines.h"
#include "Dynamic_UI.h"


NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END


NS_BEGIN(Client)

class CUI_Button : public CDynamic_UI
{
public:
	typedef struct tagButtonUIDesc : public CDynamic_UI::DYNAMIC_UI_DESC
	{
		// 
		_wstring strCaption;

		_float2  fPadding;
		_float   fFontSize = {1.f};
	}BUTTON_UI_DESC;

public:
	void Set_isMouseHover(_bool isMouseHover) { m_isMouseHover = isMouseHover; }
	void Set_isHighlight(_bool isHighlight) { m_isHighlight = isHighlight; }

	virtual json Serialize();
	virtual void Deserialize(const json& j);

protected:
	CUI_Button(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Button(const CUI_Button& Prototype);
	virtual ~CUI_Button() = default;

public:
	BUTTON_UI_DESC Get_Desc();

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();


	HRESULT Ready_Components(const wstring& strTextureTag);
	HRESULT Ready_Components_File(const wstring& strTextureTag);
	HRESULT Bind_ShaderResources();

	// 나중에 feature로 빼기?
	_bool Check_MousePos();
	_bool Check_MouseHover();
	_bool Check_Click();

	virtual void Update_UI_From_Tool(void* pArg) override;

private:

	CTexture* m_pHoverTextureCom = { nullptr };
	CTexture* m_pHighlightTextureCom = { nullptr };

	_bool   m_isHasTexture = {false};

	// 마우스가 위에 있는지, 클릭이 됬는지
	_bool   m_isMouseHover = {false};
	_bool   m_isHighlight = {false};

	// ui에 필요한 글자 만들고, 버튼 중앙에 띄운다.
	_wstring m_strCaption = {};

	_float2 m_fPadding = {};
	_float  m_fFontSize = {1.f};

public:
	static CUI_Button* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END