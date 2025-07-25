#pragma once

// Ä¸½¶°ú Ä¸½¶ Ãæµ¹ 

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CPhysXKinematicActor;
NS_END

NS_BEGIN(Client)

class CYGCapsule final : public CGameObject
{
private:
	CYGCapsule(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CYGCapsule(const CYGCapsule& Prototype);
	virtual ~CYGCapsule() = default;
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	HRESULT Bind_ShaderResources();

	virtual void On_CollisionEnter(CGameObject* pOther) override;
	virtual void On_CollisionStay(CGameObject* pOther) override;
	virtual void On_CollisionExit(CGameObject* pOther) override;

	virtual void On_Hit(CGameObject* pOther) override;
private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CPhysXKinematicActor*  m_pPhysXActorCom = { nullptr };
private:
	HRESULT Ready_Components();
	HRESULT Ready_Collider();
	void Update_ColliderPos();
	void Ray();

	PxVec3 m_vHitPos = {};
	_bool m_bRayHit = false;
public:
	static CYGCapsule* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END