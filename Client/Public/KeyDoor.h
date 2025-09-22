#pragma once
/* [ 야외 맵으로 갈 때 key 필요한 문들 ] */
#include "Client_Defines.h"
#include "DefaultDoor.h"

NS_BEGIN(Client)

class CKeyDoor : public CDefaultDoor
{
protected:
	CKeyDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKeyDoor(const CKeyDoor& Prototype);
	virtual ~CKeyDoor() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	void OpenDoor();
	void Move_Player(_float fTimeDelta);
protected:
	HRESULT Ready_Components(void* pArg);

	HRESULT LoadFromJson();

public:
	static CKeyDoor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END