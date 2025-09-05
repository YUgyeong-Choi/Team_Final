#pragma once

#include "Monster_Base.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CBone;
NS_END

NS_BEGIN(Client)

class CWatchDog final : public CMonster_Base
{
private:
	CWatchDog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWatchDog(const CWatchDog& Prototype);
	virtual ~CWatchDog() = default;

public:
	void Add_AttackCount() { ++m_iAttackCount; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void	Update_State();

	// 데미지를 준다
	virtual void Attack(CGameObject* pOther, COLLIDERTYPE eColliderType);
	// 무기를 통해 데미지를 준다
	virtual void AttackWithWeapon(CGameObject* pOther, COLLIDERTYPE eColliderType);
	// 데미지를 받는다
	virtual void ReceiveDamage(CGameObject* pOther, COLLIDERTYPE eColliderType);

	void Calc_Pos(_float fTimeDelta);

	virtual void Register_Events();

	virtual void Block_Reaction();

	virtual void Reset() override;
private:
	HRESULT Ready_Weapon();

private:
	class CWeapon_Monster* m_pWeapon = { nullptr };

	_int m_iAttackCount = {};
	_int m_iJumpConut = {};

	_float m_fDuration = {};

	// 날아가는 속도
	_float m_fAwaySpeed = { 1.f };

	CPhysXDynamicActor* m_pAttackCollider = {nullptr};
	CBone* m_pHeadBone = { nullptr };

	_bool  m_isInit = { false };

	//int m_iShapeTestState = 0; // 0: Box, 1: Sphere, 2: Capsule
	//PxBoxGeometry m_DebugBox = PxBoxGeometry(1.0f, 2.0f, 1.0f);
	//PxSphereGeometry m_DebugSphere = PxSphereGeometry(1.5f);
	//PxCapsuleGeometry m_DebugCapsule = PxCapsuleGeometry(0.8f, 2.0f);

public:
	static CWatchDog* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;

};

NS_END