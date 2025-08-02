#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "Camera_Manager.h"

#include "Player.h"
NS_BEGIN(Client)

class CPlayerState abstract : public CBase
{
public:
	CPlayerState(CPlayer* pOwner) :
		m_pOwner(pOwner),
		m_pCamera_Manager(CCamera_Manager::Get_Instance())
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
        m_bDoOnce = false;

        /* [ �ִϸ��̼� ���� ] */
        m_pOwner->m_pAnimator->CheckBool("Move");
        //m_pAnimator->SetTrigger("EquipWeapon"); �ܹ߼� Ʈ����
        //m_pAnimator->ApplyOverrideAnimController("TwoHand"); ��Ʈ�ѷ� ����
    }

    virtual void Execute(_float fTimeDelta) override
    {
        m_fStateTime += fTimeDelta;

        //�̵� ���̸� �ִϸ��̼� ���
    }

    virtual void Exit() override
    {
        m_fStateTime = 0.f;
    }

    virtual EPlayerState EvaluateTransitions(const CPlayer::InputContext& input) override
    {
        //if (input.bMove)
        //    return EPlayerState::WALK;
        //
        //if (input.bJumpPressed)
        //    return EPlayerState::JUMP;

        return EPlayerState::IDLE; // ����
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



NS_END