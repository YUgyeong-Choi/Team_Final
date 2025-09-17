#pragma once

#include "Client_Defines.h"
#include "TriggerBox.h"

NS_BEGIN(Client)

class CTriggerRain : public CTriggerBox
{
public:
	typedef struct tagTriggerNoMeshDesc : public CTriggerBox::TRIGGERBOX_DESC
	{
		_bool bBossDoor = false;
	}TRIGGERNOMESH_DESC;

protected:
	CTriggerRain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTriggerRain(const CTriggerRain& Prototype);
	virtual ~CTriggerRain() = default;

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

	_bool m_bRainActive = true;
	_bool m_bBossDoor = false;

public:
	static CTriggerRain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END