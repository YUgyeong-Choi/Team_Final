#pragma once
#include "Unit.h"
#include "Client_Defines.h"
#include "UI_MonsterHP_Bar.h"
#include "PhysX_ControllerReport.h"

NS_BEGIN(Engine)
class CBone;
class CNavigation;
class CPhysXController;
class CPhysXDynamicActor;
class CAnimController;
NS_END

NS_BEGIN(Client)
class CEliteUnit : public CUnit
{
public:
	enum class EAttackType
	{
		FURY_AIRBORNE,AIRBORNE,STRONG_KNOCKBACK,KNOCKBACK, NORMAL, FURY_STAMP, STAMP, NONE
	};
	enum class EEliteState {
		IDLE, WALK, RUN, TURN, ATTACK, GROGGY, PARALYZATION, FATAL, DEAD, CUTSCENE,NONE
	};

protected:
	enum class EMoveDirection {
		FRONT, RIGHT, BACK, LEFT
	};



protected:
	CEliteUnit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEliteUnit(const CEliteUnit& Prototype);
	virtual ~CEliteUnit() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;

public:
	void EnterFatalHit();
	const EAttackType Get_AttackType() const { return m_eAttackType; }

protected:
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void ReceiveDamage(CGameObject* pOther, COLLIDERTYPE eColliderType);
	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);


	HRESULT LoadAnimationEventsFromJson(const string& modelName);
	HRESULT LoadAnimationStatesFromJson(const string& modelName);
	HRESULT LoadFromJson();
	virtual HRESULT Ready_Components(void* pArg);
	virtual HRESULT Ready_Actor();
	virtual void Ready_EffectNames() {};
	virtual void Ready_BoneInformation() {};

	virtual void HandleMovementDecision(_float fDistance, _float fTimeDelta);
	virtual void Update_Collider();
	virtual void UpdateState(_float fTimeDelta);
	virtual void UpdateMovement(_float fDistance, _float fTimeDelta);
	virtual void UpdateAttackPattern(_float fDistance, _float fTimeDelta) ;
	virtual void UpdateStateByNodeID(_uint iNodeID);
	virtual void ProcessingEffects(const _wstring& stEffectTag) {}; // 이펙트 처리

	virtual void EnableColliders(_bool bEnable);
	virtual _bool CanMove() const;
	_bool  IsTargetInFront(_float fDectedAngle = 60.f) const;

	_bool UpdateTurnDuringAttack(_float fTimeDelta);
	_float Get_DistanceToPlayer() const;
	_vector GetTargetDirection() const;
	_int GetYawSignFromDiretion() {
		// 플레이어 방향 기준
		_vector vDir = GetTargetDirection();
		vDir = XMVectorSetY(vDir, 0.f);
		_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
		_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		_vector vCross = XMVector3Cross(vLook, vDir);
		return (XMVectorGetY(vCross) < 0.f) ? 1 : 0;
	}

	void SetTurnTimeDuringAttack(_float fTime, _float fAddtivRotSpeed = 1.f)
	{
		if (m_fTurnTimeDuringAttack != 0.f)
			return;
		m_fTurnTimeDuringAttack = fTime;
		m_fAddtiveRotSpeed = fAddtivRotSpeed;
	}

	// 이동
	void ApplyRootMotionDelta(_float fTimeDelta);
	virtual void UpdateNormalMove(_float fTimeDelta);

	virtual void UpdateSpecificBehavior() {};

	_float CalculateCurrentHpRatio() const
	{
		return m_fHP / m_fMaxHP;
	}
	virtual void UpdatePatternWeight(_int iPattern) {}
	virtual void SetupAttackByType(_int iPattern) {}
	virtual _int GetRandomAttackPattern(_float fDistance) { return -1; }

	// 이펙트 출력 관련
	virtual HRESULT EffectSpawn_Active(_int iPattern, _bool bActive, _bool bIsOnce = true) { return S_OK; }
	virtual HRESULT Spawn_Effect() { return S_OK; }

	virtual HRESULT Ready_Effect() { return S_OK; } // Initialize에서 Loop로 평생 돌릴 이펙트 ready
	_bool CanGroggyActive() const { return m_bGroggyActive; } // 그로기를 만들 수 있는 상태인지
	virtual void Reset() override;
	virtual void Register_Events() override;
	virtual void Ready_SoundEvents() {};

public:
	EEliteState GetCurrentState() const { return m_eCurrentState; }

protected:
	CNavigation* m_pNaviCom = { nullptr };
	CUI_MonsterHP_Bar* m_pHPBar = { nullptr };

	EEliteState m_eCurrentState = EEliteState::NONE;
	EEliteState m_ePrevState = EEliteState::NONE;
	_bool    m_bIsFirstAttack{ true };
	_float4x4 	m_InitWorldMatrix{};

	// 체력
	_float   m_fHP = 100.f;
	_float   m_fMaxHP = 100.f;
	

	 _bool   m_bGroggyActive = false;
	 _float  m_fGroggyScale_Weak = 0.05f;   // 약공격에 대한 게이지 증가율
	 _float  m_fGroggyScale_Strong = 0.1f; // 강공격에 대한 게이지 증가율
	 _float  m_fGroggyScale_Charge = 0.15f; // 차지공격에 대한 게이지 증가율
	_float   m_fGroggyGauge  = 0.f;       // 누적 값
	_float   m_fGroggyThreshold = 1.f;   // 발동 기준
	_float   m_fGroggyTimer = 7.f;       // 화이트 게이지 유지 시간
	_float	 m_fGroggyEndTimer = 0.f;   // 화이트 게이지 유지 시간 카운트
	_vector  m_PrevWorldDelta = XMVectorZero();
	_vector  m_PrevWorldRotation = XMVectorZero();
	_float   m_fRotSmoothSpeed = 8.0f;
	_float   m_fSmoothSpeed = 8.0f;
	_float   m_fSmoothThreshold = 0.05f;
	_float   m_fSlideClamp = 0.2f;
	_float   m_fWalkSpeed = 3.f;
	_float   m_fRunSpeed = 6.f;
	_float   m_fMaxRootMotionSpeed = 13.f;
	_float   m_fRootMotionAddtiveScale =1.2f; // 루트 모션 추가 배율
	_float   m_fChasingDistance = 1.5f; // 플레이어 추적 거리

	_float   m_fChangeMoveDirCooldown = 0.f; // 이동 방향 변경 쿨타임
	_float   m_fAddtiveRotSpeed = 1.f; // 회전 속도 추가값
	_float   m_fTurnTimeDuringAttack = 0.f;

#ifdef _DEBUG
	_bool m_bDebugMode = false;
#endif // _DEBUG


	// 공격 관련
	_int	 m_iPatternLimit = 1;
	_float	 m_fBasePatternWeight = 100.f;
	_float	 m_fMinWeight = 30.f;
	_float	 m_fMaxWeight = 250.f;
	_float	 m_fWeightDecreaseRate = 0.6f;
	_float	 m_fWeightIncreaseRate = 0.4f;
	_float   m_fAttackCooldown = 0.f; // 공격 쿨타임
	_float   m_fAttckDleay = 4.f;


	unordered_map<_int, _float> m_PatternWeightMap;
	unordered_map<_int, _float> m_PatternWeighForDisttMap;
	unordered_map<_int, _int>   m_PatternCountMap;// 패턴 연속 횟수

	EAttackType m_eAttackType = EAttackType::NONE;

	unordered_map<_int, vector<_wstring>> m_EffectMap; // 이펙트 이름 맵 (패턴, 이름)
	list<pair<_wstring, _bool>> m_ActiveEffect; // 활성화된 이펙트 (이름, 한번만 실행할지)

	_float m_fMinimumTurnAngle = 35.f;

public:
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END

