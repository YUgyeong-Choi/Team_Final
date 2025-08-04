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

        printf("Player_State : %s \n", m_pOwner->m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName.c_str());
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

        if (input.bTap) // 실드
            return EPlayerState::EQUIP;

        if (input.bShift) // 실드
            return EPlayerState::SHILD;
        
        if (input.bItem) // 아이템 사용
			return EPlayerState::USEITEM;

        if (input.bCtrl) // 컨트롤 왼팔공격
			return EPlayerState::LEFTHANDATTACK;

		if (input.bRightMouseUp && m_pOwner->m_bWeaponEquipped) // 강공
			return EPlayerState::STRONGATTACKA;
        
        if (m_bChargeStarted && m_pOwner->m_bWeaponEquipped) // 차징
			return EPlayerState::CHARGE;
        
        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped) // 약공
			return EPlayerState::WEAKATTACKA;

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

    virtual const _tchar* GetStateName() const override
    {
        return L"IDLE";
    }

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
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */
        m_pOwner->m_pAnimator->SetBool("Move", true);
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
        m_pOwner->m_pAnimator->SetBool("Move", false);
        m_bChargeStarted = false;
        m_fChargeElapsed = 0.f;
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

        if (input.bRightMouseUp && m_pOwner->m_bWeaponEquipped) // 강공
            return EPlayerState::STRONGATTACKA;

        if (m_bChargeStarted && m_pOwner->m_bWeaponEquipped) // 차징
            return EPlayerState::CHARGE;

        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped) // 약공
            return EPlayerState::WEAKATTACKA;

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

    virtual const _tchar* GetStateName() const override
    {
        return L"WALK";
    }

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
        m_pOwner->m_pAnimator->SetBool("Move", false);
        m_pOwner->m_pAnimator->SetBool("Run", false);
        m_bChargeStarted = false;
        m_fChargeElapsed = 0.f;
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

        if (input.bRightMouseUp && m_pOwner->m_bWeaponEquipped) // 강공
            return EPlayerState::STRONGATTACKA;

        if (m_bChargeStarted && m_pOwner->m_bWeaponEquipped) // 차징
            return EPlayerState::CHARGE;

        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped) // 약공
            return EPlayerState::WEAKATTACKA;

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

    virtual const _tchar* GetStateName() const override
    {
        return L"RUN";
    }
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
        m_pOwner->m_pAnimator->SetTrigger("EquipWeapon");
        m_pOwner->m_pAnimator->ApplyOverrideAnimController("TwoHand");
        m_pOwner->m_pAnimator->SetInt("Combo", 0);
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");
        // 원래는 걸으면서 아이템을 사용해야 하지만, 현재는 공격모션으로 대체합니다.

         // 아이템 사용 전 걷기 상태였는지 저장
		m_bPreWalk = m_pOwner->m_bWalk;

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
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

    virtual const _tchar* GetStateName() const override
    {
        return L"ITEM";
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
            
            /* 해당 애니메이션이 진행중일 때 */
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_H_Dash_Normal_B" || strName == "AS_Pino_T_Dash_Normal_B")
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
        
        if (0.8f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
        {   
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();            
            if(strName == "AS_Pino_H_Dash_Normal_B" || strName == "AS_Pino_T_Dash_Normal_B")
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
        m_pOwner->m_pAnimator->SetBool("Move", true);
        m_pOwner->m_pAnimator->SetTrigger("Dash");
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(6.f);

        m_fPreSpeed = m_pOwner->Get_TransfomCom()->GetfSpeedPerSec();
        m_pOwner->m_pTransformCom->Go_Front(0.01f, m_pOwner->m_pControllerCom);

        m_pOwner->m_pTransformCom->SetbSpecialMoving();
        m_pOwner->m_bMovable = false;

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (!m_bMove)
        {
            _vector vLook = m_pOwner->m_pTransformCom->Get_State(STATE::LOOK);

            /* 해당 애니메이션이 진행중일 때 */
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_H_Dash_Normal_F")
            {
                m_pOwner->m_pTransformCom->Go_Front(fTimeDelta, m_pOwner->m_pControllerCom );
                m_pOwner->SyncTransformWithController();
            }
        }
    }

    virtual void Exit() override
    {
		m_pOwner->m_pTransformCom->Set_SpeedPreSec(m_fPreSpeed);
        m_pOwner->m_pAnimator->SetBool("Move", false);
        m_pOwner->m_bMovable = true;
        m_bMove = false;

        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */
        if (0.65f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
        {
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_H_Dash_Normal_F")
            {
                if (input.bDown_Pressing || input.bLeft_Pressing || input.bRight_Pressing || input.bUp_Pressing)
                {
                    if (m_pOwner->m_bWalk)
                        return EPlayerState::WALK;
                    else
                        return EPlayerState::RUN;
                }
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

    virtual const _tchar* GetStateName() const override
    {
        return L"SPRINT";
    }
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
        m_pOwner->m_pAnimator->SetInt("Combo", 1);
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();
		m_pOwner->m_bMovable = false;

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        /* 공격 애니메이션이 0.3 이상 진행되었을 때 */
        if (0.3f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
        {
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_T_Bayonet_NA3")
            {
                if (MOUSE_DOWN(DIM::LBUTTON))
                    m_bAttack = true;
            }
        }

        // 약공 A 이동
        if (!m_bMove)
        {
            _vector vLook = m_pOwner->m_pTransformCom->Get_State(STATE::LOOK);

            /* 해당 애니메이션이 진행중일 때 */
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_T_Bayonet_NA3")
            {
                if (0.25f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
                {
                    m_bMove = m_pOwner->m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pOwner->m_pControllerCom);
                    m_pOwner->SyncTransformWithController();
                }
            }
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bMove = false;

        m_pOwner->m_bMovable = true;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */

        if (0.4f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
        {
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_T_Bayonet_NA3")
            {
                if (m_bAttack)
                    return EPlayerState::WEAKATTACKB;

                if (m_pOwner->m_pAnimator->IsFinished())
                {
                    if (input.bDown_Pressing || input.bLeft_Pressing || input.bRight_Pressing || input.bUp_Pressing)
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
    _bool   m_bAttack = {};
    _bool   m_bMove = {};
    _float  m_fTime = 0.2f;
    _float  m_fDistance = 1.f;
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
        m_pOwner->m_pAnimator->SetInt("Combo", 0);
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();
        m_pOwner->m_bMovable = false;

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        // 약공 B 이동
        if (!m_bMove)
        {
            _vector vLook = m_pOwner->m_pTransformCom->Get_State(STATE::LOOK);

            /* 해당 애니메이션이 진행중일 때 */
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_T_Bayonet_NA4")
            {
                if (0.15f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
                {
                    m_bMove = m_pOwner->m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pOwner->m_pControllerCom);
                    m_pOwner->SyncTransformWithController();
                }
            }
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bMove = false;

        m_pOwner->m_bMovable = true;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */

        if (0.7f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
        {
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_T_Bayonet_NA4")
            {
                if (input.bDown_Pressing || input.bLeft_Pressing || input.bRight_Pressing || input.bUp_Pressing)
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
    _bool   m_bAttack = {};
    _bool   m_bMove = {};
    _float  m_fTime = 0.2f;
    _float  m_fDistance = 1.f;
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
        m_pOwner->m_pTransformCom->SetbSpecialMoving();
        m_pOwner->m_bMovable = false;        
        
        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        /* 공격 애니메이션이 0.3 이상 진행되었을 때 */
        if (0.3f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
        {
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_T_Bayonet_SA4")
            {
                if (MOUSE_DOWN(DIM::RBUTTON))
                    m_bAttack = true;
            }
        }

        // 강공 A 이동
        if (!m_bMove)
        {
            _vector vLook = m_pOwner->m_pTransformCom->Get_State(STATE::LOOK);

            /* 해당 애니메이션이 진행중일 때 */
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_T_Bayonet_SA4")
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

        m_pOwner->m_bMovable = true;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */

        if (0.35f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
        {
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_T_Bayonet_SA4")
            {
                if (m_bAttack)
                    return EPlayerState::STRONGATTACKB;
        
                if (m_pOwner->m_pAnimator->IsFinished())
                {
                    if (input.bDown_Pressing || input.bLeft_Pressing || input.bRight_Pressing || input.bUp_Pressing)
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
    _bool   m_bAttack = {};
    _bool   m_bMove = {};

    _float  m_fTime = 0.3f;
    _float  m_fDistance = 1.f;
    
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
        m_pOwner->m_pTransformCom->SetbSpecialMoving();
        m_pOwner->m_bMovable = false;

        /* [ 디버깅 ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        // 강공 B 이동
        if (!m_bMove)
        {
            _vector vLook = m_pOwner->m_pTransformCom->Get_State(STATE::LOOK);

            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_T_Bayonet_SA5")
            {
                if (0.05f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
                {
                    m_bMove = m_pOwner->m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pOwner->m_pControllerCom);
                    m_pOwner->SyncTransformWithController();
                }
            }
        }
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
        m_bMove = false;

        m_pOwner->m_bMovable = true;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */

        if (0.4f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
        {
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_T_Bayonet_SA5")
            {
                if (input.bDown_Pressing || input.bLeft_Pressing || input.bRight_Pressing || input.bUp_Pressing)
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
    _bool   m_bMove = {};
    _float  m_fTime = 0.2f;
    _float  m_fDistance = 1.f;
};

/* [ 이 클래스는 차징 상태입니다. ] */
class CPlayer_Charge final : public CPlayerState
{
public:
    CPlayer_Charge(CPlayer* pOwner)
        : CPlayerState(pOwner) {
    }

    virtual ~CPlayer_Charge() = default;

public:
    virtual void Enter() override
    {
        m_fStateTime = 0.f;

        /* [ 애니메이션 설정 ] */

        // 강공은 무기 장착상태여야합니다.
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

        // 차징 이동
        if (!m_bMove)
        {
            _vector vLook = m_pOwner->m_pTransformCom->Get_State(STATE::LOOK);

            /* 해당 애니메이션이 진행중일 때 */
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_T_Bayonet_CA3")
            {
                m_bMove = m_pOwner->m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pOwner->m_pControllerCom);
                m_pOwner->SyncTransformWithController();
            }
        }
    }

    virtual void Exit() override
    {
        m_pOwner->m_pAnimator->SetBool("Charge", false);
        m_fStateTime = 0.f;
        m_bMove = false;

        m_pOwner->m_bMovable = true;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ 키 인풋을 받아서 이 상태를 유지할지 결정합니다. ] */

        string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
        if (strName == "AS_Pino_T_Bayonet_CA3")
        {
            if (m_pOwner->m_pAnimator->IsFinished())
            {
                if (input.bDown_Pressing || input.bLeft_Pressing || input.bRight_Pressing || input.bUp_Pressing)
                {
                    if (m_pOwner->m_bWalk)
                        return EPlayerState::WALK;
                    else
                        return EPlayerState::RUN;
                }
                return EPlayerState::IDLE;
            }
        }

        return EPlayerState::CHARGE;
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
    _bool   m_bAttack = {};
    _bool   m_bMove = {};

    _float  m_fTime = 0.3f;
    _float  m_fDistance = 1.f;

};

NS_END