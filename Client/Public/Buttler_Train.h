#pragma once

#include "Monster_Base.h"
#include "Client_Defines.h"

NS_BEGIN(Client)

class CButtler_Train final : public CMonster_Base
{
private:
	CButtler_Train(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CButtler_Train(const CButtler_Train& Prototype);
	virtual ~CButtler_Train() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void	Update_State();

	void Calc_Pos(_float fTimeDelta);

private:
	HRESULT Ready_Weapon(); 

private:
	class CWeapon_Monster* m_pWeapon = { nullptr };

	_int m_iAttackCount = {};
	

public:
	static CButtler_Train* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;

};

NS_END