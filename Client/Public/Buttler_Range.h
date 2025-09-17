#pragma once

#include "Monster_Base.h"
#include "Client_Defines.h"

NS_BEGIN(Client)

class CButtler_Range final : public CMonster_Base
{
private:
	CButtler_Range(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CButtler_Range(const CButtler_Range& Prototype);
	virtual ~CButtler_Range() = default;

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
	virtual void Register_SoundEvent() override;

	virtual void Start_Fatal_Reaction();

	virtual void Reset() override;

	virtual void RayCast(CPhysXActor* actor);

	virtual void PlayDetectSound();

private:
	HRESULT Ready_Weapon();
	virtual HRESULT Ready_Sound();

private:
	class CWeapon_Monster* m_pWeapon = { nullptr };

	_int m_iAttackCount = {};


	_float m_fDuration = {};

	// 날아가는 속도
	_float m_fAwaySpeed = { 1.f };

	// 맞을 때 밀리는 속도
	_float m_fHitSpeed = { 1.f };


public:
	static CButtler_Range* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;

};

NS_END