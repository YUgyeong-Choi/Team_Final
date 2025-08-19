#pragma once

#include "Client_Defines.h"
#include  "LegionArm_Base.h"

// ���� ���¸� ���� �� �ݶ��̴��� �浹 �����ϰ� �ٲ۴�

NS_BEGIN(Engine)

class CPhysXDynamicActor;

NS_END

NS_BEGIN(Client)

class CLegionArm_Steel : public CLegionArm_Base
{
private:
	CLegionArm_Steel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLegionArm_Steel(const CLegionArm_Steel& Prototype);
	virtual ~CLegionArm_Steel() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void Activate();

	void Update_Collider();

	CPhysXDynamicActor* Get_Actor() { return m_pActorCom; }

public:
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) {}
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType) {}
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType) {}

	// Ray�� ���� �浹�� �ϸ� On_Hit�� ȣ���� (HitPos & HitNormal�� ������ �� �� ����)
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType) {}

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) {}
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) {}

private:
	HRESULT Ready_Actor();


private:
	CPhysXDynamicActor* m_pActorCom = { nullptr };

	_bool m_isAttack = { false };
	

public:
	static CLegionArm_Steel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;



};

NS_END