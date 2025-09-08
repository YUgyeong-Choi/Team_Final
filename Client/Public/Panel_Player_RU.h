#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"

#include "UI_Text.h"


NS_BEGIN(Client)

class CPanel_Player_RU : public CUI_Container
{

private:
	CPanel_Player_RU(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPanel_Player_RU(const CPanel_Player_RU& Prototype);
	virtual ~CPanel_Player_RU() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	// ȹ���� ������, ������ ȹ���ϸ� ���� �ð����� ���� ������ �� ���̰Բ�
	_float m_fErgo = {};
	_float m_fRenderTime = {};

	// ���� ���ؼ� bar�� �ѱ�� ������?
	_float m_fMaxErgo = {};


public:
	static CPanel_Player_RU* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END