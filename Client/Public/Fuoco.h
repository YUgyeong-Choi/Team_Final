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
        RUN_F = 100035,
        TURN_R = 100012,
        TURN_L = 100013
    };

    enum EBossAttackPattern :_int
    {
        BAP_NONE = 0,
        SlamCombo = 1,
        Uppercut = 2,
        SwingAtk = 4,
        SlamFury = 5,
        FootAtk = 6,
        P2_FlameFiled = 7,
        SlamAtk = 8,
        StrikeFury = 9,
        P2_FireOil = 10,
        P2_FireBall = 11,
        P2_FireBall_B = 13,
    };

	enum class EFuocoState{
        IDLE,WALK,RUN,TURN,ATTACK,GROGGY,DEAD,NONE};

	enum class EMoveDirection	{
        FRONT, RIGHT, BACK, LEFT};

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
    void UpdateMovement(_float fDistance,_float fTimeDelta);
    void UpdateAttackPattern(_float fDistance,_float fTimeDelta);
    void UpdateStateByNodeID(_uint iNodeID);

	_bool  IsTargetInFront(_float fDectedAngle = 60.f) const;
	_bool UpdateTurnDuringAttack(_float fTimeDelta);
	_float Get_DistanceToPlayer() const;
	_vector GetTargetDirection() const;

    // 공견 패턴
    void SetupAttackByType(EBossAttackPattern ePattern);
    void SetTurnTimeDuringAttack(_float fTime,_float fAddtivRotSpeed = 1.f)
    {
        if (m_fTurnTimeDuringAttack != 0.f)
            return;
		m_fTurnTimeDuringAttack = fTime;
		m_fAddtiveRotSpeed = fAddtivRotSpeed;
    }

    // 이동
    void ApplyRootMotionDelta(_float fTimeDelta);
    void UpdateNormalMove(_float fTimeDelta);


	_bool IsValidAttackType(EBossAttackPattern ePattern) const
	{
		return ePattern == EBossAttackPattern::SwingAtk ||
			ePattern == EBossAttackPattern::SlamFury ||
			ePattern == EBossAttackPattern::SlamCombo ||
			ePattern == EBossAttackPattern::FootAtk ||
			ePattern == EBossAttackPattern::SlamAtk ||
			ePattern == EBossAttackPattern::Uppercut ||
			ePattern == EBossAttackPattern::StrikeFury ||
            ePattern == EBossAttackPattern::P2_FlameFiled ||
			ePattern == EBossAttackPattern::P2_FireOil ||
			ePattern == EBossAttackPattern::P2_FireBall ||
			ePattern == EBossAttackPattern::P2_FireBall_B;
	}

    virtual void Register_Events() override;

	void Ready_AttackPatternWeightForPhase1();
	void Ready_AttackPatternWeightForPhase2();

	EBossAttackPattern GetRandomAttackPattern(_float fDistance);
	void UpdatePatternWeight(EBossAttackPattern ePattern);

    _bool CheckConditionFlameFiled();
 
	_float CalculateCurrentHpRatio() const
	{
		return m_fHP / m_fMaxHP;
	}

	void ChosePatternWeightByDistance(_float fDistance);

#ifdef _DEBUG
    function<void()> PatterDebugFunc = [this]() {    cout << "=== Attack Pattern Weights ===" << endl;
    for (const auto& [pattern, weight] : m_PatternWeightMap)
    {
        cout << "Pattern " << static_cast<_int>(pattern)
            << ": Weight=" << weight
            << ", Consecutive=" << m_PatternCountMap[pattern]
            << ", Previous=" << static_cast<_int>(m_ePrevAttackPattern) <<
                ", Current=" << static_cast<_int>(m_eCurAttackPattern)
                << endl;
    }
    cout << "===============================" << endl;
        };
#endif
private:
	// 컴포넌트 관련
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorComForArm = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorComForFoot = { nullptr };
    CNavigation* m_pNaviCom = { nullptr };
	CBone* m_pFistBone{ nullptr };
	CBone* m_pFootBone{ nullptr };
	
    // 상태 관련
    EFuocoState m_eCurrentState = EFuocoState::NONE;
    _bool m_bIsFirstAttack{ true }; // 컷씬하고 돌진 처리
    _bool m_bIsPhase2{ false };
    _bool m_bStartPhase2 = false;
    _bool m_bUsedFlameFiledOnLowHp = false;

    // 체력
	_float m_fHP = 100.f;
	_float m_fMaxHP = 100.f;

    // 이동 관련

	_vector m_vRotationXDir = XMVectorZero(); // X축 회전 방향
	_float m_fRotationTimeForX = 0.f; // X축 회전 시간

    _vector  m_PrevWorldDelta = XMVectorZero();
    _vector  m_PrevWorldRotation = XMVectorZero();
    _float   m_fRotSmoothSpeed = 8.0f;
    _float   m_fSmoothSpeed = 8.0f;
    _float   m_fSmoothThreshold = 0.1f;
    _float m_fWalkSpeed = 3.f;
	_float m_fRunSpeed = 5.f;

	_float m_fChangeMoveDirCooldown = 0.f; // 이동 방향 변경 쿨타임
	_float m_fAddtiveRotSpeed = 1.f; // 회전 속도 추가값
    _float m_fTurnTimeDuringAttack = 0.f;


    // 공격 관련
    _int m_iPatternLimit = 2;
    _float m_fBasePatternWeight = 100.f;
    _float m_fMinWeight = 10.f;
    _float m_fMaxWeight = 150.f;
    _float m_fWeightDecreaseRate = 0.3f;
	_float m_fWeightIncreaseRate = 0.1f;
    _float m_fAttackCooldown = 0.f; // 공격 쿨타임
    _float m_fAttckDleay = 2.5f;

    EBossAttackPattern m_eCurAttackPattern = EBossAttackPattern::BAP_NONE;
    EBossAttackPattern m_ePrevAttackPattern = EBossAttackPattern::BAP_NONE;
    unordered_map<EBossAttackPattern, _float> m_PatternWeightMap;
    unordered_map<EBossAttackPattern, _float> m_PatternWeighForDisttMap;
    unordered_map<EBossAttackPattern, _int> m_PatternCountMap;// 연속 횟수

	vector<EBossAttackPattern> m_vecCloseAttackPatterns = {
		EBossAttackPattern::SlamCombo, EBossAttackPattern::Uppercut,
		EBossAttackPattern::SwingAtk, EBossAttackPattern::SlamFury,
		EBossAttackPattern::FootAtk,EBossAttackPattern::StrikeFury
	};

    vector<EBossAttackPattern> m_vecMiddleAttackPatterns = {
    EBossAttackPattern::StrikeFury,  EBossAttackPattern::SwingAtk,
    EBossAttackPattern::SlamFury, EBossAttackPattern::P2_FireOil,
     EBossAttackPattern::P2_FlameFiled,
    EBossAttackPattern::P2_FireBall, EBossAttackPattern::P2_FireBall_B
    };

    vector<EBossAttackPattern> m_vecFarAttackPatterns = {
         EBossAttackPattern::P2_FlameFiled,
    EBossAttackPattern::P2_FireOil,EBossAttackPattern::StrikeFury,
    EBossAttackPattern::P2_FireBall, EBossAttackPattern::P2_FireBall_B
    };

    // 상수
    const _float CHASING_DISTANCE = 3.1f;
	const _float ATTACK_DISTANCE_CLOSE = 1.f;
    const _float ATTACK_DISTANCE_MIDDLE = 7.f;
	const _float ATTACK_DISTANCE_FAR = 15.f;
    const _float MINIMUM_TURN_ANGLE = 35.f;
public:
	static CFuoco* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END

