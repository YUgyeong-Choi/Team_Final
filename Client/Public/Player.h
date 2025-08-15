#pragma once
#include "Unit.h"
#include "Client_Defines.h"
#include "PhysX_ControllerReport.h"

NS_BEGIN(Engine)
class CPhysXController;
class CPhysXDynamicActor;
class CAnimController;
NS_END

NS_BEGIN(Client)
class CPlayerState;
class CWeapon;

class CPlayer : public CUnit
{
public:
	typedef struct tagPlayerDesc : public CUnit::tagUnitDesc
	{
	}PLAYER_DESC;

	struct InputContext {
		/* [ W,A,S,D 입력 ] */
		_bool bMove;

		/* [ 마우스 입력 ] */
		_bool bLeftMouseDown;
		_bool bRightMouseDown;
		_bool bRightMousePress;
		_bool bRightMouseUp;

		/* [ 특수키 입력 ] */
		_bool bShift;
		_bool bCtrl;
		_bool bCtrlPress;
		_bool bItem;
		_bool bTap;
		_bool bSkill;
		_bool bSpaceUP;
		_bool bSpaceDown;

	} m_Input;
	
	enum class eAnimCategory
	{
		NONE,IDLE,WALK,RUN, DASH_BACK, DASH_FRONT ,DASH_FOCUS,SPRINT,GUARD,GUARD_HIT,EQUIP,EQUIP_WALK,ITEM,ITEM_WALK,NORMAL_ATTACKA,NORMAL_ATTACKB,
		STRONG_ATTACKA, STRONG_ATTACKB, CHARGE_ATTACKA, CHARGE_ATTACKB, SPRINT_ATTACKA, SPRINT_ATTACKB, MAINSKILLA, MAINSKILLB, MAINSKILLC, SIT, FIRSTDOOR,
		ARM_ATTACKA, ARM_ATTACKB, ARM_ATTACKCHARGE, ARM_FAIL, END
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

public:
	CPhysXController* Get_Controller() { return m_pControllerCom; }
	EPlayerState Get_PlayerState() { return m_eCurrentState; }

	CAnimController* GetCurrentAnimContrller();

private:
	void			SitAnimationMove(_float fTimeDelta);

/* [ 입력 처리 ] */
private: 
	void			HandleInput();									// [1] 키 입력만 처리
	EPlayerState	EvaluateTransitions();							// [2] 입력에 따라 상태 전이
	void			UpdateCurrentState(_float fTimeDelta);			// [3] 현재 상태 로직 수행
	void			TriggerStateEffects(_float fTimeDelta);			// [4] 추가적인 셋팅

private: /* [ 애니메이션 관련 ] */
	eAnimCategory	GetAnimCategoryFromName(const string& stateName);

private: /* [ 루트모션 활성화 ] */
	void			RootMotionActive(_float fTimeDelta);

private: /* [ actor 업뎃. ]*/
	void    Update_Collider_Actor();

	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
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
	HRESULT Ready_StationDoor();
	HRESULT Ready_Components();
	HRESULT Ready_Actor();
	HRESULT Ready_Controller();
	void LoadPlayerFromJson();

private: /* 옵저버 관련*/
	// 테스트 용이라 나중에 함수에 넣는 식으로 바꾸기
	void Callback_HP();
	void Callback_Stamina();
	void Callback_Mana();

	// 스탯 변화 테스트용
	void Update_Stat();

private: /* [ 상호작용 관련 ] */
	void Interaction_Door();

	void Play_CutScene_Door();

private:
	void ItemWeaponOFF(_float fTimeDelta);
	void SlidDoorMove(_float fTimeDelta);

	
private: // 슬롯 용
	// 테스트 용이라 나중에 함수에 넣는 식으로 바꾸기
	void Callback_UpBelt();
	void Callback_DownBelt();
	void Use_Item();

	void Update_Slot();

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


private: /* [ 상태 변수 ] */
	EPlayerState  m_pPreviousState = { EPlayerState::END };
	EPlayerState  m_eCurrentState = { EPlayerState::IDLE };

	CPlayerState* m_pCurrentState = { nullptr };
	CPlayerState* m_pStateArray[ENUM_CLASS(EPlayerState::END)] = { nullptr };

protected:
	class CCamera_Manager* m_pCamera_Manager = { nullptr };

	/* [ 피직스 관련 ] */
	CPhysXController* m_pControllerCom = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };
	CPhysXControllerHitReport* m_pHitReport = { nullptr };

private: /* [ 그림자 변수 ] */
	_vector m_vShadowCam_Eye = {};
	_vector m_vShadowCam_At = {};

private: /* [ 램프 온 오프 ] */


private: /* [ 소유할 수 있는 객체 ] */
	CGameObject*	m_pTarget = { nullptr };
	CWeapon*		m_pWeapon = { nullptr };
	CGameObject*	m_pInterectionStuff = { nullptr };

private: /* [ 공격관련 변수 ] */
	_bool	m_bWeaponEquipped = { false };
	_bool	m_bBackStepAttack = { false };
	_bool 	m_bIsChange = { false };
	_bool	m_bCutsceneDoor = { false };

	_float 	m_fChangeTime = {};
	_float 	m_fChangeTimeElaped = {};
	_int 	m_iCurrentCombo = { 0 };

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

private: /* [ 인터렉션 관련변수 ] */
	_bool  m_bInteraction[9] = { false };
	_bool  m_bInteractionMove[9] = { false };
	_bool  m_bInteractionRotate[9] = { false };
	_float m_fInteractionTime[9] = { 0 };


private: /* [ 아이템 사용 관련 변수 ] */
	_bool	 m_bItemSwitch = {};
	_float	 m_fItemTime = {};

	unordered_set<string> m_MovableStates = {
		"Walk_BL", "Walk_F", "Walk_FL", "Walk_FR", "Walk_B", "Walk_L", "Walk_R", "Walk_BR",
		"Run_F", "Run_F_Stop", "Run_FR", "Run_FL", "Run_BR", "Run_BL", "Run_B", "Run_L", "Run_R",
		"Sprint", "Sprint_Stop",
		"Guard_Walk_B", "Guard_Walk_F", "Guard_Walk_L", "Guard_Walk_R",
		"EquipWeapon_Walk_F", "PutWeapon_Walk_F",
		"OnLamp_Walk", "FailItem_Walk"

	};

private: /* [ 루트모션 관련 변수 ] */
	_vector  m_PrevWorldDelta = XMVectorZero();
	_vector  m_PrevWorldRotation = XMVectorZero();
	_bool    m_bIsFirstFrame = true;
	_float   m_fRotSmoothSpeed = 8.0f;
	_float   m_fSmoothSpeed = 8.0f;
	_float   m_fSmoothThreshold = 0.1f;

private: // 옵저버 관련
	// stat용
	_int m_iCurrentHP = {};
	_float m_fCurrentStamina = {};
	_int m_iMaxStamina = { 100 };
	// 일단 한칸에 100씩
	_int m_iCurrentMana = {};
	_int m_iMaxMana = { 300 };
	
	// 나중에 없애기 
	_bool m_bSwitch = { true };

	// 레벨, 다음 레벨까지 필요한 에르고, 현재 가지고 있는 에르고
	_int  m_iLevel = {};
	_int  m_iRequiredErgo = {};
	_int  m_iErgo = {};


// 벨트 슬롯 
private:
	// 위에 슬롯, 아래 슬롯
	class CBelt* m_pBelt_Up = { nullptr };
	class CBelt* m_pBelt_Down = { nullptr };

	// 어느 슬롯 선택했는지, 선택한 아이템은 무엇인지
	_bool m_isSelectUpBelt = { true };
	class CItem* m_pSelectItem = { nullptr };


public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END

