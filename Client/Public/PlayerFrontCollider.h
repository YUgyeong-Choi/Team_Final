#pragma once
#include "GameObject.h"
#include "Client_Defines.h"


NS_BEGIN(Client)
class CPlayerFrontCollider : public CGameObject
{
	/* [ ��� ���ⰴü�� �θ�Ŭ�����Դϴ�. ] */


public:
	typedef struct tagWeaponDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float3 		InitPos = { 0.f, 0.f, 0.f };
		_float3 		InitScale = { 1.f, 1.f, 1.f };
		CGameObject*	pOwner = { nullptr };
	}WEAPON_DESC;

protected:
	CPlayerFrontCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlayerFrontCollider(const CPlayerFrontCollider& Prototype);
	virtual ~CPlayerFrontCollider() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;


protected: /* [ Setup �Լ� ] */
	HRESULT Ready_Components();


protected: /* [ �浹 �� �������� ���� ] */
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

public: /* [ ������ (Unit) �� �����´�. ] */
	class CUnit* Get_Owner() const { return m_pOwner; }
	void Clear_Owner() { m_pOwner = nullptr; }

protected:				/* [ �⺻ �Ӽ� ] */
	_float3				m_InitPos = {};
	_float3				m_InitScale = {};

protected:
	const _tchar* m_szName = { nullptr };
	class CUnit* m_pOwner = { nullptr };

public:
	static CPlayerFrontCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END