#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "Camera_Manager.h"
#include "DH_ToolMesh.h"

#include "GameInstance.h"

#include "Player.h"
#include "Weapon.h"
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

        if (input.bTap) // 실드
            return EPlayerState::EQUIP;

        if (input.bShift && m_pOwner->m_bWeaponEquipped) // 가드
            return EPlayerState::GARD;
        
        if (input.bItem) // 아이템 사용
			return EPlayerState::USEITEM;

        if (input.bCtrl) // 왼팔공격
			return EPlayerState::ARMATTACKA;

        if (m_bChargeArm && m_pOwner->m_bWeaponEquipped) // 차징
            return EPlayerState::ARMATTACKCHARGE;

		if (input.bRightMouseUp && m_pOwner->m_bWeaponEquipped) // 강공
			return EPlayerState::STRONGATTACKA;
        
        if (m_bChargeStarted && m_pOwner->m_bWeaponEquipped) // 차징
			return EPlayerState::CHARGEA;
        
        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped) // 약공
			return EPlayerState::WEAKATTACKA;

        if (input.bTap) // 무기교체
            return EPlayerState::SWITCHWEAPON;

        if (input.bSkill)
            return EPlayerState::MAINSKILL;

		if (input.bSpaceDown) // 빽스탭
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
        
        if (m_fSpaceHoldTime > 0.5f)
            return EPlayerState::SPRINT;

        if (input.bShift && m_pOwner->m_bWeaponEquipped) // 가드
            return EPlayerState::GARD;

        if (input.bItem) // 아이템 사용
            return EPlayerState::USEITEM;

        if (input.bSkill)
            return EPlayerState::MAINSKILL;

        if (input.bCtrl) // 컨트롤 왼팔공격
            return EPlayerState::ARMATTACKA;

        if (input.bRightMouseUp && m_pOwner->m_bWeaponEquipped) // 강공
            return EPlayerState::STRONGATTACKA;

        if (m_bChargeStarted && m_pOwner->m_bWeaponEquipped) // 차징
            return EPlayerState::CHARGEA;

        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped) // 약공
            return EPlayerState::WEAKATTACKA;

        if (input.bSpaceUP) // 구르기
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
        m_pOwner->m_pAnimator->SetBool("Run", true);
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
    }

    virtual void Exit() override
    {
        m_pOwner->m_pAnimator->SetBool("Run", false);
        m_bChargeStarted = false;
        m_fChargeElapsed = 0.f;
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (m_fSpaceHoldTime > 0.5f)
            return EPlayerState::SPRINT;

        if (input.bShift && m_pOwner->m_bWeaponEquipped) // 가드
            return EPlayerState::GARD;

        if (input.bItem) // 아이템 사용
            return EPlayerState::USEITEM;

        if (input.bCtrl) // 컨트롤 왼팔공격
            return EPlayerState::ARMATTACKA;

        if (input.bSkill)
            return EPlayerState::MAINSKILL;

        if (input.bRightMouseUp && m_pOwner->m_bWeaponEquipped) // 강공
            return EPlayerState::STRONGATTACKA;

        if (m_bChargeStarted && m_pOwner->m_bWeaponEquipped) // 차징
            return EPlayerState::CHARGEA;

        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped) // 약공
            return EPlayerState::WEAKATTACKA;

        if (input.bSpaceUP) // 구르기
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

         // 아이템 사용 전 걷기 상태였는지 저장
        m_pOwner->m_pAnimator->SetBool("HasLamp", true);
        m_pOwner->m_pAnimator->SetTrigger("UseItem");
		m_bPreWalk = m_pOwner->m_bWalk;
		m_pOwner->m_bWalk = true;
        m_pOwner->m_pWeapon->SetbIsActive(false);
        m_pOwner->m_bItemSwitch = true;
        m_pOwner->m_fItemTime = 0.f;
        
        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (1.f < m_fStateTime && !m_bDoOnce)
        {
            // 램프 온오프 토글
			m_pOwner->m_bLampOnOff = !m_pOwner->m_bLampOnOff;
			m_bDoOnce = true;
        }
    }

    virtual void Exit() override
    {
		m_pOwner->m_bWalk = m_bPreWalk;
        m_fStateTime = 0.f;
        m_bDoOnce = false;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if(1.f < m_fStateTime)
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
        
        return EPlayerState::USEITEM;
    }

    virtual bool CanExit() const override
    {
        return true;
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
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);
        
        string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
        if (m_StateNames.find(strName) != m_StateNames.end())
        {
            if (0.6f < m_fStateTime)
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
        }
        else
        {
            m_pOwner->m_pAnimator->SetTrigger("PutWeapon");
            m_pOwner->m_pAnimator->CancelOverrideAnimController();
        }

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
                m_pOwner->m_pWeapon->SetbIsActive(true);
				m_bDoOnce = true;
            }
        }
        else if (0.5f < m_fStateTime && m_pOwner->m_bWeaponEquipped)
        {
            if (!m_bDoOnce)
            {
                m_pOwner->m_bWeaponEquipped = false;
                m_pOwner->m_pWeapon->SetbIsActive(false);
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
        if (input.bLeftMouseDown)
            return EPlayerState::SPRINTATTACKA;
        if (input.bRightMouseDown)
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
        m_pOwner->m_pAnimator->SetInt("Combo", GetCurrentCombo());
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (0.8f < m_fStateTime)
        {
            string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
            if (m_StateNames.find(strName) != m_StateNames.end())
            {
                if (MOUSE_DOWN(DIM::LBUTTON))
                    m_bAttackA = true;

                if (MOUSE_DOWN(DIM::RBUTTON))
                    m_bAttackB = true;

                if (KEY_DOWN(DIK_LCONTROL))
                    m_bArmAttack = true;

                if (KEY_DOWN(DIK_F))
                    m_bSkill = true;
            }
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
            if (m_bAttackA)
                return EPlayerState::WEAKATTACKB;
            if (m_bAttackB)
                return EPlayerState::STRONGATTACKB;
            if (m_bSkill)
                return EPlayerState::MAINSKILL;
        }

        
        if (1.5f < m_fStateTime)
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
        m_pOwner->m_pAnimator->SetInt("Combo", GetCurrentCombo());
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        /* 공격 애니메이션이 0.8 이상 진행되었을 때 */
        if (0.8f < m_fStateTime)
        {
            string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
            if (m_StateNames.find(strName) != m_StateNames.end())
            {
                if (MOUSE_DOWN(DIM::LBUTTON))
                    m_bAttackA = true;
                if (MOUSE_DOWN(DIM::RBUTTON))
                    m_bAttackB = true;
                if (KEY_DOWN(DIK_LCONTROL))
					m_bArmAttack = true;
                if (KEY_DOWN(DIK_F))
                    m_bSkill = true;
            }
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
            if (m_bAttackA)
                return EPlayerState::WEAKATTACKB;
            if (m_bAttackB)
                return EPlayerState::STRONGATTACKB;
            if (m_bArmAttack)
                return EPlayerState::ARMATTACKA;
            if (m_bSkill)
                return EPlayerState::MAINSKILL;

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
        m_pOwner->m_pAnimator->SetInt("Combo", GetCurrentCombo());
        m_pOwner->m_pAnimator->SetTrigger("StrongAttack");
        
        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());

       // m_pOwner->m_pWeapon->SetisAttack(true);
       // m_pOwner->m_pWeapon->SetDamageRatio(1.f);
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        /* 공격 애니메이션이 0.7 이상 진행되었을 때 */
        if (0.7f < m_fStateTime)
        {
            string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
            if (m_StateNames.find(strName) != m_StateNames.end())
            {
                if (MOUSE_DOWN(DIM::RBUTTON))
                    m_bAttackB = true;
                if (MOUSE_DOWN(DIM::LBUTTON))
                    m_bAttackA = true;
                if (KEY_DOWN(DIK_LCONTROL))
                    m_bArmAttack = true;
				if (KEY_DOWN(DIK_F))
					m_bSkill = true;
            }
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
            if (m_bAttackB)
                return EPlayerState::STRONGATTACKB;
            if (m_bAttackA)
                return EPlayerState::WEAKATTACKB;
            if (m_bArmAttack)
                return EPlayerState::ARMATTACKA;
			if (m_bSkill)
				return EPlayerState::MAINSKILL;
        }
        if (1.5f < m_fStateTime)
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
        m_pOwner->m_pAnimator->SetInt("Combo", GetCurrentCombo());
        m_pOwner->m_pAnimator->SetTrigger("StrongAttack");

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());

      //  m_pOwner->m_pWeapon->SetisAttack(true);
       // m_pOwner->m_pWeapon->SetDamageRatio(1.5f);
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        /* 공격 애니메이션이 0.7 이상 진행되었을 때 */
        if (0.7f < m_fStateTime)
        {
            string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
            if (m_StateNames.find(strName) != m_StateNames.end())
            {
                if (MOUSE_DOWN(DIM::RBUTTON))
                    m_bAttackB = true;
                if (MOUSE_DOWN(DIM::LBUTTON))
                    m_bAttackA = true;
                if (KEY_DOWN(DIK_LCONTROL))
                    m_bArmAttack = true;
				if (KEY_DOWN(DIK_F))
					m_bSkill = true;
            }
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
            if (m_bAttackB)
                return EPlayerState::STRONGATTACKA;
            if (m_bAttackA)
                return EPlayerState::WEAKATTACKA;
            if (m_bArmAttack)
                return EPlayerState::ARMATTACKA;
			if (m_bSkill)
				return EPlayerState::MAINSKILL;
        }

        if (2.5f < m_fStateTime)
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
            if (m_pOwner->m_bIsChange && m_pOwner->m_bWeaponEquipped)
            {
                m_pOwner->m_bIsChange = false;
                return EPlayerState::CHARGEB;
            }

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

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;
    }

    virtual void Exit() override
    {
        m_pOwner->m_pAnimator->SetBool("Guard", false);
        m_fStateTime = 0.f;

    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (!KEY_PRESSING(DIK_LSHIFT))
            return EPlayerState::IDLE;

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
        m_pOwner->m_pTransformCom->SetbSpecialMoving();

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (0.5f < m_fStateTime)
        {
            string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
            if (m_StateNames.find(strName) != m_StateNames.end())
            {
                if (MOUSE_DOWN(DIM::LBUTTON))
                    m_bAttackA = true;

                if (MOUSE_DOWN(DIM::RBUTTON))
                    m_bAttackB = true;

                if (KEY_UP(DIK_LCONTROL))
                    m_bArmAttack = true;
            }
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
                return EPlayerState::ARMATTACKB;
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
        m_pOwner->m_pTransformCom->SetbSpecialMoving();

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (0.5f < m_fStateTime)
        {
            string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
            if (m_StateNames.find(strName) != m_StateNames.end())
            {
                if (MOUSE_DOWN(DIM::LBUTTON))
                    m_bAttackA = true;

                if (MOUSE_DOWN(DIM::RBUTTON))
                    m_bAttackB = true;
            }
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

        m_pOwner->m_pAnimator->SetBool("Charge", false);
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

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        //1. F를 다시 눌렀을 경우 최대 3콤보까지 진행이된다.
        if (0.5f < m_fStateTime && m_iSkillCount == 0)
        {
            if (KEY_DOWN(DIK_F))
            {
                m_pOwner->m_pAnimator->SetTrigger("MainSkill");
                m_iSkillCount++;
            }

            if (MOUSE_DOWN(DIM::LBUTTON))
				m_bAttackA = true;
            else if (MOUSE_DOWN(DIM::RBUTTON))
				m_bAttackB = true;
        }
        else if (1.5f < m_fStateTime && m_iSkillCount == 1)
        {
            if (KEY_DOWN(DIK_F))
            {
                m_pOwner->m_pAnimator->SetTrigger("MainSkill");
                m_iSkillCount++;
            }

            if (MOUSE_DOWN(DIM::LBUTTON))
                m_bAttackA = true;
            else if (MOUSE_DOWN(DIM::RBUTTON))
                m_bAttackB = true;
        }
        else if (2.5f < m_fStateTime && m_iSkillCount == 2)
        {
            if (KEY_DOWN(DIK_F))
            {
                m_pOwner->m_pAnimator->SetTrigger("MainSkill");
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

NS_END