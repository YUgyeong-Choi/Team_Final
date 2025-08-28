#pragma once

#include "BossUnit.h"
#include "Client_Defines.h"

NS_BEGIN(Client)
class CElite_Police final : public CBossUnit
{
    enum class EliteMonsterStateID : _uint
    {
        Spawn_Loop = 100002,
        Spawn_End = 100003,
        Idle = 100004,
        Walk_F = 100005,
        Walk_L = 100006,
        Walk_R = 100007,
        Walk_B = 100008,
        Run_F = 100009,
        Turn_L = 100012,
        Turn_R = 100013,
        Groggy_Start = 100031,
        Groggy_Loop = 100032,
        Groggy_End = 100033,
        Fatal_Hit_Start = 100038,
        Fatal_Hit_Ing = 100039,
        Fatal_Hit_End = 100040,
        Death_B = 100045,
        Atk_Combo1 = 100047,
        Atk_Combo2 = 100048,
        Atk_Combo3 = 100049,
        Atk_Combo2_2 = 100050,
        Atk_Combo3_2 = 100052,
        Atk_Combo3_3 = 100054,
        Atk_Combo4 = 100056,
        Paralyzation_Start = 100058,
        Paralyzation_Loop = 100059,
        Paralyzation_End = 100060
    };

    enum EEliteAttackPattern : _int
    {
        AP_NONE = 0,
		COMBO1 = 1,
		COMBO2 = 2,
		COMBO3 = 3,
		COMBO4 = 4
    };
private:
	CElite_Police(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CElite_Police(const CElite_Police& Prototype);
	virtual ~CElite_Police() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;


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
    HRESULT Ready_Weapon();

    virtual void Update_Collider() override;
    virtual void UpdateAttackPattern(_float fDistance, _float fTimeDelta) override;
    virtual void UpdateStateByNodeID(_uint iNodeID) override;
    virtual void UpdateSpecificBehavior() override;
    virtual void EnableColliders(_bool bEnable) override;

    virtual void Ready_EffectNames() override;
    virtual void ProcessingEffects(const _wstring& stEffectTag) override;
    virtual HRESULT EffectSpawn_Active(_int iPattern, _bool bActive, _bool bIsOnce = true) override;
    virtual HRESULT Spawn_Effect();

    virtual HRESULT Ready_Effect();

    virtual void Register_Events() override;


    EEliteAttackPattern GetRandomAttackPattern(_float fDistance);
    void UpdatePatternWeight(EEliteAttackPattern ePattern);

    _bool CheckConditionFlameField();

    void ChosePatternWeightByDistance(_float fDistance);
private:
	class CWeapon_Monster* m_pWeapon = { nullptr };

    // 공격 관련
    _int   m_iPatternLimit = 1;
    _int   m_iFireBallComboCount = 0;
    _float m_fBasePatternWeight = 100.f;
    _float m_fMinWeight = 30.f;
    _float m_fMaxWeight = 250.f;
    _float m_fWeightDecreaseRate = 0.6f;
    _float m_fWeightIncreaseRate = 0.4f;

    EEliteAttackPattern m_eCurAttackPattern = EEliteAttackPattern::AP_NONE;
    EEliteAttackPattern m_ePrevAttackPattern = EEliteAttackPattern::AP_NONE;
    unordered_map<EEliteAttackPattern, _float> m_PatternWeightMap;
    unordered_map<EEliteAttackPattern, _float> m_PatternWeighForDisttMap;
    unordered_map<EEliteAttackPattern, _int> m_PatternCountMap;// 패턴 연속 횟수

    vector<EEliteAttackPattern> m_vecCloseAttackPatterns = {
		COMBO1, COMBO2, COMBO3, COMBO4
    };

    vector<EEliteAttackPattern> m_vecMiddleAttackPatterns = {
		COMBO2
    };

    const _float ATTACK_DISTANCE_CLOSE = 0.f;
    const _float ATTACK_DISTANCE_MIDDLE = 7.f;
public:
	static CElite_Police* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};


NS_END
