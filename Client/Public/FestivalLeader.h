#pragma once
#include "BossUnit.h"
#include "Client_Defines.h"
#include "PhysX_ControllerReport.h"
#include "UI_MonsterHP_Bar.h"

NS_BEGIN(Client)
class CFestivalLeader final : public CBossUnit
{
	// 주요 상태들의 NodeID
	enum class BossStateID : _uint
	{
		Atk_Slam_Start = 100002,
		Atk_Slam_Loop = 100003,
		Atk_Slam_End = 100004,

		Idle = 100008,

		CutScene_Start = 1001170,
		CutScene_End = 1001171,


		Atk_Jump_Start = 100010,
		Atk_Jump_Loop = 100011,
		Atk_Jump_End = 100012,

		Walk_F = 100017,
		Walk_B = 100126,
		Walk_R = 100127,
		Walk_L = 100128,

		Turn_L = 100129,
		Turn_R = 100130,

		Run_F = 100131,

		Groggy_Start = 100020,
		Groggy_Loop = 100021,
		Groggy_End = 100022,

		Fatal_Hit_Start = 100026,
		Fatal_Hit_Loop = 100027,
		Fatal_Hit_End = 100028,

		Atk_Strike_Start = 100033,
		Atk_Strike_Loop = 100034,
		Atk_Strike_End = 100035,

		Atk_CrossSlam_Start = 100040,
		Atk_CrossSlam_Loop = 100041,
		Atk_CrossSlam_End = 100042,

		Atk_AlternateSmash_Start = 100047,
		Atk_AlternateSmash_Loop = 100048,
		Atk_AlternateSmash_Start2 = 100049,
		Atk_AlternateSmash_Loop2 = 100050,
		Atk_AlternateSmash_Loop3 = 100051,
		Atk_AlternateSmash_Start3 = 100052,
		Atk_AlternateSmash_End = 100053,

		Atk_Phase2Start = 100067,

		Atk_HalfSpin_Start = 100068,
		Atk_HalfSpin_Loop = 100069,
		Atk_HalfSpin_End = 100070,

		Atk_Spin = 100074,

		Atk_FurySwing_Start = 100077,
		Atk_FurySwing_Loop = 100078,
		Atk_FurySwing_End = 100079,

		Atk_FuryBodySlam_Start = 100082,
		Atk_FuryBodySlam_Loop = 100083,
		Atk_FuryBodySlam_End = 100084,

		Atk_DashSwing_Start = 100089,
		Atk_DashSwing_Loop = 100090,
		Atk_DashSwing_End = 100091,

		Atk_HammerSlam_Start = 100096,
		Atk_HammerSlam_Loop = 100097,
		Atk_HammerSlam_End = 100098,

		Atk_FuryHammerSlam_Start = 100102,
		Atk_FuryHammerSlam_Loop = 100103,
		Atk_FuryHammerSlam_End = 100104,

		Atk_Swing_Start = 100109,
		Atk_Swing_Loop = 100110,
		Atk_Swing_End = 100111,

		Atk_DashSwingCom_Start = 100116,
		Atk_SwingCom_Start = 100122,

		Special_Die = 100167,
	};

	enum EBossAttackPattern : _int
	{
		BAP_NONE = 0,
		Slam = 1,
		CrossSlam = 2,
		JumpAttack = 3,
		Strike = 4,
		AlternateSmash = 5,
		Spin = 6,
		HalfSpin = 7,
		HammerSlam = 8,
		DashSwing = 9,
		Swing = 10,
		FuryHammerSlam = 11,
		FurySwing = 12,
		FuryBodySlam = 13
	};

private:
	CFestivalLeader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFestivalLeader(const CFestivalLeader& Prototype);
	virtual ~CFestivalLeader() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Reset() override;
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
	 HRESULT        Ready_Weapon();
	virtual void Ready_BoneInformation() override;

	virtual void Update_Collider() override;
	virtual void UpdateAttackPattern(_float fDistance, _float fTimeDelta) override;
	virtual void UpdateStateByNodeID(_uint iNodeID) override;
	virtual void UpdateSpecificBehavior(_float fTimeDelta) override;
	virtual void EnableColliders(_bool bEnable) override;

	virtual _bool CanProcessTurn() override;

	void ApplyHeadSpaceSwitch(_float fTimeDelta);
	// 공견 패턴
	virtual void SetupAttackByType(_int iPattern) override;

	virtual void Register_Events() override;

	virtual void Ready_AttackPatternWeightForPhase1() override;
	virtual void Ready_AttackPatternWeightForPhase2() override;

	virtual _int GetRandomAttackPattern(_float fDistance) override;
	virtual void UpdatePatternWeight(_int iPattern) override;

	void ChosePatternWeightByDistance(_float fDistance);
	virtual void Ready_EffectNames() override;
	virtual void ProcessingEffects(const _wstring& stEffectTag) override;
	virtual HRESULT EffectSpawn_Active(_int iPattern, _bool bActive, _bool bIsOnce = true) override;
	virtual HRESULT Spawn_Effect();

	virtual HRESULT Ready_Effect();
	const EBossAttackPattern GetCurrentAttackPattern() const { return m_eCurAttackPattern; }

	virtual void Ready_SoundEvents() override;

private:
	// 컴포넌트 관련
	CPhysXDynamicActor* m_pPhysXActorComForHammer = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorComForBasket = { nullptr };
	CBone* m_pHammerBone{ nullptr };
	CBone* m_pBasketBone{ nullptr };
	CBone* m_pRightHandBone{ nullptr };
	CBone* m_pLeftHandBone{ nullptr };
	class CWeapon_Monster* m_pHammer{ nullptr };
	_matrix m_pHammerWorldMatrix = XMMatrixIdentity();
	_int m_iOriginBoneIndex = -1;
	_int m_iNewParentIndex = -1;
	_float4x4 m_StoredHeadLocalMatrix{};
	_float4x4 m_HeadLocalInit{};
	_bool m_bSwitchHeadSpace = false;
	_float3   m_vHeadExtraEulerDeg = {90.f, 0.f, 0.f}; // X=90° 기본
	_float3   m_vHeadExtraOffset   = {0.f, 0.f, 0.f};  // 추가 위치 보정(로컬)
	_float     m_fHeadExtraScale    = 1.f;              // 필요 없으면 1
	// 상태 관련
	_bool m_bPlayerCollided = false;
	_bool m_bPhase2Processed = false;
	_bool m_bWaitPhase2 = false;
	_bool m_bVisibleModel = true;
	_bool m_bVisibleHeadOnce = false;


	// 공격 관련
	_float4 m_vCenterPos{ -0.195f, 0.f,-213.f ,1.f };
	_int    m_iCrossComboCount = 0;
	_int    m_iLastComboType = -1;

	EBossAttackPattern m_eCurAttackPattern = EBossAttackPattern::BAP_NONE;
	EBossAttackPattern m_ePrevAttackPattern = EBossAttackPattern::BAP_NONE;

	vector<EBossAttackPattern> m_vecCloseAttackPatterns = {
	  Slam,CrossSlam,JumpAttack,Strike
	};

	vector<EBossAttackPattern> m_vecMiddleAttackPatterns = {
		  Slam,CrossSlam,JumpAttack,Strike
	};

	vector<EBossAttackPattern> m_vecFarAttackPatterns = {
	  DashSwing ,FurySwing ,JumpAttack,Strike
	};


	// 상수
	const _int   LIMIT_FIREBALL_COMBO_COUNT = 4;
	const _float ATTACK_DISTANCE_CLOSE = 1.f;
	const _float ATTACK_DISTANCE_MIDDLE = 7.f;
	const _float ATTACK_DISTANCE_FAR = 15.f;
	const _float DAMAGE_LIGHT = 5.f;
	const _float DAMAGE_MEDIUM = 10.f;
	const _float DAMAGE_HEAVY = 15.f;
	const _float DAMAGE_FURY = 17.f;


public:
	static CFestivalLeader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END

