#pragma once

#include "Client_Defines.h"
#include "Dynamic_UI.h"


NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END


NS_BEGIN(Client)

class CUI_Canvus : public CDynamic_UI
{
public:
	typedef struct tagCanvusUIDesc : public CDynamic_UI::DYNAMIC_UI_DESC
	{

		// 
		vector< DYNAMIC_UI_DESC*> pElementDescs;

	}CANVUS_UI_DESC;


protected:
	CUI_Canvus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Canvus(const CUI_Canvus& Prototype);
	virtual ~CUI_Canvus() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();


	virtual HRESULT Ready_Components(const wstring& strTextureTag);
	virtual HRESULT Bind_ShaderResources();

	// 캔버스가 움직이면, 요소들도 같이 움직이게 한다.
	void Update_Elements();

private:
	// 같이 연동시킬
	vector<CUIObject*> m_pUIElements;

	// 캔버스 원점과, 각 요소들의 중점의 거리 비율을 처음에 저장
	vector<_float2> m_ElementRatios;

public:
	static CUI_Canvus* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END