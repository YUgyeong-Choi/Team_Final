#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"


NS_BEGIN(Client)

class CUI_Popup : public CUI_Container
{
public:
	typedef struct tagGuideStruct : public CUI_Container::UI_CONTAINER_DESC
	{

		vector<_wstring> partPaths;

	}UI_GUIDE_DESC;

protected:
	CUI_Popup(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Popup(const CUI_Popup& Prototype);
	virtual ~CUI_Popup() = default;

public:

	virtual json Serialize();
	virtual void Deserialize(const json& j);


public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;


	void Set_String(_int iTriggerType);
	

private:
	




public:
	static CUI_Popup* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END


