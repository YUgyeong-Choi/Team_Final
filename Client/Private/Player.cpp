#include "Player.h"

#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "Camera_Manager.h"
#include "PhysX_IgnoreSelfCallback.h"
#include "PhysXController.h"

#include "PlayerState.h"

#include "Observer_Player_Status.h"

CPlayer::CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUnit(pDevice, pContext)
{
}
CPlayer::CPlayer(const CPlayer& Prototype)
	: CUnit(Prototype)
{
}

HRESULT CPlayer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayer::Initialize(void* pArg)
{
	PLAYER_DESC* pDesc = static_cast<PLAYER_DESC*>(pArg);


	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	/* [ �÷��̾� ���̽� �ε� ] */
	LoadPlayerFromJson();

	/* [ ������Ʈ ���� ] */
	ReadyForState();

	/* [ �ʱ�ȭ ��ġ�� ] */
	m_pTransformCom->Set_State(STATE::POSITION, _vector{ m_InitPos.x, m_InitPos.y, m_InitPos.z });
	m_pTransformCom->Rotation(XMConvertToRadians(0.f), XMConvertToRadians(90.f), XMConvertToRadians(0.f));
	m_pTransformCom->SetUp_Scale(pDesc->InitScale.x, pDesc->InitScale.y, pDesc->InitScale.z);

	/* [ ��ġ �ʱ�ȭ �� �ݶ��̴� ���� ] */
	if (FAILED(Ready_Collider()))
		return E_FAIL;

	m_pCamera_Orbital = CCamera_Manager::Get_Instance()->GetOrbitalCam();
	CCamera_Manager::Get_Instance()->SetPlayer(this);
	SyncTransformWithController();


	// ������ ã�Ƽ� ������ �߰�
	if (nullptr == m_pGameInstance->Find_Observer(TEXT("Player_Status")))
		m_pGameInstance->Add_Observer(TEXT("Player_Status"), new CObserver_Player_Status);

	m_iCurrentHP = m_iMaxHP;

	Callback_HP();
	Callback_Mana();
	Callback_Stamina();

	return S_OK;
}



void CPlayer::Priority_Update(_float fTimeDelta)
{
	// ������ �ƾ�
	if (KEY_DOWN(DIK_N))
		CCamera_Manager::Get_Instance()->Play_CutScene(CUTSCENE_TYPE::ONE);

	/* [ ĳ�����̵� ���� ������Ʈ �Լ� ] */
	UpdateShadowCamera();
	/* [ �� ���� �����ɽ�Ʈ ] */
	RayCast();


	// ������ ������ ó��
	Update_Stat();

	__super::Priority_Update(fTimeDelta);
}
void CPlayer::Update(_float fTimeDelta)
{
	/* [ �ִϸ��̼� ������Ʈ ] */
	__super::Update(fTimeDelta);

	// �ƾ��� �� �� �����̵���
	if (!CCamera_Manager::Get_Instance()->GetbMoveable())
		return;

	/* [ �Է� ] */
	HandleInput();
	UpdateCurrentState(fTimeDelta);

	// �ٲ� ����
	Movement(fTimeDelta);
}

void CPlayer::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
	
	/* [ �̰��� �ִϸ��̼� ������Դϴ�. ] */
	if(KEY_DOWN(DIK_Y))
	{
		//m_pAnimator->ApplyOverrideAnimController("TwoHand");
		//m_pAnimator->SetInt("Combo", 1);
		//string strName = m_pAnimator->GetCurrentAnimName();
		//m_pAnimator->SetBool("Charge", true);
		//m_pAnimator->SetTrigger("StrongAttack");
		
		m_pAnimator->SetBool("Move", true);
		m_pAnimator->SetTrigger("Dash");
		m_pAnimator->GetCurrentAnim()->SetTickPerSecond(200.f);

		//m_pAnimator->SetBool("Run", true);
		//m_pAnimator->SetTrigger("Hited");
	}
	
	//m_pAnimator->ApplyOverrideAnimController("TwoHand");
	//m_pAnimator->SetBool("Charge", true);
	//m_pAnimator->SetTrigger("StrongAttack");
	//string strName = m_pAnimator->GetCurrentAnimName();

	if (KEY_PRESSING(DIK_U))
	{
		m_pAnimator->SetBool("Move", false);
	}
	if (KEY_PRESSING(DIK_I))
	{
		m_pAnimator->SetBool("Move", true);
	}

	if (m_pAnimator->IsFinished())
		int a = 0;
}

HRESULT CPlayer::Render()
{
	__super::Render();

	return S_OK;
}

void CPlayer::HandleInput()
{
	/* [ Ű �Է��� ������Ʈ�մϴ�. ] */
	m_Input.bUp = KEY_PRESSING(DIK_W);
	m_Input.bDown = KEY_PRESSING(DIK_S);
	m_Input.bLeft = KEY_PRESSING(DIK_A);
	m_Input.bRight = KEY_PRESSING(DIK_D);

	m_Input.bUp_Pressing = KEY_PRESSING(DIK_W);
	m_Input.bDown_Pressing = KEY_PRESSING(DIK_S);
	m_Input.bLeft_Pressing = KEY_PRESSING(DIK_A);
	m_Input.bRight_Pressing = KEY_PRESSING(DIK_D);

	/* [ ���콺 �Է��� ������Ʈ�մϴ�. ] */
	m_Input.bLeftMouseDown = MOUSE_DOWN(DIM::LBUTTON);
	m_Input.bRightMouseDown = MOUSE_DOWN(DIM::RBUTTON);
	m_Input.bRightMousePress = MOUSE_PRESSING(DIM::RBUTTON);
	m_Input.bRightMouseUp = MOUSE_UP(DIM::RBUTTON);


	/* [ Ư��Ű �Է��� ������Ʈ�մϴ�. ] */
	m_Input.bShift = KEY_PRESSING(DIK_LSHIFT);
	m_Input.bCtrl = KEY_DOWN(DIK_LCONTROL);
	m_Input.bTap = KEY_DOWN(DIK_TAB);
	m_Input.bItem = KEY_DOWN(DIK_R);
	m_Input.bSpaceUP = KEY_UP(DIK_SPACE);
	m_Input.bSpaceDown = KEY_DOWN(DIK_SPACE);
	
	/* [ �ٱ� �ȱ⸦ ����մϴ�. ] */
	if (KEY_DOWN(DIK_Z))
		ToggleWalkRun();
}

EPlayerState CPlayer::EvaluateTransitions()
{
	if (!m_pCurrentState)
		return m_eCurrentState;

	return m_pCurrentState->EvaluateTransitions(m_Input);
}

void CPlayer::UpdateCurrentState(_float fTimeDelta)
{
	if (!m_pCurrentState)
		return;

	EPlayerState eNextState = EvaluateTransitions();

	if (eNextState != m_eCurrentState && m_pCurrentState->CanExit())
	{
		m_pCurrentState->Exit();

		m_eCurrentState = eNextState; 
		m_pCurrentState = m_pStateArray[ENUM_CLASS(m_eCurrentState)];

		m_pCurrentState->Enter();
	}

	m_pCurrentState->Execute(fTimeDelta);
	TriggerStateEffects();
}

void CPlayer::TriggerStateEffects()
{
	switch (m_eCurrentState)
	{
	case EPlayerState::IDLE:
		//m_SoundPlayer->Play("Idle");
		break;

	case EPlayerState::WALK:
		//m_SoundPlayer->Play("Walk");
		break;

	default:
		break;
	}
}

void CPlayer::ReadyForState()
{
	m_pStateArray[ENUM_CLASS(EPlayerState::IDLE)] = new CPlayer_Idle(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::WALK)] = new CPlayer_Walk(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::RUN)] = new CPlayer_Run(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::USEITEM)] = new CPlayer_Item(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::BACKSTEP)] = new CPlayer_BackStep(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::ROLLING)] = new CPlayer_Rolling(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::EQUIP)] = new CPlayer_Equip(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::SPRINT)] = new CPlayer_Sprint(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::WEAKATTACKA)] = new CPlayer_WeakAttackA(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::WEAKATTACKB)] = new CPlayer_WeakAttackB(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::STRONGATTACKA)] = new CPlayer_StrongAttackA(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::STRONGATTACKB)] = new CPlayer_StrongAttackB(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::CHARGE)] = new CPlayer_Charge(this);

	m_pCurrentState = m_pStateArray[ENUM_CLASS(EPlayerState::IDLE)];
}



HRESULT CPlayer::Ready_Components()
{
	/* [ ���� ���� ������Ʈ�� �ٿ����� ] */

	return S_OK;
}
void CPlayer::LoadPlayerFromJson()
{
	string path = "../Bin/Save/AnimationEvents/" + m_pModelCom->Get_ModelName() + "_events.json";
	ifstream ifs(path);
	if (ifs.is_open())
	{
		json root;
		ifs >> root;
		if (root.contains("animations"))
		{
			auto& animationsJson = root["animations"];
			auto& clonedAnims = m_pModelCom->GetAnimations();

			for (const auto& animData : animationsJson)
			{
				const string& clipName = animData["ClipName"];

				for (auto& pAnim : clonedAnims)
				{
					if (pAnim->Get_Name() == clipName)
					{
						pAnim->Deserialize(animData);
						break;
					}
				}
			}
		}
	}

	path = "../Bin/Save/AnimationStates/" + m_pModelCom->Get_ModelName() + "_States.json";
	ifstream ifsStates(path);
	if (ifsStates.is_open())
	{
		json rootStates;
		ifsStates >> rootStates;
		m_pAnimator->Deserialize(rootStates);
	}
}

void CPlayer::Callback_HP()
{
	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"CurrentHP"), &m_iCurrentHP);
	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"MaxHP"), &m_iMaxHP);
}

void CPlayer::Callback_Stamina()
{
	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"CurrentStamina"), &m_iCurrentStamina);
	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"MaxStamina"), &m_iMaxStamina);
}

void CPlayer::Callback_Mana()
{
	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"CurrentMana"), &m_iCurrentMana);
	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"MaxMana"), &m_iMaxMana);
}

void CPlayer::Update_Stat()
{
	if (m_pGameInstance->Key_Down(DIK_V))
	{
		m_iCurrentHP += 10;
		m_iCurrentStamina += 20;
		m_iCurrentMana += 10;
		Callback_HP();
		Callback_Mana();
		Callback_Stamina();

	}
	else if (m_pGameInstance->Key_Down(DIK_B))
	{
		m_iCurrentHP -= 10;
		m_iCurrentStamina -= 20;
		m_iCurrentMana -= 100;
		Callback_HP();
		Callback_Mana();
		Callback_Stamina();
	}
}

void CPlayer::Movement(_float fTimeDelta)
{
	if (!CCamera_Manager::Get_Instance()->GetbMoveable())
		return;

	SetMoveState(fTimeDelta);
}


HRESULT CPlayer::UpdateShadowCamera()
{
	CShadow::SHADOW_DESC Desc{};

	// 1. ī�޶� ���� ��ġ (��: ���߿� ���ִ� ��ġ)
	_vector vFixedEye = XMVectorSet(76.f, 57.f, -21.f, 1.f);

	// 2. �÷��̾� ���� ��ġ�� Ÿ������ ����
	_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetAt = vPlayerPos;

	// 3. ����
	m_vShadowCam_Eye = vFixedEye;
	m_vShadowCam_At = vTargetAt;

	XMStoreFloat4(&Desc.vEye, m_vShadowCam_Eye);
	XMStoreFloat4(&Desc.vAt, m_vShadowCam_At);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;

	Desc.fFovy = XMConvertToRadians(40.0f);
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWA)))
		return E_FAIL;
	Desc.fFovy = XMConvertToRadians(80.0f);
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWB)))
		return E_FAIL;
	Desc.fFovy = XMConvertToRadians(120.0f);
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWC)))
		return E_FAIL;

	return S_OK;
}

void CPlayer::SetMoveState(_float fTimeDelta)
{

	_vector vCamLook = m_pCamera_Orbital->Get_TransfomCom()->Get_State(STATE::LOOK);
	_vector vCamRight = m_pCamera_Orbital->Get_TransfomCom()->Get_State(STATE::RIGHT);


	vCamLook = XMVectorSetY(vCamLook, 0.f);
	vCamRight = XMVectorSetY(vCamRight, 0.f);
	vCamLook = XMVector3Normalize(vCamLook);
	vCamRight = XMVector3Normalize(vCamRight);


	_vector vInputDir = XMVectorZero();
	if (KEY_PRESSING(DIK_W)) vInputDir += vCamLook;
	if (KEY_PRESSING(DIK_S)) vInputDir -= vCamLook;
	if (KEY_PRESSING(DIK_D)) vInputDir += vCamRight;
	if (KEY_PRESSING(DIK_A)) vInputDir -= vCamRight;

	/* [ �Է°��� ������? ] */
	if (!XMVector3Equal(vInputDir, XMVectorZero()))
	{
		/* [ ȸ���� ��Ų��. ] */
		vInputDir = XMVector3Normalize(vInputDir);

		_vector vPlayerLook = m_pTransformCom->Get_State(STATE::LOOK);
		vPlayerLook = XMVectorSetY(vPlayerLook, 0.f);
		vPlayerLook = XMVector3Normalize(vPlayerLook);

		_float fDot = XMVectorGetX(XMVector3Dot(vPlayerLook, vInputDir));
		fDot = max(-1.f, min(1.f, fDot)); // Clamp
		_float fAngle = acosf(fDot);


		_vector vCross = XMVector3Cross(vPlayerLook, vInputDir);
		if (XMVectorGetY(vCross) < 0.f)
			fAngle = -fAngle;

		// ȸ������ ���� ��������� �״�� ��� ����
		const _float fTurnSpeed = XMConvertToRadians(720.f);
		const _float fMinAngle = XMConvertToRadians(0.5f);

		if (fabsf(fAngle) > fMinAngle)
		{
			_float fClampedAngle = max(-fTurnSpeed * fTimeDelta, min(fTurnSpeed * fTimeDelta, fAngle));
			if (!m_bMovable)
				fClampedAngle = 0.f;
			m_pTransformCom->TurnAngle(XMVectorSet(0.f, 1.f, 0.f, 0.f), fClampedAngle);
		}
	}

	/* [ �̵��� �Ѵ�. ] */
	_float3 moveVec = {};
	_float fSpeed = m_pTransformCom->Get_SpeedPreSec();
	if (!m_bMovable)    fSpeed = 0.f;
	_float fDist = fSpeed * fTimeDelta;
	vInputDir *= fDist;
	XMStoreFloat3(&moveVec, vInputDir);

	// �߷� ����
	constexpr _float fGravity = -9.81f;
	m_vGravityVelocity.y += fGravity * fTimeDelta;
	moveVec.y += m_vGravityVelocity.y * fTimeDelta;

	PxVec3 pxMove(moveVec.x, moveVec.y, moveVec.z);
	PxControllerFilters filters;

	PxControllerCollisionFlags collisionFlags =
		m_pControllerCom->Get_Controller()->move(pxMove, 0.001f, fTimeDelta, filters);
	

	// 4. ���鿡 ������� �߷� �ӵ� �ʱ�ȭ
	if (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN)
		m_vGravityVelocity.y = 0.f;

	SyncTransformWithController();
}



CPlayer* CPlayer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPlayer* pInstance = new CPlayer(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPlayer");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CPlayer::Clone(void* pArg)
{
	CPlayer* pInstance = new CPlayer(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPlayer");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAnimator);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pControllerCom);

	for (size_t i = 0; i < ENUM_CLASS(EPlayerState::END); ++i)
		Safe_Delete(m_pStateArray[i]);
}
