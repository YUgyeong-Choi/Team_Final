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
#include "Lamp.h"
#include "PlayerLamp.h"
#include "PlayerFrontCollider.h"

#include "LegionArm_Base.h"
#include "LegionArm_Steel.h"

#include "EffectContainer.h"
#include "Effect_Manager.h"

#include "DoorMesh.h"

CPlayer::CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUnit(pDevice, pContext)
{
}
CPlayer::CPlayer(const CPlayer& Prototype)
	: CUnit(Prototype)
{
	m_eUnitType = EUnitType::PLAYER;
}

HRESULT CPlayer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayer::Initialize(void* pArg)
{
	PLAYER_DESC* pDesc = static_cast<PLAYER_DESC*>(pArg);
	m_bIsPlayer = true;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_Weapon()))
		return E_FAIL;

	if (FAILED(Ready_Lamp()))
		return E_FAIL;
	
	if (FAILED(Ready_FrontCollider()))
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
	m_pControllerCom->Add_IngoreActors(static_cast<CBayonet*>(m_pWeapon)->Get_PhysXActor()->Get_Actor());

	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
	m_pPhysXActorCom->Add_IngoreActors(m_pControllerCom->Get_Actor());
	m_pPhysXActorCom->Add_IngoreActors(static_cast<CBayonet*>(m_pWeapon)->Get_PhysXActor()->Get_Actor());

	SyncTransformWithController();

	/* [ 카메라 세팅 ] */
	m_pCamera_Manager = CCamera_Manager::Get_Instance();
	CCamera_Manager::Get_Instance()->SetPlayer(this);

	/* [ 락온 세팅 ] */
	m_pLockOn_Manager = CLockOn_Manager::Get_Instance();
	m_pLockOn_Manager->SetPlayer(this);
	m_iLockonBoneIndex = m_pModelCom->Find_BoneIndex("Bip001-Spine2");
	m_vRayOffset = { 0.f, 1.7f, 0.f, 0.f };

	if (FAILED(Ready_UIParameters()))
		return E_FAIL;

	if (FAILED(Ready_Arm()))
		return E_FAIL;


	if (m_pWeapon)
		m_pWeapon->SetisAttack(false);
	
	if (m_pLegionArm)
		m_pLegionArm->SetisAttack(false);


	m_pGameInstance->Notify(TEXT("Weapon_Status"), TEXT("EquipWeapon"), nullptr);
	

	return S_OK;
}



void CPlayer::Priority_Update(_float fTimeDelta)
{

	/* [ 캡스락을 누르면 위치를 볼 수 있다? ] */
	if (KEY_DOWN(DIK_CAPSLOCK))
	{
		_vector pos = m_pTransformCom->Get_State(STATE::POSITION);
		printf("PlayerPos X:%f, Y:%f, Z:%f\n", XMVectorGetX(pos), XMVectorGetY(pos), XMVectorGetZ(pos));
	}

	if (KEY_DOWN(DIK_1))
	{
		PxVec3 pos = PxVec3(-0.227397f, 1.f, 1.014072f);
		PxTransform posTrans = PxTransform(pos);
		m_pControllerCom->Set_Transform(posTrans);
	};

	if (KEY_DOWN(DIK_2))
	{
		PxVec3 pos = PxVec3(71.983032f, 1.f, -5.244843f);
		PxTransform posTrans = PxTransform(pos);
		m_pControllerCom->Set_Transform(posTrans);
	}

	if (KEY_DOWN(DIK_3))
	{
		PxVec3 pos = PxVec3(127.851753f, 2.f, -7.543217f);
		PxTransform posTrans = PxTransform(pos);
		m_pControllerCom->Set_Transform(posTrans);
	}

	if (KEY_DOWN(DIK_4))
	{
		PxVec3 pos = PxVec3(119.659203f, 3.f, -28.681953f);
		PxTransform posTrans = PxTransform(pos);
		m_pControllerCom->Set_Transform(posTrans);
	}

	/* [ 플레이어가 속한 구역탐색 ] */
	m_pGameInstance->SetPlayerPosition(m_pTransformCom->Get_State(STATE::POSITION));
	m_pGameInstance->FindAreaContainingPoint();

	/* [ 캐스케이드 전용 업데이트 함수 ] */
	UpdateShadowCamera();

	/* [ 룩 벡터 레이케스트 ] */
	RayCast(m_pControllerCom);

	/* [ 아이템 ] */
	PriorityUpdate_Slot(fTimeDelta);

	__super::Priority_Update(fTimeDelta);
}

void CPlayer::Update(_float fTimeDelta)
{
	/* [ 애니메이션 업데이트 ] */
	__super::Update(fTimeDelta);

	/* [ 입력 ] */
	if (CCamera_Manager::Get_Instance()->GetbMoveable()) // CameraOrbital일때만
		HandleInput();

	/* [ 락온 관련 ] */
	LockOnState(fTimeDelta);

	/* [ 문열기 관련 ] */
	SlidDoorMove(fTimeDelta);

	/* [ 상태 관련 ] */
	UpdateCurrentState(fTimeDelta);

	/* [ 이동 관련 ] */
	Movement(fTimeDelta);

	/* [ 콜라이더 업데이트 ] */
	Update_Collider_Actor();

	/* [ 아이템 ] */
	Update_Slot(fTimeDelta);

	/* [ 플레이어 락온 위치  ] */
	_matrix LockonMat = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(m_iLockonBoneIndex));
	_vector vLockonPos = XMVector3TransformCoord(LockonMat.r[3], m_pTransformCom->Get_WorldMatrix());
	XMStoreFloat4(&m_vLockonPos, vLockonPos);
}

void CPlayer::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_SHADOW, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
	
	/* [ 특수행동 ] */
	ItemWeaponOFF(fTimeDelta);
	SitAnimationMove(fTimeDelta);

	/* [ 이곳은 실험실입니다. ] */
	if(KEY_DOWN(DIK_Y))
	{
		CEffectContainer::DESC desc = {};
		auto worldmat =  m_pTransformCom->Get_WorldMatrix();

		// 위치 프리셋만
		XMStoreFloat4x4(&desc.PresetMatrix,
			XMMatrixTranslation(worldmat.r[3].m128_f32[0], worldmat.r[3].m128_f32[1], worldmat.r[3].m128_f32[2]));

		if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Fuoco_FlameField_Imsi_P2"), &desc))
			MSG_BOX("이펙트 생성 실패함");
	}

	if (KEY_DOWN(DIK_U))
	{
		
	}

	/* [ 아이템 ] */
	LateUpdate_Slot(fTimeDelta);

}

HRESULT CPlayer::Render()
{
	__super::Render();

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider() && m_pPhysXActorCom->Get_ReadyForDebugDraw()) {
		//m_pGameInstance->Add_DebugComponent(m_pControllerCom);
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif

	return S_OK;
}

CAnimController* CPlayer::GetCurrentAnimContrller()
{
	return m_pAnimator->Get_CurrentAnimController();
}

void CPlayer::Set_HitTarger(CUnit* pTarget, _bool bDead)
{
	if (bDead && m_pHitTarget == pTarget)
	{
		m_pHitTarget = nullptr;
		return;
	}
	
	m_pHitTarget = pTarget; 
}

CPlayer::EHitDir CPlayer::ComputeHitDir()
{
	_vector vPlayerRight = XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT));
	_vector vPlayerLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));

	_vector vIncoming = XMVector3Normalize(ProjectToXZ(m_vHitNormal));
	_vector vLookFlat = XMVector3Normalize(ProjectToXZ(vPlayerLook));
	_vector vRightFlat = XMVector3Normalize(ProjectToXZ(vPlayerRight));

	const _float fDotFront = XMVectorGetX(XMVector3Dot(vIncoming, vLookFlat));
	const _float fDotRight = XMVectorGetX(XMVector3Dot(vIncoming, vRightFlat));

	const _float fAngle = atan2f(fDotRight, fDotFront);
	const _float fSector = XM_PI / 4.f;
	_int iSectorIndex = static_cast<_int>(floorf((fAngle + fSector * 0.5f) / fSector)) & 7;

	switch (iSectorIndex)
	{
	case 0:  return EHitDir::F;
	case 1:  return EHitDir::FR;
	case 2:  return EHitDir::R;
	case 3:  return EHitDir::BR;
	case 4:  return EHitDir::B;
	case 5:  return EHitDir::BL;
	case 6:  return EHitDir::L;
	case 7:  return EHitDir::FL;


	default: return EHitDir::F;
	}
}

void CPlayer::CalculateDamage(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	/* [ 들어온 데미지 계산 ] */
	CUnit* pUnit = dynamic_cast<CUnit*>(pOther);
	if (!pUnit)
	{
		CWeapon* pWeapon = dynamic_cast<CWeapon*>(pOther);
		if (!pWeapon)
		{
			return;
		}
		else
		{
			m_fReceiveDamage = pWeapon->Get_CurrentDamage();
		}
	}
	else
	{
		m_fReceiveDamage = pUnit->Get_CurrentDamage();
	}
}

void CPlayer::HPSubtract()
{
	/* [ Hp 감소 ] */
	if (m_bPerfectGardDamege)
	{
		m_fReceiveDamage = 0.f;
		m_bPerfectGardDamege = false;
	}
	else if (m_bGardDamege)
	{
		m_fReceiveDamage /= 4.f;
		m_bGardDamege = false;
	}


	m_fHP -= m_fReceiveDamage;

	if (m_fHP <= 0.f)
		m_fHP = 0.f;

	Callback_HP();
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
	m_bWalk ? m_pAnimator->SetBool("Run", false) : m_pAnimator->SetBool("Run", true);
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
	m_Input.bCtrl = KEY_UP(DIK_LCONTROL);
	m_Input.bCtrlPress = KEY_PRESSING(DIK_LCONTROL);
	m_Input.bTap = KEY_DOWN(DIK_TAB);
	m_Input.bItem = KEY_DOWN(DIK_R);
	m_Input.bSkill = KEY_DOWN(DIK_F);
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

	if (m_bIsHit)
	{
		eNextState = EPlayerState::HITED;
		m_bIsHit = false;
	}

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
		m_fSetTime = 0.f;
		m_iMoveStep = 0;
		m_bMove = false;
		m_bMoveReset = false;
		m_bSetOnce = false;
		m_bSetTwo = false;

		m_bSetSound = false;
		if (m_bResetSoundTime)
			m_fSetSoundTime = 0.f;
		m_bSetCamera[9] = {};

		m_pWeapon->SetisAttack(false);
		m_pWeapon->Clear_CollisionObj();
		
		m_pTransformCom->SetbSpecialMoving();

		if (m_pWeapon)
		{
			m_pWeapon->SetisAttack(false);
			m_pWeapon->Set_WeaponTrail_Active(false);
		}
		if (m_pLegionArm)
			m_pLegionArm->SetisAttack(false);
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

		m_fSetTime += fTimeDelta;
		if (m_fSetTime > 0.6f)
		{
			if (!m_bSetOnce && m_fStamina >= 0.f)
			{
				m_fStamina -= 20.f;
				Callback_Stamina();
				m_bSetOnce = true;
			}
		}

		m_fSetSoundTime += fTimeDelta;
		if (m_fSetSoundTime > 0.6f)
		{
			if (!m_bSetSound)
			{
				m_pSoundCom->Play("SE_PC_SK_WS_Sword_2H_1st");
				m_bSetSound = true;
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

		m_fSetTime += fTimeDelta;
		if (m_fSetTime > 0.7f)
		{
			if (!m_bSetOnce && m_fStamina >= 0.f)
			{
				m_fStamina -= 20.f;
				Callback_Stamina();
				m_bSetOnce = true;
			}
		}

		m_fSetSoundTime += fTimeDelta;
		if (m_fSetSoundTime > 0.6f)
		{
			if (!m_bSetSound)
			{
				m_pSoundCom->Play("SE_PC_SK_WS_Sword_2H_2nd");
				m_bSetSound = true;
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

		m_fSetTime += fTimeDelta;
		if (m_fSetTime > 0.5f)
		{
			if (!m_bSetOnce && m_fStamina >= 0.f)
			{
				m_fStamina -= 20.f;
				Callback_Stamina();
				m_bSetOnce = true;
			}
		}

		m_fSetSoundTime += fTimeDelta;
		if (m_fSetSoundTime > 0.4f)
		{
			if (!m_bSetSound)
			{
				m_pSoundCom->Play("SE_PC_SK_WS_Sword_2H_1st");
				m_bSetSound = true;
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

		m_fSetTime += fTimeDelta;
		if (m_fSetTime > 0.5f)
		{
			if (!m_bSetOnce && m_fStamina >= 0.f)
			{
				m_fStamina -= 20.f;
				Callback_Stamina();
				m_bSetOnce = true;
			}
		}

		m_fSetSoundTime += fTimeDelta;
		if (m_fSetSoundTime > 0.6f)
		{
			if (!m_bSetSound)
			{
				m_pSoundCom->Play("SE_PC_SK_WS_Sword_2H_2nd");
				m_bSetSound = true;
			}
		}
		break;
	}
	case eAnimCategory::CHARGE_ATTACKA:
	{
		RootMotionActive(fTimeDelta);

		/* [ 차지 A 일 때 R버튼이 꾹 눌리면 체인지 변수가 켜진다. ] */
		m_fChangeTimeElaped += fTimeDelta;
		m_fSetTime += fTimeDelta;

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

		if (m_fSetTime > 1.f)
		{
			if (!m_bSetOnce && m_fStamina >= 0.f)
			{
				m_fStamina -= 20.f;
				Callback_Stamina();
				m_bSetOnce = true;
				m_pWeapon->Clear_CollisionObj();
			}
		}
		if (m_fSetTime > 1.8f)
		{
			if (!m_bSetTwo && m_fStamina >= 0.f)
			{
				m_fStamina -= 20.f;
				Callback_Stamina();
				m_bSetTwo = true;
				m_pWeapon->Clear_CollisionObj();
			}
		}

		break;
	}
	case eAnimCategory::CHARGE_ATTACKB:
	{
		RootMotionActive(fTimeDelta);

		m_fSetTime += fTimeDelta;

		if (m_fSetTime > 1.f)
		{
			if (!m_bSetOnce && m_fStamina >= 0.f)
			{
				m_fStamina -= 20.f;
				Callback_Stamina();
				m_bSetOnce = true;
			}
		}
		break;
	}
	case eAnimCategory::IDLE:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);

		if (m_fStamina <= m_fMaxStamina)
		{
			m_fStamina += fTimeDelta * 20.f;
			Callback_Stamina();
		}
		break;
	}
	case eAnimCategory::WALK:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);

		if (m_fStamina <= m_fMaxStamina)
		{
			m_fStamina += fTimeDelta * 20.f;
			Callback_Stamina();
		}

		m_fSetSoundTime += fTimeDelta;
		if (m_fSetSoundTime > 0.7f)
		{
			m_pSoundCom->Play_Random("SE_PC_FS_Stone_Walk_", 9);
			m_fSetSoundTime = 0.f;
		}

		break;
	}
	case eAnimCategory::RUN:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fRunSpeed);

		if (m_fStamina <= m_fMaxStamina)
		{
			m_fStamina += fTimeDelta * 20.f;
			Callback_Stamina();
		}

		if (m_fStamina <= m_fMaxStamina)
		{
			m_fStamina += fTimeDelta * 20.f;
			Callback_Stamina();
		}

		m_fSetSoundTime += fTimeDelta;
		if (m_fSetSoundTime > 0.45f)
		{
			m_pSoundCom->Play_Random("SE_PC_FS_Stone_Run_", 9);
			m_fSetSoundTime = 0.f;
		}

		break;
	}
	case eAnimCategory::SPRINT:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fSprintSpeed);

		if (m_fStamina >= 0.f)
		{
			m_fStamina -= fTimeDelta * 15.f;
			Callback_Stamina();
		}

		m_fSetSoundTime += fTimeDelta;
		if (m_fSetSoundTime > 0.35f)
		{
			m_pSoundCom->Play_Random("SE_PC_FS_Stone_Run_", 9);
			m_fSetSoundTime = 0.f;
		}

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
	case eAnimCategory::ITEM:
	{
		if (m_pSelectItem->Get_ProtoTag().find(L"Lamp") != _wstring::npos)
		{
			m_fSetSoundTime += fTimeDelta;
			if (m_fSetSoundTime >= 0.7f && !m_bSetSound)
			{
				if (!m_bCheckSound)
				{
					m_pSoundCom->Play_Random("SE_PC_MT_Item_Monard_Lamp_", 3);
					m_bSetSound = true;
					m_bCheckSound = true;
				}
			}
		}

		break;
	}
	case eAnimCategory::ITEM_WALK:
	{
		if (m_pSelectItem->Get_ProtoTag().find(L"Lamp") != _wstring::npos)
		{
			m_fSetSoundTime += fTimeDelta;
			if (m_fSetSoundTime >= 0.7f && !m_bSetSound)
			{
				if (!m_bCheckSound)
				{
					m_pSoundCom->Play_Random("SE_PC_MT_Item_Monard_Lamp_", 3);
					m_bSetSound = true;
					m_bCheckSound = true;
				}
			}
		}

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

		if (!m_bSetOnce && m_fStamina >= 0.f)
		{
			m_fStamina -= 30.f;
			Callback_Stamina();
			m_bSetOnce = true;
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

		if (!m_bSetOnce && m_fStamina >= 0.f)
		{
			m_fStamina -= 30.f;
			Callback_Stamina();
			m_bSetOnce = true;
		}


		break;
	}
	case eAnimCategory::DASH_FOCUS:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fSprintSpeed);

		m_fMoveTime += fTimeDelta;
		_float  m_fTime = 0.5f;
		_float  m_fDistance = 2.5f;
		
		if (!m_bMove)
		{
			if (!m_bMoveReset)
			{
				m_vMoveDir = ComputeLatchedMoveDir(m_bSwitchFront, m_bSwitchBack, m_bSwitchLeft, m_bSwitchRight);
				m_bMoveReset = true;
			}

			m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, m_vMoveDir, m_fDistance, m_pControllerCom);
			SyncTransformWithController();
		}

		if (!m_bSetOnce && m_fStamina >= 0.f)
		{
			m_fStamina -= 30.f;
			Callback_Stamina();
			m_bSetOnce = true;
		}

		break;
	}
	case eAnimCategory::SPRINT_ATTACKA:
	{
		RootMotionActive(fTimeDelta);

		if (!m_bSetOnce && m_fStamina >= 0.f)
		{
			m_fStamina -= 20.f;
			Callback_Stamina();
			m_bSetOnce = true;
		}

		m_fSetSoundTime += fTimeDelta;
		if (m_fSetSoundTime > 0.4f)
		{
			if (!m_bSetSound)
			{
				m_pSoundCom->Play("SE_PC_SK_WS_Sword_2H_1st");
				m_bSetSound = true;
			}
		}
		break;
	}
	case eAnimCategory::SPRINT_ATTACKB:
	{
		m_fMoveTime += fTimeDelta;
		_float  m_fTime = 0.5f;
		_float  m_fDistance = 2.5f;

		if (!m_bMove)
		{
			_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
			m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
			SyncTransformWithController();
		}

		if (!m_bSetOnce && m_fStamina >= 0.f)
		{
			m_fStamina -= 20.f;
			Callback_Stamina();
			m_bSetOnce = true;
		}

		m_fSetSoundTime += fTimeDelta;
		if (m_fSetSoundTime > 0.4f)
		{
			if (!m_bSetSound)
			{
				m_pSoundCom->Play("SE_PC_SK_WS_Sword_2H_3rd");
				m_bSetSound = true;
			}
		}

		break;
	}
	case eAnimCategory::FIRSTDOOR:
	{
		RootMotionActive(fTimeDelta);

		break;
	}
	case eAnimCategory::ARM_ATTACKA:
	{
		if (!m_bSetOnce)
		{
			m_pLegionArm->Use_LegionEnergy(20.f);

			//m_fLegionArmEnergy -= 20.f;
			m_bSetOnce = true;
		}

		RootMotionActive(fTimeDelta);

		break;
	}
	case eAnimCategory::ARM_ATTACKB:
	{
		if (!m_bSetOnce)
		{
			m_pLegionArm->Use_LegionEnergy(20.f);
			//m_fLegionArmEnergy -= 20.f;
			m_bSetOnce = true;
		}

		RootMotionActive(fTimeDelta);

		break;
	}
	case eAnimCategory::ARM_ATTACKCHARGE:
	{
		if (!m_bSetOnce)
		{
			m_pLegionArm->Use_LegionEnergy(20.f);
			//m_fLegionArmEnergy -= 20.f;
			m_bSetOnce = true;
		}

		RootMotionActive(fTimeDelta);

		break;
	}
	case eAnimCategory::MAINSKILLA:
	{
		RootMotionActive(fTimeDelta);
		break;
	}
	case eAnimCategory::MAINSKILLB:
	{
		RootMotionActive(fTimeDelta);
		break;
	}
	case eAnimCategory::MAINSKILLC:
	{
		RootMotionActive(fTimeDelta);
		break;
	}
	case eAnimCategory::FATAL:
	{
		m_fSetTime += fTimeDelta;
		if (m_fSetTime > 0.f && !m_bSetCamera[0])
		{
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Start_DistanceLerp(2.5f, 10.f);
			m_bSetCamera[0] = true;
		}
		if (m_fSetTime > 1.f && !m_bSetCamera[1])
		{
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Start_DistanceLerp(2.5f, 10.f);
			m_bSetCamera[1] = true;
		}
		if (m_fSetTime > 2.f && !m_bSetCamera[2])
		{
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Start_DistanceLerp(2.f, 8.f);
			m_bSetCamera[2] = true;
		}

		RootMotionActive(fTimeDelta);
		break;
	}
	case eAnimCategory::HITED:
	{
		RootMotionActive(fTimeDelta);

		if (!m_bSetSound)
		{
			m_pSoundCom->Play_Random("SE_PC_SK_GetHit_Guard_CarcassSkin_M_", 3);
			m_bSetSound = true;
		}

		break;
	}
	case eAnimCategory::GRINDER:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
		break;
	}
	case eAnimCategory::PULSE:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
		break;
	}
	case eAnimCategory::GUARD_HIT:
	{
		if (m_eHitedTarget == eHitedTarget::MONSTER || m_eHitedTarget == eHitedTarget::ARROW)
		{
			//가드 밀림 여부
			_float  m_fTime = 0.1f;
			_float  m_fDistance = 0.15f;

			if (!m_bMove)
			{
				_vector vLook = XMVectorNegate(m_pTransformCom->Get_State(STATE::LOOK));
				m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
				SyncTransformWithController();
			}
		}
		else if (m_eHitedTarget == eHitedTarget::BOSS)
		{
			//가드 밀림 여부
			_float  m_fTime = 0.1f;
			_float  m_fDistance = 3.f;

			if (!m_bMove)
			{
				_vector vLook = XMVectorNegate(m_pTransformCom->Get_State(STATE::LOOK));
				m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
				SyncTransformWithController();
			}
		}

		break;
	}
	case eAnimCategory::HITEDUP:
	{
		_float  m_fTime = 0.4f;
		_float  m_fDistance = 2.f;

		if (!m_bMove)
		{
			if (m_pHitedTarget)
			{
				_vector vLook = m_pHitedTarget->Get_TransfomCom()->Get_State(STATE::LOOK);

				m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
				SyncTransformWithController();
			}
		}
		break;
	}
	case eAnimCategory::HITEDSTAMP:
	{
		_float  m_fTime = 0.1f;
		_float  m_fDistance = 2.f;

		if (m_pHitedTarget)
		{
			_vector vLook = m_pHitedTarget->Get_TransfomCom()->Get_State(STATE::LOOK);

			m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
			SyncTransformWithController();
		}
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

	if (stateName.find("Dash_Normal_B") == 0)
		return eAnimCategory::DASH_BACK;
	if (stateName.find("Dash_Normal_F") == 0)
		return eAnimCategory::DASH_FRONT;
	if (stateName.find("Dash_") == 0)
		return eAnimCategory::DASH_FOCUS;

	if (stateName.find("SprintNormalAttack") == 0)
		return eAnimCategory::SPRINT_ATTACKA;
	if (stateName.find("SprintStrongAttack") == 0)
		return eAnimCategory::SPRINT_ATTACKB;
	if (stateName.find("Sprint") == 0)
		return eAnimCategory::SPRINT;

	if (stateName.find("Guard_Hit") == 0)
		return eAnimCategory::GUARD_HIT;
	if (stateName.find("Guard_Break") == 0)
		return eAnimCategory::GUARD_BREAK;
	if (stateName.find("Guard") == 0) 
		return eAnimCategory::GUARD;

	if (stateName.find("EquipWeapon") == 0) return eAnimCategory::EQUIP;
	if (stateName.find("PutWeapon") == 0) return eAnimCategory::EQUIP;

	if (stateName == "Grinder") return eAnimCategory::GRINDER; 
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

	if (stateName.find("MainSkill3") == 0)
		return eAnimCategory::MAINSKILLC;
	if (stateName.find("MainSkill2") == 0)
		return eAnimCategory::MAINSKILLB;
	if (stateName.find("MainSkill") == 0)
		return eAnimCategory::MAINSKILLA;

	if (stateName.find("Sit") == 0)
		return eAnimCategory::SIT;
	if (stateName.find("Hit_Up") == 0)
		return eAnimCategory::HITEDUP;
	if (stateName.find("Hit_Stamp") == 0)
		return eAnimCategory::HITEDSTAMP;
	if (stateName.find("Hit") == 0)
		return eAnimCategory::HITED;

	if (stateName.find("SlidingDoor") == 0)
		return eAnimCategory::FIRSTDOOR;
	if (stateName.find("Arm_NormalAttack") == 0)
		return eAnimCategory::ARM_ATTACKA;
	if (stateName.find("Arm_NormalAttack2") == 0)
		return eAnimCategory::ARM_ATTACKB;
	if (stateName.find("Arm_ChargeAttack") == 0)
		return eAnimCategory::ARM_ATTACKCHARGE;
	if (stateName.find("Fail_Arm") == 0)
		return eAnimCategory::ARM_FAIL;
	if (stateName.find("Heal") == 0)
		return eAnimCategory::PULSE;
	if (stateName.find("FatalAttack") == 0)
		return eAnimCategory::FATAL;
	
	return eAnimCategory::NONE;
}

_vector CPlayer::ComputeLatchedMoveDir(_bool bSwitchFront, _bool bSwitchBack, _bool bSwitchLeft, _bool bSwitchRight)
{
	_vector vRight = m_pTransformCom->Get_State(STATE::RIGHT);
	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);

	vRight = XMVectorSet(XMVectorGetX(vRight), 0.f, XMVectorGetZ(vRight), 0.f);
	vLook  = XMVectorSet(XMVectorGetX(vLook),  0.f, XMVectorGetZ(vLook),  0.f);

	_vector vDir = XMVectorZero();

	if (bSwitchLeft && !bSwitchRight) vDir = XMVectorAdd(vDir, XMVectorNegate(vRight));
	if (bSwitchRight && !bSwitchLeft) vDir = XMVectorAdd(vDir, vRight);
	if (bSwitchFront && !bSwitchBack) vDir = XMVectorAdd(vDir, vLook);
	if (bSwitchBack && !bSwitchFront) vDir = XMVectorAdd(vDir, XMVectorNegate(vLook));

	if (XMVector3Equal(vDir, XMVectorZero()))
		vDir = vLook;

	_float fLen = XMVectorGetX(XMVector3Length(vDir));
	if (fLen <= static_cast<_float>(1e-6f))
		vDir = vLook;
	else
		vDir = XMVectorScale(vDir, 1.f / fLen);

	return vDir;
}

void CPlayer::Register_Events()
{
	m_pAnimator->RegisterEventListener("OnSwordTrail", [this]()
		{
			if (m_pWeapon)
			{
				m_pWeapon->Set_WeaponTrail_Active(true);
			}
		});

	m_pAnimator->RegisterEventListener("OffSwordTrail", [this]()
		{
			if (m_pWeapon)
			{
				m_pWeapon->Set_WeaponTrail_Active(false);
			}
		});

	m_pAnimator->RegisterEventListener("OnWeaponCollider", [this]()
		{
			if (m_pWeapon)
			{
				m_pWeapon->SetisAttack(true);
				m_pWeapon->Clear_CollisionObj();
			}
		});

	m_pAnimator->RegisterEventListener("OffWeaponCollider", [this]()
		{
			if (m_pWeapon)
			{
				m_pWeapon->SetisAttack(false);
				
			}
		});

	m_pAnimator->RegisterEventListener("OnArmCollider", [this]()
		{
			if (m_pLegionArm)
			{
				m_pLegionArm->SetisAttack(true);
				m_pLegionArm->Clear_CollisionObj();
			}
		});

	m_pAnimator->RegisterEventListener("OffArmCollider", [this]()
		{
			if (m_pLegionArm)
			{
				m_pLegionArm->SetisAttack(false);
			}
		});

	m_pAnimator->RegisterEventListener("EquipWeapon", [this]()
		{
			if (m_pWeapon)
			{
				m_pWeapon->SetbIsActive(true);
				m_pWeapon->SetisAttack(false);
				m_pGameInstance->Notify(TEXT("Weapon_Status"), TEXT("EquipWeapon"),m_pWeapon);
			}
		});
	m_pAnimator->RegisterEventListener("PutWeapon", [this]()
		{
			if (m_pWeapon)
			{
				m_pWeapon->SetbIsActive(false);
				m_pWeapon->SetisAttack(false);
				m_pGameInstance->Notify(TEXT("Weapon_Status"), TEXT("EquipWeapon"),nullptr);
			}
		});
	m_pAnimator->RegisterEventListener("OnGrinderEffect", [this]()
		{
			Set_GrinderEffect_Active(true);
		});
	m_pAnimator->RegisterEventListener("OffGrinderEffect", [this]()
		{
			Set_GrinderEffect_Active(false);
		});

	m_pAnimator->RegisterEventListener("UseItem", [this]()
		{
			Use_Item();
		});
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
			XMMatrixRotationQuaternion(vRotQuat) *
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

void CPlayer::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	/* [ 플레이어 피격 ] */
	if (m_bIsInvincible)
		return;


	/* [ 무엇을 해야하는가? ] */
	if (eColliderType == COLLIDERTYPE::MONSTER_WEAPON)
	{
		m_eHitedTarget = eHitedTarget::MONSTER;
		CWeapon* pWeapon = dynamic_cast<CWeapon*>(pOther);
		if (pWeapon == nullptr)
			return;


		if (pWeapon->Find_CollisonObj(this, eColliderType))
			return;

		pWeapon->Add_CollisonObj(this);

		//0. 필요한 정보를 수집한다.
		CalculateDamage(pOther, eColliderType);
		CUnit* pUnit = pWeapon->Get_Owner();
		m_pHitedTarget = pUnit;
		_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
		_vector vOtherPos = pUnit->Get_TransfomCom()->Get_State(STATE::POSITION);



		m_vHitNormal = vOtherPos - vPlayerPos;
		

		//1. 애니메이션 상태를 히트로 바꾼다.

		//가드 중에 피격시 스위치를 켠다.
		if (m_bIsGuarding)
		{
			m_bGardHit = true;

			EHitDir eDir = ComputeHitDir();

			if(eDir == EHitDir::F || eDir == EHitDir::FR || eDir == EHitDir::FL)
				pUnit->Block_Reaction();

			return;
		}

		//가드 중이 아니라면 피격상태로 넘긴다.
		m_bIsHit = true;
	}
	if (eColliderType == COLLIDERTYPE::BOSS_WEAPON)
	{
		CUnit* pBoss = dynamic_cast<CUnit*>(pOther);
		if (pBoss)
		{
			m_eHitedTarget = eHitedTarget::BOSS;
			m_pHitedTarget = pBoss;

			//0. 필요한 정보를 수집한다.
			CalculateDamage(pOther, eColliderType);
			_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
			_vector vOtherPos = pBoss->Get_TransfomCom()->Get_State(STATE::POSITION);

			m_vHitNormal = vOtherPos - vPlayerPos;
		}
		else
		{
			m_eHitedTarget = eHitedTarget::ARROW;

			CGameObject* pArrow = dynamic_cast<CGameObject*>(pOther);
			m_pHitedTarget = pArrow;

			//0. 필요한 정보를 수집한다.
			CalculateDamage(pOther, eColliderType);
			_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
			_vector vOtherPos = pArrow->Get_TransfomCom()->Get_State(STATE::POSITION);

			m_vHitNormal = vOtherPos - vPlayerPos;
		}

		//1. 애니메이션 상태를 히트로 바꾼다.

		//가드 중에 피격시 스위치를 켠다.
		if (m_bIsGuarding)
		{
			m_bGardHit = true;
			return;
		}

		//가드 중이 아니라면 피격상태로 넘긴다.
		m_bIsHit = true;

	}
}

void CPlayer::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CPlayer::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{

}

void CPlayer::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CPlayer::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	/* [ 플레이어 피격 ] */
	if (m_bIsInvincible)
		return;


	/* [ 무엇을 해야하는가? ] */
	if (eColliderType == COLLIDERTYPE::MONSTER_WEAPON)
	{
		m_eHitedTarget = eHitedTarget::MONSTER;
		CWeapon* pWeapon = dynamic_cast<CWeapon*>(pOther);
		if (pWeapon == nullptr)
			return;

		if (pWeapon->Find_CollisonObj(this, eColliderType))
			return;

		pWeapon->Add_CollisonObj(this);

		CUnit* pUnit = pWeapon->Get_Owner();
		m_pHitedTarget = pUnit;

		CMonster_Base* pMonster = dynamic_cast<CMonster_Base*>(pUnit);

		if (nullptr == pMonster)
			return;

		if (pMonster->Get_CurrentHp() <= 0)
			return;

		//0. 필요한 정보를 수집한다.
		CalculateDamage(pOther, eColliderType);
		
		_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
		_vector vOtherPos = pUnit->Get_TransfomCom()->Get_State(STATE::POSITION);

		m_vHitNormal = vOtherPos - vPlayerPos;

		//1. 애니메이션 상태를 히트로 바꾼다.

		//가드 중에 피격시 스위치를 켠다.
		if (m_bIsGuarding)
		{
			m_bGardHit = true;

			EHitDir eDir = ComputeHitDir();

			if (eDir == EHitDir::F || eDir == EHitDir::FR || eDir == EHitDir::FL)
				pUnit->Block_Reaction();

			return;
		}

		//가드 중이 아니라면 피격상태로 넘긴다.
		m_bIsHit = true;

	}

	if (eColliderType == COLLIDERTYPE::BOSS_WEAPON)
	{
		m_eHitedTarget = eHitedTarget::BOSS;
		CUnit* pBoss = dynamic_cast<CUnit*>(pOther);
		if (pBoss == nullptr)
			return;

		m_pHitedTarget = pBoss;

		//0. 필요한 정보를 수집한다.
		CalculateDamage(pOther, eColliderType);
		_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
		_vector vOtherPos = pBoss->Get_TransfomCom()->Get_State(STATE::POSITION);

		m_vHitNormal = vOtherPos - vPlayerPos;

		//1. 애니메이션 상태를 히트로 바꾼다.

		//가드 중에 피격시 스위치를 켠다.
		if (m_bIsGuarding)
		{
			m_bGardHit = true;
			return;
		}

		//가드 중이 아니라면 피격상태로 넘긴다.
		m_bIsHit = true;

	}

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
	m_pStateArray[ENUM_CLASS(EPlayerState::ARMATTACKA)] = new CPlayer_ArmAttackA(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::ARMATTACKB)] = new CPlayer_ArmAttackB(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::ARMATTACKCHARGE)] = new CPlayer_ArmCharge(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::ARMFAIL)] = new CPlayer_ArmFail(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::MAINSKILL)] = new CPlayer_MainSkill(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::FATAL)] = new CPlayer_Fatal(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::HITED)] = new CPlayer_Hited(this);
	m_pStateArray[ENUM_CLASS(EPlayerState::DEAD)] = new CPlayer_Dead(this);

	m_pCurrentState = m_pStateArray[ENUM_CLASS(EPlayerState::IDLE)];
}



HRESULT CPlayer::Ready_Weapon()
{
	/* [ 무기 모델을 추가 ] */

	CBayonet::BAYONET_DESC Desc{};
	Desc.eMeshLevelID = static_cast<LEVEL>(m_pGameInstance->GetCurrentLevelIndex());
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.InitPos = { 0.f, 0.f, 0.f };
	Desc.InitScale = { 1.f, 1.f, 1.f };
	Desc.iRender = 0;
	Desc.iLevelID = m_iLevelID;
	Desc.pOwner = this;
	
	Desc.szMeshID = TEXT("PlayerWeapon");
	lstrcpy(Desc.szName, TEXT("PlayerWeapon"));

	Desc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("BN_Weapon_R"));
	Desc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(m_iLevelID, TEXT("Prototype_GameObject_PlayerWeapon"),
		m_iLevelID, TEXT("Player_Weapon"), &pGameObject, &Desc)))
		return E_FAIL;

	m_pWeapon = dynamic_cast<CWeapon*>(pGameObject);

	if (m_pWeapon == nullptr)
		return E_FAIL;

	//m_pWeapon->SetisAttack(false);

	return S_OK;
}


HRESULT CPlayer::Ready_Lamp()
{
	/* [ 램프 모델을 추가 ] */

	CPlayerLamp::PLAYERLAMP_DESC Desc{};
	Desc.eMeshLevelID = static_cast<LEVEL>(m_pGameInstance->GetCurrentLevelIndex());
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.InitPos = { 0.f, 0.f, 0.f };
	Desc.InitScale = { 0.4f, 0.4f, 0.4f };
	Desc.iRender = 0;
	Desc.iLevelID = m_iLevelID;
	Desc.pOwner = this;

	Desc.szMeshID = TEXT("PlayerLamp");
	lstrcpy(Desc.szName, TEXT("PlayerLamp"));

	Desc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("BN_Lamp_02"));
	Desc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(m_iLevelID, TEXT("Prototype_GameObject_PlayerLamp"),
		m_iLevelID, TEXT("Player_Lamp"), &pGameObject, &Desc)))
		return E_FAIL;
	
	m_pPlayerLamp = dynamic_cast<CPlayerLamp*>(pGameObject);

	if (m_pPlayerLamp == nullptr)
		return E_FAIL; 

	return S_OK;
}

HRESULT CPlayer::Ready_FrontCollider()
{
	/* [ 램프 모델을 추가 ] */

	CPlayerFrontCollider::FRONTCOLLIDER_DESC Desc{};
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.InitPos = { 0.f, 0.f, 0.f };
	Desc.InitScale = { 1.f, 1.f, 1.f };
	Desc.iLevelID = m_iLevelID;
	Desc.pOwner = this;

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(m_iLevelID, TEXT("Prototype_GameObject_PlayerFrontCollider"),
		m_iLevelID, TEXT("Player_FrontCollider"), &pGameObject, &Desc)))
		return E_FAIL;

	m_pFrontCollider = dynamic_cast<CPlayerFrontCollider*>(pGameObject);

	if (m_pFrontCollider == nullptr)
		return E_FAIL;

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

	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_Player"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
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
HRESULT CPlayer::Ready_UIParameters()
{
	Callback_HP();
	Callback_Mana();
	Callback_Stamina();

	m_pBelt_Up = static_cast<CBelt*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Belt"), nullptr));
	m_pBelt_Down = static_cast<CBelt*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Belt"), nullptr));

	auto pLamp = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Lamp"), nullptr);

	m_pBelt_Down->Add_Item(static_cast<CItem*>(pLamp), 0);

	auto pGrinder = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Grinder"), nullptr);

	m_pBelt_Down->Add_Item(static_cast<CItem*>(pGrinder), 1);

	auto pPortion = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Portion"), nullptr);

	m_pBelt_Up->Add_Item(static_cast<CItem*>(pPortion), 0);
	

	m_pSelectItem = m_pBelt_Up->Get_Items()[0];


	Callback_DownBelt();
	Callback_UpBelt();

	m_pGameInstance->Register_PullCallback(TEXT("Player_Status"), [this](_wstring eventName, void* data) {

		if (eventName == L"AddHp")
		{
			_float* fRatio = static_cast<_float*>(data);

			m_fHP = m_fHP + m_fMaxHP * (*fRatio);

			if (m_fHP > m_fMaxHP)
				m_fHP = m_fMaxHP;

			Callback_HP();
		}
			
		
		});

	//m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"CurrentHP"), &m_fHP);


	return S_OK;
}

HRESULT CPlayer::Ready_Arm()
{
	CLegionArm_Base::ARM_DESC eDesc{};
	
	eDesc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	eDesc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-L-Hand"));
	

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_LegionArm_Steel"),
		m_iLevelID, TEXT("Player_Weapon"), &pGameObject, &eDesc)))
		return E_FAIL;

	m_pLegionArm = dynamic_cast<CLegionArm_Base*>(pGameObject);

	if (m_pLegionArm->Get_MeshName().find(L"Steel") != _wstring::npos)
	{
		m_pPhysXActorCom->Add_IngoreActors(dynamic_cast<CLegionArm_Steel*>(m_pLegionArm)->Get_Actor()->Get_Actor());
		m_pControllerCom->Add_IngoreActors(dynamic_cast<CLegionArm_Steel*>(m_pLegionArm)->Get_Actor()->Get_Actor());
	}


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

//HRESULT CPlayer::Ready_Effect()
//{
//	//"Bn_L_ForeTwist"
//	//"Bip001-L-Forearm"
//	_uint iBoneIdx = m_pModelCom->Find_BoneIndex("Bn_L_ForeTwist");
//
//	CEffectContainer::DESC desc = {};
//	desc.pSocketMatrix = const_cast<_float4x4*>(m_pModelCom->Get_CombinedTransformationMatrix(iBoneIdx));
//	desc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
//	XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixIdentity());
//	m_pGrinderEffect = dynamic_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_TestGrinder_VStretch_wls_P2"), &desc));
//	
//	if (m_pGrinderEffect == nullptr)
//		MSG_BOX("이펙트 생성 실패함");
//
//	return S_OK;
//}

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
	filterData.word1 = WORLDFILTER::FILTER_MONSTERWEAPON | FILTER_MONSTERBODY; 
	m_pPhysXActorCom->Set_SimulationFilterData(filterData); 
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::PLAYER);
	m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

void CPlayer::Callback_HP()
{
	if (m_fHP < 0.f)
		m_fHP = 0.f;

	if (m_fHP >= m_fMaxHP)
		m_fHP = m_fMaxHP;

	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"CurrentHP"), &m_fHP);
	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"MaxHP"), &m_fMaxHP);
}

void CPlayer::Callback_Stamina()
{
	if (m_fStamina < 0.f)
		m_fStamina = 0.f;

	if (m_fStamina >= m_fMaxStamina)
		m_fStamina = m_fMaxStamina;

	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"CurrentStamina"), &m_fStamina);
	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"MaxStamina"), &m_fMaxStamina);
}

void CPlayer::Callback_Mana()
{
	if (m_fMana < 0.f)
		m_fMana = 0.f;

	if (m_fMana >= m_fMaxMana)
		m_fMana = m_fMaxMana;

	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"CurrentMana"), &m_fMana);
	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"MaxMana"), &m_fMaxMana);
}

void CPlayer::Interaction_Door(INTERACT_TYPE eType, CGameObject* pObj)
{
	m_pInterectionStuff = pObj;
	switch (eType)
	{
	case Client::TUTORIALDOOR:
		Play_CutScene_Door();
		break;
	default:
		break;
	}
}

void CPlayer::GetWeapon()
{
	m_pAnimator->SetTrigger("EquipWeapon");
	m_pAnimator->ApplyOverrideAnimController("TwoHand");
	m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
	m_bWeaponEquipped = true;
	m_bWalk = true;
}

void CPlayer::Play_CutScene_Door()
{	
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
			m_pCamera_Manager->Play_CutScene(CUTSCENE_TYPE::TUTORIALDOOR);
		}
	}

	if (m_bInteraction[0] && !m_bInteractionMove[0] && !m_bInteractionRotate[0])
	{
		m_fInteractionTime[0] += fTimeDelta;
		if (m_fInteractionTime[0] >= 1.f && !m_bInteractSound[0])
		{
			m_bInteractSound[0] = true;
			static_cast<CDoorMesh*>(m_pInterectionStuff)->Play_Sound();
		}

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

		if (fDeltaX <= -2.2f)
		{
			m_bInteraction[0] = false;
			m_bInteractionProb[0] = true;
		}
	}

	if (m_bInteractionProb[0])
	{
		CTransform* DoorTransCom = m_pInterectionStuff->Get_TransfomCom();

		if (!m_bInteractionProbMove[0])
		{
			_vector vDir = { -1.f,0.f,0.f,0.f };
			m_bInteractionProbMove[0] = DoorTransCom->Move_SpecialB(fTimeDelta, 0.5f, vDir, 1.f);
		}
	}
}

void CPlayer::Weapon_Collider_Active()
{
	if (nullptr == m_pWeapon)
		return;
	
	m_pWeapon->SetisAttack(true);
}

void CPlayer::Reset_Weapon()
{
	if (nullptr == m_pWeapon)
		return;

	m_pWeapon->SetisAttack(true);
	m_pWeapon->Clear_CollisionObj();
}

void CPlayer::LockOnState(_float fTimeDelta)
{
	if (m_pGameInstance->Mouse_Down(DIM::WHEELBUTTON))
		m_pLockOn_Manager->Set_Active();

	CUnit* pTarget = m_pLockOn_Manager->Get_Target();

	/* [ 락온인데 스프린트 상태라면 ] */
	if (pTarget && m_bWeaponEquipped && m_pAnimator->CheckBool("Sprint"))
	{
		/* [ 타겟이 있다면 ] */
		m_bIsLockOn = true;
		m_pAnimator->SetBool("FocusOn", m_bIsLockOn);

		if (KEY_UP(DIK_SPACE))
			m_bLockOnSprint = true;

	}
	/* [ 스페이스를 뗀 순간 회전 보간을 시작한다. ] */
	if (pTarget && m_bLockOnSprint)
	{
		_vector vTargetPos = pTarget->Get_TransfomCom()->Get_State(STATE::POSITION);
		_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);

		_vector vTargetDir = XMVector3Normalize(ProjectToXZ(XMVectorSubtract(vTargetPos, vPlayerPos)));
		_bool bIsRotate = m_pTransformCom->RotateToDirectionSmoothly(vTargetDir, fTimeDelta);

		if (!bIsRotate)
		{
			// 회전을 끝냈다면 스위치는 꺼진다.
			m_bLockOnSprint = false;
		}
		else
		{
			//회전 중이라면 탈출해버린다.
			return;
		}
	}

	/* [ 락온인데 스프린트 상태가 아니라면 ] */
	if (pTarget && m_bWeaponEquipped && !m_pAnimator->CheckBool("Sprint"))
	{
		if (m_bUseLamp)
			m_pLockOn_Manager->Set_Off(nullptr);

		/* [ 타겟이 있다면 ] */
		m_bIsLockOn = true;
		m_pAnimator->SetBool("FocusOn", m_bIsLockOn);
		_vector vTargetPos = pTarget->Get_TransfomCom()->Get_State(STATE::POSITION);
		_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
		m_pTransformCom->LookAtWithOutY(vTargetPos);
	}
	else
	{
		/* [ 타겟이 없다면 ] */
		m_bIsLockOn = false;
		m_pAnimator->SetBool("FocusOn", m_bIsLockOn);

		m_pAnimator->SetBool("Front", false);
		m_pAnimator->SetBool("Left", false);
		m_pAnimator->SetBool("Right", false);
		m_pAnimator->SetBool("Back", false);
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

	m_pSelectItem->Use();

	if(m_isSelectUpBelt)
		Callback_UpBelt();
	else
		Callback_DownBelt();
}

void CPlayer::PriorityUpdate_Slot(_float fTimeDelta)
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

	if (m_pGameInstance->Key_Down(DIK_U))
	{
		Find_Slot(L"Prototype_GameObject_Portion");
	}



	m_pBelt_Up->Priority_Update(fTimeDelta);
	m_pBelt_Down->Priority_Update(fTimeDelta);
}

void CPlayer::Update_Slot(_float fTimeDelta)
{
	m_pBelt_Up->Update(fTimeDelta);
	m_pBelt_Down->Update(fTimeDelta);
}

void CPlayer::LateUpdate_Slot(_float fTimeDelta)
{
	m_pBelt_Up->Late_Update(fTimeDelta);
	m_pBelt_Down->Late_Update(fTimeDelta);
}

_bool CPlayer::Find_Slot(const _wstring& strItemTag)
{
	if(m_pBelt_Up->Find_Item(strItemTag))
	{
		m_pSelectItem = m_pBelt_Up->Get_Current_Item();

		m_isSelectUpBelt = true;

		Callback_UpBelt();

		return true;
	}

	if (m_pBelt_Down->Find_Item(strItemTag))
	{
		m_pSelectItem = m_pBelt_Down->Get_Current_Item();

		m_isSelectUpBelt = false;

		Callback_DownBelt();

		return true;
	}



	return false;
}

void CPlayer::Set_GrinderEffect_Active(_bool bActive)
{
	if (true == bActive)
	{
		if (!m_pGrinderEffect)
		{
			//"Bn_L_ForeTwist"
			//"Bip001-L-Forearm"
			_uint iBoneIdx = m_pModelCom->Find_BoneIndex("BN_Weapon_R");

			CEffectContainer::DESC desc = {};
			desc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bn_L_ForeTwist"));

			desc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
			XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixIdentity());
			m_pGrinderEffect = dynamic_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_TestGrinder_VStretch_P2S1"), &desc));

			if (m_pGrinderEffect == nullptr)
				MSG_BOX("이펙트 생성 실패함");
		}
	}
	else
	{
		if (m_pGrinderEffect)
		{
			m_pGrinderEffect->End_Effect();
			m_pGrinderEffect = nullptr;
		}
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

	m_pGameInstance->SetPlayerPos(vPlayerPos);
	return S_OK;
}

void CPlayer::SetMoveState(_float fTimeDelta)
{
#ifdef _DEBUG
	_int iCurLevel = m_pGameInstance->GetCurrentLevelIndex();
	if (iCurLevel == ENUM_CLASS(LEVEL::JW))
		return;
#endif // _DEBUG

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

			/* [ 락온보간중이 아닐 때만 ] */
			CUnit* pTarget = m_pLockOn_Manager->Get_Target();
			if (!m_bIsLockOn)
				m_pTransformCom->TurnAngle(XMVectorSet(0.f, 1.f, 0.f, 0.f), fClampedAngle);
		}
	}

	/* [ 이동을 한다. ] */
	_float3 moveVec = {};
	_float fSpeed = m_pTransformCom->Get_SpeedPreSec();
	if (!m_bMovable){fSpeed = 0.f;}
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
	PxControllerCollisionFlags collisionFlags;
	collisionFlags = m_pControllerCom->Get_Controller()->move(pxMove, 0.001f, fTimeDelta, filters);

	//printf(" 왜 안움직이지?? : %s \n", strName.c_str());

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
//	Safe_Release(m_pPhysXActorCom);

	for (size_t i = 0; i < ENUM_CLASS(EPlayerState::END); ++i)
		Safe_Delete(m_pStateArray[i]);

	Safe_Release(m_pBelt_Down);
	Safe_Release(m_pBelt_Up);

	Safe_Delete(m_pHitReport);

}
