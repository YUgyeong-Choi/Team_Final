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
	/* [ 모든 무기객체의 부모클래스입니다. ] */
	typedef struct eTagMonsterWeaponDesc : public WEAPON_DESC {
		_float4 vAxis;
		_float  fRotationDegree; // 라디안 변환 해주기
		_float4 vLocalOffset = { 0.0f,0.f,0.f,1.f };
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

	void	Collider_FilterOff();
	void	Collider_ShapeOff();

	virtual void SetisAttack(_bool isAttack);

	void Gravity_On();

	_float4& GetLocalOffset() { return m_vLocalOffset; }
	void SetLocalOffset(_float4 vOffset) { m_vLocalOffset = vOffset; }

	_float3 GetAxis3() { return _float3(m_vAxis.x, m_vAxis.y, m_vAxis.z); }
	_float  GetRotationDegree() { return m_fRotationDegree; }
	void    SetAxis(_float4 vAxis) { m_vAxis = vAxis; }
	void    SetRotationDegree(_float fDegree) { m_fRotationDegree = fDegree; }
	_float3 GetLocalRot3() { return _float3(m_vLocalRot.x, m_vLocalRot.y, m_vLocalRot.z); }
	void    SetLocalRot(const _float3& vRot) { m_vLocalRot = _float4(vRot.x, vRot.y, vRot.z, 0.f); }

protected: /* [ Setup 함수 ] */
	HRESULT Ready_Components();
	HRESULT Ready_Actor();


protected: /* [ 충돌 시 공통으로 실행 ] */
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

private:
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };
	_float4	m_vLocalOffset = {};
	_float4 m_vLocalRot = {};
	_float4 m_vAxis = {};
	_float  m_fRotationDegree = {};

	PxVec3 m_physxExtent = {};
public:
	static CWeapon_Monster* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END