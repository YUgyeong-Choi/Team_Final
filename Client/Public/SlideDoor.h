#pragma once

#include "Client_Defines.h"
#include "DefaultDoor.h"

NS_BEGIN(Client)

class CSlideDoor : public CDefaultDoor
{
protected:
	CSlideDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSlideDoor(const CSlideDoor& Prototype);
	virtual ~CSlideDoor() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	void Play_Sound();
protected:
	HRESULT Ready_Components(void* pArg);
public:
	static CSlideDoor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END