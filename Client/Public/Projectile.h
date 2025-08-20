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
		_bool  bUseTimeTrigger = false; // 시간 트리거 사용 여부
		_bool  bUseDistTrigger = false; // 거리 트리거 사용 여부
		_float fStartTime = 0.f; // 시작 시간 (트리거 사용 시)
		_float  fGravityOnDist = 10.f; // 중력 작용 시작 거리
		_float3 vDir = { 0.f, 0.f, 0.f };
		_float3 vPos = { 0.f, 0.f, 0.f };
		_float fSpeed = 0.f; // 초기 속도
		_float fLifeTime = 5.f; // 생존 시간
		_float fRadius = 0.5f; // 충돌 반경
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
	/* Ray로 인항 충돌(HitPos& HitNormal) */
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
	_float m_fGravityOnDist = 10.f; // 중력 작용 시작 거리
	_float m_fLifeTime = 5.f; // 생존 시간
	_float m_fElapsedTime = 0.f; // 경과 시간
	_float m_fDistance = 0.f; // 날아간 이동 거리
	_float m_fStartTime = 0.f; // 시간으로 트리거 쓴다면
	_float m_fRadius = 0.5f; // 충돌 반경
	_vector m_vDirection = XMVectorZero();
	_vector m_vStartPos = XMVectorZero(); // 시작 위치
public:
	static CProjectile* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

