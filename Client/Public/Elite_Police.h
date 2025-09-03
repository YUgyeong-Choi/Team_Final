#pragma once

#include "EliteUnit.h"
#include "Client_Defines.h"

NS_BEGIN(Client)
class CElite_Police final : public CEliteUnit
{
    enum class EliteMonsterStateID : _uint
    {
        Spawn_Loop = 100002,
        Spawn_End = 100003,
        Idle = 100004,
        Walk_F = 100005,
        Walk_L = 100096,
        Walk_R = 100097,
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
        Atk_Combo5 = 100090,
        Paralyzation_Start = 100058,
        Paralyzation_Loop = 100059,
        Paralyzation_End = 100060
    };

    enum EPoliceAttackPattern : _int
    {
        AP_NONE = 0,
		COMBO1 = 1,
		COMBO2 = 2,
		COMBO3 = 3,
		COMBO4 = 4,
		COMBO5 = 5
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

	virtual void Update_Collider() override;
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

	virtual void HandleMovementDecision(_float fDistance, _float fTimeDelta) override;
    virtual void UpdateAttackPattern(_float fDistance, _float fTimeDelta) override;
    virtual void UpdateStateByNodeID(_uint iNodeID) override;
    virtual void UpdateSpecificBehavior(_float fTimeDelta) override;
    virtual void EnableColliders(_bool bEnable) override;

    virtual void Ready_EffectNames() override;
    virtual void ProcessingEffects(const _wstring& stEffectTag) override;
    virtual HRESULT EffectSpawn_Active(_int iPattern, _bool bActive, _bool bIsOnce = true) override;
    virtual HRESULT Spawn_Effect();

    virtual HRESULT Ready_Effect();

    virtual void Register_Events() override;
	virtual void Reset() override;

    virtual _int GetRandomAttackPattern(_float fDistance) override;
    virtual void UpdatePatternWeight(_int iPattern) override;
    virtual _bool CanMove() const override;

    virtual void ChosePatternWeightByDistance(_float fDistance);
    virtual void SetupAttackByType(_int iPattern);
    void         Ready_AttackPatternWeight();
private:
	CBone* m_pRightElbowBone = { nullptr };
	CPhysXDynamicActor* m_pPhysXElbow = { nullptr };
	class CWeapon_Monster* m_pWeapon = { nullptr };
    _bool m_bPlayedDetect = false;
	_bool m_bSpawned = false;
    _bool m_bReturnToSpawn = false;
	_float m_fDetectRange = 22.f;
    _float m_fDetectDiffY = 5.f;
    // 공격 관련
    _int   m_iPatternLimit = 1;
    _int   m_iFireBallComboCount = 0;
    _float m_fBasePatternWeight = 100.f;
    _float m_fMinWeight = 30.f;
    _float m_fMaxWeight = 250.f;
    _float m_fWeightDecreaseRate = 0.6f;
    _float m_fWeightIncreaseRate = 0.4f;

    _float m_fTooCloseDistance = 1.f;

    EPoliceAttackPattern m_eCurAttackPattern = EPoliceAttackPattern::AP_NONE;
    EPoliceAttackPattern m_ePrevAttackPattern = EPoliceAttackPattern::AP_NONE;

    vector<EPoliceAttackPattern> m_vecCloseAttackPatterns = {
        COMBO1, COMBO2, COMBO3,COMBO5
    };

    vector<EPoliceAttackPattern> m_vecMiddleAttackPatterns = {
			COMBO1, COMBO2, COMBO3,COMBO5
    };

    const _float ATTACK_DISTANCE_CLOSE = 0.f;
    const _float ATTACK_DISTANCE_MIDDLE = 5.f;
public:
	static CElite_Police* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};


NS_END
