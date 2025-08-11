#include "Player.h"

#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "Camera_Manager.h"
#include "LockOn_Manager.h"
#include "PhysX_IgnoreSelfCallback.h"
#include "PhysXController.h"

#include "PlayerState.h"
#include "Bayonet.h"
#include "Weapon.h"
#include "DH_ToolMesh.h"
#include "StaticMesh.h"

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

	if (FAILED(Ready_Weapon()))
		return E_FAIL;

	if (FAILED(Ready_Lamp()))
		return E_FAIL;

	if (FAILED(Ready_StationDoor()))
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
	if (FAILED(Ready_Actor()))
		return E_FAIL;

	/* [ 위치 초기화 후 컨트롤러 생성 (콜라이더 생성 후) ] */
	if (FAILED(Ready_Controller()))
		return E_FAIL;

	// 서로는 충돌 무시하게
	m_pControllerCom->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
	m_pControllerCom->Add_IngoreActors(m_pControllerCom->Get_Actor());

	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
	m_pPhysXActorCom->Add_IngoreActors(m_pControllerCom->Get_Actor());

	SyncTransformWithController();

	/* [ 카메라 세팅 ] */
	m_pCamera_Manager = CCamera_Manager::Get_Instance();
	CCamera_Manager::Get_Instance()->SetPlayer(this);

	/* [ 락온 세팅 ] */
	CLockOn_Manager::Get_Instance()->SetPlayer(this);

	m_iCurrentHP = m_iMaxHP;
	m_iCurrentStamina = m_iMaxStamina;
	m_iCurrentMana = static_cast<_int>(m_iMaxMana * 0.5f);

	Callback_HP();
	Callback_Mana();
	Callback_Stamina();

	m_pBelt_Up = static_cast<CBelt*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Belt"), nullptr));
	m_pBelt_Down = static_cast<CBelt*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Belt"), nullptr));

	auto pRamp = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Ramp"), nullptr);

	m_pBelt_Down->Add_Item(static_cast<CItem*>(pRamp), 0);

	auto pGrinder = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Grinder"), nullptr);

	m_pBelt_Down->Add_Item(static_cast<CItem*>(pGrinder), 1);

	auto pPortion = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Portion"), nullptr);

	m_pBelt_Up->Add_Item(static_cast<CItem*>(pPortion), 0);


	m_pSelectItem = m_pBelt_Up->Get_Items()[0];
	

	Callback_DownBelt();
	Callback_UpBelt();	

	return S_OK;
}



void CPlayer::Priority_Update(_float fTimeDelta)
{
	if (KEY_DOWN(DIK_CAPSLOCK))
	{
		_vector pos = m_pTransformCom->Get_State(STATE::POSITION);
		printf("PlayerPos X:%f, Y:%f, Z:%f\n", XMVectorGetX(pos), XMVectorGetY(pos), XMVectorGetZ(pos));
	}

	/* [ 캐스케이드 전용 업데이트 함수 ] */
	UpdateShadowCamera();
	/* [ 룩 벡터 레이케스트 ] */
	RayCast(m_pControllerCom);

	// 옵저버 변수들 처리
	Update_Stat();
	Update_Slot();

	/* [ 상호작용 ] */
	Interaction_Door();
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
	SlidDoorMove(fTimeDelta);
	UpdateCurrentState(fTimeDelta);
	Movement(fTimeDelta);

	Update_Collider_Actor();

	// 락온관련
	if (m_pGameInstance->Mouse_Down(DIM::WHEELBUTTON))
		CLockOn_Manager::Get_Instance()->Set_Active();
}

void CPlayer::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
	
	/* [ 특수행동 ] */
	ItemLampON(fTimeDelta);
	ItemWeaponOFF(fTimeDelta);
	SitAnimationMove(fTimeDelta);

	/* [ 이곳은 애니메이션 실험실입니다. ] */
	if(KEY_DOWN(DIK_Y))
	{
		//m_pAnimator->ApplyOverrideAnimController("TwoHand");
		//string strName = m_pAnimator->GetCurrentAnimName();
		//m_pAnimator->SetBool("Charge", true);
		//m_pAnimator->SetTrigger("StrongAttack");
		//m_pAnimator->SetInt("Combo", 1);
		m_pAnimator->Get_CurrentAnimController()->SetState("SlidingDoor");
	}
	if (KEY_PRESSING(DIK_U))
	{
		_bool ab = m_pTransformCom->RotateToDirectionSmoothly(_fvector{ 0.f, 0.f, -1.f, 0.f }, fTimeDelta);
		if (ab)
			int a = 0;
	}

	if (KEY_DOWN(DIK_T))
	{
		static _bool bCharge = false;
		static _int iTestCombo = 0;
			m_pAnimator->SetInt("ArmCombo", iTestCombo++);
			iTestCombo %= 2;
		m_pAnimator->SetBool("Charge", bCharge);
		bCharge = !bCharge;
		m_pAnimator->SetTrigger("ArmAttack");
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
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif

	return S_OK;
}

CAnimController* CPlayer::GetCurrentAnimContrller()
{
	return m_pAnimator->Get_CurrentAnimController();
}

void CPlayer::SitAnimationMove(_float fTimeDelta)
{
	if (m_pCamera_Manager->Get_StartGame())
	{
		m_fSitTime += fTimeDelta;
		_float  m_fTime = 1.5f;
		_float  m_fDistance = 0.8f;

		if (!m_bSit)
		{
			if (1.7f < m_fSitTime)
			{
				_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
				m_bSit = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
				SyncTransformWithController();
			}
		}
	}
}

void CPlayer::HandleInput()
{
	/* [ 키 입력을 업데이트합니다. ] */
	if(KEY_PRESSING(DIK_W) || KEY_PRESSING(DIK_S) || KEY_PRESSING(DIK_A) || KEY_PRESSING(DIK_D))
		m_Input.bMove = true;
	else
		m_Input.bMove = false;

	/* [ 마우스 입력을 업데이트합니다. ] */
	m_Input.bLeftMouseDown = MOUSE_DOWN(DIM::LBUTTON);
	m_Input.bRightMouseDown = MOUSE_DOWN(DIM::RBUTTON);
 	m_Input.bRightMousePress = MOUSE_PRESSING(DIM::RBUTTON);
	m_Input.bRightMouseUp = MOUSE_UP(DIM::RBUTTON);


	/* [ 특수키 입력을 업데이트합니다. ] */
	m_Input.bShift = KEY_PRESSING(DIK_LSHIFT);
	m_Input.bCtrl = false;//KEY_DOWN(DIK_LCONTROL);
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
	TriggerStateEffects(fTimeDelta);
}

void CPlayer::TriggerStateEffects(_float fTimeDelta)
{
	string stateName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
	//printf("Current State: %s\n", stateName.c_str());

	// 상태가 바뀌었으면 초기화
	if (m_strPrevStateName != stateName)
	{
		m_strPrevStateName = stateName;
		m_fMoveTime = 0.f;
		m_iMoveStep = 0;
		m_bMove = false;
	}
	
	eAnimCategory eCategory = GetAnimCategoryFromName(stateName);
	//printf("Anim Category: %d\n", static_cast<int>(eCategory));

	switch (eCategory)
	{
	case eAnimCategory::NORMAL_ATTACKA:
	{
		m_fMoveTime += fTimeDelta;
		_float  m_fTime = 0.2f;
		_float  m_fDistance = 1.f;

		if (!m_bMove)
		{
			if (0.65f < m_fMoveTime)
			{
				_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
				m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
				SyncTransformWithController();
			}
		}
		break;
	}
	case eAnimCategory::NORMAL_ATTACKB:
	{
		m_fMoveTime += fTimeDelta;
		_float  m_fTime = 0.3f;
		_float  m_fDistance = 1.f;

		if (!m_bMove)
		{
			if (0.55f < m_fMoveTime)
			{
				_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
				m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
				SyncTransformWithController();
			}
		}
		break;
	}
	case eAnimCategory::STRONG_ATTACKA:
	{
		m_fMoveTime += fTimeDelta;
		_float  m_fTime = 0.3f;
		_float  m_fDistance = 1.f;

		if (!m_bMove)
		{
			if (0.25f < m_fMoveTime)
			{
				_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
				m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
				SyncTransformWithController();
			}
		}
		break;
	}
	case eAnimCategory::STRONG_ATTACKB:
	{
		m_fMoveTime += fTimeDelta;
		_float  m_fTime = 0.2f;
		_float  m_fDistance = 1.f;

		if (!m_bMove)
		{
			if (0.4f < m_fMoveTime)
			{
				_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
				m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
				SyncTransformWithController();
			}
		}
		break;
	}
	case eAnimCategory::CHARGE_ATTACKA:
	{
		RootMotionActive(fTimeDelta);

		/* [ 차지 A 일 때 R버튼이 꾹 눌리면 체인지 변수가 켜진다. ] */
		m_fChangeTimeElaped += fTimeDelta;

		if (m_fChangeTimeElaped > 1.f)
		{
			if (MOUSE_PRESSING(DIM::RBUTTON))
			{
				m_fChangeTime += fTimeDelta;

				if (m_fChangeTime >= 0.5f)
				{
					m_bIsChange = true;
				}
			}
			else
			{
				m_fChangeTime = 0.f;
				m_bIsChange = false;
			}
		}

		break;
	}
	case eAnimCategory::CHARGE_ATTACKB:
	{
		RootMotionActive(fTimeDelta);

		break;
	}
	case eAnimCategory::IDLE:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
		break;
	}
	case eAnimCategory::WALK:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
		break;
	}
	case eAnimCategory::RUN:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fRunSpeed);
		break;
	}
	case eAnimCategory::SPRINT:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fSprintSpeed);
		break;
	}
	case eAnimCategory::EQUIP:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
		break;
	}
	case eAnimCategory::GUARD:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
		break;
	}
	case eAnimCategory::ITEM_WALK:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
		break;
	}
	case eAnimCategory::DASH_BACK:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fSprintSpeed);

		m_fMoveTime += fTimeDelta;
		_float  m_fTime = 0.4f;
		_float  m_fDistance = 3.f;

		if (!m_bMove)
		{
			_vector vLook = m_pTransformCom->Get_State(STATE::LOOK) * -1.f;
			m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
			SyncTransformWithController();
		}

		break;
	}
	case eAnimCategory::DASH_FRONT:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fSprintSpeed);

		m_fMoveTime += fTimeDelta;
		_float  m_fTime = 0.5f;
		_float  m_fDistance = 3.5f;

		if (!m_bMove)
		{
			_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
			m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
			SyncTransformWithController();
		}

		break;
	}
	case eAnimCategory::SPRINT_ATTACK:
	{
		RootMotionActive(fTimeDelta);

		break;
	}
	case eAnimCategory::FIRSTDOOR:
	{
		RootMotionActive(fTimeDelta);

		break;
	}

	default:
		break;
	}
}

CPlayer::eAnimCategory CPlayer::GetAnimCategoryFromName(const string& stateName)
{
	if (stateName == "Idle") return eAnimCategory::IDLE; 

	if (stateName.find("Walk") == 0) return eAnimCategory::WALK;
	if (stateName.find("Run") == 0) return eAnimCategory::RUN;

	if (stateName.find("Dash_Normal_B") == 0 || stateName.find("Dash_Focus_B") == 0)
		return eAnimCategory::DASH_BACK;
	if (stateName.find("Dash_") == 0) return eAnimCategory::DASH_FRONT;

	if (stateName.find("SprintNormalAttack") == 0 || stateName.find("SprintStrongAttack") == 0)
		return eAnimCategory::SPRINT_ATTACK;
	if (stateName.find("Sprint") == 0) return eAnimCategory::SPRINT;

	if (stateName.find("Guard_Hit") == 0 || stateName.find("Guard_Break") == 0)
		return eAnimCategory::GUARD_HIT;
	if (stateName.find("Guard") == 0) return eAnimCategory::GUARD;

	if (stateName.find("EquipWeapon") == 0) return eAnimCategory::EQUIP;
	if (stateName.find("PutWeapon") == 0) return eAnimCategory::EQUIP;

	if (stateName.find("OnLamp_Walk") == 0 || stateName.find("FailItem_Walk") == 0)
		return eAnimCategory::ITEM_WALK;
	if (stateName.find("OnLamp") == 0 || stateName.find("FailItem") == 0)
		return eAnimCategory::ITEM;

	if (stateName.find("NormalAttack2") == 0)
		return eAnimCategory::NORMAL_ATTACKA;
	if (stateName.find("NormalAttack") == 0)
		return eAnimCategory::NORMAL_ATTACKB;
	if (stateName.find("StrongAttack2") == 0)
		return eAnimCategory::STRONG_ATTACKB;
	if (stateName.find("StrongAttack") == 0)
		return eAnimCategory::STRONG_ATTACKA;
	if (stateName.find("ChargeStrongAttack2") == 0)
		return eAnimCategory::CHARGE_ATTACKB;
	if (stateName.find("ChargeStrongAttack") == 0)
		return eAnimCategory::CHARGE_ATTACKA;

	if (stateName.find("MainSkill") == 0)
		return eAnimCategory::MAINSKILL;

	if (stateName.find("Sit") == 0)
		return eAnimCategory::SIT;

	if (stateName.find("SlidingDoor") == 0)
		return eAnimCategory::FIRSTDOOR;

	return eAnimCategory::NONE;
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

		CIgnoreSelfCallback filter(m_pControllerCom->Get_IngoreActors());
		PxControllerFilters filters;
		filters.mFilterCallback = &filter; 

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

void CPlayer::Update_Collider_Actor()
{
	// 1. 월드 행렬 가져오기
	_matrix worldMatrix = m_pTransformCom->Get_WorldMatrix();

	// 2. 위치 추출
	_float4 vPos;
	XMStoreFloat4(&vPos, worldMatrix.r[3]);

	PxVec3 pos(vPos.x, vPos.y, vPos.z);
	pos.y += 0.5f;

	// 3. 회전 추출
	XMVECTOR boneQuat = XMQuaternionRotationMatrix(worldMatrix);
	XMFLOAT4 fQuat;
	XMStoreFloat4(&fQuat, boneQuat);
	PxQuat rot = PxQuat(fQuat.x, fQuat.y, fQuat.z, fQuat.w);

	// 4. PhysX Transform 적용
	m_pPhysXActorCom->Set_Transform(PxTransform(pos, rot));

	// 무기 추가
}

void CPlayer::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CPlayer::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CPlayer::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CPlayer::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CPlayer::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CPlayer::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
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
	m_pStateArray[ENUM_CLASS(EPlayerState::CHARGEA)] = new CPlayer_ChargeA(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::CHARGEB)] = new CPlayer_ChargeB(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::GARD)] = new CPlayer_Gard(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::SPRINTATTACKA)] = new CPlayer_SprintAttackA(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::SPRINTATTACKB)] = new CPlayer_SprintAttackB(this);

	m_pCurrentState = m_pStateArray[ENUM_CLASS(EPlayerState::IDLE)];
}



HRESULT CPlayer::Ready_Weapon()
{
	/* [ 무기 모델을 추가 ] */

	CBayonet::BAYONET_DESC Desc{};
	Desc.eLevelID = LEVEL::STATIC;
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.InitPos = { 0.f, 0.f, 0.f };
	Desc.InitScale = { 1.f, 1.f, 1.f };
	Desc.iRender = 0;
	
	Desc.szMeshID = TEXT("PlayerWeapon");
	lstrcpy(Desc.szName, TEXT("PlayerWeapon"));

	Desc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("BN_Weapon_R"));
	Desc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_PlayerWeapon"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION),TEXT("Player_Weapon"), &pGameObject, &Desc)))
		return E_FAIL;

	m_pWeapon = dynamic_cast<CWeapon*>(pGameObject);

	

	return S_OK;
}

HRESULT CPlayer::Ready_Lamp()
{
	CDH_ToolMesh::tagDH_ToolDesc Desc{};
	Desc.eLEVEL = LEVEL::KRAT_CENTERAL_STATION; // 생성레벨
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.iID = 0;
	Desc.m_vInitPos = { 0.f, 0.f, 0.f };
	Desc.szMeshID = TEXT("PointLight");
	lstrcpy(Desc.szName, TEXT("PointLight"));

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_ToolMesh"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), L"Layer_ToolMesh", &pGameObject, &Desc)))
		return E_FAIL;

	m_pLamp = dynamic_cast<CDH_ToolMesh*>(pGameObject);

	m_pLamp->SetDebug(false);
	m_pLamp->SetbVolumetric(false);
	m_pLamp->SetRange(3.f);
	m_pLamp->SetColor(_float4(1.f, 0.7f, 0.4f, 1.f));
	return S_OK;
}

HRESULT CPlayer::Ready_StationDoor()
{
	CStaticMesh::STATICMESH_DESC Desc{};
	Desc.iRender = 0;
	Desc.bUseOctoTree = false;
	Desc.m_eLevelID = LEVEL::KRAT_CENTERAL_STATION;
	Desc.szMeshID = TEXT("SM_Station_TrainDoor");
	lstrcpy(Desc.szName, TEXT("SM_Station_TrainDoor"));

	/* 문자열 받는 곳 */
	wstring ModelPrototypeTag = TEXT("Prototype_Component_Model_SM_Station_TrainDoor");
	lstrcpy(Desc.szModelPrototypeTag, ModelPrototypeTag.c_str());

	_float3 vPosition = _float3(52.6f, 0.02f, -2.4f);
	_matrix matWorld = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
	_float4x4 matWorldFloat;
	XMStoreFloat4x4(&matWorldFloat, matWorld);
	Desc.WorldMatrix = matWorldFloat;
	//Desc.eColliderType = COLLIDER_TYPE::CONVEX;

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_StaticMesh"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("TrainDoor"), &pGameObject, &Desc)))
		return E_FAIL;

	m_pInterectionStuff = pGameObject;

	return S_OK;
}

HRESULT CPlayer::Ready_Components()
{
	/* [ 따로 붙일 컴포넌트를 붙여보자 ] */

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Controller"),
		TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pControllerCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), 
		TEXT("Com_PhysX2"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;


	return S_OK;
}
HRESULT CPlayer::Ready_Controller()
{
	m_pHitReport = new CPhysXControllerHitReport();
	m_pHitReport->AddIgnoreActor(m_pPhysXActorCom->Get_Actor());

	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxExtendedVec3 pos(positionVec.x, positionVec.y, positionVec.z);
	m_pControllerCom->Create_Controller(m_pGameInstance->Get_ControllerManager(), m_pGameInstance->GetMaterial(L"Default"), pos, 0.4f, 1.0f, m_pHitReport);
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

HRESULT CPlayer::Ready_Actor()
{
	// 3. Transform에서 S, R, T 분리
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 3-1. 스케일, 회전, 위치 변환
	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
	positionVec += PxVec3{ 0.f,0.5f,0.f };
	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxCapsuleGeometry  geom = m_pGameInstance->CookCapsuleGeometry(0.4f, 0.8f);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_PLAYERBODY;
	filterData.word1 = WORLDFILTER::FILTER_MONSTERBODY | FILTER_MONSTERWEAPON; 
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::PALYER);
	m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
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
		if (m_iCurrentHP < 0)
			m_iCurrentHP = 0;

		m_iCurrentStamina -= 20;
		if (m_iCurrentStamina < 0)
			m_iCurrentStamina = 0;
		m_iCurrentMana -= 100;
		if (m_iCurrentMana < 0)
			m_iCurrentMana = 0;
		Callback_HP();
		Callback_Mana();
		Callback_Stamina();
	}
}

void CPlayer::Interaction_Door()
{
	if (KEY_DOWN(DIK_E))
	{
		_float3 vTriggerCenter = _float3(52.6f, 0.02f, -2.4f);
		_float fTriggerRadius = 5.f; // 3미터 반경

		// 플레이어 위치에서 거리 계산
		_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
		_float3 vPlayerPos = {
			XMVectorGetX(vPosition),
			XMVectorGetY(vPosition),
			XMVectorGetZ(vPosition)
		};

		_float fDistSq =
			powf(vPlayerPos.x - vTriggerCenter.x, 2) +
			powf(vPlayerPos.y - vTriggerCenter.y, 2) +
			powf(vPlayerPos.z - vTriggerCenter.z, 2);

		// 거리 조건 검사 (제곱된 거리 비교로 최적화)
		if (fDistSq <= fTriggerRadius * fTriggerRadius)
		{
			if (!m_bCutsceneDoor)
			{
				Play_CutScene_Door();
				m_bCutsceneDoor = true;
			}
		}
	}
}

void CPlayer::Play_CutScene_Door()
{	
	//m_pCamera_Manager->Play_CutScene(CUTSCENE_TYPE::ONE);
	m_bInteraction[0] = true;
	m_bInteractionMove[0] = true;
	m_bInteractionRotate[0] = true;
}

void CPlayer::ItemWeaponOFF(_float fTimeDelta)
{
	if (m_bItemSwitch)
	{
		m_fItemTime += fTimeDelta;

		if (m_fItemTime >= 2.f)
		{
			if(m_bWeaponEquipped)
				m_pWeapon->SetbIsActive(true);

			m_bItemSwitch = false;
			m_fItemTime = 0.f;
		}
	}
}
void CPlayer::ItemLampON(_float fTimeDelta)
{
	if(m_bLampOnOff)
	{
		m_pLamp->SetIntensity(1.5f);
		_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
		_vector vPosition = matWorld.r[3];
		//vPosition = XMVectorSetX(vPosition, XMVectorGetX(vPosition) - 1.f);
		vPosition = XMVectorSetY(vPosition, XMVectorGetY(vPosition) + 1.f);
		vPosition = XMVectorSetZ(vPosition, XMVectorGetZ(vPosition) + 1.f);

		matWorld.r[3] = vPosition;

		m_pLamp->Get_TransfomCom()->Set_WorldMatrix(matWorld);
	}
	else
	{
		m_pLamp->SetIntensity(0.f);
	}
}
void CPlayer::SlidDoorMove(_float fTimeDelta)
{
	if (m_bInteractionMove[0])
	{
		/* [ 위치로 이동 ] */
		m_Input.bMove = true;
		m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
		_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
		_bool SetPosition = m_pTransformCom->Go_FrontByPosition(fTimeDelta, _fvector{ 53.8f, XMVectorGetY(vPosition), -1.6f, 1.f}, m_pControllerCom);
		if (SetPosition)
		{
			m_bInteractionMove[0] = false; // 이동 완료
		}
	}

	if (m_bInteractionRotate[0])
	{
		// 이동 완료 시 회전
		_bool vRotate = m_pTransformCom->RotateToDirectionSmoothly(_fvector{ 0.f , 0.f, -1.f, 0.f }, fTimeDelta);
		if (!vRotate)
		{
			m_pAnimator->Get_CurrentAnimController()->SetState("SlidingDoor");
			m_bInteractionRotate[0] = false; // 회전 완료
		}
	}

	if (m_bInteraction[0] && !m_bInteractionMove[0] && !m_bInteractionRotate[0])
	{
		m_fInteractionTime[0] += fTimeDelta;

		//손 뼈의 컴바인드 행렬
		const _float4x4* pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("BN_Weapon_L"));
		_matrix matSocketLocal = XMLoadFloat4x4(pSocketMatrix);

		_matrix matSocketWorld = matSocketLocal * m_pTransformCom->Get_WorldMatrix();
		_vector vHandWorldPos = matSocketWorld.r[3];

		_float fCurrentHandX = XMVectorGetX(vHandWorldPos);

		static _float fStartHandX = fCurrentHandX;
		_float fDeltaX = fCurrentHandX - fStartHandX;

		// 문 위치 적용
		CTransform* DoorTransCom = m_pInterectionStuff->Get_TransfomCom();
		static _vector vDoorStartPos = DoorTransCom->Get_State(STATE::POSITION);

		_vector vNewDoorPos = vDoorStartPos + XMVectorSet(fDeltaX, 0.f, 0.f, 0.f);
		DoorTransCom->Set_State(STATE::POSITION, vNewDoorPos);

		// 컷씬이 끝날 조건 넣어주면 좋음
		if (fDeltaX <= -2.2f)
		{
			m_bInteraction[0] = false; // 컷씬 종료
		}
		
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
			m_pSelectItem = m_pBelt_Up->Get_Current_Item();
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
				m_pSelectItem = m_pBelt_Down->Get_Current_Item();
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

	if (m_pGameInstance->Key_Down(DIK_TAB))
	{	
		if(m_bSwitch)
			m_pGameInstance->Notify(TEXT("Weapon_Status"), TEXT("EquipWeapon"), m_pWeapon);
		else
			m_pGameInstance->Notify(TEXT("Weapon_Status"), TEXT("EquipWeapon"), nullptr);

		m_bSwitch = !m_bSwitch;
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
	_vector vPos = XMVectorSet((float)pos.x, (float)pos.y - 0.9f, (float)pos.z, 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
}

HRESULT CPlayer::UpdateShadowCamera()
{
	CShadow::SHADOW_DESC Desc{};

	_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetEye = vPlayerPos + XMVectorSet(-10.f, 30.f, 10.f, 0.f);
	_vector vTargetAt = vPlayerPos;

	m_vShadowCam_Eye = vTargetEye;
	m_vShadowCam_At = vTargetAt;

	XMStoreFloat4(&Desc.vEye, m_vShadowCam_Eye);
	XMStoreFloat4(&Desc.vAt, m_vShadowCam_At);
	Desc.fNear = 0.1f;
	Desc.fFar = 1000.f;

	Desc.fFovy = XMConvertToRadians(20.0f);
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWA)))
		return E_FAIL;

	vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
	vTargetEye = vPlayerPos + XMVectorSet(-10.f, 40.f, 10.f, 0.f);
	vTargetAt = vPlayerPos;

	m_vShadowCam_Eye = vTargetEye;
	m_vShadowCam_At = vTargetAt;

	XMStoreFloat4(&Desc.vEye, m_vShadowCam_Eye);
	XMStoreFloat4(&Desc.vAt, m_vShadowCam_At);
	Desc.fNear = 0.1f;
	Desc.fFar = 1000.f;
	Desc.fFovy = XMConvertToRadians(80.0f);
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWB)))
		return E_FAIL;

	vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
	vTargetEye = vPlayerPos + XMVectorSet(-10.f, 60.f, 10.f, 0.f);
	vTargetAt = vPlayerPos;

	m_vShadowCam_Eye = vTargetEye;
	m_vShadowCam_At = vTargetAt;

	XMStoreFloat4(&Desc.vEye, m_vShadowCam_Eye);
	XMStoreFloat4(&Desc.vAt, m_vShadowCam_At);
	Desc.fNear = 0.1f;
	Desc.fFar = 1000.f;
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

			string strName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
			if (m_MovableStates.find(strName) == m_MovableStates.end())
			{
				fClampedAngle = 0.f;
			}
			m_pTransformCom->TurnAngle(XMVectorSet(0.f, 1.f, 0.f, 0.f), fClampedAngle);
		}
	}

	/* [ 이동을 한다. ] */
	_float3 moveVec = {};
	_float fSpeed = m_pTransformCom->Get_SpeedPreSec();
	if (!m_bMovable)    fSpeed = 0.f;
	string strName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
	if (m_MovableStates.find(strName) == m_MovableStates.end())
	{
		fSpeed = 0.f;
	}
	_float fDist = fSpeed * fTimeDelta;
	vInputDir *= fDist;
	XMStoreFloat3(&moveVec, vInputDir);

	// 중력 적용
	constexpr _float fGravity = -9.81f;
	m_vGravityVelocity.y += fGravity * fTimeDelta;
	moveVec.y += m_vGravityVelocity.y * fTimeDelta;

	PxVec3 pxMove(moveVec.x, moveVec.y, moveVec.z);

	CIgnoreSelfCallback filter(m_pControllerCom->Get_IngoreActors());
	PxControllerFilters filters;
	filters.mFilterCallback = &filter; // 필터 콜백 지정
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
	Safe_Release(m_pPhysXActorCom);

	for (size_t i = 0; i < ENUM_CLASS(EPlayerState::END); ++i)
		Safe_Delete(m_pStateArray[i]);

	Safe_Release(m_pBelt_Down);
	Safe_Release(m_pBelt_Up);

	Safe_Delete(m_pHitReport);
}
