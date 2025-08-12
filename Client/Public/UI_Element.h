#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

// 기본적인 텍스처 띄우는 기능만 만들고
// 나머지 기능은 컴포넌트 화 시켜서 박기

// 이거는 그냥 파일로 만들게 하고,
// 나머지 ui 냅둬서 만들게 하기?



NS_BEGIN(Client)

class CUI_Element : public CUIObject
{
public:
	typedef struct tagStaticUIDesc : public CUIObject::UIOBJECT_DESC
	{

		_int iTextureIndex, iPassIndex;
		_int iTextureLevel = { ENUM_CLASS(LEVEL::STATIC) };
		_wstring strTextureTag = {};
		_wstring strShaderTag = {}; 
		
		map<_wstring, class CUI_Component*> m_UIComponents;

		//  
		//
	}UI_ELEMENT_DESC;


protected:
	CUI_Element(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Element(const CUI_Element& Prototype);
	virtual ~CUI_Element() = default;


public:

	_wstring& Get_StrTextureTag() { return m_strTextureTag; }

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
	_wstring		m_strShaderTag = {};
	_wstring		m_strTextureTag = {};
	_int			m_iPassIndex = {};
	_int			m_iTextureIndex = {};
	_int			m_iTextureLevel = { ENUM_CLASS(LEVEL::STATIC) };

	//////

	// 이걸 이제 컴포넌트로 만들어서 박을수 있게

	

	// 사용할 폰트
	_wstring m_strFontTag = {};
	// 채울 내용
	_wstring m_strCaption = {};
	//중앙 기준으로 얼마나 떨어져서 그리게 할건지
	_float2 m_fFontOffset = {};
	// 크기
	_float m_fFontScale = {};
	TEXTALIGN m_eAlignType = { TEXTALIGN::LEFT };

	vector<class CUI_Feature*> m_pUIFeatures = {};
	// 시간으로 프레임 넘길건지
	_bool    m_isUseTime = { false };
	_float m_fDuration = { 0.016f };
	_float m_fElapsedTime = {};


public:
	static CUI_Element* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END