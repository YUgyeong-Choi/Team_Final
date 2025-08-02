#include "Player.h"

#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "Camera_Manager.h"
#include "PhysX_IgnoreSelfCallback.h"
#include "PhysXController.h"

#include "PlayerState.h"

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

	/* [ �ʱ�ȭ ��ġ�� ] */
	m_pTransformCom->Set_State(STATE::POSITION, _vector{ m_InitPos.x, m_InitPos.y, m_InitPos.z });
	m_pTransformCom->SetUp_Scale(pDesc->InitScale.x, pDesc->InitScale.y, pDesc->InitScale.z);

	/* [ ��ġ �ʱ�ȭ �� �ݶ��̴� ���� ] */
	if (FAILED(Ready_Collider()))
		return E_FAIL;

	m_pCamera_Orbital = CCamera_Manager::Get_Instance()->GetOrbitalCam();
	CCamera_Manager::Get_Instance()->SetPlayer(this);
	SyncTransformWithController();
	return S_OK;
}



void CPlayer::Priority_Update(_float fTimeDelta)
{
	// ������ �ƾ�
	if (KEY_DOWN(DIK_N))
		CCamera_Manager::Get_Instance()->Play_CutScene(CUTSCENE_TYPE::ONE);

	if (KEY_DOWN(DIK_Y))
	{
		if (m_fTimeScale == 1.f)
			m_fTimeScale = 0.5f;
		else
			m_fTimeScale = 1.f;
	}

	/* [ ĳ�����̵� ���� ������Ʈ �Լ� ] */
	UpdateShadowCamera();
	/* [ �� ���� �����ɽ�Ʈ ] */
	RayCast();

	__super::Priority_Update(fTimeDelta);
}
void CPlayer::Update(_float fTimeDelta)
{
	/* [ �ִϸ��̼� ������Ʈ ] */
	__super::Update(fTimeDelta);

	/* [ �Է� ] */
	//HandleInput();
	//UpdateCurrentState(fTimeDelta);


	// �ٲ� ����
	Movement(fTimeDelta);
}

void CPlayer::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
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

	/* [ ���콺 �Է��� ������Ʈ�մϴ�. ] */
	m_Input.bRightMouseDown = MOUSE_DOWN(DIM::RBUTTON);
	m_Input.bRightMousePress = MOUSE_PRESSING(DIM::RBUTTON);
	m_Input.bRightMouseUp = MOUSE_UP(DIM::RBUTTON);
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
		m_pCurrentState = m_StateMap[m_eCurrentState];

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

void CPlayer::SetPlayerState(_float fTimeDelta)
{
	_bool bUp = m_pGameInstance->Key_Pressing(DIK_W);
	_bool bDown = m_pGameInstance->Key_Pressing(DIK_S);
	_bool bLeft = m_pGameInstance->Key_Pressing(DIK_D);
	_bool bRight = m_pGameInstance->Key_Pressing(DIK_A);
	_bool bMove = (bUp || bDown || bLeft || bRight);
	static _bool bRunToggle = false;
	static _bool bSpaceHeld = false;
	static _bool bSprinting = false;
	static _float fPressTime = 0.f;
	const _float sprintTh = 0.8f;

	static _bool bRightMouseHeld = false;
	static _float fChargeTime = 0.f;
	static _bool bCharging = false;
	const _float chargeThreshold = 0.2f; // 0.2

	_bool bSpaceDown = m_pGameInstance->Key_Down(DIK_SPACE);
	_bool bSpacePress = m_pGameInstance->Key_Pressing(DIK_SPACE);
	_bool bSpaceUp = m_pGameInstance->Key_Up(DIK_SPACE);

	_bool bRightMouseDown = m_pGameInstance->Mouse_Down(DIM::RBUTTON);
	_bool bRightMousePress = m_pGameInstance->Mouse_Pressing(DIM::RBUTTON);
	_bool bRightMouseUp = m_pGameInstance->Mouse_Up(DIM::RBUTTON);

	if (m_pGameInstance->Key_Down(DIK_TAB))
	{
		_bool test = m_pAnimator->CheckBool("Move");
		if (test)
		{
			int a = 0;
		}
		m_pAnimator->SetTrigger("EquipWeapon");
		m_pAnimator->ApplyOverrideAnimController("TwoHand");
	}

	if (bSpaceDown)
	{
		if (!bMove)
		{
			m_pAnimator->SetTrigger("Dash");
		}
		else
		{

			bSpaceHeld = true;
			bSprinting = false;
			fPressTime = 0.f;
		}
	}
	if (bMove && bSpaceHeld && bSpacePress)
	{
		fPressTime += fTimeDelta;


		if (!bSprinting && fPressTime >= sprintTh)
		{
			m_pAnimator->SetBool("Sprint", true);
			bRunToggle = true;
			bSprinting = true;
		}
	}
	_int iCombo = m_pAnimator->GetInt("Combo");
	if (m_pGameInstance->Mouse_Down(DIM::LBUTTON))
	{
		if (m_pAnimator->GetCurrentAnim())
		{
			if (m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName.find("Attack") == string::npos)
			{
				if (iCombo == 0)
					iCombo = 1;
				else
					iCombo = 0;
				m_pAnimator->SetInt("Combo", iCombo);
				m_pAnimator->SetTrigger("NormalAttack");
			}
		}
	}

	if (bRightMouseDown)
	{
		// ��Ŭ�� ����
		bRightMouseHeld = true;
		fChargeTime = 0.f;
		bCharging = false;
		m_pAnimator->SetBool("Charge", false); // ���� ���� �� false�� �ʱ�ȭ
	}

	if (bRightMouseHeld && bRightMousePress)
	{
		// ��Ŭ���� ��� ������ �ִ� ����
		fChargeTime += fTimeDelta;

		if (!bCharging && fChargeTime >= chargeThreshold)
		{
			// 0.5�� �̻� ������ ���� ���·� ����
			bCharging = true;
			m_pAnimator->SetBool("Charge", true);
			cout << "Charge activated! Time: " << fChargeTime << endl; // ����׿�
		}
		else
		{
			//m_pAnimator->SetBool("Charge", false);
		}
	}

	if (bRightMouseUp)
	{
		// ��Ŭ���� �� ��
		if (bRightMouseHeld)
		{
			if (m_pAnimator->GetCurrentAnim())
			{
				if (m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName.find("Attack") == string::npos)
				{
					if (iCombo == 0)
						iCombo = 1;
					else
						iCombo = 0;
					m_pAnimator->SetInt("Combo", iCombo);
					m_pAnimator->SetTrigger("StrongAttack");
				}
			}
		}

		// ���� ���� �ʱ�ȭ
		bRightMouseHeld = false;
		fChargeTime = 0.f;
		//if (bCharging)
		//{
		//	bCharging = false;
		//	m_pAnimator->SetBool("Charge", false); // ���� �� ���� ����
		//}
	}

	if (bSpaceHeld && bSpaceUp)
	{
		if (bMove)
		{
			if (!bSprinting)
			{

				m_pAnimator->SetTrigger("Dash");
			}
			else
			{
				m_pAnimator->SetBool("Sprint", false);
			}
		}

		bSpaceHeld = false;
		bSprinting = false;
		fPressTime = 0.f;
	}

	if (!bMove && bSprinting)
	{
		m_pAnimator->SetBool("Sprint", false);
		bSprinting = false;
		bSpaceHeld = false;
		fPressTime = 0.f;
	}

	if (m_pAnimator)
	{
		m_pAnimator->SetBool("Move", bMove);

		if (m_pGameInstance->Key_Down(DIK_Z))
		{
			bRunToggle = !bRunToggle;
		}
		m_pAnimator->SetBool("Run", bRunToggle);
	}
}

void CPlayer::Movement(_float fTimeDelta)
{
	if (!CCamera_Manager::Get_Instance()->GetbMoveable())
		return;

	CAnimation* pCurAnim = m_pAnimator->GetCurrentAnim();
	_bool        bUseRoot = (pCurAnim && pCurAnim->IsRootMotionEnabled());


	if (bUseRoot)
	{
		_float3			rootMotionDelta = m_pAnimator->GetRootMotionDelta();
		_float4 		rootMotionQuat = m_pAnimator->GetRootRotationDelta();
		XMVECTOR vLocal = XMLoadFloat3(&rootMotionDelta);

		_vector vScale, vRotQuat, vTrans;
		XMMatrixDecompose(&vScale, &vRotQuat, &vTrans, m_pTransformCom->Get_WorldMatrix());

		PxControllerFilters filters;
		XMVECTOR vWorldDelta = XMVector3Transform(vLocal, XMMatrixRotationQuaternion(vRotQuat));

		_float dy = XMVectorGetY(vWorldDelta) - 0.8f;
		vWorldDelta = XMVectorSetY(vWorldDelta, dy);

		_float fDeltaMag = XMVectorGetX(XMVector3Length(vWorldDelta));
		_vector finalDelta;
		if (fDeltaMag > m_fSmoothThreshold)
		{
			_float alpha = clamp(fTimeDelta * m_fSmoothSpeed, 0.f, 1.f);
			finalDelta = XMVectorLerp(m_PrevWorldDelta, vWorldDelta, alpha);
		}
		else
		{
			finalDelta = vWorldDelta;
		}
		m_PrevWorldDelta = finalDelta;

		PxVec3 pos{
			XMVectorGetX(finalDelta),
			XMVectorGetY(finalDelta),
			XMVectorGetZ(finalDelta)
		};

		m_pControllerCom->Get_Controller()->move(pos, 0.001f, fTimeDelta, filters);
		SyncTransformWithController();
		_vector vTmp{};
		XMMatrixDecompose(&vScale, &vTmp, &vTrans, m_pTransformCom->Get_WorldMatrix());
		_vector vRotDelta = XMLoadFloat4(&rootMotionQuat);
		_vector vNewRot = XMQuaternionMultiply(vRotDelta, vRotQuat);
		_matrix newWorld =
			XMMatrixScalingFromVector(vScale) *
			XMMatrixRotationQuaternion(vNewRot) *
			XMMatrixTranslationFromVector(vTrans);
		m_pTransformCom->Set_WorldMatrix(newWorld);

		// ���� ȸ�� �� �����
		_float4 rot;
		XMStoreFloat4(&rot, vNewRot);
	}
	else
	{
		SetMoveState(fTimeDelta);
	}

	SetPlayerState(fTimeDelta);
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
			m_pTransformCom->TurnAngle(XMVectorSet(0.f, 1.f, 0.f, 0.f), fClampedAngle);
		}
	}

	/* [ �̵��� �Ѵ�. ] */
	_float3 moveVec = {};
	_float fSpeed = m_pTransformCom->Get_SpeedPreSec();
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
}
