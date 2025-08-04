#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "Camera_Manager.h"

#include "GameInstance.h"

#include "Player.h"
NS_BEGIN(Client)

_float g_fWalkSpeed = 1.5f;
_float g_fRunSpeed = 3.5f;
_float g_fSprintSpeed = 5.f;

/* [ �÷��̾��� ���¸� �����ϴ� �߻� Ŭ�����Դϴ�. ] */
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

    //m_pAnimator->SetTrigger("EquipWeapon"); �ܹ߼� Ʈ����
    //m_pAnimator->ApplyOverrideAnimController("TwoHand"); ��Ʈ�ѷ� ����
public:
	virtual void Free()
	{
		__super::Free();
	}
};

/* [ �� Ŭ������ ������ ���ִ� �����Դϴ�. ] */
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

        /* [ �ִϸ��̼� ���� ] */
        // �ƹ��͵� ���Ѹ� Idle �Դϴ�.

        /* [ ����� ] */
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
        /* [ Ű ��ǲ�� �޾Ƽ� �� ���¸� �������� �����մϴ�. ] */

        if (input.bTap) // �ǵ�
            return EPlayerState::EQUIP;

        if (input.bShift) // �ǵ�
            return EPlayerState::SHILD;
        
        if (input.bItem) // ������ ���
			return EPlayerState::USEITEM;

        if (input.bCtrl) // ��Ʈ�� ���Ȱ���
			return EPlayerState::LEFTHANDATTACK;

		if (input.bRightMouseDown && m_pOwner->m_bWeaponEquipped) // ���� (��¡)
			return EPlayerState::STRONGATTACK;
        
        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped) // ���
			return EPlayerState::WEAKATTACKA;

        if (input.bTap) // ���ⱳü
            return EPlayerState::SWITCHWEAPON;

		if (input.bSpaceDown) // ������
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

};

/* [ �� Ŭ������ õõ�� �ȴ� �����Դϴ�. ] */
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

        /* [ �ִϸ��̼� ���� ] */
        m_pOwner->m_pAnimator->SetBool("Move", true);
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(g_fWalkSpeed);

        /* [ ����� ] */
        printf("Player_State : %ls \n", GetStateName());
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
        /* [ Ű ��ǲ�� �޾Ƽ� �� ���¸� �������� �����մϴ�. ] */
        
        if (m_fSpaceHoldTime > 0.5f)
            return EPlayerState::SPRINT;

        if (input.bShift) // �ǵ�
            return EPlayerState::SHILD;

        if (input.bItem) // ������ ���
            return EPlayerState::USEITEM;

        if (input.bCtrl) // ��Ʈ�� ���Ȱ���
            return EPlayerState::LEFTHANDATTACK;

        if (input.bRightMouseDown && m_pOwner->m_bWeaponEquipped) // ���� (��¡)
            return EPlayerState::STRONGATTACK;

        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped) // ���
            return EPlayerState::WEAKATTACKA;

        if (input.bSpaceUP) // ������
            return EPlayerState::ROLLING;

		// �ƹ��͵� �ȴ����� Idle�� ��ȯ
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
};

/* [ �� Ŭ������ �ٴ� �����Դϴ�. ] */
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

        /* [ �ִϸ��̼� ���� ] */
        m_pOwner->m_pAnimator->SetBool("Move", true);
        m_pOwner->m_pAnimator->SetBool("Run", true);
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(g_fRunSpeed);
        m_pOwner->m_bWalk = false; 

        /* [ ����� ] */
        printf("Player_State : %ls \n", GetStateName());
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
        /* [ Ű ��ǲ�� �޾Ƽ� �� ���¸� �������� �����մϴ�. ] */

        if (m_fSpaceHoldTime > 0.5f)
            return EPlayerState::SPRINT;

        if (input.bShift) // �ǵ�
            return EPlayerState::SHILD;

        if (input.bItem) // ������ ���
            return EPlayerState::USEITEM;

        if (input.bCtrl) // ��Ʈ�� ���Ȱ���
            return EPlayerState::LEFTHANDATTACK;

        if (input.bRightMouseDown && m_pOwner->m_bWeaponEquipped) // ���� (��¡)
            return EPlayerState::STRONGATTACK;

        if (input.bLeftMouseDown && m_pOwner->m_bWeaponEquipped) // ���
            return EPlayerState::WEAKATTACKA;

        if (input.bSpaceUP) // ������
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
};

/* [ �� Ŭ������ ������ ��� �����Դϴ�. ] */
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

        /* [ �ִϸ��̼� ���� ] */
        m_pOwner->m_pAnimator->SetTrigger("EquipWeapon");
        m_pOwner->m_pAnimator->ApplyOverrideAnimController("TwoHand");
        m_pOwner->m_pAnimator->SetInt("Combo", 0);
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");
        // ������ �����鼭 �������� ����ؾ� ������, ����� ���ݸ������ ��ü�մϴ�.

         // ������ ��� �� �ȱ� ���¿����� ����
		m_bPreWalk = m_pOwner->m_bWalk;

        /* [ ����� ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;


        // �����ۻ���� �ȱ�� ����� �ٲ�ϴ�.
        m_pOwner->m_bWalk = true;
    }

    virtual void Exit() override
    {
		m_pOwner->m_bWalk = m_bPreWalk;
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ Ű ��ǲ�� �޾Ƽ� �� ���¸� �������� �����մϴ�. ] */
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

/* [ �� Ŭ������ �齺�� �����Դϴ�. ] */
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

        /* [ �ִϸ��̼� ���� ] */

        // �齺���� ���� OFF �Դϴ�.
        m_pOwner->m_pAnimator->SetTrigger("Dash");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();

        /* [ ����� ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        // �齺�� �̵�
        if (!m_bMove)
        {
            _vector vLook = m_pOwner->m_pTransformCom->Get_State(STATE::LOOK) * -1.f;
            
            /* �ش� �ִϸ��̼��� �������� �� */
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
        /* [ Ű ��ǲ�� �޾Ƽ� �� ���¸� �������� �����մϴ�. ] */
        
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

/* [ �� Ŭ������ ������ �����Դϴ�. ] */
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

        /* [ �ִϸ��̼� ���� ] */

        // ������� ���� ON �Դϴ�.
        m_pOwner->m_pAnimator->SetBool("Move", true);
        m_pOwner->m_pAnimator->SetTrigger("Dash");

        m_pOwner->m_pTransformCom->SetbSpecialMoving();
        m_pOwner->m_bMovable = false;

        /* [ ����� ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        if (!m_bMove)
        {
            _vector vLook = m_pOwner->m_pTransformCom->Get_State(STATE::LOOK);

            /* �ش� �ִϸ��̼��� �������� �� */
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_H_Dash_Normal_F")
            {
                m_bMove = m_pOwner->m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pOwner->m_pControllerCom);
                m_pOwner->SyncTransformWithController();
            }
        }
    }

    virtual void Exit() override
    {
        m_pOwner->m_pAnimator->SetBool("Move", false);
        m_pOwner->m_bMovable = true;
        m_bMove = false;

        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        /* [ Ű ��ǲ�� �޾Ƽ� �� ���¸� �������� �����մϴ�. ] */
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
    _bool   m_bMove = {};
    _float  m_fTime = 0.3f;
    _float  m_fDistance = 2.f;
};

/* [ �� Ŭ������ �������� �����Դϴ�. ] */
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

        /* [ �ִϸ��̼� ���� ] */

        // �������� ����Դϴ�.
        m_pOwner->m_pAnimator->SetTrigger("EquipWeapon");
        m_pOwner->m_pAnimator->ApplyOverrideAnimController("TwoHand");
        m_pOwner->m_bMovable = false;
        m_pOwner->m_bWeaponEquipped = true;

        /* [ ����� ] */
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
        /* [ Ű ��ǲ�� �޾Ƽ� �� ���¸� �������� �����մϴ�. ] */
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

/* [ �� Ŭ������ ������Ʈ �����Դϴ�. ] */
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

        /* [ �ִϸ��̼� ���� ] */

        // ������Ʈ�� ���� ON �Դϴ�.
        m_pOwner->m_pAnimator->SetBool("Move", true);
        m_pOwner->m_pAnimator->SetBool("Sprint", true);
        m_pOwner->Get_TransfomCom()->Set_SpeedPreSec(g_fSprintSpeed);

        /* [ ����� ] */
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
        /* [ Ű ��ǲ�� �޾Ƽ� �� ���¸� �������� �����մϴ�. ] */

        //����Ű�� �ƹ��͵� �ȴ��ȴ�.
        if (!input.bDown_Pressing && !input.bLeft_Pressing && !input.bRight_Pressing && !input.bUp_Pressing)
        {
            return EPlayerState::IDLE;
        }

        //�����̽��ٰ� ��������.
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


/* [ �� Ŭ������ ��� �����Դϴ�. ] */
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

        /* [ �ִϸ��̼� ���� ] */

        // ����� ���� �������¿����մϴ�.
        m_pOwner->m_pAnimator->SetInt("Combo", 1);
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();
		m_pOwner->m_bMovable = false;

        /* [ ����� ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        /* ���� �ִϸ��̼��� 0.3 �̻� ����Ǿ��� �� */
        if (0.3f < m_pOwner->m_pAnimator->GetCurrentAnimProgress())
        {
            string strName = m_pOwner->m_pAnimator->GetCurrentAnimName();
            if (strName == "AS_Pino_T_Bayonet_NA3")
            {
                if (MOUSE_PRESSING(DIM::LBUTTON))
                    m_bAttack = true;
            }
        }

        // ��� A �̵�
        if (!m_bMove)
        {
            _vector vLook = m_pOwner->m_pTransformCom->Get_State(STATE::LOOK);

            /* �ش� �ִϸ��̼��� �������� �� */
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
        /* [ Ű ��ǲ�� �޾Ƽ� �� ���¸� �������� �����մϴ�. ] */

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

        /* [ �ִϸ��̼� ���� ] */

        // ����� ���� �������¿����մϴ�.
        m_pOwner->m_pAnimator->SetInt("Combo", 0);
        m_pOwner->m_pAnimator->SetTrigger("NormalAttack");
        m_pOwner->m_pTransformCom->SetbSpecialMoving();
        m_pOwner->m_bMovable = false;

        /* [ ����� ] */
        printf("Player_State : %ls \n", GetStateName());
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        // ��� B �̵�
        if (!m_bMove)
        {
            _vector vLook = m_pOwner->m_pTransformCom->Get_State(STATE::LOOK);

            /* �ش� �ִϸ��̼��� �������� �� */
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
        /* [ Ű ��ǲ�� �޾Ƽ� �� ���¸� �������� �����մϴ�. ] */

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


NS_END