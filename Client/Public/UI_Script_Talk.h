#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"

NS_BEGIN(Client)

// 파일에 있는 거만 가져오도록
// 파일을 따로 툴로 만들어서 저장하면 될듯

class CUI_Script_Talk : public CUI_Container
{
public:
	
protected:
	CUI_Script_Talk(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Script_Talk(const CUI_Script_Talk& Prototype);
	virtual ~CUI_Script_Talk() = default;

public:




public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void Update_Script(const string strName, const string strText);

	virtual void Active_Update(_bool isActive);


private:
	CUI_Container* m_pButtons = { nullptr };
	CUI_Container* m_pText = { nullptr };

public:
	static CUI_Script_Talk* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END