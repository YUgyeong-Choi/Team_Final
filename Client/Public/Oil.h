#pragma once
#include "Projectile.h"

NS_BEGIN(Client)
class COil final :public CProjectile
{
private:
	COil(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	COil(const COil& Prototype);
	virtual ~COil() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Explode_Oil();

private:
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;
	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;

	virtual HRESULT Ready_Components() override;
	virtual HRESULT Ready_Effect() override;

private:
	_float m_fDamge = 10.f;
	_bool m_bCanSpread = false; // 기름이 퍼질 수 있는지 여부
	_bool m_bIsSpreaded = false; // 기름이 퍼졌는지 여부
	PxBoxGeometry m_SpreadOilShape = PxBoxGeometry(1.0f,0.8f, 1.0f);
	class CPlayer* m_pPlayer{ nullptr };
	class CGameObject* m_pFuoco{ nullptr };
public:
	static COil* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

