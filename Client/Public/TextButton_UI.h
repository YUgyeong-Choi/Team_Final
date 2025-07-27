#pragma once

#include "Client_Defines.h"
#include "Button_UI.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CTextButton_UI  : public CButton_UI
{

protected:
	CTextButton_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTextButton_UI(const CTextButton_UI& Prototype);
	virtual ~CTextButton_UI() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();


	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

	

private:
	
	// � ���ڸ� ����Ұ���
	_wstring m_strCaption = {};

	// Ŭ���Ǹ� � ������ ����Ұ���
	_float4  m_vColor = {};

public:

	static CTextButton_UI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END