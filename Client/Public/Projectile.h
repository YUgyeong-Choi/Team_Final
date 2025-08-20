#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
NS_BEGIN(Engine)
class CModel;
class CShader;
class CPhysXDynamicActor;
NS_END

NS_BEGIN(Client)
class CProjectile : public CGameObject
{
public:
	typedef struct tagProjectileDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_bool  bUseTimeTrigger = false; // �ð� Ʈ���� ��� ����
		_bool  bUseDistTrigger = false; // �Ÿ� Ʈ���� ��� ����
		_float fStartTime = 0.f; // ���� �ð� (Ʈ���� ��� ��)
		_float  fGravityOnDist = 10.f; // �߷� �ۿ� ���� �Ÿ�
		_float3 vDir = { 0.f, 0.f, 0.f };
		_float3 vPos = { 0.f, 0.f, 0.f };
		_float fSpeed = 0.f; // �ʱ� �ӵ�
		_float fLifeTime = 5.f; // ���� �ð�
		_float fRadius = 0.5f; // �浹 �ݰ�
	} PROJECTILE_DESC;

protected:
	CProjectile(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CProjectile(const CProjectile& Prototype);
	virtual ~CProjectile() = default;


public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

protected:
	_float3 Get_WorldPosFromActor() const;
	virtual HRESULT Ready_Components();
	HRESULT Ready_Actor();
	HRESULT Bind_Shader();


protected:
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;
	/* Ray�� ���� �浹(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;

protected:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };
	_bool m_bGravity = false;
	_bool m_bUseTimeTrigger = false;
	_bool m_bUseDistTrigger = false;
	_float m_fSpeed = 0.f;
	_float m_fGravityOnDist = 10.f; // �߷� �ۿ� ���� �Ÿ�
	_float m_fLifeTime = 5.f; // ���� �ð�
	_float m_fElapsedTime = 0.f; // ��� �ð�
	_float m_fDistance = 0.f; // ���ư� �̵� �Ÿ�
	_float m_fStartTime = 0.f; // �ð����� Ʈ���� ���ٸ�
	_float m_fRadius = 0.5f; // �浹 �ݰ�
	_vector m_vDirection = XMVectorZero();
	_vector m_vStartPos = XMVectorZero(); // ���� ��ġ
public:
	static CProjectile* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

