#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"

NS_BEGIN(Client)

// 파일에 있는 거만 가져오도록
// 파일을 따로 툴로 만들어서 저장하면 될듯

class CUI_Script : public CUI_Container
{
public:
	typedef struct tagUIContainerDesc : public CUIObject::UIOBJECT_DESC
	{

		_wstring strFilePath;
	}UI_CONTAINER_DESC;
protected:
	CUI_Script(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Script(const CUI_Script& Prototype);
	virtual ~CUI_Script() = default;

public:


	virtual json Serialize();
	virtual void Deserialize(const json& j);

	
	virtual void Set_isReverse(_bool isReverse);

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;


	virtual void Active_Update(_bool isActive);


	void Add_UI_From_Tool(CUIObject* pObj);

protected:
	

protected:


public:
	static CUI_Script* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END