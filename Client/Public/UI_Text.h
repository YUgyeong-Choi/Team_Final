
#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CUI_Text final : public CUIObject
{
public:
	typedef struct tagTextUIDesc : public CUIObject::UIOBJECT_DESC
	{
		_wstring strFontTag = {};
		// ä�� ����
		_wstring strCaption = {};
		//�߾� �������� �󸶳� �������� �׸��� �Ұ���
		_float2 fFontOffset = {};
		// ũ��
		_float fFontScale = {};
		// �߾� ���� ���� ����
		_bool   isCenter = {};
	}TEXT_UI_DESC;


	TEXT_UI_DESC Get_Desc()
	{
		TEXT_UI_DESC eDesc = {};

		eDesc.strCaption = m_strCaption;
		eDesc.fFontOffset = m_fFontOffset;
		eDesc.fFontScale = m_fFontScale;
		eDesc.isCenter = m_isCenter;
		eDesc.fRotation = m_fRotation;
		eDesc.vColor = m_vColor;
		eDesc.fX = m_fX;
		eDesc.fY = m_fY;
		

		return eDesc;

	}

	virtual json Serialize();
	virtual void Deserialize(const json& j);

private:
	CUI_Text(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Text(const CUI_Text& Prototype);
	virtual ~CUI_Text() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Update_UI_From_Tool(TEXT_UI_DESC& eDesc);
	virtual void Update_UI_From_Tool(void* pArg) override;

	virtual HRESULT Ready_Components_File(const wstring& strTextureTag) override;

private:

private:
	// ����� ��Ʈ
	_wstring m_strFontTag = {};
	// ä�� ����
	_wstring m_strCaption = {};
	//�߾� �������� �󸶳� �������� �׸��� �Ұ���
	_float2 m_fFontOffset = {};
	// ũ��
	_float m_fFontScale = {};
	// �߾� ���� ���� ����
	_bool   m_isCenter = {};

	
public:
	static CUI_Text* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END