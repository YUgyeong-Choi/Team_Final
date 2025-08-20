#pragma once


#include "Client_Defines.h"
#include "Dynamic_UI.h"


NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CDynamic_Text_UI : public CDynamic_UI
{
private:
	CDynamic_Text_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDynamic_Text_UI(const CDynamic_Text_UI& Prototype);
	virtual ~CDynamic_Text_UI() = default;


public:
	virtual json Serialize();
	virtual void Deserialize(const json& j);

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();


	virtual void Update_UI_From_Tool(void* pArg) override;

	void Set_Caption(_wstring strCaption);


private:
	// 사용할 폰트
	_wstring m_strFontTag = {};
	// 채울 내용
	_wstring m_strCaption = {};

	TEXTALIGN m_eAlignType = { TEXTALIGN::LEFT };

public:
	static CDynamic_Text_UI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END