#include "Player.h"

#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "Camera_Manager.h"
#include "PhysX_IgnoreSelfCallback.h"
#include "PhysXController.h"


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


	if (FAILED(__super::Initialize(&pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pCamera_Orbital = CCamera_Manager::Get_Instance()->GetOrbitalCam();
	CCamera_Manager::Get_Instance()->SetPlayer(this);

	return S_OK;
}

void CPlayer::Priority_Update(_float fTimeDelta)
{
	/* [ 캐스케이드 전용 업데이트 함수 ] */
	UpdateShadowCamera();
	/* [ 움직임 전용 함수 ] */
	SetMoveState(fTimeDelta);
	/* [ 스테이트 관리 ] */
	Input_Test(fTimeDelta);
	/* [ 룩 벡터 레이케스트 ] */
	RayCast();

	__super::Priority_Update(fTimeDelta);
}
void CPlayer::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
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

HRESULT CPlayer::Ready_Components()
{
	/* [ 따로 붙일 컴포넌트를 붙여보자 ] */

	return S_OK;
}

void CPlayer::Input_Test(_float fTimeDelta)
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
		// 우클릭 시작
		bRightMouseHeld = true;
		fChargeTime = 0.f;
		bCharging = false;
		m_pAnimator->SetBool("Charge", false); // 차지 시작 시 false로 초기화
	}

	if (bRightMouseHeld && bRightMousePress)
	{
		// 우클릭을 계속 누르고 있는 동안
		fChargeTime += fTimeDelta;

		if (!bCharging && fChargeTime >= chargeThreshold)
		{
			// 0.5초 이상 누르면 차지 상태로 변경
			bCharging = true;
			m_pAnimator->SetBool("Charge", true);
			cout << "Charge activated! Time: " << fChargeTime << endl; // 디버그용
		}
		else
		{
			//m_pAnimator->SetBool("Charge", false);
		}
	}

	if (bRightMouseUp)
	{
		// 우클릭을 뗄 때
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

		// 차지 상태 초기화
		bRightMouseHeld = false;
		fChargeTime = 0.f;
		//if (bCharging)
		//{
		//	bCharging = false;
		//	m_pAnimator->SetBool("Charge", false); // 공격 후 차지 해제
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


HRESULT CPlayer::UpdateShadowCamera()
{
	CShadow::SHADOW_DESC Desc{};

	// 1. 카메라 고정 위치 (예: 공중에 떠있는 위치)
	_vector vFixedEye = XMVectorSet(76.f, 57.f, -21.f, 1.f);

	// 2. 플레이어 현재 위치를 타겟으로 설정
	_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetAt = vPlayerPos;

	// 3. 적용
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
	if (m_pGameInstance->Key_Pressing(DIK_W)) vInputDir += vCamLook;
	if (m_pGameInstance->Key_Pressing(DIK_S)) vInputDir -= vCamLook;
	if (m_pGameInstance->Key_Pressing(DIK_D)) vInputDir += vCamRight;
	if (m_pGameInstance->Key_Pressing(DIK_A)) vInputDir -= vCamRight;


	if (XMVector3Equal(vInputDir, XMVectorZero()))
	{
		_float3 moveVec = {};

		_float fSpeed = m_pTransformCom->Get_SpeedPreSec();
		_float fDist = fSpeed * fTimeDelta;
		vInputDir *= fDist;
		XMStoreFloat3(&moveVec, vInputDir);

		// 중력 적용
		constexpr float fGravity = -9.81f;
		m_vGravityVelocity.y += fGravity * fTimeDelta;
		moveVec.y += m_vGravityVelocity.y * fTimeDelta;

		PxVec3 pxMove(moveVec.x, moveVec.y, moveVec.z);
		PxControllerFilters filters;

		PxControllerCollisionFlags collisionFlags =
			m_pControllerCom->Get_Controller()->move(pxMove, 0.001f, fTimeDelta, filters);

		// 4. 지면에 닿았으면 중력 속도 초기화
		if (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN)
			m_vGravityVelocity.y = 0.f;

		SyncTransformWithController();
		return;
	}

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


	const _float fTurnSpeed = XMConvertToRadians(720.f);
	_float fClampedAngle = max(-fTurnSpeed * fTimeDelta, min(fTurnSpeed * fTimeDelta, fAngle));
	m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fClampedAngle);


	// 7. 이동
	_float3 moveVec = {};

	_float fSpeed = m_pTransformCom->Get_SpeedPreSec();
	_float fDist = fSpeed * fTimeDelta;
	vInputDir *= fDist;
	//어느방향으로 몇만큼 이동한 벡터를 구함
	XMStoreFloat3(&moveVec, vInputDir);

	// 중력 적용
	constexpr float fGravity = -9.81f;
	m_vGravityVelocity.y += fGravity * fTimeDelta;
	moveVec.y += m_vGravityVelocity.y * fTimeDelta;

	PxVec3 pxMove(moveVec.x, moveVec.y, moveVec.z);
	PxControllerFilters filters;

	PxControllerCollisionFlags collisionFlags =
		m_pControllerCom->Get_Controller()->move(pxMove, 0.001f, fTimeDelta, filters);

	// 4. 지면에 닿았으면 중력 속도 초기화
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
