#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "Camera_Manager.h"

#include "GameInstance.h"

#include "Player.h"
NS_BEGIN(Client)

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
    virtual const char* GetStateName() const = 0;

protected:
	CPlayer* m_pOwner;

	CCamera_Manager* m_pCamera_Manager = { nullptr };
    CGameInstance* m_pGameInstance = { nullptr };

	_float m_fStateTime = {};

	_bool  m_bDoOnce = {};
	_bool  m_bDoTwo = {};
	_bool  m_bDoThree = {};

    //m_pAnimator->SetTrigger("EquipWeapon"); 단발성 트리거
    //m_pAnimator->ApplyOverrideAnimController("TwoHand"); 컨트롤러 변경
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
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        // 아무것도 안켜면 Idle 입니다.
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

        if (input.bTap) // 실드
            return EPlayerState::EQUIP;

        if (input.bShift) // 실드
            return EPlayerState::SHILD;
        
        if (input.bItem) // 아이템 사용
			return EPlayerState::USEITEM;

        if (input.bCtrl) // 컨트롤 왼팔공격
			return EPlayerState::LEFTHANDATTACK;

		if (input.bRightMouseDown) // 강공 (차징)
			return EPlayerState::STRONGATTACK;
        
        if (input.bLeftMouseDown) // 약공
			return EPlayerState::WEAKATTACK;

        if (input.bTap) // 무기교체
            return EPlayerState::SWITCHWEAPON;

		if (input.bSpaceDown) // 빽스탭
            return EPlayerState::BACKSTEP;

        if (input.bDown || input.bLeft || input.bRight || input.bUp)
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

    virtual const char* GetStateName() const override
    {
        return "IDLE";
    }

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
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_pAnimator->SetBool("Move", true);
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(1.f);
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (KEY_PRESSING(DIK_SPACE))
            m_fSpaceHoldTime += fTimeDelta;
        else
            m_fSpaceHoldTime = 0.f;
    }

    virtual void Exit() override
    {
        m_pOwner->m_pAnimator->SetBool("Move", false);
        m_fSpaceHoldTime = 0.f;
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        
        if (m_fSpaceHoldTime > 0.5f)
            return EPlayerState::SPRINT;

        if (input.bShift) // 실드
            return EPlayerState::SHILD;

        if (input.bItem) // 아이템 사용
            return EPlayerState::USEITEM;

        if (input.bCtrl) // 컨트롤 왼팔공격
            return EPlayerState::LEFTHANDATTACK;

        if (input.bRightMouseDown) // 강공 (차징)
            return EPlayerState::STRONGATTACK;

        if (input.bLeftMouseDown) // 약공
            return EPlayerState::WEAKATTACK;

        if (input.bSpaceUP) // 구르기
            return EPlayerState::ROLLING;

		// 아무것도 안눌리면 Idle로 전환
        if (!input.bDown_Pressing && !input.bLeft_Pressing && !input.bRight_Pressing && !input.bUp_Pressing)
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

    virtual const char* GetStateName() const override
    {
        return "WALK";
    }

private:
	_float m_fSpaceHoldTime = 0.f;
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
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_pAnimator->SetBool("Move", true);
        m_pOwner->m_pAnimator->SetBool("Run", true);
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(3.f);
        m_pOwner->m_bWalk = false; 
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (KEY_PRESSING(DIK_SPACE))
            m_fSpaceHoldTime += fTimeDelta;
        else
            m_fSpaceHoldTime = 0.f;
    }

    virtual void Exit() override
    {
        m_pOwner->m_pAnimator->SetBool("Move", false);
        m_pOwner->m_pAnimator->SetBool("Run", false);
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */

        if (m_fSpaceHoldTime > 0.5f)
            return EPlayerState::SPRINT;

        if (input.bShift) // 실드
            return EPlayerState::SHILD;

        if (input.bItem) // 아이템 사용
            return EPlayerState::USEITEM;

        if (input.bCtrl) // 컨트롤 왼팔공격
            return EPlayerState::LEFTHANDATTACK;

        if (input.bRightMouseDown) // 강공 (차징)
            return EPlayerState::STRONGATTACK;

        if (input.bLeftMouseDown) // 약공
            return EPlayerState::WEAKATTACK;

        if (input.bSpaceUP) // 구르기
            return EPlayerState::ROLLING;

        if (!input.bDown_Pressing && !input.bLeft_Pressing && !input.bRight_Pressing && !input.bUp_Pressing)
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

    virtual const char* GetStateName() const override
    {
        return "RUN";
    }
private:
    _float m_fSpaceHoldTime = 0.f;
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
        m_pOwner->m_pAnimator->SetTrigger("EquipWeapon");
        m_pOwner->m_pAnimator->ApplyOverrideAnimController("TwoHand");
        m_pOwner->m_pAnimator->SetInt("Combo", 0);
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");
        // 원래는 걸으면서 아이템을 사용해야 하지만, 현재는 공격모션으로 대체합니다.

         // 아이템 사용 전 걷기 상태였는지 저장
		m_bPreWalk = m_pOwner->m_bWalk;
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;


        // 아이템사용은 걷기로 모션이 바뀝니다.
        m_pOwner->m_bWalk = true;
    }

    virtual void Exit() override
    {
		m_pOwner->m_bWalk = m_bPreWalk;
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        if(m_pOwner->m_pAnimator->IsFinished())
		{
            if (input.bDown || input.bLeft || input.bRight || input.bUp)
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

    virtual const char* GetStateName() const override
    {
        return "ITEM";
    }


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
        m_pOwner->m_pAnimator->SetTrigger("Dash");
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(1.f);
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
        if (m_pOwner->m_pAnimator->IsFinished())
        {          
            return EPlayerState::IDLE;
        }

        return EPlayerState::BACKSTEP;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const char* GetStateName() const override
    {
        return "BACKSTEP";
    }
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
        m_pOwner->m_pAnimator->SetBool("Move", true);
        m_pOwner->m_pAnimator->SetTrigger("Dash");

        if (m_pOwner->m_bWalk)
            m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(1.f);
        else
            m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(1.f);
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;


    }

    virtual void Exit() override
    {
        m_pOwner->m_pAnimator->SetBool("Move", false);
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        if (0.65f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
        {
            if (input.bDown || input.bLeft || input.bRight || input.bUp)
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

    virtual const char* GetStateName() const override
    {
        return "ROLLING";
    }
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
        if (m_pOwner->m_pAnimator->IsFinished())
        {
            if (input.bDown || input.bLeft || input.bRight || input.bUp)
            {
                if (m_pOwner->m_bWalk)
                    return EPlayerState::WALK;
                else
                    return EPlayerState::RUN;
            }
            return EPlayerState::IDLE;
        }

        return EPlayerState::EQUIP;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const char* GetStateName() const override
    {
        return "EQUIP";
    }
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
        m_pOwner->m_pAnimator->SetBool("Move", true);
        m_pOwner->m_pAnimator->SetBool("Sprint", true);
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(5.f);
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;

        m_pOwner->m_pAnimator->SetBool("Move", false);
        m_pOwner->m_pAnimator->SetBool("Sprint", false);
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */

        //방향키가 아무것도 안눌렸다.
        if (!input.bDown_Pressing && !input.bLeft_Pressing && !input.bRight_Pressing && !input.bUp_Pressing)
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

    virtual const char* GetStateName() const override
    {
        return "SPRINT";
    }
};

/* [ 이 클래스는 약공 상태입니다. ] */
class CPlayer_WeakAttack final : public CPlayerState
{
public:
    CPlayer_WeakAttack(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_WeakAttack() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */

        // 약공은 무기 장착상태여야합니다.
        m_pOwner->m_pAnimator->SetBool("Sprint", true);
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(5.f);
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;

        m_pOwner->m_pAnimator->SetBool("Move", false);
        m_pOwner->m_pAnimator->SetBool("Sprint", false);
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */

        //방향키가 아무것도 안눌렸다.
        if (!input.bDown_Pressing && !input.bLeft_Pressing && !input.bRight_Pressing && !input.bUp_Pressing)
        {
            return EPlayerState::IDLE;
        }

        //스페이스바가 떼어졌다.
        if (input.bSpaceUP)
            return EPlayerState::RUN;

        return EPlayerState::SPRINT;
    }

    virtual bool CanExit() const override
    {
        return true;
    }

    virtual const char* GetStateName() const override
    {
        return "SPRINT";
    }
};


NS_END