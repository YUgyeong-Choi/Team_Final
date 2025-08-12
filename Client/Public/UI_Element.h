#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

// �⺻���� �ؽ�ó ���� ��ɸ� �����
// ������ ����� ������Ʈ ȭ ���Ѽ� �ڱ�

// �̰Ŵ� �׳� ���Ϸ� ����� �ϰ�,
// ������ ui ���ּ� ����� �ϱ�?



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

	// �̰� ���� ������Ʈ�� ���� ������ �ְ�

	

	// ����� ��Ʈ
	_wstring m_strFontTag = {};
	// ä�� ����
	_wstring m_strCaption = {};
	//�߾� �������� �󸶳� �������� �׸��� �Ұ���
	_float2 m_fFontOffset = {};
	// ũ��
	_float m_fFontScale = {};
	TEXTALIGN m_eAlignType = { TEXTALIGN::LEFT };

	vector<class CUI_Feature*> m_pUIFeatures = {};
	// �ð����� ������ �ѱ����
	_bool    m_isUseTime = { false };
	_float m_fDuration = { 0.016f };
	_float m_fElapsedTime = {};


public:
	static CUI_Element* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END