#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"
#include "UI_Button.h"

NS_BEGIN(Client)

class CUI_Guide  : public CUI_Container
{
public:
	typedef struct tagGuideStruct : public CUI_Container::UI_CONTAINER_DESC
	{
	
		vector<_wstring> partPaths;
		CGameObject* pTrigger;
	}UI_GUIDE_DESC;
	
protected:
	CUI_Guide(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Guide(const CUI_Guide& Prototype);
	virtual ~CUI_Guide() = default;

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


	void Check_Button();

	void Click_Interaction();

	virtual void Active_Update(_bool isActive) override;

private:
	CUI_Container* m_pBackGround = {nullptr};
	CUI_Container* m_pButtonContainer = {nullptr};

	// 닫기 오른쪽 왼쪽 순서로 들어감
	vector<CUI_Button*> m_Buttons = {};
	vector<CUI_Container*> m_Explainations = {};

	_int    m_iSize = {};
	_int	m_iIndex = {};
	

	CGameObject* m_pTrigger = { nullptr }; // 트리거 박스 
public:
	static CUI_Guide* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END


