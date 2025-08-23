#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"

NS_BEGIN(Client)

// 파일에 있는 거만 가져오도록
// 파일을 따로 툴로 만들어서 저장하면 될듯

class CUI_Script_Text : public CUI_Container
{
public:
	typedef struct tagUIContainerDesc : public CUIObject::UIOBJECT_DESC
	{

		
	}UI_CONTAINER_DESC;
protected:
	CUI_Script_Text(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Script_Text(const CUI_Script_Text& Prototype);
	virtual ~CUI_Script_Text() = default;

public:

	
	

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void Update_Script(const string strText);
	

protected:
	

protected:


public:
	static CUI_Script_Text* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END