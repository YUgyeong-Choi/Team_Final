#pragma once
#include "Unit.h"
#include "Client_Defines.h"


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

		/* [ ���콺 �Է� ] */
		_bool bRightMouseDown;
		_bool bRightMousePress;
		_bool bRightMouseUp;
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

/* [ �Է� ó�� ] */
private: 
	void			HandleInput();									// [1] Ű �Է¸� ó��
	EPlayerState	EvaluateTransitions();							// [2] �Է¿� ���� ���� ����
	void			UpdateCurrentState(_float fTimeDelta);			// [3] ���� ���� ���� ����
	void			TriggerStateEffects();							// [4] �ִϸ��̼� ����



private:/* [ ĳ�����̵� �����Լ� ] */
	HRESULT UpdateShadowCamera();

private: /* [ �̵����� ] */
	void SetMoveState(_float fTimeDelta);
	void SetPlayerState(_float fTimeDelta);
	void Movement(_float fTimeDelta);

private: /* [ Setup �Լ� ] */
	HRESULT Ready_Components();
	void LoadPlayerFromJson();

private: /* ������ ����*/
	void Callback_HP();
	void Callback_Stamina();
	void Callback_Mana();

	// ���� ��ȭ �׽�Ʈ��
	void Update_Stat();


private: /* [ �������� ] */
	friend class CPlayer_Idle;


private: /* [ ���� ���� ] */
	EPlayerState  m_pPreviousState = { EPlayerState::END };
	EPlayerState  m_eCurrentState = { EPlayerState::IDLE };

	CPlayerState* m_pCurrentState;
	map<EPlayerState, CPlayerState*> m_StateMap;

protected:
	class CCamera_Orbital* m_pCamera_Orbital = { nullptr };

private: /* [ �׸��� ���� ] */
	_vector m_vShadowCam_Eye = {};
	_vector m_vShadowCam_At = {};

private: /* [ ���� ���� ] */
	CGameObject* m_pTarget = { nullptr };
	CGameObject* m_pWeapon = { nullptr };

private: /* [ �̵����� ���� ] */
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


public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END

