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
	
	// 어떤 글자를 출력할건지
	_wstring m_strCaption = {};

	// 클릭되면 어떤 색으로 출력할건지
	_float4  m_vColor = {};

public:

	static CTextButton_UI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END