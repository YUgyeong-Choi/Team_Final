#pragma once
#include "BossUnit.h"
#include "Client_Defines.h"
#include "PhysX_ControllerReport.h"

NS_BEGIN(Client)
class CFuoco final : public CBossUnit
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
        ATK_SWING_SEQ_START = 100050,
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
        TURN_L = 100013,
        PARALYZATION_START = 100043,
        PARALYZATION_LOOP = 100044,
        PARALYZATION_END = 100045,
        FATAL_START = 100076,
		FATAL_LOOP = 100077,
		FATAL_END = 100078
    };

    enum EBossAttackPattern : _int
    {
        BAP_NONE = 0,
        SlamCombo = 1,
        Uppercut = 2,
        SwingAtkSeq = 3,
        SwingAtk = 4,
        SlamFury = 5,
        FootAtk = 6,
        P2_FlameField = 7,
        SlamAtk = 8,
        StrikeFury = 9,
        P2_FireOil = 10,
        P2_FireBall = 11,
        P2_FireFlame = 12,
        P2_FireBall_B = 13,
    };

    enum class ProjectileType {
        FireBall,
        Oil
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
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

private:
	virtual HRESULT Ready_Components(void* pArg) override;
	virtual HRESULT Ready_Actor() override;
	virtual void Ready_BoneInformation() override;

	virtual void Update_Collider() override;
    virtual void UpdateAttackPattern(_float fDistance,_float fTimeDelta) override;
    virtual void UpdateStateByNodeID(_uint iNodeID) override;
    virtual void UpdateSpecificBehavior() override;

    // 공견 패턴
    void SetupAttackByType(EBossAttackPattern ePattern);
    void SetTurnTimeDuringAttack(_float fTime,_float fAddtivRotSpeed = 1.f)
    {
        if (m_fTurnTimeDuringAttack != 0.f)
            return;
		m_fTurnTimeDuringAttack = fTime;
		m_fAddtiveRotSpeed = fAddtivRotSpeed;
    }

    virtual void Register_Events() override;

	virtual void Ready_AttackPatternWeightForPhase1() override;
	virtual void Ready_AttackPatternWeightForPhase2() override;

	EBossAttackPattern GetRandomAttackPattern(_float fDistance);
	void UpdatePatternWeight(EBossAttackPattern ePattern);

    _bool CheckConditionFlameField();

	void ChosePatternWeightByDistance(_float fDistance);
	void FireProjectile(ProjectileType type, _float fSpeed = 10.f);
	void FlamethrowerAttack(_float fConeAngle = 10.f, _int iRayCount = 7, _float fDistance = 15.f);
	void SpawnFlameField();

    virtual HRESULT EffectSpawn_Active(_int iPattern, _bool bActive);
    HRESULT Effect_FlameField();
    virtual HRESULT Ready_Effect();

#ifdef _DEBUG
    function<void()> PatterDebugFunc = [this]() {    cout << "=== Attack Pattern Weights ===" << endl;
    for (const auto& [pattern, weight] : m_PatternWeightMap)
    {
       /* cout << "Pattern " << static_cast<_int>(pattern)
            << ": Weight=" << weight
            << ", Consecutive=" << m_PatternCountMap[pattern]
            <<"\n"
            << ", Previous=" << static_cast<_int>(m_ePrevAttackPattern) <<"\n"<<
                ", Current=" << static_cast<_int>(m_eCurAttackPattern)
                << endl;*/
    }
    cout << "===============================" << endl;
        };
#endif
private:
	// 컴포넌트 관련
	CPhysXDynamicActor* m_pPhysXActorComForArm = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorComForFoot = { nullptr };
    CNavigation* m_pNaviCom = { nullptr };
	CBone* m_pFistBone{ nullptr };
	CBone* m_pFootBone{ nullptr };
	CBone* m_pLeftBone{ nullptr };
	CBone* m_pCannonBone{ nullptr };
	
    // 상태 관련
    _bool m_bUsedFlameFiledOnLowHp = false;


    // 공격 관련
    _int   m_iPatternLimit = 3;
    _int   m_iFireBallComboCount = 0;
    _float m_fBasePatternWeight = 100.f;
    _float m_fMinWeight = 50.f;
    _float m_fMaxWeight = 150.f;
    _float m_fWeightDecreaseRate = 0.15f;
	_float m_fWeightIncreaseRate = 0.12f;
    //_float m_fAttackCooldown = 0.f; // 공격 쿨타임
    //_float m_fAttckDleay = 4.f;
    _float m_fFireFlameDuration = 0.f;
    array<_float3, 4> m_vOilSpawnPos =
    {
        _float3{ 0.f, 0.f, 0.f }, // 중앙
        _float3{ -2.f, 0.f, -2.f }, // 좌하
        _float3{ 2.f, 0.f, -2.f }, // 우하
        _float3{ 0.f, 0.f, -4.f } // 앞
    };
    EBossAttackPattern m_eCurAttackPattern = EBossAttackPattern::BAP_NONE;
    EBossAttackPattern m_ePrevAttackPattern = EBossAttackPattern::BAP_NONE;
    unordered_map<EBossAttackPattern, _float> m_PatternWeightMap;
    unordered_map<EBossAttackPattern, _float> m_PatternWeighForDisttMap;
    unordered_map<EBossAttackPattern, _int> m_PatternCountMap;// 패턴 연속 횟수

	vector<EBossAttackPattern> m_vecCloseAttackPatterns = {
	  SlamCombo, Uppercut, SlamAtk, SwingAtk, 
      SlamFury, P2_FireBall,
	  SwingAtkSeq,FootAtk,
      StrikeFury,  P2_FireOil,
      P2_FireBall_B, P2_FireFlame,
	};

    vector<EBossAttackPattern> m_vecMiddleAttackPatterns = {
     SlamCombo,
     StrikeFury, SwingAtk,
     SlamFury, P2_FireOil
    ,SwingAtkSeq, P2_FireBall,
     P2_FireBall_B, P2_FireFlame

    };

    vector<EBossAttackPattern> m_vecFarAttackPatterns = {
     P2_FireOil,StrikeFury,
     P2_FireBall, P2_FireBall_B,
     P2_FireFlame
    };

    // 상수
	const _float ATTACK_DISTANCE_CLOSE = 0.f;
    const _float ATTACK_DISTANCE_MIDDLE = 10.f;
	const _float ATTACK_DISTANCE_FAR = 15.f;
    const _int LIMIT_FIREBALL_COMBO_COUNT = 3;
public:
	static CFuoco* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END

