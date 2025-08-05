#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "Camera_Manager.h"

#include "GameInstance.h"

#include "Player.h"
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

                if (m_fChargeElapsed >= 0.2f)
                    m_bChargeStarted = true; 
            }
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bChargeStarted = false;
        m_fChargeElapsed = 0.f;
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

        if (input.bCtrl) // 컨트롤 왼팔공격
			return EPlayerState::LEFTHANDATTACK;

		if (input.bRightMouseUp && m_pOwner->m_bWeaponEquipped) // 강공
			return EPlayerState::STRONGATTACKA;
        
        if (m_bChargeStarted && m_pOwner->m_bWeaponEquipped) // 차징
			return EPlayerState::CHARGEA;
        
        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped) // 약공
			return EPlayerState::WEAKATTACKA;

        if (input.bTap) // 무기교체
            return EPlayerState::SWITCHWEAPON;

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
    _float  m_fChargeElapsed = 0.f;
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
        SetCurrentCombo(0);
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(g_fWalkSpeed);

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

        if (input.bCtrl) // 컨트롤 왼팔공격
            return EPlayerState::LEFTHANDATTACK;

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
        SetCurrentCombo(0);
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_pAnimator->SetBool("Move", true);
        m_pOwner->m_pAnimator->SetBool("Run", true);
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(g_fRunSpeed);
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
            return EPlayerState::LEFTHANDATTACK;

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
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(g_fWalkSpeed);
		m_bPreWalk = m_pOwner->m_bWalk;
		m_pOwner->m_bWalk = true;

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;
    }

    virtual void Exit() override
    {
		m_pOwner->m_bWalk = m_bPreWalk;
        m_fStateTime = 0.f;
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

        // 백스탭 이동
        if (!m_bMove)
        {
            _vector vLook = m_pOwner->m_pTransformCom->Get_State(STATE::LOOK) * -1.f;
            
            string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
            if (m_StateNames.find(strName) != m_StateNames.end())
            {
                m_bMove = m_pOwner->m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pOwner->m_pControllerCom);
                m_pOwner->SyncTransformWithController();
            }
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bMove = false;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
        if (m_StateNames.find(strName) != m_StateNames.end())
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

private:
    _bool   m_bMove = {};
    _float  m_fTime = 0.3f;
    _float  m_fDistance = 3.f;
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
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(6.f);

        m_fPreSpeed = m_pOwner->Get_TransfomCom()->GetfSpeedPerSec();

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;
		
        if (!m_bMove)
        {            
            string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
            if (m_StateNames.find(strName) != m_StateNames.end())
            {
                m_pOwner->m_pTransformCom->Go_Front(fTimeDelta, m_pOwner->m_pControllerCom);
                m_pOwner->SyncTransformWithController();
            }
        }
    }

    virtual void Exit() override
    {
		m_pOwner->m_pTransformCom->Set_SpeedPreSec(m_fPreSpeed);
        m_bMove = false;

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

private:
    _bool   m_bMove = {};
    _float  m_fTime = 0.3f;
    _float  m_fDistance = 2.f;
    _float  m_fPreSpeed = 1.f;
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
        m_pOwner->m_pAnimator->SetTrigger("EquipWeapon");
        m_pOwner->m_pAnimator->ApplyOverrideAnimController("TwoHand");
        m_pOwner->m_bMovable = false;
        m_pOwner->m_bWeaponEquipped = true;

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;
    }

    virtual void Exit() override
    {
        m_pOwner->m_bMovable = true;
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (m_pOwner->m_pAnimator->IsFinished())
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
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(g_fSprintSpeed);

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

        m_pOwner->m_pAnimator->SetBool("Sprint", false);
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        //방향키가 아무것도 안눌렸다.
        if (!input.bMove)
        {
            return EPlayerState::IDLE;
        }

        //스페이스바가 떼어졌다.
        if(input.bSpaceUP)
            return EPlayerState::RUN;

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
        _int iCurrentCombo = GetCurrentCombo();
        _int iCombo = (iCurrentCombo + 1) % 2;
        m_pOwner->m_pAnimator->SetInt("Combo", iCombo);
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();

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
            }
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bAttackA = false;
        m_bAttackB = false;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (0.8f < m_fStateTime)
        {
            string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
            if (m_StateNames.find(strName) != m_StateNames.end())
            {
                if (m_bAttackA)
                    return EPlayerState::WEAKATTACKB;
                if (m_bAttackB)
                    return EPlayerState::STRONGATTACKB;
            }
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
        _int iCurrentCombo = GetCurrentCombo();
        _int iCombo = (iCurrentCombo + 1) % 2;
        m_pOwner->m_pAnimator->SetInt("Combo", iCombo);
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();

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
            }
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
		m_bAttackA = false;
		m_bAttackB = false;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (0.8f < m_fStateTime)
        {
            string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
            if (m_StateNames.find(strName) != m_StateNames.end())
            {
                if (m_bAttackA)
                    return EPlayerState::WEAKATTACKB;
                if (m_bAttackB)
                    return EPlayerState::STRONGATTACKB;

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
        m_pOwner->m_pTransformCom->SetbSpecialMoving();
        
        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
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
            }
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bAttackB = false;
        m_bAttackA = false;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (0.75f < m_fStateTime)
        {
            string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
            if (m_StateNames.find(strName) != m_StateNames.end())
            {
                if (m_bAttackB)
                    return EPlayerState::STRONGATTACKB;
                if (m_bAttackA)
                    return EPlayerState::WEAKATTACKB;
        
                if (m_pOwner->m_pAnimator->IsFinished())
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
        m_pOwner->m_pTransformCom->SetbSpecialMoving();

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
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
            }
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bAttackB = false;
		m_bAttackA = false;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        if (1.f < m_fStateTime)
        {
            string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
            if (m_StateNames.find(strName) != m_StateNames.end())
            {
                if (m_bAttackB)
                    return EPlayerState::STRONGATTACKA;
                if (m_bAttackA)
                    return EPlayerState::WEAKATTACKA;


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

        string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
        if (m_StateNames.find(strName) != m_StateNames.end())
        {
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
    }

    virtual void Exit() override
    {
        m_pOwner->m_pAnimator->SetBool("Charge", false);
        m_fStateTime = 0.f;
        m_bChargeStarted = false;
        m_fChargeElapsed = 0.f;

        m_pOwner->m_bMovable = true;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
        if (m_StateNames.find(strName) != m_StateNames.end() && m_fStateTime > 1.f)
        {
            if (m_bChargeStarted && m_pOwner->m_bWeaponEquipped)
                return EPlayerState::CHARGEB;

            if (1.f < m_fStateTime)
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

        return EPlayerState::CHARGEA;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const _tchar* GetStateName() const override
    {
        return L"CHARGE";
    }

private:
    unordered_set<string> m_StateNames = {
        "ChargeStrongAttack", "ChargeStrongAttack2"
    };

private:
    _bool   m_bAttack = {};
    _bool   m_bChargeStarted = {};
    _float   m_fChargeElapsed = {};

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
        m_pOwner->m_pAnimator->SetBool("Charge", false);
        m_fStateTime = 0.f;

        m_pOwner->m_bMovable = true;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        m_pOwner->m_pAnimator->SetBool("Move", input.bMove);

        string strName = m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
        if (m_StateNames.find(strName) != m_StateNames.end())
        {
            if (1.f < m_fStateTime)
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
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(g_fWalkSpeed);

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

NS_END