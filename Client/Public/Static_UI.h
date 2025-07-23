#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CStatic_UI final : public CUIObject
{
public:
	typedef struct tagStaticUIDesc : public CUIObject::UIOBJECT_DESC
	{
		_int iPassIndex = { 0 };
		_int iTextureIndex = { 0 };
		wstring strTextureTag;
	}STATIC_UI_DESC;

private:
	CStatic_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStatic_UI(const CStatic_UI& Prototype);
	virtual ~CStatic_UI() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

private:


	// 월드에 그릴 사이즈
	_float m_fScale = {};
	_int   m_iPassIndex = {};
	_int   m_iTextureIndex = {};

private:
	HRESULT Ready_Components(const wstring strTextureTag);

public:
	static CStatic_UI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END