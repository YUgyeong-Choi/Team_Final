

#pragma once

#include "Client_Defines.h"
#include "TriggerBox.h"

NS_BEGIN(Client)

class CTrigger_Effect : public CTriggerBox
{
public:
	typedef struct tagTriggerEffectDesc : public CTriggerBox::TRIGGERBOX_DESC
	{
		vector<_wstring> strEffectTag;
		vector<_float4> vMakePos = {};
	}TRIGGER_EFFECT_DESC;

protected:
	CTrigger_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTrigger_Effect(const CTrigger_Effect& Prototype);
	virtual ~CTrigger_Effect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
private:
	HRESULT Ready_Components();


	vector<_wstring> m_strEffectTag = {};
	vector<_float4> m_vMakePos = {};

	

public:
	static CTrigger_Effect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END