#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
NS_END

NS_BEGIN(Client)
class CWeapon : public CGameObject
{
	/* [ ��� ���ⰴü�� �θ�Ŭ�����Դϴ�. ] */


public:
	typedef struct tagWeaponDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar*	szMeshID;
		LEVEL			eLevelID;
		_int			iRender = 0;
		_float3 		InitPos = { 0.f, 0.f, 0.f };
		_float3 		InitScale = { 1.f, 1.f, 1.f };
	}WEAPON_DESC;

protected:
	CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWeapon(const CWeapon& Prototype);
	virtual ~CWeapon() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;


protected: /* [ Setup �Լ� ] */
	HRESULT Bind_Shader();
	HRESULT Ready_Components();


protected: /* [ �浹 �� �������� ���� ] */
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	/* Ray�� ���� �浹(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);


protected:				/* [ �⺻ �Ӽ� ] */
	_float				m_bDamage = { 10.f };
	_bool				m_isActive = { true };
	_float				m_fSpeedPerSec = 5.f;
	_float				m_fRotationPerSec = XMConvertToRadians(90.f);
	_float3				m_InitPos = {};
	_float3				m_InitScale = {};
	_int				m_iRender = {};

protected: 				/* [ �⺻ Ÿ�� ] */
	const _tchar*		m_szName = { nullptr };
	const _tchar*		m_szMeshID = { nullptr };
	LEVEL				m_eLevelID = { LEVEL::END };

protected:				/* [ �׸��� ���� ] */
	SHADOW				m_eShadow = SHADOW::SHADOW_END;

protected:              /* [ ������Ʈ ] */
	CModel*				m_pModelCom = { nullptr };
	CShader*			m_pShaderCom = { nullptr };
	CAnimator*			m_pAnimator = { nullptr };

protected:				/* [ ����ĳ��Ʈ ���� ] */
	PxVec3				m_vRayHitPos = {};
	_bool				m_bRayHit = {};

public:
	static CWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END