#pragma once
#include "Unit.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
class CPhysXDynamicActor;
NS_END

NS_BEGIN(Client)

class CMonster_Test : public CUnit
{
public:
	enum class STATE_MONSTER {IDLE, WALK, RUN, DEAD, HIT, GROGGY, FATAL, ATTACK, END};
	enum class MONSTER_DIR {F, B, L,R, END};

protected:
	CMonster_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster_Test(const CMonster_Test& Prototype);
	virtual ~CMonster_Test() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

private: /* [ Setup 함수 ] */
	HRESULT Ready_Components();
	HRESULT Ready_Actor();

	void	Add_PartObject();
	void	RootMotionActive(_float fTimeDelta);
	void	Update_State();

	void    LoadAnimDataFromJson();

	void    Update_Collider();


private:

	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };

	CGameObject*	m_pTarget = { nullptr };
	STATE_MONSTER	m_eCurrentState = {};

	vector<class CPartObject*> m_PartObjects; // hp 바, 필요하면 무기 넣기

	// 특정 상태에만 계속 바라보게 해야?
	_bool			m_isLookAt = {};
	
	// 행동 결정을 위한 변수들
	_int			m_iStamina = {};
	_int			m_iGroggyThreshold = {};
	_bool			m_isCanGroggy = {};
	_bool			m_isCanFatal = {};
	


public:
	static CMonster_Test* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END

