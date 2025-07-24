

#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CDynamic_UI abstract : public CUIObject
{
public:
	typedef struct tagDynamicUIDesc : public CUIObject::UIOBJECT_DESC
	{
		_int iPassIndex = { 0 };
		_int iTextureIndex = { 0 };
		wstring strTextureTag;
	}DYNAMIC_UI_DESC;


	_wstring& Get_StrTextureTag() { return m_strTextureTag; }

	DYNAMIC_UI_DESC Get_Desc()
	{
		DYNAMIC_UI_DESC eDesc = {};
		eDesc.fSizeX = m_fSizeX;
		eDesc.fSizeY = m_fSizeY;
		eDesc.iPassIndex = m_iPassIndex;
		eDesc.iTextureIndex = m_iTextureIndex;
		eDesc.fX = m_fX;
		eDesc.fY = m_fY;
		eDesc.fOffset = m_fOffset;
		eDesc.strTextureTag = m_strTextureTag;

		return eDesc;

	}



protected:
	CDynamic_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDynamic_UI(const CDynamic_UI& Prototype);
	virtual ~CDynamic_UI() = default;

public:
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	virtual void Update_UI_From_Tool(DYNAMIC_UI_DESC& eDesc) {};

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

private:

	// tool에서 사용할...
	_wstring m_strTextureTag = {};


	// 월드에 그릴 사이즈
	_float m_fScale = {};
	_int   m_iPassIndex = {};
	_int   m_iTextureIndex = {};


public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;

};

NS_END