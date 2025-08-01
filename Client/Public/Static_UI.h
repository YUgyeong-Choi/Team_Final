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

		_int iTextureIndex, iPassIndex;
		_int iTextureLevel = { ENUM_CLASS(LEVEL::STATIC) };
		_wstring strTextureTag = {};

	}STATIC_UI_DESC;


	_wstring& Get_StrTextureTag() { return m_strTextureTag; }

	STATIC_UI_DESC Get_Desc()
	{
		STATIC_UI_DESC eDesc = {};
		eDesc.fSizeX = m_fSizeX;
		eDesc.fSizeY = m_fSizeY;
		eDesc.iPassIndex = m_iPassIndex;
		eDesc.iTextureIndex = m_iTextureIndex;
		eDesc.fX = m_fX;
		eDesc.fY = m_fY;
		eDesc.fOffset = m_fOffset;
		eDesc.strTextureTag = m_strTextureTag;
		eDesc.vColor = m_vColor;
		eDesc.fAlpha = m_fCurrentAlpha;
		eDesc.fRotation = m_fRotation;

		return eDesc;

	}

	virtual json Serialize();
	virtual void Deserialize(const json& j);

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

	void Update_UI_From_Tool(STATIC_UI_DESC& eDesc);
	virtual void Update_UI_From_Tool(void* pArg) override;

	HRESULT Ready_Components(const wstring& strTextureTag);

	virtual HRESULT Ready_Components_File(const wstring& strTextureTag) override;

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

private:

	_wstring		m_strTextureTag = {};
	_int			m_iPassIndex = {};
	_int			m_iTextureIndex = {};
	_int			m_iTextureLevel = { ENUM_CLASS(LEVEL::STATIC)};
	

private:
	

public:
	static CStatic_UI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END