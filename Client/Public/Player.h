#pragma once
#include "Unit.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CPhysXController;
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
		_bool bItem;
		_bool bTap;
		_bool bSpaceUP;
		_bool bSpaceDown;

	} m_Input;
	
	enum class eAnimCategory
	{
		NONE,IDLE,WALK,RUN, DASH_BACK, DASH_FRONT ,DASH_FOCUS,SPRINT,GUARD,GUARD_HIT,EQUIP,EQUIP_WALK,ITEM,ITEM_WALK,NORMAL_ATTACKA,NORMAL_ATTACKB,
		STRONG_ATTACKA,STRONG_ATTACKB,CHARGE_ATTACKA,CHARGE_ATTACKB,SPRINT_ATTACK,MAINSKILL,SIT,INTERACTION
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

/* [ �Է� ó�� ] */
private: 
	void			HandleInput();									// [1] Ű �Է¸� ó��
	EPlayerState	EvaluateTransitions();							// [2] �Է¿� ���� ���� ����
	void			UpdateCurrentState(_float fTimeDelta);			// [3] ���� ���� ���� ����
	void			TriggerStateEffects(_float fTimeDelta);			// [4] �߰����� ����

private: /* [ �ִϸ��̼� ���� ] */
	eAnimCategory	GetAnimCategoryFromName(const string& stateName);

private: /* [ ��Ʈ��� Ȱ��ȭ ] */
	void			RootMotionActive(_float fTimeDelta);
	

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
	HRESULT Ready_Controller();
	void LoadPlayerFromJson();

private: /* ������ ����*/
	// �׽�Ʈ ���̶� ���߿� �Լ��� �ִ� ������ �ٲٱ�
	void Callback_HP();
	void Callback_Stamina();
	void Callback_Mana();

	// ���� ��ȭ �׽�Ʈ��
	void Update_Stat();

	
private: // ���� ��
	// �׽�Ʈ ���̶� ���߿� �Լ��� �ִ� ������ �ٲٱ�
	void Callback_UpBelt();
	void Callback_DownBelt();
	void Use_Item();

	void Update_Slot();

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


private: /* [ ���� ���� ] */
	EPlayerState  m_pPreviousState = { EPlayerState::END };
	EPlayerState  m_eCurrentState = { EPlayerState::IDLE };

	CPlayerState* m_pCurrentState = { nullptr };
	CPlayerState* m_pStateArray[ENUM_CLASS(EPlayerState::END)] = { nullptr };

protected:
	class CCamera_Orbital* m_pCamera_Orbital = { nullptr };
	CPhysXController* m_pControllerCom = { nullptr };

private: /* [ �׸��� ���� ] */
	_vector m_vShadowCam_Eye = {};
	_vector m_vShadowCam_At = {};

private: /* [ ������ �� �ִ� ��ü ] */
	CGameObject*	m_pTarget = { nullptr };
	CWeapon*		m_pWeapon = { nullptr };

private: /* [ ���ݰ��� ���� ] */
	_bool	m_bWeaponEquipped = { false };
	_bool	m_bBackStepAttack = { false };
	_bool 	m_bIsChange = { false };

	_float 	m_fChangeTime = {};
	_float 	m_fChangeTimeElaped = {};
	_int 	m_iCurrentCombo = { 0 };

private: /* [ �̵����� ���� ] */
	_bool    m_bWalk = { true };
	_bool    m_bMovable = { true };

	string	 m_strPrevStateName;
	_bool    m_bMove = {};
	_float   m_fMoveTime = {};
	_int	 m_iMoveStep = {};

	unordered_set<string> m_MovableStates = {
		"Walk_BL", "Walk_F", "Walk_FL", "Walk_FR", "Walk_B", "Walk_L", "Walk_R", "Walk_BR",
		"Run_F", "Run_F_Stop", "Run_FR", "Run_FL", "Run_BR", "Run_BL", "Run_B", "Run_L", "Run_R",
		"Sprint", "Sprint_Stop",
		"Guard_Walk_B", "Guard_Walk_F", "Guard_Walk_L", "Guard_Walk_R",
		"EquipWeapon_Walk_F", "PutWeapon_Walk_F",
		"OnLamp_Walk", "FailItem_Walk"

	};

private: /* [ ��Ʈ��� ���� ���� ] */
	_vector  m_PrevWorldDelta = XMVectorZero();
	_vector  m_PrevWorldRotation = XMVectorZero();
	_bool    m_bIsFirstFrame = true;
	_float   m_fRotSmoothSpeed = 8.0f;
	_float   m_fSmoothSpeed = 8.0f;
	_float   m_fSmoothThreshold = 0.1f;

private: // ������ ����
	// stat��
	_int m_iCurrentHP = {};
	_int m_iCurrentStamina = {};
	_int m_iMaxStamina = { 100 };
	// �ϴ� ��ĭ�� 100��
	_int m_iCurrentMana = {};
	_int m_iMaxMana = { 300 };
	
	// ���߿� ���ֱ� 
	_bool m_bSwitch = { true };

	// ����, ���� �������� �ʿ��� ������, ���� ������ �ִ� ������
	_int  m_iLevel = {};
	_int  m_iRequiredErgo = {};
	_int  m_iErgo = {};


// ��Ʈ ���� 
private:
	// ���� ����, �Ʒ� ����
	class CBelt* m_pBelt_Up = { nullptr };
	class CBelt* m_pBelt_Down = { nullptr };

	// ��� ���� �����ߴ���, ������ �������� ��������
	_bool m_isSelectUpBelt = { true };
	class CItem* m_pSelectItem = { nullptr };


public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END

