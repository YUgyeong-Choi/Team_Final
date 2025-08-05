#pragma once
#include "Unit.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CPhysXController;
NS_END

NS_BEGIN(Client)
class CPlayerState;

class CPlayer : public CUnit
{
public:
	typedef struct tagPlayerDesc : public CUnit::tagUnitDesc
	{
	}PLAYER_DESC;

	struct InputContext {
		/* [ W,A,S,D �Է� ] */
		_bool bUp;
		_bool bDown;
		_bool bLeft;
		_bool bRight;

		_bool bUp_Pressing;
		_bool bDown_Pressing;
		_bool bLeft_Pressing;
		_bool bRight_Pressing;

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
	void			TriggerStateEffects();							// [4] �ִϸ��̼� ����

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
	void Callback_UpBelt();
	void Callback_DownBelt();
	void Use_Item();

	void Update_Slot();

private: /* [ �������� ] */
	void ReadyForState();
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
	friend class CPlayer_Charge;
	friend class CPlayer_Gard;


private: /* [ ���� ���� ] */
	EPlayerState  m_pPreviousState = { EPlayerState::END };
	EPlayerState  m_eCurrentState = { EPlayerState::IDLE };

	CPlayerState* m_pCurrentState = { nullptr };
	CPlayerState* m_pStateArray[ENUM_CLASS(EPlayerState::END)] = { nullptr };

protected:
	class CCamera_Orbital* m_pCamera_Orbital = { nullptr };
private:
	CPhysXController* m_pControllerCom = { nullptr };
private: /* [ �׸��� ���� ] */
	_vector m_vShadowCam_Eye = {};
	_vector m_vShadowCam_At = {};

private: /* [ ������ �� �ִ� ��ü ] */
	CGameObject* m_pTarget = { nullptr };
	CGameObject* m_pWeapon = { nullptr };

private: /* [ ���ݰ��� ���� ] */
	_bool	m_bWeaponEquipped = { false };

private: /* [ �̵����� ���� ] */
	_bool    m_bWalk = { true };
	_bool    m_bMovable = { true };

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
	_int m_iMaxHP = { 200 };
	_int m_iCurrentStamina = {};
	_int m_iMaxStamina = { 100 };
	// �ϴ� ��ĭ�� 100��
	_int m_iCurrentMana = {};
	_int m_iMaxMana = { 300 };

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

