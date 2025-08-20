#pragma once
#include "Weapon.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
class CPhysXDynamicActor;
NS_END

NS_BEGIN(Client)
class CWeapon_Monster : public CWeapon
{
public:
	/* [ ��� ���ⰴü�� �θ�Ŭ�����Դϴ�. ] */
	typedef struct eTagMonsterWeaponDesc : public WEAPON_DESC {
		_float4 vAxis;
		_float  fRotationDegree; // ���� ��ȯ ���ֱ�
		_float4 vLocalOffset; // ������ ������ �� ������
		PxVec3 vPhsyxExtent;
		
	}MONSTER_WEAPON_DESC;


protected:
	CWeapon_Monster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWeapon_Monster(const CWeapon_Monster& Prototype);
	virtual ~CWeapon_Monster() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;


	void 	Update_Collider();
	CPhysXDynamicActor* Get_PhysXActor() { return m_pPhysXActorCom; }

	void	Collider_Off();


protected: /* [ Setup �Լ� ] */
	HRESULT Ready_Components();
	HRESULT Ready_Actor();


protected: /* [ �浹 �� �������� ���� ] */
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);

	/* Ray�� ���� �浹(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

private:
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };
	_float4	m_vLocalOffset = {};

	PxVec3 m_physxExtent = {};

public:
	static CWeapon_Monster* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END