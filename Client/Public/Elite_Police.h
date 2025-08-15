#pragma once

#include "Monster_Base.h"
#include "Client_Defines.h"

NS_BEGIN(Client)


class CElite_Police final : public CMonster_Base
{
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

	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void	Update_State();
};

NS_END
