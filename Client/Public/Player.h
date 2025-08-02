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
		/* [ W,A,S,D 입력 ] */
		_bool bUp;
		_bool bDown;
		_bool bLeft;
		_bool bRight;

		/* [ 마우스 입력 ] */
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

/* [ 입력 처리 ] */
private: 
	void			HandleInput();									// [1] 키 입력만 처리
	EPlayerState	EvaluateTransitions();							// [2] 입력에 따라 상태 전이
	void			UpdateCurrentState(_float fTimeDelta);			// [3] 현재 상태 로직 수행
	void			TriggerStateEffects();							// [4] 애니메이션 적용



private:/* [ 캐스케이드 전용함수 ] */
	HRESULT UpdateShadowCamera();

private: /* [ 이동로직 ] */
	void SetMoveState(_float fTimeDelta);
	void SetPlayerState(_float fTimeDelta);
	void Movement(_float fTimeDelta);

private: /* [ Setup 함수 ] */
	HRESULT Ready_Components();
	void LoadPlayerFromJson();

private: /* 옵저버 관련*/
	void Callback_HP();
	void Callback_Stamina();
	void Callback_Mana();

	// 스탯 변화 테스트용
	void Update_Stat();


private: /* [ 상태패턴 ] */
	friend class CPlayer_Idle;


private: /* [ 상태 변수 ] */
	EPlayerState  m_pPreviousState = { EPlayerState::END };
	EPlayerState  m_eCurrentState = { EPlayerState::IDLE };

	CPlayerState* m_pCurrentState;
	map<EPlayerState, CPlayerState*> m_StateMap;

protected:
	class CCamera_Orbital* m_pCamera_Orbital = { nullptr };

private: /* [ 그림자 변수 ] */
	_vector m_vShadowCam_Eye = {};
	_vector m_vShadowCam_At = {};

private: /* [ 락온 변수 ] */
	CGameObject* m_pTarget = { nullptr };
	CGameObject* m_pWeapon = { nullptr };

private: /* [ 이동관련 변수 ] */
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


public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END

