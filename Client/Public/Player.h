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
class CLegionArm_Base;

class CPlayer : public CUnit
{
public:
	typedef struct tagPlayerDesc : public CUnit::tagUnitDesc
	{
	}PLAYER_DESC;

	enum class EHitDir {F,FL,FR,L,R,BL,BR,B, END};

	struct InputContext {
		/* [ W,A,S,D �Է� ] */
		_bool bMove;

		/* [ ���콺 �Է� ] */
		_bool bLeftMouseDown;
		_bool bRightMouseDown;
		_bool bRightMousePress;
		_bool bRightMouseUp;

		/* [ Ư��Ű �Է� ] */
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
		ARM_ATTACKA, ARM_ATTACKB, ARM_ATTACKCHARGE, ARM_FAIL, GRINDER, END
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

private: /* [ �ǰ� �����Լ� ] */
	EHitDir			ComputeHitDir();


private:
	void			SitAnimationMove(_float fTimeDelta);

/* [ �Է� ó�� ] */
private: 
	void			HandleInput();									// [1] Ű �Է¸� ó��
	EPlayerState	EvaluateTransitions();							// [2] �Է¿� ���� ���� ����
	void			UpdateCurrentState(_float fTimeDelta);			// [3] ���� ���� ���� ����
	void			TriggerStateEffects(_float fTimeDelta);			// [4] �߰����� ����

private: /* [ �ִϸ��̼� ���� ] */
	eAnimCategory	GetAnimCategoryFromName(const string& stateName);
	_vector ComputeLatchedMoveDir(_bool bSwitchFront, _bool bSwitchBack, _bool bSwitchLeft, _bool bSwitchRight);
	virtual void Register_Events() override;

private: /* [ ��Ʈ��� Ȱ��ȭ ] */
	void			RootMotionActive(_float fTimeDelta);

private: /* [ Actor ������Ʈ ] */
	void    Update_Collider_Actor();

	virtual void ReceiveDamage(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);

	/* [ Ray�� ���� �浹(HitPos& HitNormal) ] */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);
	

private:/* [ ĳ�����̵� �����Լ� ] */
	HRESULT UpdateShadowCamera();

private: /* [ �̵����� ] */
	void SetMoveState(_float fTimeDelta);
	void Movement(_float fTimeDelta);
	void ToggleWalkRun() { m_bWalk = !m_bWalk; }
	void SyncTransformWithController(); // ��ġ����ȭ(��Ʈ�ѷ�)

private: /* [ Setup �Լ� ] */
	HRESULT Ready_Weapon();
	HRESULT Ready_Components();
	HRESULT Ready_Actor();
	HRESULT Ready_Controller();
	HRESULT Ready_UIParameters();
	HRESULT Ready_Arm();
	void LoadPlayerFromJson();
	//HRESULT Ready_Effect();

private: /* [ ������ ���� ] */
	void Callback_HP();
	void Callback_Stamina();
	void Callback_Mana();

public: /* [ ��ȣ�ۿ� ���� ] */
	void Interaction_Door(INTERACT_TYPE eType, CGameObject* pObj);
private:
	void Play_CutScene_Door();

private:
	void ItemWeaponOFF(_float fTimeDelta);
	void SlidDoorMove(_float fTimeDelta);

	void Weapon_Collider_Active();
	void Reset_Weapon();

private: /* [ ���� �Լ� ] */
	void LockOnState();

private: /* [ ���� �Լ� ] */
	void Callback_UpBelt();
	void Callback_DownBelt();
	void Use_Item();

	void PriorityUpdate_Slot(_float fTimeDelta);
	void Update_Slot(_float fTimeDelta);
	void LateUpdate_Slot(_float fTimeDelta);

private: /* [ ����Ʈ ���� �Լ� ]*/
	void Set_GrinderEffect_Active(_bool bActive);

private: /* [ �������� ] */
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
	friend class CPlayer_ArmFail;
	friend class CPlayer_Hited;


private: /* [ ���� ���� ] */
	EPlayerState  m_pPreviousState = { EPlayerState::END };
	EPlayerState  m_eCurrentState = { EPlayerState::IDLE };

	CPlayerState* m_pCurrentState = { nullptr };
	CPlayerState* m_pStateArray[ENUM_CLASS(EPlayerState::END)] = { nullptr };

protected:
	class CCamera_Manager* m_pCamera_Manager = { nullptr };
	class CLockOn_Manager* m_pLockOn_Manager = { nullptr };

	/* [ ������ ���� ] */
	CPhysXController* m_pControllerCom = { nullptr };
	CPhysXControllerHitReport* m_pHitReport = { nullptr };

private: /* [ �׸��� ���� ] */
	_vector m_vShadowCam_Eye = {};
	_vector m_vShadowCam_At = {};

private: /* [ ������ �� �ִ� ��ü ] */
	CGameObject*	m_pTarget = { nullptr };
	CWeapon*		m_pWeapon = { nullptr };
	CGameObject*	m_pInterectionStuff = { nullptr };

private: /* [ �������� ���� ] */
	_bool	m_bWeaponEquipped = { false };
	_bool	m_bBackStepAttack = { false };
	_bool 	m_bIsChange = { false };

	_float 	m_fChangeTime = {};
	_float 	m_fChangeTimeElaped = {};
	_int 	m_iCurrentCombo = { 0 };

	_vector m_vHitPos = {};
	_vector m_vHitNormal = {};

	_bool   m_bGardHit = {};
	EHitDir m_eDir = EHitDir::END;

private: /* [ �̵����� ���� ] */
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

private: /* [ ���ͷ��� ���ú��� ] */
	_bool  m_bInteraction[9] = { false };
	_bool  m_bInteractionProb[9] = { false };
	_bool  m_bInteractionProbMove[9] = { false };
	_bool  m_bInteractionMove[9] = { false };
	_bool  m_bInteractionRotate[9] = { false };
	_bool  m_bInteractSound[9] = { false };
	_float m_fInteractionTime[9] = { 0 };

	unordered_set<string> m_MovableStates = {
		"Walk_BL", "Walk_F", "Walk_FL", "Walk_FR", "Walk_B", "Walk_L", "Walk_R", "Walk_BR",
		"Run_F", "Run_F_Stop", "Run_FR", "Run_FL", "Run_BR", "Run_BL", "Run_B", "Run_L", "Run_R",
		"Sprint", "Sprint_Stop",
		"Guard_Walk_B", "Guard_Walk_F", "Guard_Walk_L", "Guard_Walk_R",
		"EquipWeapon_Walk_F", "PutWeapon_Walk_F",
		"OnLamp_Walk", "FailItem_Walk",
		"Grinder_Start", "Grinder_Loop", "Grinder_Loop_Walk_F", "Grinder_Loop_Walk_R", "Grinder_Loop_Walk_L" , "Grinder_Loop_Walk_B",//"Grinder_End"
		"Heal","Heal_Walk_R","Heal_Walk_F","Heal_Walk_FR","Heal_Walk_FL","Heal_Walk_L", "Heal_Walk_B","Heal_Walk_BL","Heal_Walk_BR"

	};

private: /* [ ��Ʈ��� ���� ���� ] */
	_vector  m_PrevWorldDelta = XMVectorZero();
	_vector  m_PrevWorldRotation = XMVectorZero();
	_bool    m_bIsFirstFrame = true;
	_float   m_fRotSmoothSpeed = 8.0f;
	_float   m_fSmoothSpeed = 8.0f;
	_float   m_fSmoothThreshold = 0.1f;

private: /* [ �÷��̾� ���� ] */
	_float  m_fSetTime = {};
	_bool   m_bSetOnce = {};
	_bool   m_bSetTwo = {};

	_float	m_fMaxHP = { 100.f };
	_float	m_fHP = { 100.f };
	
	_float	m_fMaxStamina = { 100.f };
	_float	m_fStamina = { 100.f };
	
	_float	m_fMaxMana = { 300.f };
	_float	m_fMana = { 300.f };
	
	_float	m_fMaxErgo = { 100.f };
	_float	m_fErgo = { 0.f };

private: /* [ ���� ������ ] - weapon �� ���� */


private: /* [ ���� �� ������ ] */
	CLegionArm_Base* m_pLegionArm = { nullptr };
	_float  m_fLegionArmEnergy = { 100.f };
	_float  m_fMaxLegionArmEnergy = { 100.f };

private: /* [ ���� ���� ] */
	_bool	m_bIsGuarding = { false };
	_bool	m_bIsHit = { false };
	_bool	m_bIsInvincible = { false };
	_float	m_fIsInvincible = {};
	_bool	m_bIsLockOn = { false };

private: /* [ ���� �÷��̾� ���� ] */
	_int	m_iLevel = { 0 };

private: /* [ ������ ��� ���� ���� ] */
	_bool	 m_bItemSwitch = {};
	_float	 m_fItemTime = {};
	
private: /* [ ��Ʈ ���� ] */
	_bool	 m_bUseLamp = {};
	_bool	 m_bUseGrinder = {};
	_bool	 m_bUsePulse = {};

	_bool m_bSwitch = { true };
	
	class CBelt* m_pBelt_Up = { nullptr };
	class CBelt* m_pBelt_Down = { nullptr };

	_bool m_isSelectUpBelt = { true };
	// ���� ���������� ǥ�õǰ� �ִ� ������
	class CItem* m_pSelectItem = { nullptr };
	// ���� ������� ������
	class CItem* m_pUseItem = { nullptr };

private: /* [ ����Ʈ ] */
	class CEffectContainer* m_pGrinderEffect = { nullptr };

public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END

