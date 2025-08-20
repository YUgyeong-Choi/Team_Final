#pragma once

// Ä¸½¶°ú Ä¸½¶ Ãæµ¹ 

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CPhysXStaticActor;
NS_END

NS_BEGIN(Client)

class CYGTrrigerWithoutModel final : public CGameObject
{
private:
	CYGTrrigerWithoutModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CYGTrrigerWithoutModel(const CYGTrrigerWithoutModel& Prototype);
	virtual ~CYGTrrigerWithoutModel() = default;
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;

	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
private:
	CPhysXStaticActor*  m_pPhysXActorCom = { nullptr };
private:
	HRESULT Ready_Components();
	HRESULT Ready_Collider();
public:
	static CYGTrrigerWithoutModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END