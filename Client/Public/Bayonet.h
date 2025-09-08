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
class CBayonet : public CWeapon
{
	/* [ 모든 무기객체의 부모클래스입니다. ] */


public:
	typedef struct tagBayonetDesc : public CWeapon::WEAPON_DESC
	{
	}BAYONET_DESC;

protected:
	CBayonet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBayonet(const CBayonet& Prototype);
	virtual ~CBayonet() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;

	void 	Update_Collider();
	CPhysXDynamicActor* Get_PhysXActor() { return m_pPhysXActorCom; }

	virtual void SetisAttack(_bool isAttack);

	virtual void Calc_Durability(_float fDelta);

	virtual void Reset();

	virtual void Set_WeaponTrail_Active(_bool bActive, TRAILTYPE eType = TRAIL_DEFAULT) override;

protected: /* [ Setup 함수 ] */
	HRESULT Ready_Components();
	HRESULT Ready_Actor();
	HRESULT Ready_Effect();

protected: /* [ 충돌 시 공통으로 실행 ] */
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

private:
	HRESULT Create_SlashEffect(CGameObject* pOther, COLLIDERTYPE eColliderType);
	HRESULT Create_AttackEffect(CGameObject* pOther, COLLIDERTYPE eColliderType);

private:
	CPhysXDynamicActor*			m_pPhysXActorCom = { nullptr };

	//class CSwordTrailEffect*	m_pHitTrailEffect = { nullptr }; // 타격 시에만 나오는 트레일 이펙트 전용(피)
	//class CSwordTrailEffect*	m_pSkillTrailEffect = { nullptr }; // 스킬 시에 나오는 트레일 이펙트 전용

	_bool						m_bHitEffect = { false };
	_float3						m_vEndSocketPrevPos = {};	// 타격 시 방향 구하는 용도로 씀
	_float3						m_vEndSocketCurPos = {};
	const _float4x4*			m_pWeaponEndMatrix = {nullptr};
	class CGameObject*			m_pLastHitObject = { nullptr };
	COLLIDERTYPE				m_eLastHitColType = { COLLIDERTYPE::PLAYER };
	_float3						m_vSlashDir = {};

public:
	static CBayonet* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END