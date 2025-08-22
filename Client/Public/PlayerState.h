#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "Camera_Manager.h"
#include "LockOn_Manager.h"
#include "DH_ToolMesh.h"
#include "LegionArm_Base.h"

#include "GameInstance.h"

#include "Player.h"
#include "Weapon.h"
#include "Item.h"
#include "Belt.h"

#include "EffectContainer.h"
#include "Effect_Manager.h"

NS_BEGIN(Client)

_float g_fWalkSpeed = 1.5f;
_float g_fRunSpeed = 3.5f;
_float g_fSprintSpeed = 6.f;

/* [ 플레이어의 상태를 정의하는 추상 클래스입니다. ] */
class CPlayerState abstract : public CBase
{
public:
	CPlayerState(CPlayer* pOwner) :
		m_pOwner(pOwner),
		m_pCamera_Manager(CCamera_Manager::Get_Instance()),
		m_pGameInstance(CGameInstance::Get_Instance())
	{
	}
	virtual ~CPlayerState() = default;

public:
	virtual void Enter() PURE;
	virtual void Execute(_float fTimeDelta) PURE;
	virtual void Exit() PURE;

	virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) PURE;
	virtual bool CanExit() const { return true; }
    virtual const _tchar* GetStateName() const = 0;

protected:
    _int    GetCurrentCombo() 
    {
        int result = m_pOwner->m_iCurrentCombo;
        m_pOwner->m_iCurrentCombo = (m_pOwner->m_iCurrentCombo + 1) % 2;
        return result;
    }
	void    SetCurrentCombo(_int Combo)
	{
		m_pOwner->m_iCurrentCombo = Combo;
	}

protected: /* [ 필요 자원 ] */
	_bool   IsStaminaEnough(_float fStaminaCost)
	{
		if (m_pOwner->m_fStamina >= fStaminaCost)
			return true;

		return false;
	}
    _bool   IsManaEnough(_float fManaCost)
    {
        if (m_pOwner->m_fMana >= fManaCost)
            return true;

        return false;
    }
    _bool   IsLegionArmEnergyEnough(_float fLegionArmEnergy)
    {
        if (m_pOwner->m_fLegionArmEnergy >= fLegionArmEnergy)
            return true;

        return false;
    }

protected: /* [ 락온 관련 ] */
    void LockOnMovement()
    {
        if (m_pOwner->m_bIsLockOn)
        {
            if (KEY_PRESSING(DIK_W))
                m_pOwner->m_pAnimator->SetBool("Front", true);
            else
                m_pOwner->m_pAnimator->SetBool("Front", false);

            if (KEY_PRESSING(DIK_A))
                m_pOwner->m_pAnimator->SetBool("Left", true);
            else
                m_pOwner->m_pAnimator->SetBool("Left", false);

            if (KEY_PRESSING(DIK_S))
                m_pOwner->m_pAnimator->SetBool("Back", true);
            else
                m_pOwner->m_pAnimator->SetBool("Back", false);

            if (KEY_PRESSING(DIK_D))
                m_pOwner->m_pAnimator->SetBool("Right", true);
            else
                m_pOwner->m_pAnimator->SetBool("Right", false);

            m_pOwner->m_bSwitchLeft = m_pOwner->m_pAnimator->CheckBool("Left");
            m_pOwner->m_bSwitchRight = m_pOwner->m_pAnimator->CheckBool("Right");
            m_pOwner->m_bSwitchFront = m_pOwner->m_pAnimator->CheckBool("Front");
            m_pOwner->m_bSwitchBack = m_pOwner->m_pAnimator->CheckBool("Back");
        }
    }
    void LockOnMovement4Way()
    {
        if (m_pOwner->m_bIsLockOn)
        {
            static _uint iPressSeq = 0;
            static _uint iTickLeft = 0;
            static _uint iTickRight = 0;
            static _uint iTickFront = 0;
            static _uint iTickBack = 0;

            // A
            if (KEY_DOWN(DIK_A))              iTickLeft = ++iPressSeq;
            else if (!KEY_PRESSING(DIK_A))    iTickLeft = 0;

            // D
            if (KEY_DOWN(DIK_D))              iTickRight = ++iPressSeq;
            else if (!KEY_PRESSING(DIK_D))    iTickRight = 0;

            // W
            if (KEY_DOWN(DIK_W))              iTickFront = ++iPressSeq;
            else if (!KEY_PRESSING(DIK_W))    iTickFront = 0;

            // S
            if (KEY_DOWN(DIK_S))              iTickBack = ++iPressSeq;
            else if (!KEY_PRESSING(DIK_S))    iTickBack = 0;

            const _bool bLeftPressed = (iTickLeft != 0);
            const _bool bRightPressed = (iTickRight != 0);
            const _bool bFrontPressed = (iTickFront != 0);
            const _bool bBackPressed = (iTickBack != 0);

            _bool bSetLeft = false;
            _bool bSetRight = false;
            _bool bSetFront = false;
            _bool bSetBack = false;

            if (bLeftPressed || bRightPressed)
            {
                bSetLeft = bLeftPressed && (!bRightPressed || (iTickLeft <= iTickRight));
                bSetRight = bRightPressed && (!bLeftPressed || (iTickRight < iTickLeft));
            }
            else if (bFrontPressed || bBackPressed)
            {
                bSetFront = bFrontPressed && (!bBackPressed || (iTickFront <= iTickBack));
                bSetBack = bBackPressed && (!bFrontPressed || (iTickBack < iTickFront));
            }

            m_pOwner->m_pAnimator->SetBool("Left", bSetLeft);
            m_pOwner->m_pAnimator->SetBool("Right", bSetRight);
            m_pOwner->m_pAnimator->SetBool("Front", bSetFront);
            m_pOwner->m_pAnimator->SetBool("Back", bSetBack);

            m_pOwner->m_bSwitchLeft = m_pOwner->m_pAnimator->CheckBool("Left");
            m_pOwner->m_bSwitchRight = m_pOwner->m_pAnimator->CheckBool("Right");
            m_pOwner->m_bSwitchFront = m_pOwner->m_pAnimator->CheckBool("Front");
            m_pOwner->m_bSwitchBack = m_pOwner->m_pAnimator->CheckBool("Back");
        }
        else
        {
            m_pOwner->m_pAnimator->SetBool("Left", false);
            m_pOwner->m_pAnimator->SetBool("Right", false);
            m_pOwner->m_pAnimator->SetBool("Front", false);
            m_pOwner->m_pAnimator->SetBool("Back", false);
        }
    }
    void PrintfMoveSwitch()
    {
        printf("Left : %d | Right : %d | Front : %d | Back : %d\n",
            static_cast<_int>(m_pOwner->m_pAnimator->CheckBool("Left")),
            static_cast<_int>(m_pOwner->m_pAnimator->CheckBool("Right")),
            static_cast<_int>(m_pOwner->m_pAnimator->CheckBool("Front")),
            static_cast<_int>(m_pOwner->m_pAnimator->CheckBool("Back")));

    }

protected:
	CPlayer* m_pOwner;

	CCamera_Manager* m_pCamera_Manager = { nullptr };
    CGameInstance* m_pGameInstance = { nullptr };

	_float m_fStateTime = {};

	_bool  m_bDoOnce = {};
	_bool  m_bDoTwo = {};
	_bool  m_bDoThree = {};

public:
	virtual void Free()
	{
		__super::Free();
	}
};

/* [ 이 클래스는 가만히 서있는 상태입니다. ] */
class CPlayer_Idle final : public CPlayerState
{
public:
	CPlayer_Idle(CPlayer* pOwner)
		: CPlayerState(pOwner) {
	}

	virtual ~CPlayer_Idle() = default;

public:
    virtual void Enter() override
    {
		SetCurrentCombo(0);
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        // 아이들 상태는 아무것도 안받음

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (MOUSE_PRESSING(DIM::RBUTTON))
        {
            if (!m_bChargeStarted)
            {
                m_fChargeElapsed += fTimeDelta;

                if (m_fChargeElapsed >= 0.3f)
                    m_bChargeStarted = true; 
            }
        }

        if (KEY_PRESSING(DIK_LCONTROL))
        {
            if (!m_bChargeArm)
            {
                m_fChargeArm += fTimeDelta;

                if (m_fChargeArm >= 0.3f)
                    m_bChargeArm = true;
            }
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bChargeStarted = false;
        m_fChargeElapsed = 0.f;
        m_bChargeArm = false;
        m_fChargeArm = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (input.bShift && m_pOwner->m_bWeaponEquipped) // 가드
            return EPlayerState::GARD;
        
        if (input.bItem) // 아이템 사용
			return EPlayerState::USEITEM;

        if (input.bCtrl) // 왼팔공격
        {
            if (IsLegionArmEnergyEnough(20.f))
                return EPlayerState::ARMATTACKA;
            else
                return EPlayerState::ARMFAIL;
        }

        if (m_bChargeArm && m_pOwner->m_bWeaponEquipped) // 차징
        {
            if (IsLegionArmEnergyEnough(20.f))
                return EPlayerState::ARMATTACKCHARGE;
            else
                return EPlayerState::ARMFAIL;
        }

		if (input.bRightMouseUp && m_pOwner->m_bWeaponEquipped && IsStaminaEnough(1.f)) // 강공
			return EPlayerState::STRONGATTACKA;
        
        if (m_bChargeStarted && m_pOwner->m_bWeaponEquipped && IsStaminaEnough(21.f)) // 차징
			return EPlayerState::CHARGEA;
        
        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped && IsStaminaEnough(1.f)) // 약공
			return EPlayerState::WEAKATTACKA;

        if (input.bTap) // 무기교체
            return EPlayerState::EQUIP;

        if (input.bSkill && m_pOwner->m_bWeaponEquipped && IsManaEnough(100.f))
            return EPlayerState::MAINSKILL;

		if (input.bSpaceDown && IsStaminaEnough(20.f)) // 빽스탭
            return EPlayerState::BACKSTEP;

        if (input.bMove)
        {
            if(m_pOwner->m_bWalk)
                return EPlayerState::WALK;
            else
                return EPlayerState::RUN;
        }

        return EPlayerState::IDLE;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"IDLE";
    }

private:
    unordered_set<string> m_StateNames = {
        "Idle"
    };

private:
    _bool   m_bChargeStarted = {};
    _bool   m_bChargeArm = {};
    _float  m_fChargeElapsed = 0.f;
    _float  m_fChargeArm = 0.f;
};

/* [ 이 클래스는 천천히 걷는 상태입니다. ] */
class CPlayer_Walk final : public CPlayerState
{
public:
    CPlayer_Walk(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_Walk() = default;

public:
    virtual void Enter() override
    {
        m_pOwner->m_pAnimator->SetBool("Sprint", false);
        SetCurrentCombo(0);
        m_fStateTime = 0.f;

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (KEY_PRESSING(DIK_SPACE))
            m_fSpaceHoldTime += fTimeDelta;
        else
            m_fSpaceHoldTime = 0.f;

        if (MOUSE_PRESSING(DIM::RBUTTON))
        {
            if (!m_bChargeStarted)
            {
                m_fChargeElapsed += fTimeDelta;

                if (m_fChargeElapsed >= 0.2f)
                    m_bChargeStarted = true;
            }
        }


        LockOnMovement();        
    }

    virtual void Exit() override
    {
        m_bChargeStarted = false;
        m_fChargeElapsed = 0.f;
        m_fSpaceHoldTime = 0.f;
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);
        
        if (m_fSpaceHoldTime > 0.5f && IsStaminaEnough(40.f))
            return EPlayerState::SPRINT;

        if (input.bShift && m_pOwner->m_bWeaponEquipped) // 가드
            return EPlayerState::GARD;

        if (input.bItem) // 아이템 사용
            return EPlayerState::USEITEM;

        if (input.bSkill && m_pOwner->m_bWeaponEquipped && IsManaEnough(100.f))
            return EPlayerState::MAINSKILL;

        if (input.bCtrl) // 왼팔공격
        {
            if (IsLegionArmEnergyEnough(20.f))
                return EPlayerState::ARMATTACKA;
            else
                return EPlayerState::ARMFAIL;
        }

        if (input.bRightMouseUp && m_pOwner->m_bWeaponEquipped && IsStaminaEnough(1.f)) // 강공
            return EPlayerState::STRONGATTACKA;

        if (m_bChargeStarted && m_pOwner->m_bWeaponEquipped && IsStaminaEnough(21.f)) // 차징
            return EPlayerState::CHARGEA;

        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped && IsStaminaEnough(1.f)) // 약공
            return EPlayerState::WEAKATTACKA;

        if (input.bSpaceUP && IsStaminaEnough(20.f)) // 구르기
            return EPlayerState::ROLLING;

		// 아무것도 안눌리면 Idle로 전환
        if (!input.bMove)
        {
            return EPlayerState::IDLE;
        }

        if (m_pOwner->m_bWalk)
            return EPlayerState::WALK;

        return EPlayerState::RUN;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"WALK";
    }

private:
    unordered_set<string> m_StateNames = {
        "Walk_BL", "Walk_F", "Walk_FL", "Walk_FR", "Walk_B", "Walk_L", "Walk_R", "Walk_BR"
    };

private:
	_float m_fSpaceHoldTime = 0.f;

private:
    _bool   m_bChargeStarted = {};
    _float  m_fChargeElapsed = 0.f;
};

/* [ 이 클래스는 뛰는 상태입니다. ] */
class CPlayer_Run final : public CPlayerState
{
public:
    CPlayer_Run(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_Run() = default;

public:
    virtual void Enter() override
    {
        m_pOwner->m_pAnimator->SetBool("Sprint", false);
        SetCurrentCombo(0);
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_pAnimator->SetBool("Move", true);
        m_pOwner->m_bWalk = false; 

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (KEY_PRESSING(DIK_SPACE))
            m_fSpaceHoldTime += fTimeDelta;
        else
            m_fSpaceHoldTime = 0.f;

        if (MOUSE_PRESSING(DIM::RBUTTON))
        {
            if (!m_bChargeStarted)
            {
                m_fChargeElapsed += fTimeDelta;

                if (m_fChargeElapsed >= 0.2f)
                    m_bChargeStarted = true;
            }
        }


		LockOnMovement();
    }

    virtual void Exit() override
    {
        m_bChargeStarted = false;
        m_fChargeElapsed = 0.f;
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (m_fSpaceHoldTime > 0.5f && IsStaminaEnough(40.f))
            return EPlayerState::SPRINT;

        if (input.bShift && m_pOwner->m_bWeaponEquipped) // 가드
            return EPlayerState::GARD;

        if (input.bItem) // 아이템 사용
            return EPlayerState::USEITEM;

        if (input.bCtrl) // 왼팔공격
        {
            if (IsLegionArmEnergyEnough(20.f))
                return EPlayerState::ARMATTACKA;
            else
                return EPlayerState::ARMFAIL;
        }

        if (input.bSkill && m_pOwner->m_bWeaponEquipped && IsManaEnough(100.f))
            return EPlayerState::MAINSKILL;

        if (input.bRightMouseUp && m_pOwner->m_bWeaponEquipped && IsStaminaEnough(1.f)) // 강공
            return EPlayerState::STRONGATTACKA;

        if (m_bChargeStarted && m_pOwner->m_bWeaponEquipped && IsStaminaEnough(21.f)) // 차징
            return EPlayerState::CHARGEA;

        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped && IsStaminaEnough(1.f)) // 약공
            return EPlayerState::WEAKATTACKA;

        if (input.bSpaceUP && IsStaminaEnough(20.f)) // 구르기
            return EPlayerState::ROLLING;

        if (!input.bMove)
        {
            return EPlayerState::IDLE;
        }
        if (m_pOwner->m_bWalk)
            return EPlayerState::WALK;
        
        return EPlayerState::RUN;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"RUN";
    }

private:
    unordered_set<string> m_StateNames = {
        "Run_F", "Run_F_Stop", "Run_FR", "Run_FL", "Run_BR", "Run_BL", "Run_B", "Run_L", "Run_R"
    };

private:
    _float m_fSpaceHoldTime = 0.f;

private:
    _bool   m_bChargeStarted = {};
    _float  m_fChargeElapsed = 0.f;
};

/* [ 이 클래스는 아이템 사용 상태입니다. ] */
class CPlayer_Item final : public CPlayerState
{
public:
    CPlayer_Item(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_Item() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_fItemTime = 0.f;

         // 아이템 사용 전 걷기 상태였는지 저장
		m_bPreWalk = m_pOwner->m_bWalk;
		m_pOwner->m_bWalk = true;

        /* [ 램프전용 스위치들 ] */
        if (m_pOwner->m_bPulseReservation)
        {
            m_pOwner->m_bPulseReservation = false;

            /* [ 순회하면서 아이템 찾기 ] */
            _bool PulseUse = FindPulseObject();
            
            if (!PulseUse)
            {
                m_pOwner->m_pAnimator->SetBool("Fail", true);
                m_pOwner->m_pAnimator->SetBool("HasLamp", false);
                m_pOwner->m_pAnimator->SetTrigger("UseItem");
            }
            else
            {
                m_pOwner->m_pAnimator->SetBool("HasLamp", false);
                m_pOwner->m_pAnimator->SetTrigger("Pulse");
                m_pOwner->m_pAnimator->SetTrigger("UseItem");
                m_pOwner->m_bUsePulse = true;
            }
        }
        else if (m_pOwner->m_pSelectItem->Get_ProtoTag().find(L"Lamp") != _wstring::npos)
        {
            m_pOwner->m_pWeapon->SetbIsActive(false);
            m_pOwner->m_pAnimator->SetBool("HasLamp", true);
            m_pOwner->m_pAnimator->SetTrigger("UseItem");

            m_pOwner->m_bUseLamp = true;
            m_pOwner->m_bItemSwitch = true;
        }
        else if (m_pOwner->m_pSelectItem->Get_ProtoTag().find(L"Grinder") != _wstring::npos)
        {
            m_pOwner->m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);

            if (m_pOwner->m_bWeaponEquipped)
            {
                m_pOwner->m_pAnimator->SetTrigger("Grinder");
                m_pOwner->m_bUseGrinder = true;
            }
        }
        else if (m_pOwner->m_pSelectItem->Get_ProtoTag().find(L"Portion") != _wstring::npos)
        {
            /* [ 순회하면서 아이템 찾기 ] */
            _bool PulseUse = FindPulseObject();
            
            if (!PulseUse)
                m_pOwner->m_pAnimator->SetBool("Fail", true);

            if (!PulseUse)
            {
                m_pOwner->m_pAnimator->SetBool("Fail", true);
                m_pOwner->m_pAnimator->SetBool("HasLamp", false);
                m_pOwner->m_pAnimator->SetTrigger("UseItem");
            }
            else
            {
                m_pOwner->m_pAnimator->SetBool("HasLamp", false);
                m_pOwner->m_pAnimator->SetTrigger("Pulse");
                m_pOwner->m_pAnimator->SetTrigger("UseItem");
                m_pOwner->m_bUsePulse = true;
            }
        }

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        /* [ 그라인더 : 이동가능 ] */
        if(m_pOwner->m_bUseGrinder)
        {
            if (KEY_PRESSING(DIK_R))
            {
                m_pOwner->m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
                if (!m_pOwner->m_pAnimator->CheckBool("Grinding"))
                {
                    m_pOwner->m_pAnimator->SetTrigger("Grinder");
                }
				m_bGrinderEnd = false;
                m_fGrinderTime = 0.f;
                m_pOwner->m_pAnimator->SetBool("Grinding", true);
            }

            if (KEY_UP(DIK_R))
            {
                m_bGrinderEnd = true;
                m_pOwner->m_pAnimator->SetBool("Grinding", false);
                m_pOwner->m_pSelectItem->Activate(false);
            }

            if (m_bGrinderEnd)
				m_fGrinderTime += fTimeDelta;

            LockOnMovement4Way();
        }

        /* [ 펄스 : 이동가능 ] */
        if (m_pOwner->m_bUsePulse)
        {
            m_fPulseTime += fTimeDelta;
            LockOnMovement();
        }
    }

    virtual void Exit() override
    {
        m_pOwner->m_pAnimator->SetBool("Fail", false);
        m_pOwner->m_pAnimator->SetBool("Grinding", false);
        m_pOwner->m_bUseLamp = false;
        m_pOwner->m_bUseGrinder = false;
		m_pOwner->m_bWalk = m_bPreWalk;
        m_pOwner->m_bUsePulse = false;
        m_fGrinderTime = 0.f;
        m_fPulseTime = 0.f;
        m_fStateTime = 0.f;
        m_bGrinderEnd = false;
        m_bDoOnce = false;

        if (m_pOwner->m_isSelectUpBelt)
            m_pGameInstance->Notify(TEXT("Slot_Belts"), TEXT("UseUpSelectItem"), m_pOwner-> m_pSelectItem);
        else
            m_pGameInstance->Notify(TEXT("Slot_Belts"), TEXT("UseDownSelectItem"), m_pOwner-> m_pSelectItem);
        
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if(1.f < m_fStateTime && m_pOwner->m_bUseLamp)
		{
            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                    return EPlayerState::WALK;
                else
                    return EPlayerState::RUN;
            }
        
            return EPlayerState::IDLE;
		}

        if (1.f < m_fStateTime && m_pOwner->m_bUsePulse)
        {
            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                    return EPlayerState::WALK;
                else
                    return EPlayerState::RUN;
            }

            return EPlayerState::IDLE;
        }

        if (m_bGrinderEnd && m_fGrinderTime > 0.3f)
        {
            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                {
                    return EPlayerState::WALK;
                }
                else
                {
                    return EPlayerState::RUN;
                }
            }

            return EPlayerState::IDLE;
        }
        
        return EPlayerState::USEITEM;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    _bool FindPulseObject()
    {
        _bool bUse = false;
        // 위의 벨트를 먼저 순회한다.
        for (auto Item : m_pOwner->m_pBelt_Up->Get_Items())
        {
            if (Item)
            {
                if (TEXT("Prototype_GameObject_Portion") == Item->Get_ProtoTag())
                {
                    bUse = Item->Get_isUsable();
                }
            }
        }
        //m_pOwner->Callback_UpBelt();

        // 아래의 벨트를 다음으로 순회한다.
        for (auto Items : m_pOwner->m_pBelt_Down->Get_Items())
        {
            if (Items)
            {
                if (TEXT("Prototype_GameObject_Portion") == Items->Get_ProtoTag())
                    bUse = Items->Get_isUsable();
            }
        }
        //m_pOwner->Callback_DownBelt();

        return bUse;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"ITEM";
    }

private:
    unordered_set<string> m_StateNames = {
        "OnLamp", "FailItem"
    };

private:
	_bool m_bPreWalk = { false };
    _bool m_bGrinderEnd = {};
    _float m_fPulseTime = {};
    _float m_fGrinderTime = {};
};

/* [ 이 클래스는 백스탭 상태입니다. ] */
class CPlayer_BackStep final : public CPlayerState
{
public:
    CPlayer_BackStep(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_BackStep() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        LockOnMovement();

        // 백스탭은 무브 OFF 입니다.
        m_pOwner->m_pAnimator->SetBool("Move", false);
        m_pOwner->m_pAnimator->SetTrigger("Dash");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (0.3f < m_fStateTime)
        {
            if (MOUSE_PRESSING(DIM::LBUTTON))
            {
                m_pOwner->m_bBackStepAttack = true;
                m_pOwner->m_pAnimator->SetTrigger("NormalAttack");
            }

            if (KEY_DOWN(DIK_SPACE))
            {
                m_fStateTime = 0.f;
                m_pOwner->m_pAnimator->SetTrigger("Dash");
            }
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (m_pOwner->m_bBackStepAttack)
        {
            if (0.5f < m_fStateTime)
                return EPlayerState::IDLE;
        }
        else
        {
            if (0.4f < m_fStateTime)
                return EPlayerState::IDLE;
        }         

        return EPlayerState::BACKSTEP;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"BACKSTEP";
    }

private:
    unordered_set<string> m_StateNames = {
        "Dash_Normal_B",
        "Dash_Focus_B"
    };
    
};

/* [ 이 클래스는 구르기 상태입니다. ] */
class CPlayer_Rolling final : public CPlayerState
{
public:
    CPlayer_Rolling(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_Rolling() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */

        // 구르기는 무브 ON 입니다.
        m_pOwner->m_pAnimator->SetTrigger("Dash");

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (KEY_DOWN(DIK_SPACE))
        {
            m_fStateTime = 0.f;
            m_pOwner->m_pAnimator->SetTrigger("Dash");
        }

        LockOnMovement();
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);
        
        if (0.4f < m_fStateTime)
        {
            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                    return EPlayerState::WALK;
                else
                    return EPlayerState::RUN;
            }

            return EPlayerState::IDLE;
        }
        

        return EPlayerState::ROLLING;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"ROLLING";
    }

private:
	unordered_set<string> m_StateNames = {
        "Dash_Normal_F",
        "Dash_Focus_F",
        "Dash_Focus_L",
        "Dash_Focus_R"
	};
};

/* [ 이 클래스는 무기장착 상태입니다. ] */
class CPlayer_Equip final : public CPlayerState
{
public:
    CPlayer_Equip(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_Equip() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */

        // 무기장착 모션입니다.
        if (!m_pOwner->m_bWeaponEquipped)
        {
            m_pOwner->m_pAnimator->SetTrigger("EquipWeapon");
            m_pOwner->m_pAnimator->ApplyOverrideAnimController("TwoHand");
     /*       m_pGameInstance->Notify(TEXT("Weapon_Status"), TEXT("EquipWeapon"), m_pOwner->m_pWeapon);*/
        }
        else
        {
            m_pOwner->m_pAnimator->SetTrigger("PutWeapon");
            m_pOwner->m_pAnimator->CancelOverrideAnimController();
        }

        m_pOwner->m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
        m_pOwner->m_bWalk = true;

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if(0.5f < m_fStateTime && !m_pOwner->m_bWeaponEquipped)
        {
            if (!m_bDoOnce)
            {
                m_pOwner->m_bWeaponEquipped = true;
              //  m_pOwner->m_pWeapon->SetbIsActive(true);
				m_bDoOnce = true;
            }
        }
        else if (0.5f < m_fStateTime && m_pOwner->m_bWeaponEquipped)
        {
            if (!m_bDoOnce)
            {
                m_pOwner->m_bWeaponEquipped = false;
               // m_pOwner->m_pWeapon->SetbIsActive(false);
                m_bDoOnce = true;
            }
        }

        _bool bMoving =
            KEY_PRESSING(DIK_W) ||
            KEY_PRESSING(DIK_A) ||
            KEY_PRESSING(DIK_S) ||
            KEY_PRESSING(DIK_D);

        m_pOwner->m_pAnimator->SetBool("Move", bMoving);
    }

    virtual void Exit() override
    {
        m_pOwner->m_bMovable = true;
        m_fStateTime = 0.f;
        m_bDoOnce = false;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {

        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (1.f < m_fStateTime)
        {
            return EPlayerState::IDLE;
        }

        return EPlayerState::EQUIP;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"EQUIP";
    }

private:
    unordered_set<string> m_StateNames = {
        "EquipWeapon_Walk_F", "PutWeapon_Walk_F"
    };
};

/* [ 이 클래스는 스프린트 상태입니다. ] */
class CPlayer_Sprint final : public CPlayerState
{
public:
    CPlayer_Sprint(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_Sprint() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */

        // 스프린트는 무브 ON 입니다.
        m_pOwner->m_pAnimator->SetBool("Sprint", true);

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
    }

    

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (!IsStaminaEnough(10.f))
        {
            m_pOwner->m_pAnimator->SetBool("Sprint", false);
            return EPlayerState::RUN;
        }

        //방향키가 아무것도 안눌렸다.
        if (!input.bMove)
        {
            m_pOwner->m_pAnimator->SetBool("Sprint", false);
            return EPlayerState::IDLE;
        }

        //스페이스바가 떼어졌다.
        if(input.bSpaceUP)
        {
            m_pOwner->m_pAnimator->SetBool("Sprint", false);
            return EPlayerState::RUN;
        }

        //왼클릭 or 우클릭 (약공 , 강공)
        if (input.bLeftMouseDown && IsStaminaEnough(20.f))
            return EPlayerState::SPRINTATTACKA;
        if (input.bRightMouseDown && IsStaminaEnough(20.f))
            return EPlayerState::SPRINTATTACKB;

        return EPlayerState::SPRINT;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"SPRINT";
    }

private:
    unordered_set<string> m_StateNames = {
        "Sprint", "Sprint_Stop"
    };
};


/* [ 이 클래스는 약공 상태입니다. ] */
class CPlayer_WeakAttackA final : public CPlayerState
{
public:
    CPlayer_WeakAttackA(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_WeakAttackA() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */

        // 약공은 무기 장착상태여야합니다.        
        m_pOwner->m_pAnimator->SetInt("Combo", 0);
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (m_fStateTime > 0.2f)
        {
            if (MOUSE_DOWN(DIM::LBUTTON))
                m_bAttackA = true;

            if (MOUSE_DOWN(DIM::RBUTTON))
                m_bAttackB = true;

            if (KEY_DOWN(DIK_LCONTROL))
                m_bArmAttack = true;

            if (KEY_DOWN(DIK_F))
                m_bSkill = true;

            if (KEY_DOWN(DIK_R) && !m_pOwner->m_bPulseReservation)
                m_pOwner->m_bPulseReservation = true;
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bAttackA = false;
        m_bAttackB = false;
        m_bArmAttack = false;
        m_bSkill = false;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (0.8f < m_fStateTime)
        {
            if (m_bAttackA && IsStaminaEnough(1.f))
                return EPlayerState::WEAKATTACKB;
            if (m_bAttackB && IsStaminaEnough(1.f))
                return EPlayerState::STRONGATTACKB;
            if (m_bArmAttack)
            {
                if (IsLegionArmEnergyEnough(20.f))
                    return EPlayerState::ARMATTACKA;
                else
                    return EPlayerState::ARMFAIL;
            }
            if (m_bSkill && IsManaEnough(100.f))
                return EPlayerState::MAINSKILL;

            if (KEY_UP(DIK_SPACE))
                return EPlayerState::BACKSTEP;
        }

        
        if (1.5f < m_fStateTime)
        {
            /* [ 펄스 예약제 ] */
            if (m_pOwner->m_bPulseReservation)
                return EPlayerState::USEITEM;

            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                    return EPlayerState::WALK;
                else
                    return EPlayerState::RUN;
            }

            return EPlayerState::IDLE;
        }

        return EPlayerState::WEAKATTACKA;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"WEAPATTACKA";
    }

private:
    unordered_set<string> m_StateNames = {
       "NormalAttack"
    };

private:
    _bool   m_bAttackA = {};
    _bool   m_bAttackB = {};
    _bool   m_bArmAttack = {};
    _bool   m_bSkill = {};
};
class CPlayer_WeakAttackB final : public CPlayerState
{
public:
    CPlayer_WeakAttackB(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_WeakAttackB() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */

        // 약공은 무기 장착상태여야합니다.
        m_pOwner->m_pAnimator->SetInt("Combo", 1);
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (m_fStateTime > 0.2f)
        {
            if (MOUSE_DOWN(DIM::LBUTTON))
                m_bAttackA = true;
            if (MOUSE_DOWN(DIM::RBUTTON))
                m_bAttackB = true;
            if (KEY_DOWN(DIK_LCONTROL))
                m_bArmAttack = true;
            if (KEY_DOWN(DIK_F))
                m_bSkill = true;
            if (KEY_DOWN(DIK_R) && !m_pOwner->m_bPulseReservation)
                m_pOwner->m_bPulseReservation = true;
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
		m_bAttackA = false;
		m_bAttackB = false;
        m_bArmAttack = false;
        m_bSkill = false;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (0.8f < m_fStateTime)
        {
            if (m_bAttackA && IsStaminaEnough(1.f))
                return EPlayerState::WEAKATTACKA;
            if (m_bAttackB && IsStaminaEnough(1.f))
                return EPlayerState::STRONGATTACKA;
            if (m_bArmAttack)
            {
                if (IsLegionArmEnergyEnough(20.f))
                    return EPlayerState::ARMATTACKA;
                else
                    return EPlayerState::ARMFAIL;
            }
            if (m_bSkill && IsManaEnough(100.f))
                return EPlayerState::MAINSKILL;


            if (KEY_UP(DIK_SPACE))
                return EPlayerState::BACKSTEP;
        }

        if (1.4f < m_fStateTime)
        {
            /* [ 펄스 예약제 ] */
            if (m_pOwner->m_bPulseReservation)
                return EPlayerState::USEITEM;

            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                    return EPlayerState::WALK;
                else
                    return EPlayerState::RUN;
            }
            return EPlayerState::IDLE;
        }

        return EPlayerState::WEAKATTACKB;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"WEAPATTACKB";
    }


private:
    unordered_set<string> m_StateNames = {
        "NormalAttack2"
    };

private:
    _bool   m_bAttackA = {};
    _bool   m_bAttackB = {};
    _bool   m_bArmAttack = {};
    _bool   m_bSkill = {};
};

/* [ 이 클래스는 강공 상태입니다. ] */
class CPlayer_StrongAttackA final : public CPlayerState
{
public:
    CPlayer_StrongAttackA(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_StrongAttackA() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */

        // 강공은 무기 장착상태여야합니다.
        m_pOwner->m_pAnimator->SetInt("Combo", 0);
        m_pOwner->m_pAnimator->SetTrigger("StrongAttack");
        
        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());

       // m_pOwner->m_pWeapon->SetisAttack(true);
       // m_pOwner->m_pWeapon->SetDamageRatio(1.f);
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (m_fStateTime > 0.2f)
        {
            if (MOUSE_DOWN(DIM::RBUTTON))
                m_bAttackB = true;
            if (MOUSE_DOWN(DIM::LBUTTON))
                m_bAttackA = true;
            if (KEY_DOWN(DIK_LCONTROL))
                m_bArmAttack = true;
            if (KEY_DOWN(DIK_F))
                m_bSkill = true;
            if (KEY_DOWN(DIK_R) && !m_pOwner->m_bPulseReservation)
                m_pOwner->m_bPulseReservation = true;
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bAttackB = false;
        m_bAttackA = false;
        m_bArmAttack = false;
        m_bSkill = false;

       // m_pOwner->m_pWeapon->SetisAttack(false);
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (0.75f < m_fStateTime)
        {
            if (m_bAttackB && IsStaminaEnough(1.f))
                return EPlayerState::STRONGATTACKB;
            if (m_bAttackA && IsStaminaEnough(1.f))
                return EPlayerState::WEAKATTACKB;
            if (m_bArmAttack)
            {
                if (IsLegionArmEnergyEnough(20.f))
                    return EPlayerState::ARMATTACKA;
                else
                    return EPlayerState::ARMFAIL;
            }
			if (m_bSkill && IsManaEnough(100.f))
				return EPlayerState::MAINSKILL;


            if (KEY_UP(DIK_SPACE))
                return EPlayerState::BACKSTEP;
        }
        if (1.5f < m_fStateTime)
        {
            /* [ 펄스 예약제 ] */
            if (m_pOwner->m_bPulseReservation)
                return EPlayerState::USEITEM;

            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                    return EPlayerState::WALK;
                else
                    return EPlayerState::RUN;
            }
            return EPlayerState::IDLE;
        }
        
        return EPlayerState::STRONGATTACKA;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"STRONGATTACKA";
    }

private:
    unordered_set<string> m_StateNames = {
        "StrongAttack"
    };

private:
    _bool   m_bAttackA = {};
    _bool   m_bAttackB = {};
    _bool   m_bArmAttack = {};
    _bool   m_bSkill = {};
    
};
class CPlayer_StrongAttackB final : public CPlayerState
{
public:
    CPlayer_StrongAttackB(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_StrongAttackB() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */

        // 강공은 무기 장착상태여야합니다.
        m_pOwner->m_pAnimator->SetInt("Combo", 1);
        m_pOwner->m_pAnimator->SetTrigger("StrongAttack");

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());

      //  m_pOwner->m_pWeapon->SetisAttack(true);
       // m_pOwner->m_pWeapon->SetDamageRatio(1.5f);
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (m_fStateTime > 0.2f)
        {
            if (MOUSE_DOWN(DIM::RBUTTON))
                m_bAttackB = true;
            if (MOUSE_DOWN(DIM::LBUTTON))
                m_bAttackA = true;
            if (KEY_DOWN(DIK_LCONTROL))
                m_bArmAttack = true;
            if (KEY_DOWN(DIK_F))
                m_bSkill = true;
            if (KEY_DOWN(DIK_R) && !m_pOwner->m_bPulseReservation)
                m_pOwner->m_bPulseReservation = true;
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bAttackB = false;
		m_bAttackA = false;
        m_bArmAttack = false;
        m_bSkill = false;

        //m_pOwner->m_pWeapon->SetisAttack(false);
       // m_pOwner->m_pWeapon->SetDamageRatio(1.f);
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (1.f < m_fStateTime)
        {
            if (m_bAttackB && IsStaminaEnough(1.f))
                return EPlayerState::STRONGATTACKA;
            if (m_bAttackA && IsStaminaEnough(1.f))
                return EPlayerState::WEAKATTACKA;
            if (m_bArmAttack)
            {
                if (IsLegionArmEnergyEnough(20.f))
                    return EPlayerState::ARMATTACKA;
                else
                    return EPlayerState::ARMFAIL;
            }
			if (m_bSkill && IsManaEnough(100.f))
				return EPlayerState::MAINSKILL;


            if (KEY_UP(DIK_SPACE))
                return EPlayerState::BACKSTEP;
        }

        if (2.5f < m_fStateTime)
        {
            /* [ 펄스 예약제 ] */
            if (m_pOwner->m_bPulseReservation)
                return EPlayerState::USEITEM;

            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                    return EPlayerState::WALK;
                else
                    return EPlayerState::RUN;
            }
            return EPlayerState::IDLE;
        }

        return EPlayerState::STRONGATTACKB;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"STRONGATTACKB";
    }

private:
    unordered_set<string> m_StateNames = {
        "StrongAttack2"
    };

private:
    _bool   m_bAttackA = {};
    _bool   m_bAttackB = {};
    _bool   m_bArmAttack = {};
    _bool   m_bSkill = {};

};

/* [ 이 클래스는 차징 상태입니다. ] */
class CPlayer_ChargeA final : public CPlayerState
{
public:
    CPlayer_ChargeA(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_ChargeA() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */

        // 차지는 무기 장착상태여야합니다.
        m_pOwner->m_pAnimator->SetInt("Combo", 0);
        m_pOwner->m_pAnimator->SetBool("Charge", true);
        m_pOwner->m_pAnimator->SetTrigger("StrongAttack");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();
        m_pOwner->m_bMovable = false;
        

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (m_fStateTime > 1.f)
        {
            if (KEY_DOWN(DIK_R) && !m_pOwner->m_bPulseReservation)
                m_pOwner->m_bPulseReservation = true;
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;

        m_pOwner->m_bMovable = true;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (2.5f < m_fStateTime)
        {
            if (KEY_UP(DIK_SPACE))
                return EPlayerState::BACKSTEP;

            if (m_pOwner->m_bUseLockon && KEY_UP(DIK_SPACE))
                return EPlayerState::ROLLING;

            if (m_pOwner->m_bIsChange && m_pOwner->m_bWeaponEquipped && IsStaminaEnough(20.f))
            {
                m_pOwner->m_bIsChange = false;
                return EPlayerState::CHARGEB;
            }

            /* [ 펄스 예약제 ] */
            if (m_pOwner->m_bPulseReservation)
                return EPlayerState::USEITEM;

            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                {
                    m_pOwner->m_pAnimator->SetBool("Charge", false);
                    return EPlayerState::WALK;
                }
                else
                {
                    m_pOwner->m_pAnimator->SetBool("Charge", false);
                    return EPlayerState::RUN;
                }
            }
            m_pOwner->m_pAnimator->SetBool("Charge", false);
            return EPlayerState::IDLE;
        }

        return EPlayerState::CHARGEA;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"CHARGEA";
    }

private:
    unordered_set<string> m_StateNames = {
        "ChargeStrongAttack", "ChargeStrongAttack2"
    };

private:
    _bool   m_bAttack = {};

};
class CPlayer_ChargeB final : public CPlayerState
{
public:
    CPlayer_ChargeB(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_ChargeB() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */

        // 차지는 무기 장착상태여야합니다.
        m_pOwner->m_pAnimator->SetInt("Combo", 1);
        m_pOwner->m_pAnimator->SetTrigger("StrongAttack");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();
        m_pOwner->m_bMovable = false;


        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (m_fStateTime)
        {
            if (KEY_DOWN(DIK_R) && !m_pOwner->m_bPulseReservation)
                m_pOwner->m_bPulseReservation = true;
        }
    }

    virtual void Exit() override
    {
        m_pOwner->m_pAnimator->SetBool("Charge", false);
        m_fStateTime = 0.f;

        m_pOwner->m_bMovable = true;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (2.f < m_fStateTime)
        {
            if (KEY_UP(DIK_SPACE))
                return EPlayerState::BACKSTEP;

            /* [ 펄스 예약제 ] */
            if (m_pOwner->m_bPulseReservation)
                return EPlayerState::USEITEM;

            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                    return EPlayerState::WALK;
                else
                    return EPlayerState::RUN;
            }
            return EPlayerState::IDLE;
        }

        return EPlayerState::CHARGEB;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"CHARGEB";
    }

private:
    unordered_set<string> m_StateNames = {
        "ChargeStrongAttack", "ChargeStrongAttack2"
    };

private:
    _bool   m_bAttack = {};

};


/* [ 이 클래스는 가드 상태입니다. ] */
class CPlayer_Gard final : public CPlayerState
{
public:
    CPlayer_Gard(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_Gard() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */

        // 가드는 아이들 , 걷기 상태입니다.
        m_pOwner->m_pAnimator->SetBool("Guard", true);
        m_pOwner->m_bIsGuarding = true;

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        LockOnMovement();

        /* [ 가드중에 피격당했을 시 ] */
        if (m_pOwner->m_bGardHit)
        {
            m_bIsHitAnimation = true;
            m_pOwner->m_bGardHit = false;

            m_pOwner->m_eDir = m_pOwner->ComputeHitDir();
            if (m_pOwner->m_eDir == CPlayer::EHitDir::F ||
                m_pOwner->m_eDir == CPlayer::EHitDir::FL ||
                m_pOwner->m_eDir == CPlayer::EHitDir::FR)
            {
                m_pOwner->m_pAnimator->SetInt("HitDir", 0);
                m_pOwner->m_pAnimator->SetTrigger("Hited");

                /* [ 퍼펙트 가드와 그냥 가드를 분기 ] */
                if (m_fStateTime < 0.3f)
                {
					// 퍼펙트 가드입니다.
                    printf(" 너 퍼펙트 가드성공했어. \n");
                    m_pOwner->m_bPerfectGardDamege = true;
                    m_pOwner->HPSubtract();

                    /*********************************************************/
                    _vector vPos = m_pOwner->m_pTransformCom->Get_State(STATE::POSITION);
                    _vector vDir = XMVector3Normalize(m_pOwner->m_pTransformCom->Get_State(STATE::LOOK));

                    vPos += vDir * 1.5f;
                    _float3 vEffPos = {};
                    XMStoreFloat3(&vEffPos, vPos);
                    vEffPos.y += 0.5f;
                    
                    CEffectContainer::DESC desc = {};

                    XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixScaling(2.f, 2.f, 2.f) * XMMatrixTranslation(vEffPos.x, vEffPos.y, vEffPos.z));

                    CGameObject* pEffect = MAKE_EFFECT(ENUM_CLASS(m_pOwner->m_iLevelID), TEXT("EC_PlayerGuardPerfect_P3S6"), &desc);

                    if (pEffect == nullptr)
                        MSG_BOX("이펙트 생성 실패함");
                    /*********************************************************/
                }
                else
                {
                    // 일반 가드입니다.
                    printf(" 너 가드성공했어. \n");
                    m_pOwner->m_bGardDamege = true;
                    m_pOwner->HPSubtract();

                    /*********************************************************/
                    _vector vPos = m_pOwner->m_pTransformCom->Get_State(STATE::POSITION);
                    _vector vDir = XMVector3Normalize(m_pOwner->m_pTransformCom->Get_State(STATE::LOOK));

                    vPos += vDir * 1.5f;
                    _float3 vEffPos = {};
                    XMStoreFloat3(&vEffPos, vPos);
                    vEffPos.y += 0.5f;

                    CEffectContainer::DESC desc = {};

                    XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixScaling(2.f, 2.f, 2.f) * XMMatrixTranslation(vEffPos.x, vEffPos.y, vEffPos.z));

                    CGameObject* pEffect = MAKE_EFFECT(ENUM_CLASS(m_pOwner->m_iLevelID), TEXT("EC_PlayerGuardNormal_P2"), &desc);

                    if (pEffect == nullptr)
                        MSG_BOX("이펙트 생성 실패함");
                    /*********************************************************/

                }
            }
            else
            {
                //그 외의 방향을 맞으면 피격당한다.
                m_pOwner->HPSubtract();
                switch (m_pOwner->m_eDir)
                {
                case CPlayer::EHitDir::L:
                {
                    printf(" 너 왼쪽에서 맞았어. \n");
                    m_pOwner->m_pAnimator->SetInt("HitDir", 5);
                    m_pOwner->m_pAnimator->SetTrigger("Hited");
                    break;
                }
                case CPlayer::EHitDir::R:
                {
                    printf(" 너 오른쪽에서 맞았어. \n");
                    m_pOwner->m_pAnimator->SetInt("HitDir", 4);
                    m_pOwner->m_pAnimator->SetTrigger("Hited");
                    break;
                }

                default:
                    printf(" 너 뒤쪽에서 맞았어. \n");
                    m_pOwner->m_pAnimator->SetInt("HitDir", 2);
                    m_pOwner->m_pAnimator->SetTrigger("Hited");
                    break;
                }
            }
        }

        if (m_bIsHitAnimation)
            m_fGardTime += fTimeDelta;
    }

    virtual void Exit() override
    {
        m_pOwner->m_pAnimator->SetBool("Guard", false);
        m_pOwner->m_bIsGuarding = false;
        m_bIsHitAnimation = false;
        m_fStateTime = 0.f;
        m_fGardTime = 0.f;

    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);


        //만약 가드상태라면 0.3 초의 대기 상태 이후에 상태가 변경된다.
        if (m_bIsHitAnimation)
        {
            if (m_fGardTime > 0.3f)
            {
                m_bIsHitAnimation = false;
                if (!KEY_PRESSING(DIK_LSHIFT))
                    return EPlayerState::IDLE;
            }
        }
        else
        {
            if (!KEY_PRESSING(DIK_LSHIFT))
                return EPlayerState::IDLE;
        }

        return EPlayerState::GARD;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"GARD";
    }

private:
    unordered_set<string> m_StateNames = {
        "Guard", "Guard_Walk_B", "Guard_Walk_F", "Guard_Walk_L", "Guard_Walk_R", "Guard_Hit", "Guard_Break"
    };

private:
    _bool   m_bAttack = {};
    _bool   m_bMove = {};

    _float  m_fTime = 0.3f;
    _float  m_fDistance = 1.f;

    _float  m_fGardTime = {};

    _bool   m_bIsHitAnimation = {};
};


/* [ 이 클래스는 대쉬공격 상태입니다. ] */
class CPlayer_SprintAttackA final : public CPlayerState
{
public:
    CPlayer_SprintAttackA(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_SprintAttackA() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (1.f < m_fStateTime)
        {
            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                {
                    m_pOwner->m_pAnimator->SetBool("Sprint", false);
                    return EPlayerState::WALK;
                }
                else
                {
                    m_pOwner->m_pAnimator->SetBool("Sprint", false);
                    return EPlayerState::RUN;
                }
            }
            m_pOwner->m_pAnimator->SetBool("Sprint", false);
            return EPlayerState::IDLE;
        }

        return EPlayerState::SPRINTATTACKA;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"SPRINTATTACKA";
    }
};
class CPlayer_SprintAttackB final : public CPlayerState
{
public:
    CPlayer_SprintAttackB(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_SprintAttackB() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_pAnimator->SetTrigger("StrongAttack");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (1.f < m_fStateTime)
        {
            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                {
                    m_pOwner->m_pAnimator->SetBool("Sprint", false);
                    return EPlayerState::WALK;
                }
                else
                {
                    m_pOwner->m_pAnimator->SetBool("Sprint", false);
                    return EPlayerState::RUN;
                }
            }
            m_pOwner->m_pAnimator->SetBool("Sprint", false);
            return EPlayerState::IDLE;
        }

        return EPlayerState::SPRINTATTACKB;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"SPRINTATTACKB";
    }
};

/* [ 이 클래스는 왼팔공격 상태입니다. ] */
class CPlayer_ArmAttackA final : public CPlayerState
{
public:
    CPlayer_ArmAttackA(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_ArmAttackA() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_pAnimator->SetInt("ArmCombo", 0);
        m_pOwner->m_pAnimator->SetTrigger("ArmAttack");

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (m_fStateTime > 0.3f)
        {
            if (MOUSE_DOWN(DIM::LBUTTON))
                m_bAttackA = true;

            if (MOUSE_DOWN(DIM::RBUTTON))
                m_bAttackB = true;

            if (KEY_UP(DIK_LCONTROL))
                m_bArmAttack = true;

            if (KEY_DOWN(DIK_R) && !m_pOwner->m_bPulseReservation)
                m_pOwner->m_bPulseReservation = true;
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bAttackA = false;
        m_bAttackB = false;
        m_bArmAttack = false;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (1.f < m_fStateTime)
        {
            if (KEY_DOWN(DIK_SPACE))
                return EPlayerState::BACKSTEP;

            /* [ 펄스 예약제 ] */
            if (m_pOwner->m_bPulseReservation)
                return EPlayerState::USEITEM;

            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                {
                    return EPlayerState::WALK;
                }
                else
                {
                    return EPlayerState::RUN;
                }
            }
            return EPlayerState::IDLE;
        }

        if (0.5f < m_fStateTime)
        {
            if (m_bAttackA)
                return EPlayerState::WEAKATTACKB;
            if (m_bAttackB)
                return EPlayerState::STRONGATTACKB;
            if (m_bArmAttack)
            {
                if (IsLegionArmEnergyEnough(20.f))
                    return EPlayerState::ARMATTACKB;
                else
                    return EPlayerState::ARMFAIL;
            }
        }

        return EPlayerState::ARMATTACKA;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"ARMATTACKA";
    }

private:
    unordered_set<string> m_StateNames = {
       "Arm_NormalAttack"
    };

	_bool m_bAttackA = {};
	_bool m_bAttackB = {};
	_bool m_bArmAttack = {};
};
class CPlayer_ArmAttackB final : public CPlayerState
{
public:
    CPlayer_ArmAttackB(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_ArmAttackB() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_pAnimator->SetInt("ArmCombo", 1);
        m_pOwner->m_pAnimator->SetTrigger("ArmAttack");

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (MOUSE_DOWN(DIM::LBUTTON))
            m_bAttackA = true;

        if (MOUSE_DOWN(DIM::RBUTTON))
            m_bAttackB = true;

        if (KEY_DOWN(DIK_R) && !m_pOwner->m_bPulseReservation)
            m_pOwner->m_bPulseReservation = true;
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;

        m_bAttackA = false;
        m_bAttackB = false;
        m_bArmAttack = false;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (2.f < m_fStateTime)
        {
            if (KEY_DOWN(DIK_SPACE))
                return EPlayerState::BACKSTEP;

            /* [ 펄스 예약제 ] */
            if (m_pOwner->m_bPulseReservation)
                return EPlayerState::USEITEM;

            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                {
                    return EPlayerState::WALK;
                }
                else
                {
                    return EPlayerState::RUN;
                }
            }
            return EPlayerState::IDLE;
        }

        if (0.5f < m_fStateTime)
        {
            if (m_bAttackA)
                return EPlayerState::WEAKATTACKA;
            if (m_bAttackB)
                return EPlayerState::STRONGATTACKA;
        }

        return EPlayerState::ARMATTACKB;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"ARMATTACKB";
    }

private:
    unordered_set<string> m_StateNames = {
       "Arm_NormalAttack2"
    };

    _bool m_bAttackA = {};
    _bool m_bAttackB = {};
    _bool m_bArmAttack = {};
};
class CPlayer_ArmCharge final : public CPlayerState
{
public:
    CPlayer_ArmCharge(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_ArmCharge() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_pAnimator->SetBool("Charge", true);
        m_pOwner->m_pAnimator->SetTrigger("ArmAttack");

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (m_fStateTime > 1.f)
        {
            if (KEY_DOWN(DIK_R) && !m_pOwner->m_bPulseReservation)
                m_pOwner->m_bPulseReservation = true;
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;

        m_pOwner->m_pAnimator->SetBool("Charge", false);
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (1.5f < m_fStateTime)
        {
            if (KEY_DOWN(DIK_SPACE))
                return EPlayerState::BACKSTEP;

            /* [ 펄스 예약제 ] */
            if (m_pOwner->m_bPulseReservation)
                return EPlayerState::USEITEM;

            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                {
                    return EPlayerState::WALK;
                }
                else
                {
                    return EPlayerState::RUN;
                }
            }
            return EPlayerState::IDLE;
        }

        return EPlayerState::ARMATTACKCHARGE;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"ARMATTACKCHARGE";
    }

private:
    unordered_set<string> m_StateNames = {
       "Arm_ChargeAttack"
    };

};
class CPlayer_ArmFail final : public CPlayerState
{
public:
    CPlayer_ArmFail(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_ArmFail() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_pAnimator->SetBool("Fail", true);
        m_pOwner->m_pAnimator->SetTrigger("ArmAttack");

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;

        m_pOwner->m_pAnimator->SetBool("Fail", false);
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (2.f < m_fStateTime)
        {
            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                {
                    return EPlayerState::WALK;
                }
                else
                {
                    return EPlayerState::RUN;
                }
            }
            return EPlayerState::IDLE;
        }

        return EPlayerState::ARMFAIL;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"ARMATTACKFAIL";
    }

private:
    unordered_set<string> m_StateNames = {
       "Fail_Arm"
    };

};

/* [ 이 클래스는 스킬 상태입니다. ] */
class CPlayer_MainSkill final : public CPlayerState
{
public:
    CPlayer_MainSkill(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_MainSkill() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_pAnimator->SetTrigger("MainSkill");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();
        m_pOwner->m_fMana -= 100.f;
        m_pOwner->Callback_Mana();

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        //1. F를 다시 눌렀을 경우 최대 3콤보까지 진행이된다.
        if (0.3f < m_fStateTime && m_iSkillCount == 0 && IsManaEnough(100.f))
        {
            if (KEY_DOWN(DIK_F))
            {
                m_pOwner->m_pAnimator->SetTrigger("MainSkill");
                m_pOwner->m_fMana -= 100.f;
                m_pOwner->Callback_Mana();
                m_iSkillCount++;
            }

            if (MOUSE_DOWN(DIM::LBUTTON))
				m_bAttackA = true;
            else if (MOUSE_DOWN(DIM::RBUTTON))
				m_bAttackB = true;
        }
        else if (1.f < m_fStateTime && m_iSkillCount == 1 && IsManaEnough(100.f))
        {
            if (KEY_DOWN(DIK_F))
            {
                m_pOwner->m_pAnimator->SetTrigger("MainSkill");
                m_pOwner->m_fMana -= 100.f;
                m_pOwner->Callback_Mana();
                m_iSkillCount++;
            }

            if (MOUSE_DOWN(DIM::LBUTTON))
                m_bAttackA = true;
            else if (MOUSE_DOWN(DIM::RBUTTON))
                m_bAttackB = true;
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_iSkillCount = 0;
        m_bAttackA = false;
        m_bAttackB = false;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */

        //1. 스킬의 진행도에 따라 탈출 조건이 달라진다.
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (1.f < m_fStateTime && m_iSkillCount == 0)
        {
            if (KEY_DOWN(DIK_SPACE))
                return EPlayerState::BACKSTEP;

            if(m_bAttackA)
				return EPlayerState::WEAKATTACKA;
			if (m_bAttackB)
				return EPlayerState::STRONGATTACKA;

            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                {
                    return EPlayerState::WALK;
                }
                else
                {
                    return EPlayerState::RUN;
                }
            }
            return EPlayerState::IDLE;
        }
        if (2.f < m_fStateTime && m_iSkillCount == 1)
        {
            if (KEY_DOWN(DIK_SPACE))
                return EPlayerState::BACKSTEP;

            if (m_bAttackA)
                return EPlayerState::WEAKATTACKA;
            if (m_bAttackB)
                return EPlayerState::STRONGATTACKA;

            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                {
                    return EPlayerState::WALK;
                }
                else
                {
                    return EPlayerState::RUN;
                }
            }
            return EPlayerState::IDLE;
        }
        if (3.5f < m_fStateTime && m_iSkillCount >= 2)
        {
            if (KEY_DOWN(DIK_SPACE))
                return EPlayerState::BACKSTEP;

            if (m_bAttackA)
                return EPlayerState::WEAKATTACKA;
            if (m_bAttackB)
                return EPlayerState::STRONGATTACKA;

            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                {
                    return EPlayerState::WALK;
                }
                else
                {
                    return EPlayerState::RUN;
                }
            }
            return EPlayerState::IDLE;
        }

        return EPlayerState::MAINSKILL;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"MAINSKILL";
    }

private:
    unordered_set<string> m_StateNames = {
       "MainSkill", "MainSkill2", "MainSkill3"
    };

	_int m_iSkillCount = 0;
	_bool m_bAttackA = false;
	_bool m_bAttackB = false;
};

/* [ 이 클래스는 피격 상태입니다. ] */
class CPlayer_Hited final : public CPlayerState
{

public:
    CPlayer_Hited(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_Hited() = default;

    /* [ 가드가 아닌 정말 데미지를 입었을 때의 상태입니다. ] */
public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 어느방향에서 맞는지 설정하기 ] */
        m_pOwner->m_eDir = m_pOwner->ComputeHitDir();
        if (m_pOwner->m_eDir == CPlayer::EHitDir::F){ m_pOwner->m_pAnimator->SetInt("HitDir", 0); }
        else if (m_pOwner->m_eDir == CPlayer::EHitDir::FL){ m_pOwner->m_pAnimator->SetInt("HitDir", 3); }
		else if (m_pOwner->m_eDir == CPlayer::EHitDir::FR) { m_pOwner->m_pAnimator->SetInt("HitDir", 1); }
		else if (m_pOwner->m_eDir == CPlayer::EHitDir::L) { m_pOwner->m_pAnimator->SetInt("HitDir", 5); }
		else if (m_pOwner->m_eDir == CPlayer::EHitDir::R) { m_pOwner->m_pAnimator->SetInt("HitDir", 4); }
        else if (m_pOwner->m_eDir == CPlayer::EHitDir::BL) { m_pOwner->m_pAnimator->SetInt("HitDir", 2); }
        else if (m_pOwner->m_eDir == CPlayer::EHitDir::BR) { m_pOwner->m_pAnimator->SetInt("HitDir", 2); }
		else if (m_pOwner->m_eDir == CPlayer::EHitDir::B) { m_pOwner->m_pAnimator->SetInt("HitDir", 2); }
        m_pOwner->m_pAnimator->SetTrigger("Hited");
        m_pOwner->HPSubtract();

        printf(" 너 맞은방향 FtoB, R, B, L, RtoL, LtoR 중에 %d 방향이야. \n", static_cast<int>(m_pOwner->m_eDir));

        if (m_pOwner->m_pWeapon)
        {
            m_pOwner->m_pWeapon->SetisAttack(false);
            m_pOwner->m_pWeapon->Set_WeaponTrail_Active(false);
        }
        if (m_pOwner->m_pLegionArm)
            m_pOwner->m_pLegionArm->SetisAttack(false);

        m_pOwner->m_pAnimator->SetBool("Front", false);
        m_pOwner->m_pAnimator->SetBool("Left", false);
        m_pOwner->m_pAnimator->SetBool("Right", false);
        m_pOwner->m_pAnimator->SetBool("Back", false);

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */

        //1. 스킬의 진행도에 따라 탈출 조건이 달라진다.
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (0.4f < m_fStateTime)
        {
            if (input.bMove)
            {
                if (m_pOwner->m_bWalk)
                {
                    return EPlayerState::WALK;
                }
                else
                {
                    return EPlayerState::RUN;
                }
            }
            return EPlayerState::IDLE;
        }

        return EPlayerState::HITED;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"HITED";
    }

private:
    unordered_set<string> m_StateNames = {
       "Hit_FtoB", "Hit_R", "Hit_B", "Hit_L", "Hit_RtoL", "Hit_LtoR"
    };

};



NS_END