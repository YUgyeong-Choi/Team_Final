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
#include "Oil.h"

#include "Observer_Player_Status.h"

#include "Belt.h"
#include "Item.h"
#include "Lamp.h"
#include "PlayerLamp.h"
#include "PlayerFrontCollider.h"

#include "LegionArm_Base.h"
#include "LegionArm_Steel.h"
#include "EliteUnit.h"

#include "EffectContainer.h"
#include "Effect_Manager.h"

#include "SlideDoor.h"
#include <FlameField.h>
#include <Elite_Police.h>
#include <KeyDoor.h>
#include "Bullet.h"
#include "Client_Calculation.h"

#include "SpringBoneSys.h"
#include <ShortCutDoor.h>

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

	if (FAILED(Ready_Effect()))
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

	CSpringBoneSys::SpringInitParams springParams;
	springParams.restDirBiasEnable = false;
	m_pSpringBoneSys = CSpringBoneSys::Create(m_pModelCom, vector<string>{"Hair","Cloth","Frill", "Lamp","BN_Robe_B_L"}, springParams);
	if (m_pSpringBoneSys == nullptr)
		return E_FAIL;

	if (FAILED(Ready_Stat()))
		return E_FAIL;

	m_iLevel = 5;

	Add_Ergo(10000.f);

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
		PxVec3 pos = PxVec3(128.767609f, 4.716591f, -34.020145f);
		PxTransform posTrans = PxTransform(pos);
		m_pControllerCom->Set_Transform(posTrans);
	}

	if (KEY_DOWN(DIK_5))
	{
		PxVec3 pos = PxVec3(-1.4f, 1.f, -237.f);
		PxTransform posTrans = PxTransform(pos);
		m_pControllerCom->Set_Transform(posTrans);
	}

	if (KEY_DOWN(DIK_6))
	{
		PxVec3 pos = PxVec3(188.27f, 10.18f, -8.23f);
		PxTransform posTrans = PxTransform(pos);
		m_pControllerCom->Set_Transform(posTrans);
	}

	if (KEY_DOWN(DIK_7))
	{
		PxVec3 pos = PxVec3(360.78f, 15.90f, -48.81f);
		PxTransform posTrans = PxTransform(pos);
		m_pControllerCom->Set_Transform(posTrans);
	}

	if (KEY_DOWN(DIK_8))
	{
		PxVec3 pos = PxVec3(383.f, 16.90f, -48.81f);
		PxTransform posTrans = PxTransform(pos);
		m_pControllerCom->Set_Transform(posTrans);
	}

	if (KEY_PRESSING(DIK_LALT))
	{
		if (KEY_DOWN(DIK_R))
		{
			m_fTimeScale = 0.5f;
		}
		if (KEY_DOWN(DIK_T))
		{
			m_fTimeScale = 1.f;
		}
		if (KEY_DOWN(DIK_E))
		{
			CEffectContainer::DESC Lightdesc = {};
			Lightdesc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bn_L_ForeTwist"));
			Lightdesc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
			XMStoreFloat4x4(&Lightdesc.PresetMatrix, XMMatrixIdentity());
			if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Player_TESTCutscene_Fuoco_LeftarmLightning"), &Lightdesc))
				MSG_BOX("이펙트 생성 실패함");
		}

		if (KEY_DOWN(DIK_Z))
		{
			static _bool bEfActv = { true };
			bEfActv = !bEfActv;

			EFFECT_MANAGER->Set_Active_Effect(TEXT("PlayerRainVolume"), bEfActv);
		}
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

	/* [ 불타는 셰이더 ] */
	BurnActive(fTimeDelta);
	if (m_bBurnSwitch)
		OnBurn(fTimeDelta);
	else
		OffBurn(fTimeDelta);

	/* [ 림라이트 셰이더 ] */
	if (m_bLimSwitch)
		OnLim(fTimeDelta);
	else
		OffLim(fTimeDelta);

	/* [ 가드시간(0.2f) ] */
	IsPerfectGard(fTimeDelta);

	/* [ 플레이어 락온 위치  ] */
	_matrix LockonMat = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(m_iLockonBoneIndex));
	_vector vLockonPos = XMVector3TransformCoord(LockonMat.r[3], m_pTransformCom->Get_WorldMatrix());
	XMStoreFloat4(&m_vLockonPos, vLockonPos);


	m_pSpringBoneSys->Update(fTimeDelta);
}

void CPlayer::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_SHADOW, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_LIMLIGHT, this);

	/* [ 특수행동 ] */
	ItemWeapOnOff(fTimeDelta);
	SitAnimationMove(fTimeDelta);

	/* [ 이곳은 실험실입니다. ] */
	if (KEY_DOWN(DIK_Y))
	{	
	}

	/* [ 소모자원 리셋 ] */
	if (KEY_DOWN(DIK_U))
		Reset();

	/* [ 아이템 ] */
	LateUpdate_Slot(fTimeDelta);
	m_fBurnTime += fTimeDelta;
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

HRESULT CPlayer::Render_Burn()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;

	_float4x4 ViewMatrix, ProjViewMatrix;
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));


	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pBurn->Bind_ShaderResource(m_pShaderCom, "g_Burn", 0)))
		return E_FAIL;
	if (FAILED(m_pBurnMask->Bind_ShaderResource(m_pShaderCom, "g_BurnMask", 0)))
		return E_FAIL;
	if (FAILED(m_pBurnMask2->Bind_ShaderResource(m_pShaderCom, "g_BurnMask2", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fBurnPhase", &m_fBurnPhase, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fBurnTime", &m_fBurnTime, sizeof(_float))))
		return E_FAIL;

	_uint	iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (i >= 3 && i != 9)
			continue;

		if (FAILED(m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(8)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CPlayer::Render_LimLight()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fLimLightIntensity", &m_fLimPhase, sizeof(_float))))
		return E_FAIL;

	_float4 vLimLightColor = { 0.f, 0.749f, 1.f, 1.f };
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fLimLightColor", &m_vLimLightColor, sizeof(_float4))))
		return E_FAIL;

	_float vRimPower = 3.6f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_RimPower", &vRimPower, sizeof(_float))))
		return E_FAIL;
	_float vRimStart = 0.62f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fBandStart", &vRimStart, sizeof(_float))))
		return E_FAIL;
	_float vRimEnd = 0.82f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fBandEnd", &vRimEnd, sizeof(_float))))
		return E_FAIL;

	_bool vLimLightMask = { false };
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fLimLightColor", &vLimLightMask, sizeof(_bool))))
		return E_FAIL;

	_float4 vCamPostion = {};
	XMStoreFloat4(&vCamPostion, m_pTransformCom->Get_State(STATE::POSITION));
	if (FAILED(m_pShaderCom->Bind_RawValue("g_CamposWS", &vCamPostion, sizeof(_float4))))
		return E_FAIL;

	_float4x4 matWorldInv = {};
	XMStoreFloat4x4(&matWorldInv, m_pTransformCom->Get_WorldMatrix_Inverse());
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrixInvTrans", &matWorldInv)))
		return E_FAIL;

	_float4x4 ViewMatrix, ProjViewMatrix;
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pNoiseMap->Bind_ShaderResource(m_pShaderCom, "g_NoiseMap", 0)))
		return E_FAIL;

	_uint	iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMeshes; i++)
	{
		//if (i >= 3 && i != 9)
		//	continue;

		if (FAILED(m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(6)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}


void CPlayer::Reset()
{	
	/* [ 무기 장착 해제 ] */
	m_pAnimator->CancelOverrideAnimController();
	m_pWeapon->SetbIsActive(false);
	m_bWeaponEquipped = false;
	m_pWeapon->Reset();

	m_fHp = m_fMaxHp;
	Callback_HP();
	m_fStamina = m_fMaxStamina;
	Callback_Stamina();
	m_fMana = m_fMaxMana;
	Callback_Mana();

	m_bIsRrevival = true;

	m_pBelt_Down->Reset();
	m_pBelt_Up->Reset();
	m_pPlayerLamp->Reset();

	m_pLegionArm->Reset();

	Callback_DownBelt();
	Callback_UpBelt();

}

CAnimController* CPlayer::GetCurrentAnimContrller()
{
	return m_pAnimator->Get_CurrentAnimController();
}

void CPlayer::Set_HitTarget(CUnit* pTarget, _bool bDead)
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


	m_fHp -= m_fReceiveDamage;

	if (m_fHp <= 0.f)
		m_fHp = 0.f;

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
	else if (m_bIsForceDead)
	{
		eNextState = EPlayerState::DEAD;
		m_bIsForceDead = false;
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
	m_eCategory = GetAnimCategoryFromName(stateName);

	// 상태가 바뀌었으면 초기화
	//if (m_strPrevStateName != stateName)
	if (m_eCategory != m_ePreCategory)
	{
		printf("Current State: %s\n", stateName.c_str());
		m_ePreCategory = m_eCategory;
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
		for (int i = 0; i < 9; ++i)
			m_bSetCamera[i] = false;

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
	
	m_eCategory = GetAnimCategoryFromName(stateName);
	//printf("Anim Category: %d\n", static_cast<int>(eCategory));

	switch (m_eCategory)
	{
	case eAnimCategory::NORMAL_ATTACKA:
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
	case eAnimCategory::NORMAL_ATTACKB:
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

		m_fSetTime += fTimeDelta;
		if (m_fSetTime > 1.f)
		{
			if (!m_bSetOnce && m_fStamina >= 0.f)
			{
				m_fStamina -= 20.f;
				Callback_Stamina();
				m_bSetOnce = true;
				m_pWeapon->Clear_CollisionObj();
				m_pSoundCom->Play("SE_PC_SK_WS_Sword_2H_1st");
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

		m_fSetSoundTime += fTimeDelta;
		if (m_fSetSoundTime > 1.5f)
		{
			if (!m_bSetSound)
			{
				m_pSoundCom->Play("SE_PC_SK_WS_Sword_2H_2nd");
				m_bSetSound = true;
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

		m_fSetSoundTime += fTimeDelta;
		if (m_fSetSoundTime > 1.f)
		{
			if (!m_bSetSound)
			{
				m_pSoundCom->Play("SE_PC_SK_WS_Sword_2H_2nd");
				m_bSetSound = true;
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

		if (!m_bSetOnce && m_fStamina >= 0.f)
		{
			m_fStamina -= 30.f;
			Callback_Stamina();
			m_bSetOnce = true;
		}

		if (!m_bSetSound)
		{
			m_pSoundCom->Play_Random("SE_PC_MT_Rustle_Jump_", 4);
			m_bSetSound = true;
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

		if (!m_bSetSound)
		{
			m_pSoundCom->Play_Random("SE_PC_MT_Rustle_Jump_", 4);
			m_bSetSound = true;
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

		if (!m_bSetSound)
		{
			m_pSoundCom->Play_Random("SE_PC_MT_Rustle_Jump_", 4);
			m_bSetSound = true;
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
	case eAnimCategory::FESTIVALDOOR:
	{
		RootMotionActive(fTimeDelta);
		break;
	}
	case eAnimCategory::STATIONDOOR:
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

		if (!m_bSetTwo)
		{
			m_fMana -= 100.f;
			m_bIsInvincible = true;
			m_bSetTwo = true;
		}

		
		m_fSetTime += fTimeDelta;
		if (m_fSetTime > 0.6f)
		{
			if (!m_bSetOnce && m_fMana >= 0.f)
			{
				m_bIsInvincible = false;
				Callback_Mana();
				m_bSetOnce = true;
			}
		}

		if (!m_bSetSound)
		{
			//m_pSoundCom->Play_Random("SE_PC_SK_WS_Glaive_P_B_S_", 3);
			//m_pSoundCom->Play_Random("SE_PC_SK_FX_ClockworkBlunt_2H_FableArts_Whoosh_", 4);
			//m_pSoundCom->Play_Random("SE_PC_SK_FX_SwordLance_2H_FableArts_Whoosh_End_", 3);
			//m_pSoundCom->Play("SE_PC_SK_FX_Saber_1H_B_FableArts_Motor_0");
			//m_pSoundCom->Play("SE_PC_SK_FX_Frenzy_Rise");
			//m_pSoundCom->Play("SE_PC_SK_FX_Saber_1H_B_FableArts_Start_01");
			m_bSetSound = true;
		}

		break; 
	}
	case eAnimCategory::MAINSKILLB:
	{
		RootMotionActive(fTimeDelta);

		if (!m_bSetTwo)
		{
			m_fMana -= 100.f;
			m_bIsInvincible = true;
			m_bSetTwo = true;
		}

		m_fSetTime += fTimeDelta;
		if (m_fSetTime > 0.6f)
		{
			if (!m_bSetOnce && m_fMana >= 0.f)
			{
				m_bIsInvincible = false;
				Callback_Mana();
				m_bSetOnce = true;
			}
		}

		break;
	}
	case eAnimCategory::MAINSKILLC:
	{
		RootMotionActive(fTimeDelta);

		if (!m_bSetTwo)
		{
			m_fMana -= 100.f;
			m_bIsInvincible = true;
			m_bSetTwo = true;
		}

		m_fSetTime += fTimeDelta;
		if (m_fSetTime > 0.6f)
		{
			if (!m_bSetOnce && m_fMana >= 0.f)
			{
				m_bIsInvincible = false;
				Callback_Mana();
				m_bSetOnce = true;
			}
		}

		break;
	}
	case eAnimCategory::FATAL:
	{
		if (m_pFatalTarget == nullptr)
			break;
		
		_vector vDir = XMVector3Normalize(m_pTransformCom->Get_State(STATE::POSITION) - m_pFatalTarget->Get_TransfomCom()->Get_State(STATE::POSITION));
		auto& vLockonPos = m_pFatalTarget->Get_LockonPos();
		_float3 vModifiedPos = _float3(vLockonPos.x + vDir.m128_f32[0], vLockonPos.y + vDir.m128_f32[1], vLockonPos.z + vDir.m128_f32[2]);
		CEffectContainer::DESC desc = {};
		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixScaling(1.3f, 1.3f, 1.3f) * XMMatrixTranslation(vModifiedPos.x, vModifiedPos.y, vModifiedPos.z));
		CGameObject* pEffect = { nullptr };

		m_fSetTime += fTimeDelta;
		if (m_fSetTime > 0.2f && !m_bSetCamera[0])
		{
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Start_DistanceLerp(2.5f, 0.1f, 0.2f);
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->StartShake(0.4f, 0.3f,100.f, 40.f, 0.05f);
			m_bSetCamera[0] = true;
			pEffect = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Player_FatalCombo1_P3S6"), &desc);
			if (pEffect == nullptr)
				MSG_BOX("이펙트 생성 실패함");

	 		m_pSoundCom->Play("SE_PC_SK_Hit_FatalAttack_Oil_0");

		}
		if (m_fSetTime > 1.f && !m_bSetCamera[1])
		{
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Start_DistanceLerp(2.5f, 0.1f, 0.2f); 
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->StartShake(0.4f, 0.3f, 100.f, 40.f, 0.05f);
			m_bSetCamera[1] = true;
			pEffect = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Player_FatalCombo2_P3S6"), &desc);
			if (pEffect == nullptr)
				MSG_BOX("이펙트 생성 실패함");
			m_pSoundCom->Play("SE_PC_SK_Hit_FatalAttack_Oil_1");
		}
		if (m_fSetTime > 2.f && !m_bSetCamera[2])
		{
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Start_DistanceLerp(2.2f, 0.1f, 0.4f, 0.2f);
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->StartShake(0.8f, 0.3f, 100.f, 40.f, 0.05f);
			m_bSetCamera[2] = true;
			pEffect = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Player_FatalCombo3_P5S7"), &desc);
			if (pEffect == nullptr)
				MSG_BOX("이펙트 생성 실패함");

			m_pSoundCom->Play("SE_PC_SK_Hit_M_FinishHit_Oil_0");
		}

		RootMotionActive(fTimeDelta);
		break;
	}
	case eAnimCategory::HITED:
	{
		RootMotionActive(fTimeDelta);
		break;
	}
	case eAnimCategory::GRINDER:
	{
		m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
		break;
	}
	case eAnimCategory::PULSE:
	{
		m_fSetSoundTime += fTimeDelta;
		if (m_fSetSoundTime > 0.5f)
		{
			if (!m_bSetSound)
			{
				m_bSetSound = true;
				m_pSoundCom->Play("SE_PC_FX_Item_Heal");

				LimActive(true, 10.5f, { 0.1f ,0.15f, 1.f, 1.f });

				// 이펙트 임시로 한번에 몰아둠 
				CEffectContainer::DESC desc = {};
				_uint iBoneIdx = m_pModelCom->Find_BoneIndex("BN_Weapon_L");
				//auto a = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(iBoneIdx))
				_matrix vWorldMat = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(iBoneIdx)) * m_pTransformCom->Get_WorldMatrix();
				XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(vWorldMat.r[3].m128_f32[0], vWorldMat.r[3].m128_f32[1], vWorldMat.r[3].m128_f32[2]));
				if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Player_HealSprite_Hand_S3"), &desc))
					MSG_BOX("이펙트 생성 실패함");

				CEffectContainer::DESC Partdesc = {};	// 고정 월드 위치로 넣기
				CEffectContainer::DESC socketdesc = {}; // 소켓으로 넣기
				socketdesc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();

				vWorldMat = XMLoadFloat4x4(
					m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bn_L_Deltoid"))) * // 왼쪽 어깨
					m_pTransformCom->Get_WorldMatrix();
				XMStoreFloat4x4(&Partdesc.PresetMatrix, vWorldMat);
				XMStoreFloat4x4(&socketdesc.PresetMatrix, XMMatrixRotationRollPitchYaw(m_pGameInstance->Compute_Random_Normal(), m_pGameInstance->Compute_Random_Normal(), m_pGameInstance->Compute_Random_Normal()));
				socketdesc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bn_L_Deltoid"));
				if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Player_Heal_Particle_P1"), &socketdesc)) // Partdesc
					MSG_BOX("이펙트 생성 실패함");

				vWorldMat = XMLoadFloat4x4(
					m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-Spine"))) * // 척추 중앙
					m_pTransformCom->Get_WorldMatrix();
				XMStoreFloat4x4(&Partdesc.PresetMatrix, vWorldMat);
				XMStoreFloat4x4(&socketdesc.PresetMatrix, XMMatrixRotationRollPitchYaw(m_pGameInstance->Compute_Random_Normal(), m_pGameInstance->Compute_Random_Normal(), m_pGameInstance->Compute_Random_Normal()));
				socketdesc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-Spine"));
				if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Player_Heal_Particle_P1"), &socketdesc)) // Partdesc
					MSG_BOX("이펙트 생성 실패함");

				vWorldMat = XMLoadFloat4x4(
					m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bn_R_Cucullaris"))) * // 모름 승모근?
					m_pTransformCom->Get_WorldMatrix();
				XMStoreFloat4x4(&Partdesc.PresetMatrix, vWorldMat);
				XMStoreFloat4x4(&socketdesc.PresetMatrix, XMMatrixRotationRollPitchYaw(m_pGameInstance->Compute_Random_Normal(), m_pGameInstance->Compute_Random_Normal(), m_pGameInstance->Compute_Random_Normal()));
				socketdesc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bn_R_Cucullaris"));
				if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Player_Heal_Particle_P1"), &socketdesc)) // Partdesc
					MSG_BOX("이펙트 생성 실패함");

				vWorldMat = XMLoadFloat4x4(
					m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bn_R_Hip"))) * // 언더니
					m_pTransformCom->Get_WorldMatrix();
				XMStoreFloat4x4(&Partdesc.PresetMatrix, vWorldMat);
				XMStoreFloat4x4(&socketdesc.PresetMatrix, XMMatrixRotationRollPitchYaw(m_pGameInstance->Compute_Random_Normal(), m_pGameInstance->Compute_Random_Normal(), m_pGameInstance->Compute_Random_Normal()));
				socketdesc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bn_R_Hip"));
				if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Player_Heal_Particle_P1"), &socketdesc)) // Partdesc
					MSG_BOX("이펙트 생성 실패함");

				CEffectContainer::DESC Lightdesc = {};
				Lightdesc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bn_L_ForeTwist"));
				Lightdesc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
				XMStoreFloat4x4(&Lightdesc.PresetMatrix, XMMatrixIdentity());
				if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Test_Lightning_P3"), &Lightdesc))
					MSG_BOX("이펙트 생성 실패함");
			} 
		}
		m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);



		break;
	}
	case eAnimCategory::GUARD_HIT:
	{
		if (m_pHitedTarget == nullptr)
			return;

		if (m_eHitedTarget == eHitedTarget::MONSTER || m_eHitedTarget == eHitedTarget::RANGED)
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
			// 퓨리 가드 성공 = 퍼펙트 가드 성공
			if (m_eHitedAttackType == CBossUnit::EAttackType::FURY_AIRBORNE)
			{
				_float  m_fTime = 0.4f;
				_float  m_fDistance = 3.f;

				if (m_pHitedTarget && !m_bMove)
				{
					const _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

					_vector vForward = m_pHitedTarget->Get_TransfomCom()->Get_State(STATE::LOOK);
					vForward = XMVector3Normalize(XMVectorSet(XMVectorGetX(vForward), 0.f, XMVectorGetZ(vForward), 0.f));

					const _vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vForward));

					const _vector vTargetPos = m_pHitedTarget->Get_TransfomCom()->Get_State(STATE::POSITION);
					const _vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);

					_vector vToPlayer = XMVectorSubtract(vPlayerPos, vTargetPos);
					vToPlayer = XMVector3Normalize(XMVectorSet(XMVectorGetX(vToPlayer), 0.f, XMVectorGetZ(vToPlayer), 0.f));

					const _float fSide = XMVectorGetX(XMVector3Dot(vToPlayer, vRight));
					const _float fSign = (fSide >= 0.f) ? static_cast<_float>(1.f) : static_cast<_float>(-1.f);

					const _float fCos45 = 0.17f;
					const _float fSin45 = 0.98f;

					_vector vPushDir = XMVector3Normalize(XMVectorAdd(XMVectorScale(vForward, fCos45), XMVectorScale(vRight, fSign * fSin45)));

					m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vPushDir, m_fDistance, m_pControllerCom);
					SyncTransformWithController();
				}
			}
			else
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
		}
		break;
	}
	case eAnimCategory::HITEDUP:
	{
		if (m_pHitedTarget == nullptr)
			return;

		_float  m_fTime = 0.4f;
		_float  m_fDistance = 3.f;

		if (m_eHitedAttackType == CBossUnit::EAttackType::FURY_AIRBORNE)
		{
			if (m_pHitedTarget && !m_bMove)
			{
				const _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

				_vector vForward = m_pHitedTarget->Get_TransfomCom()->Get_State(STATE::LOOK);
				vForward = XMVector3Normalize(XMVectorSet(XMVectorGetX(vForward), 0.f, XMVectorGetZ(vForward), 0.f));

				const _vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vForward));

				const _vector vTargetPos = m_pHitedTarget->Get_TransfomCom()->Get_State(STATE::POSITION);
				const _vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);

				_vector vToPlayer = XMVectorSubtract(vPlayerPos, vTargetPos);
				vToPlayer = XMVector3Normalize(XMVectorSet(XMVectorGetX(vToPlayer), 0.f, XMVectorGetZ(vToPlayer), 0.f));

				const _float fSide = XMVectorGetX(XMVector3Dot(vToPlayer, vRight));
				const _float fSign = (fSide >= 0.f) ? static_cast<_float>(1.f) : static_cast<_float>(-1.f);

				const _float fCos45 = 0.17f;
				const _float fSin45 = 0.98f;

				_vector vPushDir = XMVector3Normalize(XMVectorAdd(XMVectorScale(vForward, fCos45),XMVectorScale(vRight, fSign * fSin45)));

				m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vPushDir, m_fDistance, m_pControllerCom);
				SyncTransformWithController();
			}
		}
		else
		{
			if (m_pHitedTarget && !m_bMove)
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
		if (m_pHitedTarget == nullptr)
			return;

		_float  m_fTime = 0.1f;
		_float  m_fDistance = 2.f;

		if (m_pHitedTarget && !m_bMove)
		{
			_vector vLook = m_pHitedTarget->Get_TransfomCom()->Get_State(STATE::LOOK);
		
			m_bMove = m_pTransformCom->Move_Special(fTimeDelta, m_fTime, vLook, m_fDistance, m_pControllerCom);
			SyncTransformWithController();
		}
		break;
	}
	case eAnimCategory::ITEMFAIL:
	{
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

	if (stateName.find("Fail_Item") == 0)
		return eAnimCategory::ITEMFAIL;
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
		return eAnimCategory::NORMAL_ATTACKB;
	if (stateName.find("NormalAttack") == 0)
		return eAnimCategory::NORMAL_ATTACKA;
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

	if (stateName.find("DoubleDoor_Push_TrainStation") == 0)
		return eAnimCategory::STATIONDOOR;
	if (stateName.find("DoubleDoor_Boss") == 0)
		return eAnimCategory::FESTIVALDOOR;
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

	m_pAnimator->RegisterEventListener("OnSwordSkillTrail", [this]()
		{
			if (m_pWeapon&& Get_PlayerState() == EPlayerState::MAINSKILL)
			{
				
				m_pWeapon->Set_WeaponTrail_Active(true,TRAIL_SKILL_BLUE);
			}
		});


	m_pAnimator->RegisterEventListener("OffSwordSkillTrail", [this]()
		{
			if (m_pWeapon)
			{
				m_pWeapon->Set_WeaponTrail_Active(false, TRAIL_SKILL_BLUE);
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
				m_pSoundCom->Play_Random("SE_PC_WP_Bayonet_On_", 3);
			}
		});
	m_pAnimator->RegisterEventListener("PutWeapon", [this]()
		{
			if (m_pWeapon)
			{
				m_pWeapon->SetbIsActive(false);
				m_pWeapon->SetisAttack(false);
				m_pSoundCom->Play_Random("SE_PC_WP_Bayonet_Off_", 3);
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
	m_pAnimator->RegisterEventListener("ToggleLamp", [this]()
		{
			if(m_pPlayerLamp)
			{
				m_pPlayerLamp->ToggleLamp();
				CEffectContainer::DESC desc = {};
				_vector vPos = m_pPlayerLamp->Get_TransfomCom()->Get_State(STATE::POSITION);
				_matrix vWorldMat = XMMatrixTranslation(0.13f, 0.f, 0.05f) * XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("BN_Lamp_02"))) * m_pTransformCom->Get_WorldMatrix();
				XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(vWorldMat.r[3].m128_f32[0], vWorldMat.r[3].m128_f32[1], vWorldMat.r[3].m128_f32[2]));
				if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Player_Monad_P1"), &desc))
					return;

				m_pSoundCom->Play_Random("SE_PC_MT_Item_Monard_Lamp_", 3);
			}
		});
	


	m_pAnimator->RegisterEventListener("ReceiveDamageToFatalTarget", [this]()
		{
			if (m_pWeapon && m_pFatalTarget)
			{
				m_pWeapon->Clear_CollisionObj();
				//++m_iFatalAttackCount;
				//m_pWeapon->SetDamageRatio(0.5f + m_iFatalAttackCount);
				m_pFatalTarget->ReceiveDamage(m_pWeapon, COLLIDERTYPE::PLAYER_WEAPON);
				//m_pWeapon->SetDamageRatio(1.f);

			}

			if (m_iFatalAttackCount == 3)
				m_iFatalAttackCount = 0;
		});
	m_pAnimator->RegisterEventListener("SetRootStep", [this]()
		{
			m_fMaxRootMotionSpeed = 40.f;
			m_fRootMotionAddtiveScale = 1.3f;
		});

	m_pAnimator->RegisterEventListener("ResetRootStep", [this]()
		{
			m_fMaxRootMotionSpeed = 18.f;
			m_fRootMotionAddtiveScale = 1.f;
		});


		m_pAnimator->RegisterEventListener("OnInteractionKeyDoor", [this]()
			{
				if (auto pKeyDoor = dynamic_cast<CKeyDoor*>(m_pInterectionStuff))
				{
					pKeyDoor->OpenDoor();
				}
			});

		m_pAnimator->RegisterEventListener("OnInteractionShortCutDoor", [this]()
			{
				if (auto pShortCutDoor = dynamic_cast<CShortCutDoor*>(m_pInterectionStuff))
				{
					pShortCutDoor->ActivateUnlock();
				}
			});

		
}

_bool CPlayer::MoveToDoor(_float fTimeDelta, _vector vTargetPos)
{
	m_Input.bMove = true;
	m_bWalk = true;

	m_pAnimator->SetBool("Sprint", false);
	m_pAnimator->SetBool("Run", false);

	m_pTransformCom->SetfSpeedPerSec(g_fWalkSpeed);
	_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	_bool bFinishSetPosition = m_pTransformCom->Go_FrontByPosition(fTimeDelta, _fvector{ XMVectorGetX(vTargetPos), XMVectorGetY(vPosition), XMVectorGetZ(vTargetPos), 1.f}, m_pControllerCom);

	SyncTransformWithController();
	return bFinishSetPosition;
}

_bool CPlayer::RotateToDoor(_float fTimeDelta, _vector vRotation)
{
	m_Input.bMove = false;
	m_bWalk = false;
	_bool bFinishRotate = m_pTransformCom->RotateToDirectionSmoothly(vRotation, fTimeDelta);
	return bFinishRotate;
}

void CPlayer::RootMotionActive(_float fTimeDelta)
{
#ifdef _DEBUG
	_int iLevelIndex = m_pGameInstance->GetCurrentLevelIndex();
	if (iLevelIndex == ENUM_CLASS(LEVEL::JW))
		return;
#endif

	CAnimation* pCurAnim = m_pAnimator->GetCurrentAnim();
	_bool bUseRoot = (pCurAnim && pCurAnim->IsRootMotionEnabled());
	if (bUseRoot)
	{
		_float3 rootMotionDelta = m_pAnimator->GetRootMotionDelta();
		_vector vLocal = XMLoadFloat3(&rootMotionDelta);
		vLocal = XMVectorScale(vLocal, m_fRootMotionAddtiveScale);

		_vector vScale, vRotQuat, vTrans;
		XMMatrixDecompose(&vScale, &vRotQuat, &vTrans, m_pTransformCom->Get_WorldMatrix());
		XMVECTOR vWorldDelta = XMVector3Transform(vLocal, XMMatrixRotationQuaternion(vRotQuat));

		// Y값 처리
		_float dy = XMVectorGetY(vWorldDelta) - 0.8f;
		_vector finalDelta = XMVectorSetY(vWorldDelta, dy);

		_float fDeltaMag = XMVectorGetX(XMVector3Length(finalDelta));

		_float fMaxDeltaPerFrame = m_fMaxRootMotionSpeed / 60.0f;
		if (fDeltaMag > fMaxDeltaPerFrame)
		{
			finalDelta = XMVector3Normalize(finalDelta) * fMaxDeltaPerFrame;
			fDeltaMag = fMaxDeltaPerFrame;
		}

		if (fDeltaMag < 1e-6f)
		{
			m_PrevWorldDelta = XMVectorZero();
			return;
		}

		_float fSmoothThreshold = 0.3f; 
		_float fSmoothSpeed = 8.0f;  

		if (fDeltaMag > fSmoothThreshold)
		{
			_float alpha = clamp(fTimeDelta * fSmoothSpeed, 0.f, 1.f);
			finalDelta = XMVectorLerp(m_PrevWorldDelta, finalDelta, alpha);
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
		_matrix newWorld =
			XMMatrixScalingFromVector(vScale) *
			XMMatrixRotationQuaternion(vRotQuat) *
			XMMatrixTranslationFromVector(vTrans);
		m_pTransformCom->Set_WorldMatrix(newWorld);
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


	/* [ 일반 몬스터 피격 ] */
	if (eColliderType == COLLIDERTYPE::MONSTER_WEAPON)
	{
		//히트한 몬스터타입
		m_eHitedTarget = eHitedTarget::MONSTER;


		// 

		CBullet* pBullet = dynamic_cast<CBullet*>(pOther);

		if (pBullet)
		{
			m_eHitedTarget = eHitedTarget::RANGED;
			m_eHitedAttackType = CBossUnit::EAttackType::NONE;
			m_eHitMotion = HITMOTION::END;
			CGameObject* pRANGED = dynamic_cast<CGameObject*>(pOther);
			m_pHitedTarget = nullptr;

			_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
			_vector vOtherPos = pRANGED->Get_TransfomCom()->Get_State(STATE::POSITION);
			m_vHitNormal = vOtherPos - vPlayerPos;
		}


		cout << "몬스터 웨폰 충돌 들어옴" << endl;
		//피격한 객체를 찾는다.
		CUnit* pUnit = nullptr;
		CWeapon* pWeapon = dynamic_cast<CWeapon*>(pOther);
		if (pWeapon)
		{
			//이미 피격한 오브젝트라면 리턴
			if (pWeapon->Find_CollisonObj(this, eColliderType))
				return;

			pWeapon->Add_CollisonObj(this);
			pUnit = pWeapon->Get_Owner();
		}
		else
		{
			pUnit = dynamic_cast<CUnit*>(pOther);
		}

		if (pUnit)
		{
			//히트한 몬스터를 찾는다.
		//일반 몬스터나 엘리트 몬스터가 아니라면 리턴
			if (pUnit->Get_UnitType() != EUnitType::ELITE_MONSTER &&
				pUnit->Get_UnitType() != EUnitType::NORMAL_MONSTER)
				return;

			//Hp 가 0 이하면 리턴
			if (pUnit->GetHP() <= 0.f)
				return;

			// 필요한 정보를 수집한다.
			CalculateDamage(pOther, eColliderType);

			_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
			_vector vOtherPos = pUnit->Get_TransfomCom()->Get_State(STATE::POSITION);

			m_pHitedTarget = pUnit;
			m_vHitNormal = vOtherPos - vPlayerPos;
		}

		//가드 중이라면?
		if (m_bIsGuarding)
		{
			//퓨리어택이라면? 가드불가
			if (m_eHitedAttackType == CBossUnit::EAttackType::FURY_AIRBORNE ||
				m_eHitedAttackType == CBossUnit::EAttackType::FURY_STAMP)
			{
				//근데 퍼펙트 가드 타임 안이라면 가드 가능
				if (m_fPerfectGardTime < 0.2f)
				{
					m_eHitedAttackType = CBossUnit::EAttackType::NONE;
					m_bGardHit = true;
					m_bIsInvincible = true;
					return;
				}

				m_bIsHit = true;
				//Create_HitEffect();
				m_eHitedAttackType = CBossUnit::EAttackType::NONE;
				return;
			}

			m_bGardHit = true;

			//몬스터 가드리액션
			EHitDir eDir = ComputeHitDir();
			if (eDir == EHitDir::F || eDir == EHitDir::FR || eDir == EHitDir::FL)
				if(pUnit)
					pUnit->Block_Reaction();

			return;
		}

		//가드 중이 아니라면 피격상태로 넘긴다.
		m_bIsHit = true;
		//Create_HitEffect();

	}
	/* [ 보스 몬스터 피격 ] */
	if (eColliderType == COLLIDERTYPE::BOSS_WEAPON)
	{
		//만약 히트된 대상이 오일이라면 피격스위치는 켜지지않는다.
		COil* pOil = dynamic_cast<COil*>(pOther);
		if (pOil)
		{
			return;
		}
		//플레임 필드에 맞았을 경우
		CFlameField* pFlame = dynamic_cast<CFlameField*>(pOther);
		if (pFlame)
		{
			return;
		}


		//보스몬스터라면?
		CUnit* pBoss = dynamic_cast<CUnit*>(pOther);
		if (pBoss)
		{
			//필요한 정보를 수집한다.
			m_eHitedTarget = eHitedTarget::BOSS;
			
			m_pHitedTarget = pBoss;

			_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
			_vector vOtherPos = pBoss->Get_TransfomCom()->Get_State(STATE::POSITION);
			m_vHitNormal = vOtherPos - vPlayerPos;
		}
		else
		{
			//보스 몬스터가 아니라면 원거리 공격으로 간주한다.
			m_eHitedTarget = eHitedTarget::RANGED;
			m_eHitedAttackType = CBossUnit::EAttackType::NONE;
			m_eHitMotion = HITMOTION::END;
			CGameObject* pRANGED = dynamic_cast<CGameObject*>(pOther);
			m_pHitedTarget = nullptr;

			_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
			_vector vOtherPos = pRANGED->Get_TransfomCom()->Get_State(STATE::POSITION);
			m_vHitNormal = vOtherPos - vPlayerPos;
		}

		cout << " 현재 보스 공격 타입 :" << static_cast<_int>(m_eHitedAttackType) << endl;
		if (m_bIsGuarding)
		{
			//퓨리어택이라면? 가드불가
			if (m_eHitedAttackType == CBossUnit::EAttackType::FURY_AIRBORNE ||
				m_eHitedAttackType == CBossUnit::EAttackType::FURY_STAMP)
			{
				//근데 퍼펙트 가드 타임 안이라면 가드 가능
				if (m_fPerfectGardTime < 0.2f)
				{
					m_eHitedAttackType = CBossUnit::EAttackType::NONE;
					m_bIsInvincible = true;
					m_bGardHit = true;
					return;
				}

				m_bIsHit = true;
				m_eHitedAttackType = CBossUnit::EAttackType::NONE;
				return;
			}

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


	/* [ 일반 몬스터 피격 ] */
	if (eColliderType == COLLIDERTYPE::MONSTER_WEAPON || eColliderType == COLLIDERTYPE::MONSTER_WEAPON_BODY)
	{
		//히트한 몬스터타입
		m_eHitedTarget = eHitedTarget::MONSTER;

		cout << "몬스터 웨폰 충돌 들어옴" << endl;
		//피격한 객체를 찾는다.
		CUnit* pUnit = nullptr;
		CWeapon* pWeapon = dynamic_cast<CWeapon*>(pOther);
		if (pWeapon)
		{
			//이미 피격한 오브젝트라면 리턴
			if (pWeapon->Find_CollisonObj(this, eColliderType))
				return;

			pWeapon->Add_CollisonObj(this);
			pUnit = pWeapon->Get_Owner();
		}
		else
		{
			pUnit = dynamic_cast<CUnit*>(pOther);
			if (pUnit == nullptr)
				return;
		}

		//히트한 몬스터를 찾는다.
		//일반 몬스터나 엘리트 몬스터가 아니라면 리턴
		if (pUnit->Get_UnitType() != EUnitType::ELITE_MONSTER &&
			pUnit->Get_UnitType() != EUnitType::NORMAL_MONSTER &&
			pUnit == nullptr)
			return;


		//Hp 가 0 이하면 리턴
		if ( pUnit->GetHP() <= 0.f)
			return;
		if (eColliderType == COLLIDERTYPE::MONSTER_WEAPON_BODY &&
			pUnit->Get_UnitType() == EUnitType::ELITE_MONSTER)
		{
			auto pElite = static_cast<CElite_Police*>(pUnit);
			if (pElite->Get_ElbowHit())
				return;

			pElite->Set_ElbowHit(true);
		}


		// 필요한 정보를 수집한다.
		if (pUnit->Get_UnitType() == EUnitType::NORMAL_MONSTER)
			CalculateDamage(pOther, eColliderType);
		
		_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
		_vector vOtherPos = pUnit->Get_TransfomCom()->Get_State(STATE::POSITION);

		m_pHitedTarget = pUnit;
		m_vHitNormal = vOtherPos - vPlayerPos;

		//가드 중이라면?
		if (m_bIsGuarding)
		{
			//퓨리어택이라면? 가드불가
			if (m_eHitedAttackType == CBossUnit::EAttackType::FURY_AIRBORNE ||
				m_eHitedAttackType == CBossUnit::EAttackType::FURY_STAMP)
			{
				//근데 퍼펙트 가드 타임 안이라면 가드 가능
				if (m_fPerfectGardTime < 0.2f)
				{
					m_eHitedAttackType = CBossUnit::EAttackType::NONE;
					m_bGardHit = true;
					m_bIsInvincible = true;
					return;
				}

				m_bIsHit = true;
				m_eHitedAttackType = CBossUnit::EAttackType::NONE;
				return;
			}

			m_bGardHit = true;

			//몬스터 가드리액션
			EHitDir eDir = ComputeHitDir();
			if (eDir == EHitDir::F || eDir == EHitDir::FR || eDir == EHitDir::FL)
				pUnit->Block_Reaction();

			return;
		}

		//가드 중이 아니라면 피격상태로 넘긴다.
		m_bIsHit = true;
		//Create_HitEffect();
	}
	/* [ 보스 몬스터 피격 ] */
	if (eColliderType == COLLIDERTYPE::BOSS_WEAPON)
	{
		//만약 히트된 대상이 오일이라면 피격스위치는 켜지지않는다.
		COil* pOil = dynamic_cast<COil*>(pOther);
		if (pOil)
		{
			return;
		}
		//플레임 필드에 맞았을 경우
		CFlameField* pFlame = dynamic_cast<CFlameField*>(pOther);
		if (pFlame)
		{
			return;
		}


		//보스몬스터라면?
		CBossUnit* pBoss = dynamic_cast<CBossUnit*>(pOther);
		if (pBoss)
		{
			//필요한 정보를 수집한다.
			m_eHitedTarget = eHitedTarget::BOSS;
			m_pHitedTarget = pBoss;

			_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
			_vector vOtherPos = pBoss->Get_TransfomCom()->Get_State(STATE::POSITION);
			m_vHitNormal = vOtherPos - vPlayerPos;
		}
		else
		{
			//보스 몬스터가 아니라면 원거리 공격으로 간주한다.
			m_eHitedTarget = eHitedTarget::RANGED;
			m_eHitedAttackType = CBossUnit::EAttackType::NONE;
			m_eHitMotion = HITMOTION::END;
			CGameObject* pRANGED = dynamic_cast<CGameObject*>(pOther);
			m_pHitedTarget = nullptr;

			_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
			_vector vOtherPos = pRANGED->Get_TransfomCom()->Get_State(STATE::POSITION);
			m_vHitNormal = vOtherPos - vPlayerPos;
		}

		cout << " 현재 보스 공격 타입 :" << static_cast<_int>(m_eHitedAttackType) << endl;
		if (m_bIsGuarding)
		{
			//퓨리어택이라면? 가드불가
			if (m_eHitedAttackType == CBossUnit::EAttackType::FURY_AIRBORNE ||
				m_eHitedAttackType == CBossUnit::EAttackType::FURY_STAMP)
			{
				//근데 퍼펙트 가드 타임 안이라면 가드 가능
				if (m_fPerfectGardTime < 0.2f)
				{
					m_eHitedAttackType = CBossUnit::EAttackType::NONE;
					m_bIsInvincible = true;
					m_bGardHit = true;
					return;
				}

				m_bIsHit = true;
				m_eHitedAttackType = CBossUnit::EAttackType::NONE;
				//Create_HitEffect();
				return;
			}

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

void CPlayer::IsPerfectGard(_float fTimeDelta)
{
	if (m_Input.bShift)
		m_fPerfectGardTime += fTimeDelta;
	else
		m_fPerfectGardTime = 0.f;
}

void CPlayer::SetElementTypeWeight(EELEMENT eElement, _float fValue)
{
	
	_float fWeight = m_vecElements[eElement].fElementWeight + fValue;
	m_vecElements[eElement].fElementWeight = min(fWeight, 1.f);

	cout << "Element : " << static_cast<_int>(eElement) << " Weight : " << m_vecElements[eElement].fElementWeight << endl;

	if(EELEMENT::FIRE == eElement)
		m_pGameInstance->Notify(L"Player_Status", L"Fire", &m_vecElements[eElement].fElementWeight);
	
}

void CPlayer::Initialize_ElementConditions(const _float fDefaultDuration, const _float fDefaultWeight)
{
	const _uint uElementCount = static_cast<_uint>(m_vecElements.size());

	for (_uint uElemIndex = 0u; uElemIndex < uElementCount; ++uElemIndex)
	{
		EELEMENT eElemType = static_cast<EELEMENT>(uElemIndex);

		EELEMENTCONDITION& tElemCond = m_vecElements[uElemIndex];
		tElemCond.fDuration = fDefaultDuration;
		tElemCond.fElementWeight = fDefaultWeight;
	}
}

void CPlayer::Set_Ergo(_float fErgo)
{
	m_fErgo = fErgo;

	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"CurrentErgo"), &m_fErgo);
}

CWeapon* CPlayer::Get_Equip_Legion()
{
	return m_pLegionArm;
}

void CPlayer::Apply_Stat()
{

	if (m_eStat.iVitality > 1)
	{
		m_fMaxHp = floorf(ComputeLog(_float(m_eStat.iVitality), 2) * 100.f);
		m_fHp = m_fMaxHp;
	}

	if (m_eStat.iStamina > 1)
	{
		m_fMaxStamina = floorf(ComputeLog(_float(m_eStat.iStamina), 5) * 100.f);
		m_fStamina = m_fMaxStamina;
	}

	// 무기에 스탯이랑, 무기 기본공격력 이용해서 실제 주는 데미지를 계산해놓는다.
	// 효율? 이것도 만들어서 나중에 고쳐놓기
	if (nullptr != m_pWeapon)
	{
		_float fBaseDamage = m_pWeapon->GetBaseDamage();
		
		if (m_eStat.iMotivity > 1)
		{
			fBaseDamage += floorf(fBaseDamage * (ComputeLog(_float(m_eStat.iMotivity), 10)) * 0.5f );
		}
		if (m_eStat.iTechnique > 1)
		{
			fBaseDamage += floorf(fBaseDamage * (ComputeLog(_float(m_eStat.iTechnique), 10)) * 0.3f );
		}


		m_pWeapon->SetDamage(fBaseDamage);
	
	}

	if (nullptr != m_pLegionArm)
	{
		_float fBaseDamage = m_pLegionArm->GetBaseDamage();

		if (m_eStat.iMotivity > 1)
		{
			fBaseDamage += floorf(fBaseDamage * (ComputeLog(_float(m_eStat.iMotivity), 10)) * 0.1f);
		}
		if (m_eStat.iTechnique > 1)
		{
			fBaseDamage += floorf(fBaseDamage * (ComputeLog(_float(m_eStat.iTechnique), 10)) * 0.1f);
		}


		m_pLegionArm->SetDamage(fBaseDamage);

	}
	
	// 방어력 수치 계산은 나중에? 

	m_fDamageReduction = m_fArmor * 0.6f / (100 + m_fArmor);
	

	// 저항은 일단 나중에? 잘 모르겟음

	// 값 동기화
	Callback_HP();
	Callback_Stamina();

}

void CPlayer::Add_Ergo(_float fErgo)
{
	m_fErgo += fErgo;
	
	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"CurrentErgo"), &m_fErgo);
}

void CPlayer::Compute_MaxErgo(_int iLevel)
{
	if (iLevel == 0)
	{
		m_fMaxErgo = 100;
		return;
	}


	m_fMaxErgo = powf(1.2f, _float(iLevel)) * 100.f;
	
}

_bool CPlayer::Check_LevelUp(_int iLevel)
{
	_float fRequireErgo = powf(1.2f, _float(iLevel)) * 100.f;

	if (m_fErgo >= fRequireErgo)
		return true;

	return false;
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


	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_FireElement"),
		TEXT("Burn_Com"), reinterpret_cast<CComponent**>(&m_pBurn))))
		return E_FAIL;
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_FireElementMask"),
		TEXT("BurnMask_Com"), reinterpret_cast<CComponent**>(&m_pBurnMask))))
		return E_FAIL;
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_FireElementMask2"),
		TEXT("BurnMask_Com2"), reinterpret_cast<CComponent**>(&m_pBurnMask2))))
		return E_FAIL;

	m_pSoundCom->Set_AllVolume(g_fPlayerSoundVolume);


	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_Grinder"), TEXT("Com_Sound2"), reinterpret_cast<CComponent**>(&m_pGrinderSound))))
		return E_FAIL;
	m_pGrinderSound->Set_AllVolume(g_fPlayerSoundVolume);
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
	m_fHp = 358.f;
	m_fMaxHp = 358.f;

	Callback_HP();
	Callback_Mana();
	Callback_Stamina();

	m_pBelt_Up = static_cast<CBelt*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Belt"), nullptr));
	m_pBelt_Down = static_cast<CBelt*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Belt"), nullptr));


	auto pGrinder = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Grinder"), nullptr);

	m_pBelt_Down->Add_Item(static_cast<CItem*>(pGrinder), 0);

	auto pPortion = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Portion"), nullptr);

	m_pBelt_Up->Add_Item(static_cast<CItem*>(pPortion), 0);
	

	m_pSelectItem = m_pBelt_Up->Get_Items()[0];


	Callback_DownBelt();
	Callback_UpBelt();

	//auto pLamp = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Lamp"), nullptr);
	//m_pBelt_Down->Add_Item(static_cast<CItem*>(pLamp), 1);

	m_pGameInstance->Register_PullCallback(TEXT("Player_Status"), [this](_wstring eventName, void* data) {

		if (eventName == L"AddHp")
		{
			_float* fRatio = static_cast<_float*>(data);

			m_fHp = m_fHp + m_fMaxHp * (*fRatio);

			if (m_fHp > m_fMaxHp)
				m_fHp = m_fMaxHp;

			Callback_HP();
		}
			
		
		});

	//m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"CurrentHP"), &m_fHp);

	


	return S_OK;
}

HRESULT CPlayer::Ready_Arm()
{
	CLegionArm_Base::ARM_DESC eDesc{};
	
	eDesc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	eDesc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-L-Hand"));
	eDesc.pOwner = this;

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

HRESULT CPlayer::Ready_Stat()
{
	// 기본 값? 나중에 바꾸기.
	m_eStat.iVitality = 12;
	m_eStat.iStamina = 10;
	m_eStat.iCapacity = 8;
	m_eStat.iMotivity = 9;
	m_eStat.iTechnique = 5;
	m_eStat.iAdvance = 6;

	Apply_Stat();

	return S_OK;
}
HRESULT CPlayer::Ready_Effect()
{
	CEffectContainer::DESC desc = {};
	desc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(1.f, 6.f, 0.f)); // 조금 더 플레이어 전방에 있었으면 좋겠어서,,
	CEffectContainer* pEffect = { nullptr };
	pEffect = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Rain_PlayerFollow"), &desc));

	if (pEffect == nullptr)
		MSG_BOX("이펙트 생성 실패함");

	EFFECT_MANAGER->Store_EffectContainer(TEXT("PlayerRainVolume"), pEffect);
	EFFECT_MANAGER->Set_Active_Effect(TEXT("PlayerRainVolume"), false);

	return S_OK;
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

	PxCapsuleGeometry  geom = m_pGameInstance->CookCapsuleGeometry(0.45f, 0.9f);
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
	if (m_fHp < 0.f)
		m_fHp = 0.f;

	if (m_fHp >= m_fMaxHp)
		m_fHp = m_fMaxHp;

	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"CurrentHP"), &m_fHp);
	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"MaxHP"), &m_fMaxHp);
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

void CPlayer::Add_Mana(_float fMana)
{
	m_fMana += fMana;
	if (m_fMana > m_fMaxMana)
		m_fMana = m_fMaxMana;

	m_pGameInstance->Notify(TEXT("Player_Status"), _wstring(L"CurrentMana"), &m_fMana);
}

void CPlayer::Interaction_Door(INTERACT_TYPE eType, CGameObject* pObj, _bool bOpen)
{
	m_pInterectionStuff = pObj;
	string stateName;
	switch (eType)
	{
	case Client::TUTORIALDOOR:
		Play_CutScene_Door();
		break;
	case Client::FUOCO:
		stateName = "SlidingDoor";
		break;
	case Client::FESTIVALDOOR:
		stateName = "DoubleDoor_Boss";
		break;
	case Client::OUTDOOR:
		if (Get_HaveKey())
		{
			stateName = "Door_Unlock";
			m_pAnimator->SetBool("Outdoor", true);
		}
		else
		{
			stateName = "Door_Check";
			m_pAnimator->SetBool("Outdoor", false);
		}
		break;
	case Client::SHORTCUT:
		if (bOpen)
			stateName = "LiftDoor_Activate";
		else
			stateName = "LiftDoor_Fail";
		break;
	default:
		break;
	}

	if (stateName.empty() == false)
		m_pAnimator->Get_CurrentAnimController()->SetState(stateName);
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

void CPlayer::ItemWeapOnOff(_float fTimeDelta)
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
		if (vRotate)
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
			static_cast<CSlideDoor*>(m_pInterectionStuff)->Play_Sound();
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

void CPlayer::BurnActive(_float fDeltaTime)
{
	if (m_vecElements[0].fElementWeight <= 0.01f)
	{
		m_bBurnSwitch = false;
		return;
	}

	_float fFireWeight = m_vecElements[0].fElementWeight;
	_float fFireDuration = m_vecElements[0].fDuration;

	if (fFireWeight > 0.f)
	{
		//점화 감소시간
		if (m_pAnimator->CheckBool("Sprint"))
		{
			//스프린트 중이라면
			_float Speed = 0.2f;
			m_vecElements[0].fElementWeight -= fDeltaTime * Speed;
		}
		else if (m_bWalk)
		{
			//걷고있다면
			_float Speed = 0.1f;
			m_vecElements[0].fElementWeight -= fDeltaTime * Speed;
		}
		else
		{
			//뛰고 있다면
			_float Speed = 0.1f;
			m_vecElements[0].fElementWeight -= fDeltaTime * Speed;
		}
		
		
		if (m_fHp > 0.f)
		{
			m_fHp -= 0.05f;
		}
		else
		{
			m_vecElements[0].fElementWeight = 0.f;
			m_vecElements[0].fDuration = 0.f;
			m_bIsForceDead = true;
		}

		Callback_HP();
		m_pGameInstance->Notify(L"Player_Status", L"Fire", &m_vecElements[0].fElementWeight);
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_BURN, this);
		
	}

	/* [ 화속성 시작 ] */
	if (fFireWeight > 0.2f)
	{
		// 플레이어 점화 걸림
		m_bBurnSwitch = true;
	}
	else
	{
		//플레이어 점화 종료
		m_bBurnSwitch = false;
	}

	m_pGameInstance->Notify(L"Player_Status", L"Fire", &fFireWeight);
}

void CPlayer::OnBurn(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_BURN, this);

	if (m_fBurnPhase <= 0.25f)
	{
		m_fBurnPhase += (fTimeDelta * 0.1f) * m_fBurnSpeed;
		m_fBurnPhase = min(m_fBurnPhase, 0.25f);
	}
}
void CPlayer::OffBurn(_float fTimeDelta)
{
	if (m_fBurnPhase >= 0.f)
	{
		m_fBurnPhase -= (fTimeDelta * 0.1f) * m_fBurnSpeed;
		if (m_fBurnPhase < 0.f)
		{
			m_fBurnPhase = 0.f;
			m_pGameInstance->Notify(L"Player_Status", L"Fire", &m_fBurnPhase);
		}
		else
			m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_BURN, this);
	}
}



void CPlayer::LimActive(_bool bOnOff, _float fSpeed, _float4 vColor)
{
	m_vLimLightColor = vColor;
	m_bLimSwitch = bOnOff;
	m_fLimSpeed = fSpeed;
}

void CPlayer::OnLim(_float fTimeDelta)
{
	if (m_fLimPhase <= 1.f)
	{
		m_fLimPhase += fTimeDelta * m_fLimSpeed;
		m_fLimPhase = min(m_fLimPhase, 1.f);
	}
}
void CPlayer::OffLim(_float fTimeDelta)
{
	if (m_fLimPhase >= 0.f)
	{
		m_fLimPhase -= fTimeDelta * m_fLimSpeed;
		if (m_fLimPhase < 0.f)
			m_fLimPhase = 0.f;
	}
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

		if (KEY_UP(DIK_SPACE) && 
			m_eCategory != eAnimCategory::HITED &&
			m_eCategory != eAnimCategory::HITEDSTAMP &&
			m_eCategory != eAnimCategory::HITEDUP)
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

void CPlayer::Add_Icon(const _wstring& strItemTag)
{
	if (strItemTag == L"Prototype_GameObject_Lamp")
	{
		auto pLamp = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Lamp"), nullptr);

		m_pBelt_Down->Add_Item(static_cast<CItem*>(pLamp), 1);

		Callback_DownBelt();
		Callback_UpBelt();
	}
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

void CPlayer::Create_HitEffect()
{
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	m_vHitPos = m_vHitPos;
	int a = 0;
		
	vPos += XMVector3Normalize(m_vHitNormal) * 0.3f;
	_float3 vEffPos = {};
	XMStoreFloat3(&vEffPos, vPos);
	vEffPos.y += 1.7f;

	CEffectContainer::DESC desc = {};

	XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixTranslation(vEffPos.x, vEffPos.y, vEffPos.z));

	// 스파크 이펙트
	if (MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_PlayerHit_Basic_Spark_1_P1S3"), &desc) == nullptr)
		MSG_BOX("이펙트 생성 실패함");

	_vector vFrom = XMVectorSet(0.f, 1.f, 0.f, 0.f); // 기준: +Y
	_vector vTo = XMVector3Normalize(-m_vHitNormal);         // 원하는 방향

	_vector qRot = XMQuaternionRotationVectorToVector(vFrom, vTo);
	rand() % 4 == 0 ? qRot = XMQuaternionIdentity() : qRot; // 위로 피 나오는 것도 넣어야하는데 당장 조건이 뭔지 모르겠어서 일단 랜덤으로 함
	XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixRotationQuaternion(qRot) * XMMatrixTranslation(vEffPos.x, vEffPos.y, vEffPos.z));
	// 피 이펙트
	if (MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_TEST_BLOOD_UP"), &desc) == nullptr)
		MSG_BOX("이펙트 생성 실패함");
}

void CPlayer::Create_GuardEffect(_bool isPerfect)
{
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vDir = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));

	vPos += vDir * 1.5f;

	_float3 vEffPos = {};
	XMStoreFloat3(&vEffPos, vPos);
	vEffPos.y += 0.5f;

	CEffectContainer::DESC desc = {};
	XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixScaling(2.f, 2.f, 2.f) * XMMatrixTranslation(vEffPos.x, vEffPos.y, vEffPos.z));

	CGameObject* pEffect = { nullptr };
	if (isPerfect)
		pEffect = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_PlayerGuardPerfect_P3S6pls"), &desc);
	else
		pEffect = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_PlayerGuardNormal_P2"), &desc);

	if (pEffect == nullptr)
		MSG_BOX("이펙트 생성 실패함");
}

void CPlayer::Create_LeftArm_Lightning()
{
	CEffectContainer::DESC Lightdesc = {};
	Lightdesc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bn_L_ForeTwist"));
	Lightdesc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	XMStoreFloat4x4(&Lightdesc.PresetMatrix, XMMatrixIdentity());
	if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Player_TESTCutscene_Fuoco_LeftarmLightning"), &Lightdesc))
		MSG_BOX("이펙트 생성 실패함");
}

void CPlayer::Movement(_float fTimeDelta)
{

	SyncTransformWithController();

	if (!CCamera_Manager::Get_Instance()->GetbMoveable())
		return;

	SetMoveState(fTimeDelta);
}

void CPlayer::SyncTransformWithController()
{
	if (!m_pControllerCom) return;

	PxExtendedVec3 pos = m_pControllerCom->Get_Controller()->getPosition();
	_vector vPos = XMVectorSet((float)pos.x, (float)pos.y - 0.95f, (float)pos.z, 1.f);
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
//#ifdef _DEBUG
//	_int iCurLevel = m_pGameInstance->GetCurrentLevelIndex();
//	if (iCurLevel == ENUM_CLASS(LEVEL::JW))
//		return;
//#endif // _DEBUG

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
	PxExtendedVec3 exPos = m_pControllerCom->Get_Controller()->getPosition();
	
	CIgnoreSelfCallback filter(m_pControllerCom->Get_IngoreActors());
	PxControllerFilters filters;
	filters.mFilterCallback = &filter; // 필터 콜백 지정
	PxControllerCollisionFlags collisionFlags;
	collisionFlags = m_pControllerCom->Get_Controller()->move(pxMove, 0.001f, fTimeDelta, filters);

	//printf(" 왜 안움직이지?? : %s \n", strName.c_str());
	PxExtendedVec3 nowPos = m_pControllerCom->Get_Controller()->getPosition();

	// 4. 지면에 닿았으면 중력 속도 초기화
	if (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN)
		m_vGravityVelocity.y = 0.f;

	if (collisionFlags & PxControllerCollisionFlag::eCOLLISION_UP)
	{
		m_pControllerCom->Get_Controller()->setPosition(PxExtendedVec3(nowPos.x, exPos.y, nowPos.z));
		printf("Call Collider Up\n");
	}
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
	Safe_Release(m_pBurn);
	Safe_Release(m_pBurnMask);
	Safe_Release(m_pBurnMask2);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAnimator);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pControllerCom);
	Safe_Release(m_pGrinderSound);
	Safe_Release(m_pSpringBoneSys);

	for (size_t i = 0; i < ENUM_CLASS(EPlayerState::END); ++i)
		Safe_Delete(m_pStateArray[i]);

	Safe_Release(m_pBelt_Down);
	Safe_Release(m_pBelt_Up);

	Safe_Delete(m_pHitReport);
}
