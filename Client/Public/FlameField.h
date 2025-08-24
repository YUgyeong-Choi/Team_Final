#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
NS_BEGIN(Engine)
class CPhysXDynamicActor;
NS_END

NS_BEGIN(Client)
class CFlameField final : public CGameObject
{
public:
	typedef struct tagFlameFieldDesc :public GAMEOBJECT_DESC
	{
		_float3 vPos = { 0.f, 0.f, 0.f }; // 불꽃 필드의 위치
		_float fExpandTime = 5.f; // 불꽃이 확장되는 시간
		_float fInitialRadius = 0.5f; // 불꽃의 초기 반지름
		_float fExpandRadius = 3.f; // 불꽃이 최종 확장되는 반지름
	} FLAMEFIELD_DESC;
private:
	CFlameField(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFlameField(const CFlameField& Prototype);
	virtual ~CFlameField() = default;


public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;


private:
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;
	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;

	HRESULT Ready_Components();
	HRESULT Ready_Actor();

private:
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };
	PxBoxGeometry m_ModifyFlame = PxBoxGeometry(0.5f, 0.5f, 0.5f);
	_bool  m_bIsExpanded = false; // 불꽃이 확장되었는지 여부
	_float m_fExpandRadius = 3.f; // 불꽃이 확장되는 반지름
	_float m_fInitialRadius = 0.5f; // 불꽃의 초기 반지름
	_float m_fExpandTime = 0.f; // 불꽃이 확장까지 걸리는 시간
	_float m_fExpandElapsedTime = 0.f; // 불꽃이 확장된 시간
	_float m_fRemainTime = 2.f;

public:
	static CFlameField* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END

