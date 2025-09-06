#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"

NS_BEGIN(Client)

// 파일에 있는 거만 가져오도록
// 파일을 따로 툴로 만들어서 저장하면 될듯

class CUI_SelectLocation : public CUI_Container
{
public:
	typedef struct tagLocationUIStruct : public CUI_Container::UI_CONTAINER_DESC
	{

		CGameObject* pTarget;

	}UI_SELECT_LOCATION_DESC;
protected:
	CUI_SelectLocation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_SelectLocation(const CUI_SelectLocation& Prototype);
	virtual ~CUI_SelectLocation() = default;

public:




public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Button();


protected:

	CGameObject* m_pTarget = { nullptr };

	CUI_Container* m_pLocationImg = { nullptr };
	CUI_Container* m_pBehindButtons = { nullptr };
	vector <class CUI_Button* > m_pButtons = {};
	class CDynamic_UI* m_pSelectUI = { nullptr };

public:
	static CUI_SelectLocation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END