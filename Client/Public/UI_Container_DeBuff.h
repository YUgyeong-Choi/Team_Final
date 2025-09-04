#pragma once

#include "Client_Defines.h"
#include "UI_Container.h"


// ����� �̹���, ��, �����̻� �̸� ��
// ���⼭ �޾Ƽ� ������Ʈ �Ѵ�

NS_BEGIN(Client)

class CUI_Container_DeBuff : public CUI_Container
{

private:
	CUI_Container_DeBuff(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Container_DeBuff(const CUI_Container_DeBuff& Prototype);
	virtual ~CUI_Container_DeBuff() = default;

public:

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void Update_Info();

private:
	_float m_fRatio = {};
	_int   m_iType = {};

	

public:
	static CUI_Container_DeBuff* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END