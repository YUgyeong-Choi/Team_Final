#pragma once
#include "Unit.h"
#include "Client_Defines.h"
#include "PhysX_ControllerReport.h"

NS_BEGIN(Engine)
class CBone;
class CNavigation;
class CPhysXController;
class CPhysXDynamicActor;
class CAnimController;
NS_END

NS_BEGIN(Client)
class CFuoco : public CUnit
{
    // 주요 상태들의 NodeID
    enum class BossStateID : _uint
    {
        IDLE = 1,
        ATK_SLAM_COMBO_START = 2,
        ATK_SWING_START = 3,
        ATK_SWING_END = 6,
        ATK_SLAM_COMBO_LOOP = 11,
        ATK_SLAM_COMBO_LEFT_END = 13,
        ATK_SLAM_COMBO_END = 14,
        ATK_SLAM_COMBO_RIGHT_END = 15,
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

    enum EBossAttackPattern :_int
    {
        BAP_NONE = 0,
        SlamCombo = 1,
		Uppercut = 2,
        SwingAtk = 4,
        FootAtk = 6,
        FlameFiled = 7,
        SlamAtk = 8,
        StrikeFury = 9,
        P2_FireOil = 10,
        P2_FireBall = 11,
        P2_FireBall_B = 13,
    };

	enum class EFuocoState
	{
		IDLE,
		WALK,
		RUN,
        TURN,
		ATTACK,
        GROGGY,
        DEAD,
		NONE
	};

	enum class EMoveDirection
	{
		FRONT,
		RIGHT,
		BACK,
		LEFT
	};

	enum class EAttackDirection
	{
		FRONT,
		LEFT,
		RIGHT
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
    void ApplyRootMotionDelta(_float fTimeDelta);
    void UpdateNormalMove(_float fTimeDelta);
	_bool UpdateTurnDuringAttack(_float fTimeDelta);
    void SetupAttackByType(EBossAttackPattern ePattern);
    void SetTurnTimeDuringAttack(_float fTime)
    {
        if (m_fTurnTimeDuringAttack != 0.f)
            return;
		m_fTurnTimeDuringAttack = fTime;
    }

	_bool IsValidAttackType(EBossAttackPattern ePattern) const
	{
		return ePattern == EBossAttackPattern::SwingAtk ||
			ePattern == EBossAttackPattern::FootAtk ||
			ePattern == EBossAttackPattern::SlamAtk ||
			ePattern == EBossAttackPattern::Uppercut ||
			ePattern == EBossAttackPattern::StrikeFury ||
			ePattern == EBossAttackPattern::P2_FireOil ||
			ePattern == EBossAttackPattern::P2_FireBall ||
			ePattern == EBossAttackPattern::P2_FireBall_B;
	}

private:
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorComForArm = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorComForFoot = { nullptr };
    CNavigation* m_pNaviCom = { nullptr };
	CBone* m_pFistBone{ nullptr };
	CBone* m_pFootBone{ nullptr };
	EFuocoState m_eCurrentState = EFuocoState::NONE;
    _bool m_bIsFirstAttack{ true }; // 컷씬하고 돌진 처리
    _bool m_bIsPhase2{ false };
    _bool m_bStartPhase2 = false;
	_float m_fAttackCooldown = 0.f; // 공격 쿨타임

    _vector  m_PrevWorldDelta = XMVectorZero();
    _vector  m_PrevWorldRotation = XMVectorZero();
    _float   m_fRotSmoothSpeed = 8.0f;
    _float   m_fSmoothSpeed = 8.0f;
    _float   m_fSmoothThreshold = 0.1f;

    _float m_fTurnTimeDuringAttack = 0.f;
    const _float CHASING_DISTANCE = 3.f;
	const _float ATTACK_DISTANCE = 7.f; // 이건 나중에 원거리 공격을 좀 처리할 때 다시 사용
    const _float MINIMUM_TURN_ANGLE = 35.f;

public:
	static CFuoco* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END

