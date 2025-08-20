#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CPhysXDynamicActor;
NS_END

NS_BEGIN(Client)

class CYGObject final : public CGameObject
{
private:
	CYGObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CYGObject(const CYGObject& Prototype);
	virtual ~CYGObject() = default;
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
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CPhysXDynamicActor*  m_pPhysXActorCom = { nullptr };
	CPhysXDynamicActor*  m_pPhysXActorWeaponCom = { nullptr };
private:
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Components();
	HRESULT Ready_Collider();
	void Update_ColliderPos();
	void Ray();
	void ChangeColliderSize();
	PxVec3 m_vRayHitPos = {};
	_bool m_bRayHit = false;

	_bool m_bAttack = false;
public:
	static CYGObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END