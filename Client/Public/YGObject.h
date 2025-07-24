#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CPhysXStaticActor;
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

	HRESULT Bind_ShaderResources();

	virtual void On_CollisionEnter(CGameObject* pOther) override;
	virtual void On_CollisionStay(CGameObject* pOther) override;
	virtual void On_CollisionExit(CGameObject* pOther) override;

	virtual void On_Hit(_int iDamage, _float3 HitPos) override;
private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CPhysXStaticActor*  m_pPhysXActorCom = { nullptr };
private:
	HRESULT Ready_Components();
	HRESULT Ready_Collider();
public:
	static CYGObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END