#pragma once
#include "Client_Defines.h"
#include "PhysX_ControllerReport.h"
#include "BossUnit.h"

NS_BEGIN(Engine)
class CPhysXController;
class CPhysXDynamicActor;
class CAnimController;
NS_END

NS_BEGIN(Client)
class CPlayerState;
class CWeapon;
class CLegionArm_Base;
class CPlayerLamp;
class CPlayerFrontCollider;

class CPlayer : public CUnit
{
public:
	typedef struct tagPlayerDesc : public CUnit::tagUnitDesc
	{
	}PLAYER_DESC;

	enum class EHitDir {F,FL,FR,L,R,BL,BR,B, END};

	struct InputContext {
		/* [ W,A,S,D 입력 ] */
		_bool bMove = false;

		/* [ 마우스 입력 ] */
		_bool bLeftMouseDown = false;
		_bool bRightMouseDown = false;
		_bool bRightMousePress = false;
		_bool bRightMouseUp = false;

		/* [ 특수키 입력 ] */
		_bool bShift = false;
		_bool bCtrl = false;
		_bool bCtrlPress = false;
		_bool bItem = false;
		_bool bTap = false;
		_bool bSkill = false;
		_bool bSpaceUP = false;
		_bool bSpaceDown = false;

	} m_Input;
	
	enum class eAnimCategory
	{
		NONE,IDLE,WALK,RUN, DASH_BACK, DASH_FRONT ,DASH_FOCUS,SPRINT,GUARD,GUARD_HIT, GUARD_BREAK,EQUIP,EQUIP_WALK,ITEM,ITEM_WALK,NORMAL_ATTACKA,NORMAL_ATTACKB,
		STRONG_ATTACKA, STRONG_ATTACKB, CHARGE_ATTACKA, CHARGE_ATTACKB, SPRINT_ATTACKA, SPRINT_ATTACKB, MAINSKILLA, MAINSKILLB, MAINSKILLC, SIT, FIRSTDOOR,FESTIVALDOOR,
		ARM_ATTACKA, ARM_ATTACKB, ARM_ATTACKCHARGE, ARM_FAIL, GRINDER, HITED, HITEDUP, HITEDSTAMP, PULSE, FATAL, ITEMFAIL, STATIONDOOR, END
	};

	enum class eHitedTarget
	{
		MONSTER, BOSS, RANGED, END
	};

protected:
	CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlayer(const CPlayer& Prototype);
	virtual ~CPlayer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Burn() override;
	virtual HRESULT Render_LimLight() override;

	virtual void Reset() override;

public:
	CPhysXController*	Get_Controller() { return m_pControllerCom; }
	EPlayerState		Get_PlayerState() { return m_eCurrentState; }

	CAnimController* GetCurrentAnimContrller();

	// 몬스터가 죽을 때 불러줌
	void Set_HitTarget(CUnit* pTarget, _bool bDead);

private: /* [ 피격 헬퍼함수 ] */
	EHitDir			ComputeHitDir();
	void			CalculateDamage(CGameObject* pOther, COLLIDERTYPE eColliderType);
	void			HPSubtract();

private:
	void			SitAnimationMove(_float fTimeDelta);


private: /* [ 입력 처리 ] */
	void			HandleInput();									// [1] 키 입력만 처리
	EPlayerState	EvaluateTransitions();							// [2] 입력에 따라 상태 전이
	void			UpdateCurrentState(_float fTimeDelta);			// [3] 현재 상태 로직 수행
	void			TriggerStateEffects(_float fTimeDelta);			// [4] 추가적인 셋팅

public: /* [ 애니메이션 관련 ] */
	eAnimCategory	GetAnimCategoryFromName(const string& stateName);
	_vector ComputeLatchedMoveDir(_bool bSwitchFront, _bool bSwitchBack, _bool bSwitchLeft, _bool bSwitchRight);
	virtual void Register_Events() override;
public: /* [ 컷씬 용 ] */
	_bool Get_HaveKey() { return m_bHaveKey; }
	void Set_GetKey() { m_bHaveKey = true; }
	_bool MoveToDoor(_float fTimeDelta, _vector vTargetPos);
	_bool RotateToDoor(_float fTimeDelta, _vector vRotation);
private: /* [ 루트모션 활성화 ] */
	void	RootMotionActive(_float fTimeDelta);

private: /* [ Actor 업데이트 ] */
	void    Update_Collider_Actor();


	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);

	/* [ Ray로 인항 충돌(HitPos& HitNormal) ] */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);
	

private:/* [ 캐스케이드 전용함수 ] */
	HRESULT UpdateShadowCamera();

private: /* [ 이동로직 ] */
	void SetMoveState(_float fTimeDelta);
	void Movement(_float fTimeDelta);
	void ToggleWalkRun() { m_bWalk = !m_bWalk; }
	void SyncTransformWithController(); // 위치동기화(컨트롤러)

private: /* [ Setup 함수 ] */
	HRESULT Ready_Weapon();
	HRESULT Ready_Lamp();
	HRESULT Ready_FrontCollider();
	HRESULT Ready_Components();
	HRESULT Ready_Actor();
	HRESULT Ready_Controller();
	HRESULT Ready_UIParameters();
	HRESULT Ready_Arm();
	void LoadPlayerFromJson();
	HRESULT Ready_Stat();
	HRESULT Ready_Effect();

public: /* [ 옵저버 관련 ] */
	void Callback_HP();
	void Callback_Stamina();
	void Callback_Mana();



	void Add_Mana(_float fMana);

public: /* [ 상호작용 관련 ] */
	void Interaction_Door(INTERACT_TYPE eType, CGameObject* pObj);
	void GetWeapon();
private:
	void Play_CutScene_Door();
	void ItemWeapOnOff(_float fTimeDelta);
	void SlidDoorMove(_float fTimeDelta);

private: /* [ 무기 관련 ] */
	void Weapon_Collider_Active();
	void Reset_Weapon();


public: /* [ 불타는 셰이딩 ] */
	void BurnActive(_float fDeltaTime);
	void OnBurn(_float fTimeDelta);
	void OffBurn(_float fTimeDelta);

public: /* [ 림라이트 셰이딩 ] */
	void LimActive(_bool bOnOff, _float fSpeed, _float4 vColor = { 0.f, 0.749f, 1.f, 1.f }); // 기본 스킬 색상
	void OnLim(_float fTimeDelta);
	void OffLim(_float fTimeDelta);


public:
	CPlayerLamp* Get_PlayerLamp() { return m_pPlayerLamp; }

private: /* [ 락온 함수 ] */
	void LockOnState(_float fTimeDelta);

public: /* [ 슬룻 함수 ] */
	void Callback_UpBelt();
	void Callback_DownBelt();
	void Use_Item();

	void PriorityUpdate_Slot(_float fTimeDelta);
	void Update_Slot(_float fTimeDelta);
	void LateUpdate_Slot(_float fTimeDelta);

	_bool Find_Slot(const _wstring& strItemTag);

	void Add_Icon(const _wstring& strItemTag);

private: /* [ 이펙트 관리 함수 ]*/
	void Set_GrinderEffect_Active(_bool bActive);
	void Create_HitEffect();
	void Create_GuardEffect(_bool isPerfect);
public:
	void Create_LeftArm_Lightning();

public: /* [ 페이탈 함수 ] */
	void SetbIsBackAttack(_bool bIsBackAttack) { m_bIsBackAttack = bIsBackAttack; }
	_bool GetbIsBackAttack() const { return m_bIsBackAttack; }
	void SetbIsGroggyAttack(_bool bIsGroggyAttack) { m_bIsGroggyAttack = bIsGroggyAttack; }
	_bool GetbIsGroggyAttack() const { return m_bIsGroggyAttack; }
	void SetFatalTarget(CUnit* pTarget) { m_pFatalTarget = pTarget; }
	void SetFatalTargetNull() { m_pFatalTarget = nullptr; }
	void SetIsFatalBoss(_bool bIsFatalBoss) { m_bIsFatalBoss = bIsFatalBoss; }
	_bool GetIsFatalBoss() const { return m_bIsFatalBoss; }
	CUnit* GetFatalTarget() { return m_pFatalTarget; }

private:
    void InitializeSpringBones();
    void Update_HairSpring(_float fTimeDelta);
    void Build_SpringBoneHierarchy();
    
	class CSpringBoneSys* m_pSpringBoneSys = { nullptr };

private: /* [ 상태패턴 ] */
	void ReadyForState();
	friend class CPlayerState;
	friend class CPlayer_Idle;
	friend class CPlayer_Walk;
	friend class CPlayer_Run;
	friend class CPlayer_Item;
	friend class CPlayer_BackStep;
	friend class CPlayer_Rolling;
	friend class CPlayer_Equip;
	friend class CPlayer_Sprint;
	friend class CPlayer_WeakAttackA;
	friend class CPlayer_WeakAttackB;
	friend class CPlayer_StrongAttackA;
	friend class CPlayer_StrongAttackB;
	friend class CPlayer_ChargeA;
	friend class CPlayer_ChargeB;
	friend class CPlayer_Gard;
	friend class CPlayer_SprintAttackA;
	friend class CPlayer_SprintAttackB;
	friend class CPlayer_ArmAttackA;
	friend class CPlayer_ArmAttackB;
	friend class CPlayer_ArmCharge;
	friend class CPlayer_MainSkill;
	friend class CPlayer_Fatal;
	friend class CPlayer_ArmFail;
	friend class CPlayer_Hited;
	friend class CPlayer_Dead;

public: /* [ 특수 모션 관련 ] */
	void SetHitMotion(HITMOTION eHitMotion) { m_eHitMotion = eHitMotion; }
	HITMOTION GetHitMotion() const { return m_eHitMotion; }
	void SetfReceiveDamage(_float fDamage) { m_fReceiveDamage = fDamage; }
	_float GetfReceiveDamage() const { return m_fReceiveDamage; }
	void SetHitedAttackType(CBossUnit::EAttackType eType) { m_eHitedAttackType = eType; }
	void IsPerfectGard(_float fTimeDelta);
	eAnimCategory GetAnimCategory() const { return m_eCategory; }

public: /* [ 부여 속성 관련 ] */
	void SetElementTypeDuration(EELEMENT eElement, _float fValue) { m_vecElements[eElement].fDuration = fValue; }
	void SetElementTypeWeight(EELEMENT eElement, _float fValue);
	

	void Initialize_ElementConditions(const _float fDefaultDuration, const _float fDefaultWeight);

public:/*[스탯 관련]*/
	void Set_Stat(STAT_DESC eDesc) { m_eStat = eDesc; }
	STAT_DESC& Get_Stat() { return m_eStat; }

	_float Get_Ergo() { return m_fErgo; }
	void Set_Ergo(_float fErgo);
	_int Get_Player_Level() { return m_iLevel; }
	void Set_Player_Level(_int iLevel) { m_iLevel = iLevel; }
	CWeapon* Get_Equip_Weapon() { return m_pWeapon; }
	CWeapon* Get_Equip_Legion();

	// 스탯 바뀌면 이제 체력, 스태미나 등등을 바꾸기...
	void Apply_Stat();

	// 죽을 때 몬스터가 부르기.
	void Add_Ergo(_float fErgo);
	// 레벨 업 하면 레벨에 맞춰서 값이 증가하게
	void Compute_MaxErgo(_int iLevel);


	_bool Check_LevelUp(_int iLevel);

private: /* [ 부여 속성 ] */
	array<EELEMENTCONDITION, ELEMENT_END> m_vecElements;

private: /* [ 특수 모션 ] */
	HITMOTION m_eHitMotion = { HITMOTION::END };
	eHitedTarget m_eHitedTarget = { eHitedTarget::END };
	CBossUnit::EAttackType m_eHitedAttackType = { CBossUnit::EAttackType::NONE };

private: /* [ 상태 변수 ] */
	EPlayerState  m_pPreviousState = { EPlayerState::END };
	EPlayerState  m_eCurrentState = { EPlayerState::IDLE };
	eAnimCategory m_eCategory = { eAnimCategory::NONE };
	eAnimCategory m_ePreCategory = { eAnimCategory::NONE };

	CPlayerState* m_pCurrentState = { nullptr };
	CPlayerState* m_pStateArray[ENUM_CLASS(EPlayerState::END)] = { nullptr };

private: /* [ 불타버려~ ] */
	CTexture* m_pBurn = { nullptr };
	CTexture* m_pBurnMask = { nullptr };
	CTexture* m_pBurnMask2 = { nullptr };
	_float	m_fBurnTime = {};
	_float	m_fBurnPhase = {};
	_float	m_fBurnSpeed = 1.7f;
	_bool	m_bBurnSwitch = {};

private: /* [ 림라이트 ] */
	_float	m_fLimTime = {};
	_float	m_fLimPhase = {};
	_float	m_fLimSpeed = 1.7f;
	_bool	m_bLimSwitch = {};
	_float4 m_vLimLightColor = { 1.f ,1.f ,1.f ,1.f };

protected:
	class CCamera_Manager* m_pCamera_Manager = { nullptr };
	class CLockOn_Manager* m_pLockOn_Manager = { nullptr };

		/* [ 피직스 관련 ] */
	CPhysXController* m_pControllerCom = { nullptr };
	CPhysXControllerHitReport* m_pHitReport = { nullptr };

private: /* [ 그림자 변수 ] */
	_vector m_vShadowCam_Eye = {};
	_vector m_vShadowCam_At = {};

private: /* [ 소유할 수 있는 객체 ] */
	CGameObject*			m_pTarget = { nullptr };
	CUnit*					m_pFatalTarget = { nullptr };
	CGameObject*			m_pHitedTarget = { nullptr };
	CWeapon*				m_pWeapon = { nullptr };
	CGameObject*			m_pInterectionStuff = { nullptr };
	CPlayerLamp*			m_pPlayerLamp = { nullptr };
	CPlayerFrontCollider*	m_pFrontCollider = { nullptr };

private: /* [ 전투관련 변수 ] */
	_bool	m_bWeaponEquipped = { false };
	_bool	m_bBackStepAttack = { false };
	_bool 	m_bLockOnSprint = { false };
	_bool   m_bIsBackAttack = { false };
	_bool   m_bIsGroggyAttack = { false };
	_bool   m_bIsFatalBoss = { false };

	_int 	m_iCurrentCombo = { 0 };

	_float  m_fReceiveDamage = {};
	_float  m_fPerfectGardTime = {};
	_vector m_vHitPos = {};
	_vector m_vHitNormal = {};
	EHitDir m_eDir = EHitDir::END;

	_bool   m_bGardHit = {};
	_bool   m_bGardDamege = {};
	_bool   m_bPerfectGardDamege = {};

private: /* [ 이동관련 변수 ] */
	_bool    m_bWalk = { true };
	_bool    m_bMovable = { true };

	string	 m_strPrevStateName;
	_bool    m_bMoveReset = {};
	_bool    m_bMove = {};
	_bool    m_bSit = {};
	_float   m_fSitTime = {};
	_float   m_fMoveTime = {};
	_int	 m_iMoveStep = {};

	_vector	 m_vMoveDir = {};
	_bool	 m_bSwitchLeft = {};
	_bool	 m_bSwitchRight = {};
	_bool	 m_bSwitchFront = {};
	_bool	 m_bSwitchBack = {};

private: /* [ 인터렉션 관련변수 ] */
	_bool  m_bInteraction[9] = { false };
	_bool  m_bInteractionProb[9] = { false };
	_bool  m_bInteractionProbMove[9] = { false };
	_bool  m_bInteractionMove[9] = { false };
	_bool  m_bInteractionRotate[9] = { false };
	_bool  m_bInteractSound[9] = { false };
	_float m_fInteractionTime[9] = { 0 };
	_bool  m_bHaveKey = false;

	unordered_set<string> m_MovableStates = {
		"Walk_BL", "Walk_F", "Walk_FL", "Walk_FR", "Walk_B", "Walk_L", "Walk_R", "Walk_BR",
		"Run_F", "Run_F_Stop", "Run_FR", "Run_FL", "Run_BR", "Run_BL", "Run_B", "Run_L", "Run_R",
		"Sprint", //"Sprint_Stop",
		"Guard_Walk_B", "Guard_Walk_F", "Guard_Walk_L", "Guard_Walk_R",
		"EquipWeapon_Walk_F", "PutWeapon_Walk_F",
		"OnLamp_Walk",
		"Grinder_Start", "Grinder_Loop", "Grinder_Loop_Walk_F", "Grinder_Loop_Walk_R", "Grinder_Loop_Walk_L" , "Grinder_Loop_Walk_B",//"Grinder_End"
		"Heal","Heal_Walk_R","Heal_Walk_F","Heal_Walk_FR","Heal_Walk_FL","Heal_Walk_L", "Heal_Walk_B","Heal_Walk_BL","Heal_Walk_BR",
		"Item_Get_Walk"
	};
	_int m_iTestInt = { 0 };

private: /* [ 루트모션 관련 변수 ] */
	_vector  m_PrevWorldDelta = XMVectorZero();
	_vector  m_PrevWorldRotation = XMVectorZero();
	_bool    m_bIsFirstFrame = true;
	_float   m_fRotSmoothSpeed = 8.0f;
	_float   m_fSmoothSpeed = 8.0f;
	_float   m_fSmoothThreshold = 0.1f;
	_float   m_fMaxRootMotionSpeed = 18.f;
	_float   m_fRootMotionAddtiveScale = 1.f; // 루트 모션 추가 배율

private: /* [ 플레이어 변수 ] */
	_float  m_fSetTime = {};
	_bool   m_bSetOnce = {};
	_bool   m_bSetTwo = {};
	_float  m_fSetSoundTime = {};
	_bool   m_bSetSound = {};
	_bool   m_bCheckSound = {};
	_bool   m_bResetSoundTime = true;
	_bool   m_bSetCamera[9] = {};

	_float	m_fMaxStamina = { 143.f };
	_float	m_fStamina = { 143.f };
	
	_float	m_fMaxMana = { 300.f };
	_float	m_fMana = { 300.f };


private: /* [ 특수키 ] */
	_bool   m_bPulseReservation = {};

private: /* [ 리전 암 내구도 ] */
	CLegionArm_Base* m_pLegionArm = { nullptr };

private: /* [ 현재 상태 ] */
	_bool	m_bIsGuarding = { false };
	_bool	m_bIsHit = { false };
	_bool	m_bIsForceDead = { false };
	_bool	m_bIsInvincible = { false };
	_float	m_fIsInvincible = { false };
	_bool	m_bIsLockOn = { false };
	_bool	m_bIsRrevival = { false };

private: /* [ 현재 플레이어 레벨 ] */
	_int	m_iLevel = { 0 };

	// 다음 레벨까지 필요한 에르고, 현재 가지고 있는 에르고
	// 레벨 업 하면 필요한 에르고 필요치를 높힌다. 
	_float	m_fMaxErgo = { 100.f };
	_float	m_fErgo = { 0.f };

private: /* [ 아이템 사용 관련 변수 ] */
	_bool	 m_bItemSwitch = {};
	_float	 m_fItemTime = {};
	_bool	 m_bLampSwitch = {};
	_float	 m_fLampTime = {};
	
private: /* [ 벨트 슬롯 ] */
	_bool	 m_bUseLamp = {};
	_bool	 m_bUseGrinder = {};
	_bool	 m_bUsePulse = {};

	_bool m_bSwitch = { true };
	
	class CBelt* m_pBelt_Up = { nullptr };
	class CBelt* m_pBelt_Down = { nullptr };

	_bool m_isSelectUpBelt = { true };
	// 현재 빨간색으로 표시되고 있는 아이템
	class CItem* m_pSelectItem = { nullptr };
	// 현재 사용중인 아이템
	class CItem* m_pUseItem = { nullptr };

private: /* [ 이펙트 ] */
	class CEffectContainer* m_pGrinderEffect = { nullptr };
private: /* [ 공격한 적 ] */
	class CUnit* m_pHitTarget = { nullptr };

	_int m_iFatalAttackCount = {};



private: // 그라인더용 변수
	CSoundController* m_pGrinderSound = { nullptr };

private:
	STAT_DESC m_eStat = {};
	_float m_fArmor = {};
	_float m_fDamageReduction = {};
public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END

