#pragma once
#include "Projectile.h"

NS_BEGIN(Client)
class CFireBall final :public CProjectile
{
private:
	CFireBall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFireBall(const CFireBall& Prototype);
	virtual ~CFireBall() = default;
public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

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
	vector<CGameObject*> m_CollisionObjects; // 데미지를 입은 오브젝트 리스트
public:
	static CFireBall* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

