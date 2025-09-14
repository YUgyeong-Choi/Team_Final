#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"
#include "UI_Button.h"

NS_BEGIN(Client)

class CUI_Letter : public CUI_Container
{
public:
	typedef struct tagGuideStruct : public CUI_Container::UI_CONTAINER_DESC
	{

		vector<_wstring> partPaths;
	}UI_LETTER_DESC;

protected:
	CUI_Letter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Letter(const CUI_Letter& Prototype);
	virtual ~CUI_Letter() = default;



public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void Check_Button();

	virtual void Active_Update(_bool isActive) override;

private:
	CUI_Container* m_pBackGround = { nullptr };

	vector<CUI_Container*> m_Explainations = {};

	_int    m_iSize = {};
	_int	m_iIndex = {};


public:
	static CUI_Letter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END


