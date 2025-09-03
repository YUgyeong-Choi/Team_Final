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
		_float3 vPos = { 0.f, 0.f, 0.f }; // �Ҳ� �ʵ��� ��ġ
		_float fExpandTime = 5.f; // �Ҳ��� Ȯ��Ǵ� �ð�
		_float fInitialRadius = 0.5f; // �Ҳ��� �ʱ� ������
		_float fExpandRadius = 3.f; // �Ҳ��� ���� Ȯ��Ǵ� ������
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
	/* Ray�� ���� �浹(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;

	HRESULT Effect_FlameField(const _fvector& vSpawnPos);
	HRESULT Ready_Components();
	HRESULT Ready_Actor();
	// 12�������� ���̽��� ���� ��ų ���� Ȯ��
	void Check_SpawnEffectDistance();
private:
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };
	PxBoxGeometry m_ModifyFlame = PxBoxGeometry(0.5f, 0.5f, 0.5f);
	_bool  m_bEnterPlayer = false;
	_bool  m_bIsExpanded = false; // �Ҳ��� Ȯ��Ǿ����� ����
	_float m_fExpandRadius = 3.f; // �Ҳ��� Ȯ��Ǵ� ������
	_float m_fInitialRadius = 0.5f; // �Ҳ��� �ʱ� ������
	_float m_fExpandTime = 0.f; // �Ҳ��� Ȯ����� �ɸ��� �ð�
	_float m_fExpandElapsedTime = 0.f; // �Ҳ��� Ȯ��� �ð�
	_float m_fRemainTime = 2.f;
	_float m_fDamageInterval = 0.25f; // ������ ����
	_float m_fDamgeElapsedTime = 0.f; // ���� ������ �ð�
	_vector m_vBegningRayPos = XMVectorZero();
	vector<_float> m_SpawnEffectDistanceList; // ����Ʈ�� ������ �Ÿ� ����Ʈ
	vector<_float> m_LastSpawnDist; // ����Ʈ�� ������ �Ÿ� ����Ʈ
	vector<_float> m_MergeDist; // 72���⿡�� ���̸� ���� ���� �Ÿ��� ���
	CPlayer* m_pPlayer = nullptr;
public:
	static CFlameField* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END

