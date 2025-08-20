#pragma once

// Ä¸½¶°ú Ä¸½¶ Ãæµ¹ 

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CPhysXStaticActor;
NS_END

NS_BEGIN(Client)

class CYGConvexMesh final : public CGameObject
{
private:
	CYGConvexMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CYGConvexMesh(const CYGConvexMesh& Prototype);
	virtual ~CYGConvexMesh() = default;
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
	CPhysXStaticActor*  m_pPhysXActorCom = { nullptr };
private:
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Components();
	HRESULT Ready_Collider();
	void ChangeColliderSize();
public:
	static CYGConvexMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END