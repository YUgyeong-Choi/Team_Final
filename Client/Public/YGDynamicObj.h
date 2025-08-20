#pragma once

// 플레이어와 dynamic 충돌

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CPhysXDynamicActor;
NS_END

NS_BEGIN(Client)

class CYGDynamicObj final : public CGameObject
{
public:
	typedef struct tagDynamicDataDesc : public CGameObject::GAMEOBJECT_DESC
	{
		wstring colliderType;
	}DYNAMICDATA_DESC;
private:
	CYGDynamicObj(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CYGDynamicObj(const CYGDynamicObj& Prototype);
	virtual ~CYGDynamicObj() = default;
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
private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CPhysXDynamicActor*  m_pPhysXActorCom = { nullptr };

	wstring m_colliderType = {};
private:
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Components();
	HRESULT Ready_Collider();
public:
	static CYGDynamicObj* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END