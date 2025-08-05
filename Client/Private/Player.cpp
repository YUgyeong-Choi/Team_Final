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

#include "Belt.h"
#include "Item.h"
#include "Ramp.h"

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

	/* [ 플레이어 제이슨 로딩 ] */
	LoadPlayerFromJson();

	/* [ 스테이트 시작 ] */
	ReadyForState();

	/* [ 초기화 위치값 ] */
	m_pTransformCom->Set_State(STATE::POSITION, _vector{ m_InitPos.x, m_InitPos.y, m_InitPos.z });
	m_pTransformCom->Rotation(XMConvertToRadians(0.f), XMConvertToRadians(90.f), XMConvertToRadians(0.f));
	m_pTransformCom->SetUp_Scale(pDesc->InitScale.x, pDesc->InitScale.y, pDesc->InitScale.z);

	/* [ 위치 초기화 후 콜라이더 생성 ] */
	if (FAILED(Ready_Controller()))
		return E_FAIL;

	m_pCamera_Orbital = CCamera_Manager::Get_Instance()->GetOrbitalCam();
	CCamera_Manager::Get_Instance()->SetPlayer(this);
	SyncTransformWithController();



	m_iCurrentHP = m_iMaxHP;

	Callback_HP();
	Callback_Mana();
	Callback_Stamina();

	m_pBelt_Up = static_cast<CBelt*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Belt"), nullptr));
	m_pBelt_Down = static_cast<CBelt*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Belt"), nullptr));

	auto pRamp = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Ramp"), nullptr);

	m_pBelt_Down->Add_Item(static_cast<CRamp*>(pRamp), 0);

	Callback_UpBelt();
	Callback_DownBelt();
	

	return S_OK;
}



void CPlayer::Priority_Update(_float fTimeDelta)
{
	// 문여는 컷씬
	if (KEY_DOWN(DIK_N))
		CCamera_Manager::Get_Instance()->Play_CutScene(CUTSCENE_TYPE::ONE);

	/* [ 캐스케이드 전용 업데이트 함수 ] */
	UpdateShadowCamera();
	/* [ 룩 벡터 레이케스트 ] */
	RayCast(m_pControllerCom);


	// 옵저버 변수들 처리
	Update_Stat();
	Update_Slot();


	__super::Priority_Update(fTimeDelta);
}
void CPlayer::Update(_float fTimeDelta)
{
	/* [ 애니메이션 업데이트 ] */
	__super::Update(fTimeDelta);

	// 컷씬일 때 못 움직이도록
	if (!CCamera_Manager::Get_Instance()->GetbMoveable())
		return;

	/* [ 입력 ] */
	HandleInput();
	UpdateCurrentState(fTimeDelta);

	// 바꿀 예정
	Movement(fTimeDelta);
}

void CPlayer::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
	
	/* [ 이곳은 애니메이션 실험실입니다. ] */
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

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		m_pGameInstance->Add_DebugComponent(m_pControllerCom);
	}
#endif

	return S_OK;
}

void CPlayer::HandleInput()
{
	/* [ 키 입력을 업데이트합니다. ] */
	m_Input.bUp = KEY_PRESSING(DIK_W);
	m_Input.bDown = KEY_PRESSING(DIK_S);
	m_Input.bLeft = KEY_PRESSING(DIK_A);
	m_Input.bRight = KEY_PRESSING(DIK_D);

	m_Input.bUp_Pressing = KEY_PRESSING(DIK_W);
	m_Input.bDown_Pressing = KEY_PRESSING(DIK_S);
	m_Input.bLeft_Pressing = KEY_PRESSING(DIK_A);
	m_Input.bRight_Pressing = KEY_PRESSING(DIK_D);

	/* [ 마우스 입력을 업데이트합니다. ] */
	m_Input.bLeftMouseDown = MOUSE_DOWN(DIM::LBUTTON);
	m_Input.bRightMouseDown = MOUSE_DOWN(DIM::RBUTTON);
 	m_Input.bRightMousePress = MOUSE_PRESSING(DIM::RBUTTON);
	m_Input.bRightMouseUp = MOUSE_UP(DIM::RBUTTON);


	/* [ 특수키 입력을 업데이트합니다. ] */
	m_Input.bShift = KEY_PRESSING(DIK_LSHIFT);
	m_Input.bCtrl = KEY_DOWN(DIK_LCONTROL);
	m_Input.bTap = KEY_DOWN(DIK_TAB);
	m_Input.bItem = KEY_DOWN(DIK_R);
	m_Input.bSpaceUP = KEY_UP(DIK_SPACE);
	m_Input.bSpaceDown = KEY_DOWN(DIK_SPACE);
	
	/* [ 뛰기 걷기를 토글합니다. ] */
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

void CPlayer::RootMotionActive(_float fTimeDelta)
{
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

		_float4 rot;
		XMStoreFloat4(&rot, vNewRot);
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
	m_pStateArray[ENUM_CLASS(EPlayerState::GARD)] = new CPlayer_Gard(this);

	m_pCurrentState = m_pStateArray[ENUM_CLASS(EPlayerState::IDLE)];
}



HRESULT CPlayer::Ready_Components()
{
	/* [ 따로 붙일 컴포넌트를 붙여보자 ] */

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Controller"),
		TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pControllerCom))))
		return E_FAIL;

	return S_OK;
}
HRESULT CPlayer::Ready_Controller()
{
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxExtendedVec3 pos(positionVec.x, positionVec.y, positionVec.z);
	m_pControllerCom->Create_Controller(m_pGameInstance->Get_ControllerManager(), m_pGameInstance->GetMaterial(L"Default"), pos, 0.4f, 1.0f);
	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_PLAYERBODY;
	filterData.word1 = WORLDFILTER::FILTER_MONSTERBODY;
	m_pControllerCom->Set_SimulationFilterData(filterData);
	m_pControllerCom->Set_QueryFilterData(filterData);
	m_pControllerCom->Set_Owner(this);
	m_pControllerCom->Set_ColliderType(COLLIDERTYPE::E);
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

void CPlayer::Callback_UpBelt()
{
	m_pGameInstance->Notify(TEXT("Slot_Belts"), _wstring(L"ChangeUpBelt"), m_pBelt_Up);
}

void CPlayer::Callback_DownBelt()
{
	m_pGameInstance->Notify(TEXT("Slot_Belts"), _wstring(L"ChangeDownBelt"), m_pBelt_Down);
}

void CPlayer::Use_Item()
{
	if (nullptr == m_pSelectItem)
		return;

	m_pSelectItem->Activate();
}

void CPlayer::Update_Slot()
{
	if (m_pGameInstance->Key_Down(DIK_T))
	{
		if (m_isSelectUpBelt)
		{
			m_pBelt_Up->Change_Next_Item();
		}
		else
		{
			m_pSelectItem = m_pBelt_Up->Get_Current_Item();
		}
		
		m_isSelectUpBelt = true;

		Callback_UpBelt();
	}
	else if (m_pGameInstance->Key_Down(DIK_G))
	{
		if (!m_isSelectUpBelt)
		{
			m_pBelt_Down->Change_Next_Item();
		}
		else
		{
			m_pSelectItem = m_pBelt_Down->Get_Current_Item();
		}
			
		m_isSelectUpBelt = false;


		Callback_DownBelt();
	}

	if (m_pGameInstance->Key_Down(DIK_R))
	{
		if (nullptr == m_pSelectItem)
			return;

		m_pSelectItem->Activate();

		if (m_isSelectUpBelt)
			m_pGameInstance->Notify(TEXT("Slot_Belts"), TEXT("UseUpSelectItem"), m_pSelectItem);
		else
			m_pGameInstance->Notify(TEXT("Slot_Belts"), TEXT("UseDownSelectItem"), m_pSelectItem);
	}
}

void CPlayer::Movement(_float fTimeDelta)
{
	if (!CCamera_Manager::Get_Instance()->GetbMoveable())
		return;

	SetMoveState(fTimeDelta);
}

void CPlayer::SyncTransformWithController()
{
	if (!m_pControllerCom) return;

	PxExtendedVec3 pos = m_pControllerCom->Get_Controller()->getPosition();
	_vector vPos = XMVectorSet((float)pos.x, (float)pos.y - 1.0f, (float)pos.z, 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
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
	if (KEY_PRESSING(DIK_W)) vInputDir += vCamLook;
	if (KEY_PRESSING(DIK_S)) vInputDir -= vCamLook;
	if (KEY_PRESSING(DIK_D)) vInputDir += vCamRight;
	if (KEY_PRESSING(DIK_A)) vInputDir -= vCamRight;

	/* [ 입력값이 있으면? ] */
	if (!XMVector3Equal(vInputDir, XMVectorZero()))
	{
		/* [ 회전을 시킨다. ] */
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

		// 회전각을 직접 계산했으니 그대로 사용 가능
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

	/* [ 이동을 한다. ] */
	_float3 moveVec = {};
	_float fSpeed = m_pTransformCom->Get_SpeedPreSec();
	if (!m_bMovable)    fSpeed = 0.f;
	_float fDist = fSpeed * fTimeDelta;
	vInputDir *= fDist;
	XMStoreFloat3(&moveVec, vInputDir);

	// 중력 적용
	constexpr _float fGravity = -9.81f;
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

	for (size_t i = 0; i < ENUM_CLASS(EPlayerState::END); ++i)
		Safe_Delete(m_pStateArray[i]);

	Safe_Release(m_pBelt_Down);
	Safe_Release(m_pBelt_Up);
}
