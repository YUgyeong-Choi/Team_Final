#pragma once

// 메쉬 dynamic 이펙트 용

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CPhysXDynamicActor;
NS_END

NS_BEGIN(Client)

class CYGDynamicGib final : public CGameObject
{
public:
	typedef struct tagGibDataDesc : public CGameObject::GAMEOBJECT_DESC
	{
		wstring protoTag;
	}GIBDATA_DESC;
private:
	CYGDynamicGib(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CYGDynamicGib(const CYGDynamicGib& Prototype);
	virtual ~CYGDynamicGib() = default;
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;

	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CPhysXDynamicActor*  m_pPhysXActorCom = { nullptr };

	_float m_fLifeTime = {};
	_bool m_isNextFrame = false;
private:
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Components(GIBDATA_DESC* desc);
	HRESULT Ready_Collider();
public:
	static CYGDynamicGib* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END