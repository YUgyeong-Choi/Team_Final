#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"

NS_BEGIN(Client)

// 파일에 있는 거만 가져오도록
// 파일을 따로 툴로 만들어서 저장하면 될듯

class CUI_Button_Script : public CUI_Container
{
public:

protected:
	CUI_Button_Script(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Button_Script(const CUI_Button_Script& Prototype);
	virtual ~CUI_Button_Script() = default;

public:
	_float2 Get_ButtonPos();

	_bool Get_isSelect() { return m_isSelect; }
	void Set_isSelect(_bool isSelect);

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	//
	void Update_Script(const string strText);
	void Check_Select();
	_bool Check_MousePos();

	void Update_Position(_float fX, _float fY);

private:
	_bool m_isSelect = {};




public:
	static CUI_Button_Script* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END