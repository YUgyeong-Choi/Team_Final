#pragma once
#include "Unit.h"
#include "Client_Defines.h"
#include "PhysX_ControllerReport.h"

NS_BEGIN(Engine)
class CPhysXController;
class CPhysXDynamicActor;
class CAnimController;
class CBone;
NS_END

NS_BEGIN(Client)
class CFuoco : public CUnit
{
    // 주요 상태들의 NodeID
    enum class BossStateID : _uint
    {
        IDLE = 1,
        SKILL1_START = 2,
        ATK_SWING_START = 3,
        ATK_SWING_END = 6,
        SKILL1_LOOP = 11,
        SKILL1_LEFT_END = 13,
        SKILL1_END = 14,
        SKILL1_RIGHT_END = 15,
        ATK_FOOT = 18,
        ATK_SLAM_FURY_START = 19,
        ATK_SLAM_FURY = 21,
        ATK_SLAM_FURY_END = 22,
        ATK_STRIKE_FURY = 27,
        ATK_SLAM = 30,
        ATK_UPPERCUT_START = 33,
        ATK_UPPERCUT_END = 34,
        ATK_UPPERCUT_FRONT = 39,
        CUTSCENE = 42,
        ATK_SWING_R = 45,
        ATK_SWING_L_COM1 = 46,
        ATK_SWING_R_COM1 = 47,
        ATK_SWING_R_COM2 = 48,
        ATK_SWING_L_COM2 = 49,
        ATK_SWING_SEQ = 50,
        ATK_SWING_SEQ2 = 51,
        ATK_SWING_SEQ3 = 52,
        ATK_SWING_SEQ_RESET = 53,
        ATK_SWING_SEQ_RESET2 = 54,
        P2_ATK_FIRE_EAT = 69,
        P2_ATK_FLAME_FILED = 70,
        P2_ATK_FIRE_BALL_B = 74,
        P2_ATK_FLAME_L = 77,
        P2_ATK_FLAME_R = 78,
        P2_ATK_FLAME_START = 79,
        P2_ATK_FLAME_END = 82,
        P2_ATK_FIRE_BALL_START = 87,
        P2_ATK_FIRE_BALL_L = 88,
        P2_ATK_FIRE_BALL_R = 89,
        P2_ATK_FIRE_BALL_END = 90,
        P2_ATK_FIRE_BALL_F = 99,
        SPECIAL_DIE = 102,
        GROGGY_START = 103,
        GROGGY_LOOP = 104,
        GROGGY_END = 105,
        DEAD_F = 110,
        DEAD_B = 111,
        P2_ATK_FIRE_OIL = 114,
        WALK_F = 117,
        WALK_B = 121,
        WALK_R = 122,
        WALK_L = 123,
        TURN_R = 100012,
        TURN_L = 100013
    };

	enum class EFuocoState
	{
		IDLE,
		WALK,
		RUN,
		ATTACK,
        GROGGY,
        DEAD,
		NONE
	};

	enum class EDirection
	{
		FRONT,
		RIGHT,
		BACK,
		LEFT
	};

private:
	CFuoco(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFuoco(const CFuoco& Prototype);
	virtual ~CFuoco() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;

private:
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

private:
	HRESULT LoadFromJson();
	HRESULT Ready_Components();
	HRESULT Ready_Actor();

	void Update_Collider();
	void UpdateBossState(_float fTimeDelta);
    void UpdateMove(_float fTimeDelta);
    void UpdateAttackPattern(_float fDistance,_float fTimeDelta);

	_float Get_DistanceToPlayer() const;
	_bool  IsTargetInFront() const;
	_vector GetTargetDirection() const;


private:
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorComForArm = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorComForFoot = { nullptr };

	CBone* m_pFistBone{ nullptr };
	CBone* m_pFootBone{ nullptr };
	EFuocoState m_eCurrentState = EFuocoState::NONE;
    _bool m_bIsFirstAttack{ false }; // 컷씬하고 돌진 처리
	_float m_fAttackCooldown = 0.f; // 공격 쿨타임

    const _float CHASING_DISTANCE = 50.f;
	const _float ATTACK_DISTANCE = 7.f;

public:
	static CFuoco* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END

