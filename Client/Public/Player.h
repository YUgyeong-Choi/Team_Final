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
		/* [ W,A,S,D 입력 ] */
		_bool bUp;
		_bool bDown;
		_bool bLeft;
		_bool bRight;

		_bool bUp_Pressing;
		_bool bDown_Pressing;
		_bool bLeft_Pressing;
		_bool bRight_Pressing;

		/* [ 마우스 입력 ] */
		_bool bLeftMouseDown;
		_bool bRightMouseDown;
		_bool bRightMousePress;
		_bool bRightMouseUp;

		/* [ 특수키 입력 ] */
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

/* [ 입력 처리 ] */
private: 
	void			HandleInput();									// [1] 키 입력만 처리
	EPlayerState	EvaluateTransitions();							// [2] 입력에 따라 상태 전이
	void			UpdateCurrentState(_float fTimeDelta);			// [3] 현재 상태 로직 수행
	void			TriggerStateEffects();							// [4] 애니메이션 적용

private: /* [ 루트모션 활성화 ] */
	void			RootMotionActive(_float fTimeDelta);
	

private:/* [ 캐스케이드 전용함수 ] */
	HRESULT UpdateShadowCamera();

private: /* [ 이동로직 ] */
	void SetMoveState(_float fTimeDelta);
	void Movement(_float fTimeDelta);
	void ToggleWalkRun() { m_bWalk = !m_bWalk; }
	void SyncTransformWithController(); // 위치동기화(컨트롤러)
private: /* [ Setup 함수 ] */
	HRESULT Ready_Components();
	HRESULT Ready_Controller();
	void LoadPlayerFromJson();

private: /* 옵저버 관련*/
	// 테스트 용이라 나중에 함수에 넣는 식으로 바꾸기
	void Callback_HP();
	void Callback_Stamina();
	void Callback_Mana();

	// 스탯 변화 테스트용
	void Update_Stat();

	
private: // 슬롯 용
	void Callback_UpBelt();
	void Callback_DownBelt();
	void Use_Item();

	void Update_Slot();

private: /* [ 상태패턴 ] */
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


private: /* [ 상태 변수 ] */
	EPlayerState  m_pPreviousState = { EPlayerState::END };
	EPlayerState  m_eCurrentState = { EPlayerState::IDLE };

	CPlayerState* m_pCurrentState = { nullptr };
	CPlayerState* m_pStateArray[ENUM_CLASS(EPlayerState::END)] = { nullptr };

protected:
	class CCamera_Orbital* m_pCamera_Orbital = { nullptr };
private:
	CPhysXController* m_pControllerCom = { nullptr };
private: /* [ 그림자 변수 ] */
	_vector m_vShadowCam_Eye = {};
	_vector m_vShadowCam_At = {};

private: /* [ 소유할 수 있는 객체 ] */
	CGameObject* m_pTarget = { nullptr };
	CGameObject* m_pWeapon = { nullptr };

private: /* [ 공격관련 변수 ] */
	_bool	m_bWeaponEquipped = { false };

private: /* [ 이동관련 변수 ] */
	_bool    m_bWalk = { true };
	_bool    m_bMovable = { true };

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
	_int m_iMaxHP = { 200 };
	_int m_iCurrentStamina = {};
	_int m_iMaxStamina = { 100 };
	// 일단 한칸에 100씩
	_int m_iCurrentMana = {};
	_int m_iMaxMana = { 300 };

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

