

#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"

#include "UI_Text.h"


NS_BEGIN(Client)

class CPanel_Player_Arm : public CUI_Container
{

private:
	CPanel_Player_Arm(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPanel_Player_Arm(const CPanel_Player_Arm& Prototype);
	virtual ~CPanel_Player_Arm() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:



public:
	static CPanel_Player_Arm* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END