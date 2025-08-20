#pragma once

#include "Monster_Base.h"
#include "Client_Defines.h"

NS_BEGIN(Client)


class CElite_Police final : public CMonster_Base
{
public:
	typedef enum ATTACKTYPE {ATTACK_A, ATTACK_B, ATTACK_C, ATTACK_D, ATTACK_END}ATTACKTYPE;

private:
	CElite_Police(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CElite_Police(const CElite_Police& Prototype);
	virtual ~CElite_Police() = default;

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

	_int Update_AttackType();

private: /* [ Setup 함수 ] */

	HRESULT Ready_Weapon();



private:

	
	class CWeapon_Monster* m_pWeapon = { nullptr };

	







public:
	static CElite_Police* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};


NS_END
