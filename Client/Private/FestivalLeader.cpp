#include "FestivalLeader.h"

#include "Bone.h"
#include <Oil.h>
#include <Player.h>
#include "Projectile.h"
#include "FlameField.h"
#include "Static_Decal.h"
#include "SpringBoneSys.h"
#include "GameInstance.h"
#include "Effect_Manager.h"
#include "LockOn_Manager.h"
#include "Camera_Manager.h"
#include "Client_Calculation.h"
#include "UI_MonsterHP_Bar.h"
#include <PhysX_IgnoreSelfCallback.h>
#include "Weapon_Monster.h"

#include "EffectContainer.h"
#include "Effect_Manager.h"

CFestivalLeader::CFestivalLeader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBossUnit(pDevice, pContext)
{
}

CFestivalLeader::CFestivalLeader(const CFestivalLeader& Prototype)
	: CBossUnit(Prototype)
{
}
HRESULT CFestivalLeader::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFestivalLeader::Initialize(void* pArg)
{
	m_fMaxHp = 900.f;
	m_fDamage = 15.f;
	m_fAttckDleay = 1.5f;
	m_iPatternLimit = 1;
	m_fChasingDistance = 3.f;
	m_fMinimumTurnAngle = 45.f;
	m_fPhase2HPThreshold = 0.65f;
	m_fMaxRootMotionSpeed = 30.f;
	m_fChangeMoveDirCooldown = 2.f;

	m_fGroggyScale_Weak = 0.02f;
	m_fGroggyScale_Strong = 0.03f;
	m_fGroggyScale_Charge = 0.12f;

	if (pArg == nullptr)
	{
		UNIT_DESC UnitDesc{};
		UnitDesc.eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
		UnitDesc.fRotationPerSec = XMConvertToRadians(180.f);
		UnitDesc.fSpeedPerSec = m_fWalkSpeed;
		lstrcpy(UnitDesc.szName, TEXT("FestivalLeader"));
		UnitDesc.szMeshID = TEXT("FestivalLeader");
		UnitDesc.InitPos = _float3(200.520279f, 7.415279f, -8.159760f);
		UnitDesc.InitScale = _float3(0.9f, 0.9f, 0.9f);

		if (FAILED(__super::Initialize(&UnitDesc)))
			return E_FAIL;
	}
	else
	{
		UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);

		lstrcpy(pDesc->szName, TEXT("FestivalLeader"));
		pDesc->szMeshID = TEXT("FestivalLeader");
		pDesc->fRotationPerSec = XMConvertToRadians(180.f);
		pDesc->fSpeedPerSec = m_fWalkSpeed;

		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;
	}

	if (m_pAnimator)
	{
		m_HeadLocalInit = m_pHammerBone->Get_TransformationMatrix_Float4x4();
		m_pAnimator->PlayClip(m_pModelCom->GetAnimationClipByName("AS_Idle_C_1"), false);
		m_pAnimator->Update(0.016f);
		m_pModelCom->Update_Bones();

		CSpringBoneSys::SpringInitParams springParams;
		springParams.restDirBiasEnable = true;
		m_pSpringBoneSys = CSpringBoneSys::Create(m_pModelCom, vector<string>{"Hair", "Head_Rope"}, springParams);
		if (m_pSpringBoneSys == nullptr)
			return E_FAIL;
		if (m_pHammerBone)
		{
			_matrix cmHead = XMLoadFloat4x4(m_pHammerBone->Get_CombinedTransformationMatrix()); // 머리 컴바인드
			_matrix cmHand = XMLoadFloat4x4(m_pRightWeaponBone->Get_CombinedTransformationMatrix()); // 오른손 무기 소켓 컴바인드

			//  손 기준 로컬 오프셋
			_matrix attachOffset = cmHead * XMMatrixInverse(nullptr, cmHand); 
			XMStoreFloat4x4(&m_StoredHeadLocalMatrix, attachOffset);
		}
		m_pAnimator->Get_CurrentAnimController()->SetStateToEntry();

	}
	// 0번 메시는 다리,1번은 몸통, 4번 양팔, 5번 머리
	// 2,3번은 바스켓


	// 플레이어 카메라 레이충돌 무시하기 위한
	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorComForHammer->Get_Actor());
	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorComForBasket->Get_Actor());
	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorComForLeftHand->Get_Actor());
	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorComForRightHand->Get_Actor());
	//m_pPhysXActorCom->Add_IngoreActors(static_cast<CWeapon_Monster*>(m_pHammer)->Get_PhysXActor()->Get_Actor());
	
	return S_OK;
}

void CFestivalLeader::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_bDead)
		m_pHPBar->Set_bDead();

#ifdef _DEBUG

	if (KEY_DOWN(DIK_X))
	{

		static _int i = 0;
		static array<_int, 13> testArray{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,13 };

		m_eCurAttackPattern = static_cast<EBossAttackPattern>(i + 1);
		switch (m_eCurAttackPattern)
		{
		case CFestivalLeader::Slam:
			cout << "Slam" << endl;
			break;
		case CFestivalLeader::CrossSlam:
			cout << "CrossSlam" << endl;
			break;
		case CFestivalLeader::JumpAttack:
			cout << "JumpAttack" << endl;
			break;
		case CFestivalLeader::Strike:
			cout << "Strike" << endl;
			break;
		case CFestivalLeader::Spin:
			cout << "Spin" << endl;
			break;
		case CFestivalLeader::HalfSpin:
			cout << "HalfSpin" << endl;
			break;
		case CFestivalLeader::HammerSlam:
			cout << "HammerSlam" << endl;
			break;
		case CFestivalLeader::DashSwing:
			cout << "DashSwing" << endl;
			break;
		case CFestivalLeader::Swing:
			cout << "Swing" << endl;
			break;
		case CFestivalLeader::FuryHammerSlam:
			cout << "FuryHammerSlam" << endl;
			break;
		case CFestivalLeader::FurySwing:
			cout << "FurySwing" << endl;
			break;
		case CFestivalLeader::FuryBodySlam:
			cout << "FuryBodySlam" << endl;
			break;
		default:
			cout << "Unknown" << endl;;
			break;
		}
		m_pAnimator->SetInt("SkillType", testArray[i++]);
		if (i >= 13)
			i = 0;
	}

	if (KEY_DOWN(DIK_C))
	{
		m_pAnimator->SetTrigger("Attack");
	}

	if (KEY_DOWN(DIK_B))
	{
		EnterCutScene();
		m_bDebugMode = !m_bDebugMode;
	}

	if (KEY_PRESSING(DIK_LALT))
	{
		if (KEY_DOWN(DIK_Q))
		{
			CEffectContainer::DESC desc = {};

			desc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
			desc.pSocketMatrix = m_pLeftForearmBone->Get_CombinedTransformationMatrix();
			XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixIdentity());

			CGameObject* pEC = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_OldSparkDrop_P1"), &desc);
			if (pEC == nullptr)
				MSG_BOX("이펙트 생성 실패함");
		}
		if (KEY_DOWN(DIK_2))
		{

		}
	}
#endif

	if (nullptr != m_pHPBar)
		m_pHPBar->Priority_Update(fTimeDelta);
}

void CFestivalLeader::Update(_float fTimeDelta)
{
	if (CalculateCurrentHpRatio() <= 0.f)
	{
		// 죽음 처리
		m_bUseLockon = false;
		if (m_eCurrentState != EEliteState::DEAD)
		{
			m_eCurrentState = EEliteState::DEAD;
			//m_pSoundCom->Play_Random("VO_NPC_NHM_Boss_Fire_Eater_Dead_", 3);
			m_pAnimator->SetTrigger("SpecialDie");
			CLockOn_Manager::Get_Instance()->Set_Off(this);
		}
		Safe_Release(m_pHPBar);
	}

	__super::Update(fTimeDelta);
	if (m_bSwitchHeadSpace)
	{
		ApplyHeadSpaceSwitch(fTimeDelta);
		Update_Collider(); 
	}
	m_pSpringBoneSys->Update(fTimeDelta);
	//Update_HairSpring(fTimeDelta);
	if (nullptr != m_pHPBar)
		m_pHPBar->Update(fTimeDelta);

	if (m_pPlayer && static_cast<CUnit*>(m_pPlayer)->GetHP() <= 0 && m_pHPBar)
		m_pHPBar->Set_RenderTime(0.f);

}

void CFestivalLeader::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider())
	{
		if (m_pPhysXActorComForHammer->Get_ReadyForDebugDraw())
			m_pGameInstance->Add_DebugComponent(m_pPhysXActorComForHammer);
		if (m_pPhysXActorComForBasket->Get_ReadyForDebugDraw())
			m_pGameInstance->Add_DebugComponent(m_pPhysXActorComForBasket);
		if (m_pPhysXActorComForRightHand->Get_ReadyForDebugDraw())
			m_pGameInstance->Add_DebugComponent(m_pPhysXActorComForRightHand);
		if (m_pPhysXActorComForLeftHand->Get_ReadyForDebugDraw())
			m_pGameInstance->Add_DebugComponent(m_pPhysXActorComForLeftHand);
	}
#endif

	if (nullptr != m_pHPBar)
		m_pHPBar->Late_Update(fTimeDelta);
}

HRESULT CFestivalLeader::Render()
{
	__super::Render();
	return S_OK;
}

void CFestivalLeader::Reset()
{
	__super::Reset();
	m_bPlayerCollided = false;
	m_bPhase2Processed = false;
	m_bWaitPhase2 = false;
	m_iLastComboType = -1;
	m_eCurAttackPattern = EBossAttackPattern::BAP_NONE;
	m_ePrevAttackPattern = EBossAttackPattern::BAP_NONE;
	if (m_pAnimator)
	{
		m_pAnimator->CancelOverrideAnimController();
	}

	m_pModelCom->SetMeshVisible(2, true);
	m_pModelCom->SetMeshVisible(3, true);
	m_pModelCom->SetMeshVisible(5, true);

	m_bSwitchHeadSpace = false;                       
	if (m_pHammerBone && m_iOriginBoneIndex >= 0)
	{
		m_pHammerBone->Set_ParentBoneIndex(m_iOriginBoneIndex);
		m_pHammerBone->Set_TransformationMatrix(XMLoadFloat4x4(&m_HeadLocalInit));
	}
	m_pAnimator->Update(0.016f);
	m_pModelCom->Update_Bones();                      // 뼈 재계산
	Update_Collider();                                // 콜라이더도 같은 프레임에 동기화
}

HRESULT CFestivalLeader::Ready_Components(void* pArg)
{
	if (FAILED(__super::Ready_Components(pArg)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX2"), reinterpret_cast<CComponent**>(&m_pPhysXActorComForHammer))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX3"), reinterpret_cast<CComponent**>(&m_pPhysXActorComForBasket))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX4"), reinterpret_cast<CComponent**>(&m_pPhysXActorComForRightHand))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX5"), reinterpret_cast<CComponent**>(&m_pPhysXActorComForLeftHand))))
		return E_FAIL;

	///* For.Com_Sound */
	//if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_FireEater"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CFestivalLeader::Ready_Actor()
{
	if (FAILED(__super::Ready_Actor()))
		return E_FAIL;
	PxFilterData Filter{};
	Filter.word0 = WORLDFILTER::FILTER_MONSTERWEAPON;
	Filter.word1 = WORLDFILTER::FILTER_PLAYERBODY;
	// 해머
	if (m_pHammerBone)
	{
		_vector vWorldOffset = XMVectorSet(0.f,0.f, -2.5f, 1.f);
		_matrix matWorldOffset = XMMatrixTranslationFromVector(vWorldOffset);
		const PxTransform hammerPose = GetBonePose(m_pHammerBone, &matWorldOffset);

		PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(PxVec3(1.8f,0.7f,0.7f));

		m_pPhysXActorComForHammer->Create_Collision(
			m_pGameInstance->GetPhysics(),
			geom,
			hammerPose,
			m_pGameInstance->GetMaterial(L"Default")
		);
		m_pPhysXActorComForHammer->Set_ShapeFlag(false, true, true);

	
		m_pPhysXActorComForHammer->Set_SimulationFilterData(Filter);
		m_pPhysXActorComForHammer->Set_QueryFilterData(Filter);

		m_pPhysXActorComForHammer->Set_Owner(this);
		m_pPhysXActorComForHammer->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
		m_pPhysXActorComForHammer->Set_Kinematic(true);

		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorComForHammer->Get_Actor());
	}

	// 바스켓
	if (m_pBasketBone)
	{
		const PxTransform basketPose = GetBonePose(m_pBasketBone);

		PxSphereGeometry basketGeom = m_pGameInstance->CookSphereGeometry(1.0f);
		m_pPhysXActorComForBasket->Create_Collision(
			m_pGameInstance->GetPhysics(),
			basketGeom,
			basketPose,
			m_pGameInstance->GetMaterial(L"Default")
		);
		m_pPhysXActorComForBasket->Set_ShapeFlag(false, true, true);


		m_pPhysXActorComForBasket->Set_SimulationFilterData(Filter);
		m_pPhysXActorComForBasket->Set_QueryFilterData(Filter);

		m_pPhysXActorComForBasket->Set_Owner(this);
		m_pPhysXActorComForBasket->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
		m_pPhysXActorComForBasket->Set_Kinematic(true);

		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorComForBasket->Get_Actor());
	}


	if (m_pLeftHandBone)
	{
		const PxTransform leftPose = GetBonePose(m_pLeftHandBone);

		PxSphereGeometry leftGeom = m_pGameInstance->CookSphereGeometry(0.8f);
		m_pPhysXActorComForLeftHand->Create_Collision(
			m_pGameInstance->GetPhysics(),
			leftGeom,
			leftPose,
			m_pGameInstance->GetMaterial(L"Default")
		);
		m_pPhysXActorComForLeftHand->Set_ShapeFlag(false, true, true);


		m_pPhysXActorComForLeftHand->Set_SimulationFilterData(Filter);
		m_pPhysXActorComForLeftHand->Set_QueryFilterData(Filter);

		m_pPhysXActorComForLeftHand->Set_Owner(this);
		m_pPhysXActorComForLeftHand->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
		m_pPhysXActorComForLeftHand->Set_Kinematic(true);

		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorComForLeftHand->Get_Actor());
	}

	if (m_pRightHandBone)
	{
		const PxTransform rightPose = GetBonePose(m_pRightHandBone);

		PxSphereGeometry rightGeom = m_pGameInstance->CookSphereGeometry(0.8f);
		m_pPhysXActorComForRightHand->Create_Collision(
			m_pGameInstance->GetPhysics(),
			rightGeom,
			rightPose,
			m_pGameInstance->GetMaterial(L"Default")
		);
		m_pPhysXActorComForRightHand->Set_ShapeFlag(false, true, true);

		m_pPhysXActorComForRightHand->Set_SimulationFilterData(Filter);
		m_pPhysXActorComForRightHand->Set_QueryFilterData(Filter);
		m_pPhysXActorComForRightHand->Set_Owner(this);
		m_pPhysXActorComForRightHand->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
		m_pPhysXActorComForRightHand->Set_Kinematic(true);
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorComForRightHand->Get_Actor());
	}

	return S_OK;
}

void CFestivalLeader::Ready_BoneInformation()
{
	m_iLockonBoneIndex = m_pModelCom->Find_BoneIndex("Bip001-Spine");

	auto it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Bn_Head_Weapon"); });
	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pHammerBone = *it;
	}

	it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Bn_Basket_Body"); });

	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pBasketBone = *it;
	}


	it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "BN_Weapon_R"); });

	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pRightWeaponBone = *it;
	}


	it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Ref_Bip001-L-Hand"); });

	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pLeftHandBone = *it;
	}

	it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Bip001-R-Hand"); });

	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pRightHandBone = *it;
	}

	it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Ref_Bip001-L-Forearm"); }); // 왼쪽 팔꿈치

	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pLeftForearmBone = *it;
	}

	it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "Skin_Bip001-R-Forearm"); }); // 오른쪽 팔꿈치 이름 통일 안시키는거 진짜 깨  빢 치네 

	if (it != m_pModelCom->Get_Bones().end())
	{
		m_pRightForearmBone = *it;
	}
}

void CFestivalLeader::Update_Collider()
{
	__super::Update_Collider();

	if (m_pPhysXActorComForHammer && m_pHammerBone)
	{
		_vector vWorldOffset = XMVectorSet(0.f, 0.f, -2.5f, 1.f);
		_matrix matWorldOffset = XMMatrixTranslationFromVector(vWorldOffset);
		m_pPhysXActorComForHammer->Set_Transform(GetBonePose(m_pHammerBone, &matWorldOffset));
	}

	if (m_pPhysXActorComForBasket && m_pBasketBone)
		m_pPhysXActorComForBasket->Set_Transform(GetBonePose(m_pBasketBone));

	if (m_pPhysXActorComForRightHand && m_pRightHandBone)
		m_pPhysXActorComForRightHand->Set_Transform(GetBonePose(m_pRightHandBone));

	if (m_pPhysXActorComForLeftHand && m_pLeftHandBone)
		m_pPhysXActorComForLeftHand->Set_Transform(GetBonePose(m_pLeftHandBone));


}

void CFestivalLeader::UpdateAttackPattern(_float fDistance, _float fTimeDelta)
{
	if (m_fFirstChaseBeforeAttack >= 0.f)
	{
		m_fFirstChaseBeforeAttack -= fTimeDelta;
		return;
	}
	// 퓨리 몸빵
	if (m_bIsFirstAttack)
	{
		m_pAnimator->SetTrigger("Attack");
		m_pAnimator->SetInt("SkillType", Strike);
		m_bIsFirstAttack = false;
		m_pAnimator->SetBool("Move", false);
		m_fAttackCooldown = m_fAttckDleay;
		SetTurnTimeDuringAttack(1.5f, 1.4f);
		m_eAttackType = EAttackType::AIRBORNE;
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
			pPlayer->SetHitedAttackType(EAttackType::AIRBORNE);
		return;
	}

	if (fDistance > 10.f)
	{
		if (m_bIsPhase2)
		{
			if (m_fAttackCooldown <= 0.f)
			{
				m_pAnimator->SetInt("SkillType", DashSwing);
				m_pAnimator->SetTrigger("Attack");
				m_ePrevAttackPattern = m_eCurAttackPattern;
				m_eCurAttackPattern = DashSwing;
				m_ePrevState = m_eCurrentState;
				m_eCurrentState = EEliteState::ATTACK;
				m_fAttackCooldown = m_fAttckDleay + 2.f;
			}
		}
		return;
	}


	if (false == UpdateTurnDuringAttack(fTimeDelta))
	{
		return;
	}


	if (m_eCurrentState == EEliteState::ATTACK)
	{
		return;
	}

	if (m_fAttackCooldown > 0.f)
	{
		m_fAttackCooldown -= fTimeDelta;
		m_fAttackCooldown = max(m_fAttackCooldown, 0.f);
		if (m_fAttackCooldown > 0.f)
			return; // 공격 쿨타임이 남아있으면 업데이트 중지
	}


	if (fDistance >= 25.f)
		return;


#ifdef _DEBUG
	if (m_bDebugMode)
		return;
#endif // _DEBUG



	EBossAttackPattern eSkillType = static_cast<EBossAttackPattern>(GetRandomAttackPattern(fDistance));


	SetupAttackByType(eSkillType);

	m_pAnimator->SetBool("Move", false);
	m_pAnimator->SetInt("SkillType", eSkillType);
	m_pAnimator->SetTrigger("Attack");
	m_eCurrentState = EEliteState::ATTACK;
	m_fAttackCooldown = m_fAttckDleay;
}

void CFestivalLeader::UpdateStateByNodeID(_uint iNodeID)
{
	m_iPrevNodeID = m_iCurNodeID;
	m_iCurNodeID = iNodeID;
	m_ePrevState = m_eCurrentState;
	m_bRootMotionClamped = false;
	switch (iNodeID)
	{
	case ENUM_CLASS(BossStateID::CutScene_Start):
	case ENUM_CLASS(BossStateID::CutScene_End):
		m_eCurrentState = EEliteState::CUTSCENE;
		break;
	case ENUM_CLASS(BossStateID::Idle):
		m_eCurrentState = EEliteState::IDLE;
		break;
	case ENUM_CLASS(BossStateID::Walk_B):
	case ENUM_CLASS(BossStateID::Walk_F):
	case ENUM_CLASS(BossStateID::Walk_R):
	case ENUM_CLASS(BossStateID::Walk_L):
	{
		m_pTransformCom->SetfSpeedPerSec(m_fWalkSpeed);
		m_eCurrentState = EEliteState::WALK;
	}
	break;
	case ENUM_CLASS(BossStateID::Run_F):
		m_pTransformCom->SetfSpeedPerSec(m_fRunSpeed);
		m_eCurrentState = EEliteState::RUN;
		break;
	case ENUM_CLASS(BossStateID::Groggy_Start):
	case ENUM_CLASS(BossStateID::Groggy_Loop):
	case ENUM_CLASS(BossStateID::Groggy_End):
		m_eCurrentState = EEliteState::GROGGY;
		break;
	case ENUM_CLASS(BossStateID::Special_Die):
		m_eCurrentState = EEliteState::DEAD;
		break;
	case ENUM_CLASS(BossStateID::Turn_L):
	case ENUM_CLASS(BossStateID::Turn_R):
		m_eCurrentState = EEliteState::TURN;
		break;
	case ENUM_CLASS(BossStateID::Fatal_Hit_Start):
	case ENUM_CLASS(BossStateID::Fatal_Hit_Loop):
	case ENUM_CLASS(BossStateID::Fatal_Hit_End):
		m_eCurrentState = EEliteState::FATAL;
		break;
	case ENUM_CLASS(BossStateID::Atk_SwingCom_Start):
	case ENUM_CLASS(BossStateID::Atk_DashSwingCom_Start):
		m_iSwingComboCount++;
		if (m_iSwingComboCount > m_iSwingComboLimit)
		{
			m_pAnimator->SetBool("IsCombo", false);
			m_iSwingComboCount = 0;
		}
		break;
	case ENUM_CLASS(BossStateID::Atk_HalfSpin_Start):
		break;
	case ENUM_CLASS(BossStateID::Atk_FuryBodySlam_Start):
		m_eAttackType = EAttackType::FURY_STAMP;	
		break;
	case ENUM_CLASS(BossStateID::Atk_FurySwing_Start):
		m_eAttackType = EAttackType::STRONG_KNOCKBACK;
		break;
	case ENUM_CLASS(BossStateID::Atk_HammerSlam_Start):
		m_eAttackType = EAttackType::STAMP;
		break;
	case ENUM_CLASS(BossStateID::Atk_AlternateSmash_Start):
	case ENUM_CLASS(BossStateID::Atk_AlternateSmash_Start2):
	case ENUM_CLASS(BossStateID::Atk_AlternateSmash_Start3):
		m_eAttackType = EAttackType::NORMAL;
		break;
	case ENUM_CLASS(BossStateID::Atk_HammerSlam_Loop):
		m_eAttackType = EAttackType::STAMP;
		break;
	case ENUM_CLASS(BossStateID::Atk_HammerSlam_End):
		m_pAnimator->SetFloat("Distance", Get_DistanceToPlayer());
		break;
	case ENUM_CLASS(BossStateID::Atk_FuryHammerSlam_Start):
		if (m_iPrevNodeID != m_iCurNodeID)
		{
			m_pAnimator->SetPlayRate(1.3f);
			SwitchFury(true, 1.f);
		}
		m_eAttackType = EAttackType::FURY_STAMP;
		break;
	case ENUM_CLASS(BossStateID::Atk_FuryHammerSlam_End):
		if (m_iPrevNodeID != m_iCurNodeID)
		{
			m_pAnimator->SetPlayRate(1.f);
			SwitchFury(true, 1.f);
		}
		break;
	case ENUM_CLASS(BossStateID::Atk_Strike_Start):
	case ENUM_CLASS(BossStateID::Atk_Strike_Loop):
	case ENUM_CLASS(BossStateID::Atk_Strike_End):
	case ENUM_CLASS(BossStateID::Atk_Jump_Start):
	case ENUM_CLASS(BossStateID::Atk_Jump_Loop):
	case ENUM_CLASS(BossStateID::Atk_Jump_End):
		m_bRootMotionClamped = true;
		break;
	default:
		m_eCurrentState = EEliteState::ATTACK;
		break;
	}
	ApplyAttackTypeToPlayer(m_eAttackType);
	if (m_ePrevState == EEliteState::FATAL && m_eCurrentState != EEliteState::FATAL)
	{
		m_fMaxRootMotionSpeed = 18.f;
		m_fRootMotionAddtiveScale = 1.2f;
	}
}

void CFestivalLeader::UpdateSpecificBehavior(_float fTimeDelta)
{
	if (m_eCurrentState == EEliteState::DEAD)
		return;

	if (m_eCurrentState == EEliteState::ATTACK)
	{
		m_pAnimator->SetBool("Move", false);
	}

	if ((m_eCurrentState == EEliteState::RUN || m_eCurrentState == EEliteState::WALK)
		&& m_eCurrentState != EEliteState::ATTACK
		&& m_eCurrentState != EEliteState::TURN)  // Turn 상태 제외
	{
		m_pTransformCom->LookAtWithOutY(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION));
	}

}

void CFestivalLeader::EnableColliders(_bool bEnable)
{
	__super::EnableColliders(bEnable);
	if (bEnable)
	{
		m_pPhysXActorComForHammer->Set_SimulationFilterData(m_pPhysXActorComForHammer->Get_FilterData());
		m_pPhysXActorComForBasket->Set_SimulationFilterData(m_pPhysXActorComForBasket->Get_FilterData());
		m_pPhysXActorComForLeftHand->Set_SimulationFilterData(m_pPhysXActorComForLeftHand->Get_FilterData());
		m_pPhysXActorComForRightHand->Set_SimulationFilterData(m_pPhysXActorComForRightHand->Get_FilterData());
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			if (auto pController = pPlayer->Get_Controller())
			{
				pController->Remove_IgnoreActors(m_pPhysXActorComForHammer->Get_Actor());
				pController->Remove_IgnoreActors(m_pPhysXActorComForBasket->Get_Actor());
				pController->Remove_IgnoreActors(m_pPhysXActorComForLeftHand->Get_Actor());
				pController->Remove_IgnoreActors(m_pPhysXActorComForRightHand->Get_Actor());
			}
		}
	}
	else
	{
		m_pPhysXActorComForHammer->Init_SimulationFilterData();
		m_pPhysXActorComForBasket->Init_SimulationFilterData();
		m_pPhysXActorComForLeftHand->Init_SimulationFilterData();
		m_pPhysXActorComForRightHand->Init_SimulationFilterData();
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			if (auto pController = pPlayer->Get_Controller())
			{
				pController->Add_IngoreActors(m_pPhysXActorComForHammer->Get_Actor());
				pController->Add_IngoreActors(m_pPhysXActorComForBasket->Get_Actor());
				pController->Add_IngoreActors(m_pPhysXActorComForLeftHand->Get_Actor());
				pController->Add_IngoreActors(m_pPhysXActorComForRightHand->Get_Actor());
			}
		}
	}
}

_bool CFestivalLeader::CanProcessTurn()
{
	// 2페이즈이고 중앙으로 턴을 기다리는 중이면 false
	return !(m_bIsPhase2 && m_bWaitPhase2);
}

void CFestivalLeader::ApplyHeadSpaceSwitch(_float fTimeDelta)
{
	// 거꾸로 뒤집혀 있어서 x축으로 90도 회적
	_matrix rotExtra = XMMatrixRotationX(XMConvertToRadians(90.f));

	// 로컬 = 추가회전과 저장해둔 부착 오프셋 회전 후에 위치
	_matrix L = rotExtra * XMLoadFloat4x4(&m_StoredHeadLocalMatrix);

	m_pHammerBone->Set_TransformationMatrix(L);

	// 뼈 정보 반영
	m_pModelCom->Update_Bones();
}


void CFestivalLeader::SetupAttackByType(_int iPattern)
{
	_bool bIsCombo = (GetRandomFloat(0.f, 1.f) < 0.8f);
	switch (iPattern)
	{
	case Client::CFestivalLeader::Slam:
	{
	
		m_pAnimator->SetBool("IsCombo", bIsCombo);
		if (m_bIsPhase2)
		{
			m_pAnimator->SetBool("IsCombo", false);
		}
		m_eAttackType = EAttackType::NORMAL;
	}
	break;
	case Client::CFestivalLeader::CrossSlam:
	{
		m_pAnimator->SetBool("IsCombo", bIsCombo);
		m_eAttackType = EAttackType::NORMAL;
	}
	break;
	case Client::CFestivalLeader::JumpAttack:
		m_eAttackType = EAttackType::FURY_STAMP;
		break;
	case Client::CFestivalLeader::Strike:
	{
		_int iStrikeCombo = (GetRandomFloat(0.f, 1.f) < 0.8f);
		m_pAnimator->SetInt("IsCombo", bIsCombo);
		m_pAnimator->SetInt("StrikeCombo", iStrikeCombo);
		m_eAttackType = EAttackType::AIRBORNE;
	}
		break;
	case Client::CFestivalLeader::AlternateSmash:
	{
		_int iSmashCount = GetRandomInt(0, 2);
		m_pAnimator->SetInt("SmashCount", iSmashCount);
		m_pAnimator->SetInt("IsCombo", bIsCombo);
		m_eAttackType = EAttackType::STAMP;
	}
		break;
	case Client::CFestivalLeader::Spin:
		m_eAttackType = EAttackType::NORMAL;
		break;
	case Client::CFestivalLeader::HalfSpin:
		m_pAnimator->SetBool("IsCombo", bIsCombo);
		m_eAttackType = EAttackType::FURY_AIRBORNE;
		break;
	case Client::CFestivalLeader::HammerSlam:
	{

	}
	break;
	case Client::CFestivalLeader::DashSwing:
		m_eAttackType = EAttackType::KNOCKBACK;
		break;
	case Client::CFestivalLeader::Swing:
	{
		m_pAnimator->SetInt("IsCombo", bIsCombo);
		m_eAttackType = EAttackType::KNOCKBACK;
	}
	break;
	case Client::CFestivalLeader::FuryHammerSlam:
		m_eAttackType = EAttackType::FURY_STAMP;
		break;
	case Client::CFestivalLeader::FurySwing:
		m_eAttackType = EAttackType::STRONG_KNOCKBACK;
		m_pAnimator->SetBool("IsCombo", bIsCombo);
		break;
	case Client::CFestivalLeader::FuryBodySlam:
		m_eAttackType = EAttackType::FURY_STAMP;
		break;
	default:
		break;
	}
	if (iPattern == Client::CFestivalLeader::Strike
		||iPattern ==Client::CFestivalLeader::JumpAttack )
	{
		m_bRootMotionClamped = true;
	}
	else
	{
		m_bRootMotionClamped = false;
	}
	static_cast<CPlayer*>(m_pPlayer)->SetHitedAttackType(m_eAttackType);
}

void CFestivalLeader::Register_Events()
{
	if (nullptr == m_pAnimator)
		return;

	CEliteUnit::Register_Events();


	m_pAnimator->RegisterEventListener("Turnning", [this]()
		{
			SetTurnTimeDuringAttack(0.7f,1.15f);
		});

	m_pAnimator->RegisterEventListener("ActiveHpBar", [this]()
		{
			if (m_pHPBar)
				return;

			CUI_MonsterHP_Bar::HPBAR_DESC eDesc{};
			eDesc.strName = TEXT("축제 인도자");
			eDesc.isBoss = true;
			eDesc.pHP = &m_fHp;
			eDesc.pIsGroggy = &m_bGroggyActive;

			m_pHPBar = static_cast<CUI_MonsterHP_Bar*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT,
				ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_HPBar"), &eDesc));

		});

	m_pAnimator->RegisterEventListener("CollidersOff", [this]() {
		EnableColliders(false);
		});
	m_pAnimator->RegisterEventListener("CollidersOn", [this]() {
		EnableColliders(true);
		});

	m_pAnimator->RegisterEventListener("ColliderHammerOn", [this]()
		{
			m_pPhysXActorComForHammer->Set_SimulationFilterData(m_pPhysXActorComForHammer->Get_FilterData());
		});
	m_pAnimator->RegisterEventListener("ColliderHammerOff", [this]()
		{
			m_pPhysXActorComForHammer->Init_SimulationFilterData();
		});
	m_pAnimator->RegisterEventListener("ColliderBaskettOn", [this]()
		{
			m_pPhysXActorComForBasket->Set_SimulationFilterData(m_pPhysXActorComForBasket->Get_FilterData());
		});
	m_pAnimator->RegisterEventListener("ColliderBasketOff", [this]()
		{
			m_pPhysXActorComForBasket->Init_SimulationFilterData();
		});

	m_pAnimator->RegisterEventListener("ColliderRightHandOn", [this]()
		{
			m_pPhysXActorComForRightHand->Set_SimulationFilterData(m_pPhysXActorComForRightHand->Get_FilterData());
		});
	m_pAnimator->RegisterEventListener("ColliderRightHandOff", [this]()
		{
			m_pPhysXActorComForRightHand->Init_SimulationFilterData();
		});
	m_pAnimator->RegisterEventListener("ColliderLeftHandOn", [this]()
		{
			m_pPhysXActorComForLeftHand->Set_SimulationFilterData(m_pPhysXActorComForLeftHand->Get_FilterData());
		});
	m_pAnimator->RegisterEventListener("ColliderLeftHandOff", [this]()
		{
			m_pPhysXActorComForLeftHand->Init_SimulationFilterData();
		});


	m_pAnimator->RegisterEventListener("IgnorePlayerCollision", [this]()
		{
			if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
			{
				if (auto pController = pPlayer->Get_Controller())
				{
					pController->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
				}
			}
		});

	m_pAnimator->RegisterEventListener("BeginStrikeCollisionFlag", [this]()
		{
			if (m_pPhysXActorCom)
			{
				m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
			}
		});

	m_pAnimator->RegisterEventListener("EndStrikeCollisionFlag", [this]()
		{
			if (m_pPhysXActorCom)
			{
				m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::MONSTER);
			}
		});

	m_pAnimator->RegisterEventListener("OnSlamEffect", [this]()
		{
			//EffectSpawn_Active(SlamAtk, true);

		});

	m_pAnimator->RegisterEventListener("SetRootStep", [this]()
		{
			m_fRootMotionAddtiveScale = 7.f;
			m_fMaxRootMotionSpeed = 30.f;
		});

	m_pAnimator->RegisterEventListener("ResetRootStep", [this]()
		{
			m_fMaxRootMotionSpeed = 18.f;
			m_fRootMotionAddtiveScale = 1.2f;
		});


	m_pAnimator->RegisterEventListener("Phase2InvisibledModel", [this]()
		{
			if (m_pModelCom)
			{
				if (m_pModelCom->IsMeshVisible(2))
				{
					m_pModelCom->SetMeshVisible(2, false);
				}

				if (m_pModelCom->IsMeshVisible(3))
				{
					m_pModelCom->SetMeshVisible(3, false);
				}
			}
			if (m_pAnimator)
			{
				m_pAnimator->ApplyOverrideAnimController("Phase2");
				m_pAnimator->SetInt("SkillType", DashSwing);
				m_pAnimator->SetTrigger("Attack");
				m_ePrevState = m_eCurrentState;
				m_eCurrentState = EEliteState::ATTACK;
			}

			if (m_pHammerBone)
			{
				if(m_iOriginBoneIndex == -1)
					m_iOriginBoneIndex = m_pHammerBone->Get_ParentBoneIndex();

				m_iNewParentIndex = m_pModelCom->Find_BoneIndex("BN_Weapon_R");
				if (m_iNewParentIndex >= 0)
				{
					m_pHammerBone->Set_ParentBoneIndex(m_iNewParentIndex);
					m_bSwitchHeadSpace = true;
				}
			}
		});


	m_pAnimator->RegisterEventListener("RestoreHeadBoneIndex", [this]()
		{
			if (!m_pModelCom || !m_pHammerBone || m_iOriginBoneIndex < 0)
				return;
			// 부모 교체
			m_pHammerBone->Set_ParentBoneIndex(m_iOriginBoneIndex);
			m_pHammerBone->Set_TransformationMatrix(XMLoadFloat4x4(&m_HeadLocalInit));
			m_pModelCom->Update_Bones();
			m_bSwitchHeadSpace = false;
		});
	
	m_pAnimator->RegisterEventListener("EndPhase2Change", [this]()
		{
			if (!m_pModelCom || !m_pHammerBone || m_iOriginBoneIndex < 0 || m_bSwitchHeadSpace == false)
				return;
			m_pHammerBone->Set_ParentBoneIndex(m_iOriginBoneIndex);
			m_bSwitchHeadSpace = false;
		});

}

void CFestivalLeader::Ready_AttackPatternWeightForPhase1()
{
	m_PatternWeightMap.clear();
	m_PatternCountMap.clear();
	vector<EBossAttackPattern> m_vecBossPatterns = {
		Slam,CrossSlam,JumpAttack,Strike,AlternateSmash,FuryBodySlam
	};

	for (const auto& pattern : m_vecBossPatterns)
	{
		m_PatternWeightMap[pattern] = m_fBasePatternWeight;
		if (pattern == AlternateSmash)
			m_PatternWeightMap[pattern] *=1.5f; // 번갈아 공격을 좀 더 많이
		m_PatternCountMap[pattern] = 0;
	}
}

void CFestivalLeader::Ready_AttackPatternWeightForPhase2()
{
	if (m_eCurrentState == EEliteState::FATAL
		|| m_eCurrentState == EEliteState::ATTACK)
		return;
	if (m_bStartPhase2 == false)
	{

	m_pAnimator->SetTrigger("Phase2Start");
	m_pAnimator->SetBool("Phase2Combo", true);
	m_pAnimator->SetPlayRate(1.f);
	m_bIsPhase2 = true;
	static_cast<CPlayer*>(m_pPlayer)->SetHitedAttackType(EAttackType::STRONG_KNOCKBACK); // 바스켓에 충돌했을 때를 생각해서 
	m_bStartPhase2 = true;
	vector<EBossAttackPattern> m_vecBossPatterns = {
		Slam, JumpAttack ,Strike ,Spin ,HalfSpin ,HammerSlam ,
		DashSwing ,Swing,FuryHammerSlam ,FurySwing 
	};
	m_PatternWeightMap.clear();
	m_PatternCountMap.clear();
	for (const auto& pattern : m_vecBossPatterns)
	{
		m_PatternWeightMap[pattern] = m_fBasePatternWeight;
		m_PatternCountMap[pattern] = 0;
	}
	SwitchFury(false, 1.f);
	}
}

_int CFestivalLeader::GetRandomAttackPattern(_float fDistance)
{
	EBossAttackPattern ePattern = BAP_NONE;
	m_PatternWeighForDisttMap = m_PatternWeightMap;
	ChosePatternWeightByDistance(fDistance);


	_float fTotalWeight = accumulate(m_PatternWeighForDisttMap.begin(), m_PatternWeighForDisttMap.end(), 0.f,
		[](_float fAcc, const pair<_int, _float>& Pair) { return fAcc + Pair.second; });

	_float fRandomVal = GetRandomFloat(0.f, fTotalWeight);
	_float fCurWeight = 0.f;
	for (const auto& [pattern, weight] : m_PatternWeighForDisttMap)
	{
		if (weight <= 0.f)
			continue; // 가중치가 0 이하인 패턴은 무시
		fCurWeight += weight;
		if (fRandomVal <= fCurWeight)
		{
			ePattern = static_cast<EBossAttackPattern>(pattern);
			m_ePrevAttackPattern = m_eCurAttackPattern;
			m_eCurAttackPattern = ePattern;
			UpdatePatternWeight(ePattern);
			break;
		}
	}
	return ePattern;
}

void CFestivalLeader::ChosePatternWeightByDistance(_float fDistance)
{
	if (fDistance >= ATTACK_DISTANCE_CLOSE && fDistance < ATTACK_DISTANCE_MIDDLE)
	{

		for (auto& [pattern, weight] : m_PatternWeighForDisttMap)
		{
			auto it = find(m_vecCloseAttackPatterns.begin(), m_vecCloseAttackPatterns.end(), pattern);
			if (it == m_vecCloseAttackPatterns.end())
			{
				m_PatternWeighForDisttMap[pattern]  *= 0.05f;

			}
		}

	}
	else if (fDistance >= ATTACK_DISTANCE_MIDDLE)
	{
		for (auto& [pattern, weight] : m_PatternWeighForDisttMap)
		{
			auto it = find(m_vecMiddleAttackPatterns.begin(), m_vecMiddleAttackPatterns.end(), pattern);
			if (it == m_vecMiddleAttackPatterns.end())
			{
				m_PatternWeighForDisttMap[pattern] *= 0.05f;
			}
		}
	}

	// 2페이즈 패턴 많이 보여주려고 가중치 올림
	if (m_bIsPhase2)
	{
		for (auto& [pattern, weight] : m_PatternWeighForDisttMap)
		{
			if (pattern == FuryHammerSlam || pattern == FurySwing ||
				pattern == FuryBodySlam || pattern == HammerSlam)
			{
				weight *= m_fWeightIncreaseRate; // 2페이즈 패턴은 확률 높임
			}
		}
	}
}


void CFestivalLeader::Ready_EffectNames()
{

	// Phase 1

	// Phase 2

}


void CFestivalLeader::ProcessingEffects(const _wstring& stEffectTag)
{
	if (m_pHammerBone == nullptr || m_pBasketBone == nullptr)
		return;

	CEffectContainer::DESC desc = {};

	if (MAKE_EFFECT(ENUM_CLASS(m_iLevelID), stEffectTag, &desc) == nullptr)
		MSG_BOX("이펙트 생성 실패함");
}

HRESULT CFestivalLeader::EffectSpawn_Active(_int iPattern, _bool bActive, _bool bIsOnce) // 어떤 이펙트를 스폰할지 결정
{
	auto it = m_EffectMap.find(iPattern);
	if (it == m_EffectMap.end())
		return E_FAIL; // 해당 패턴 이펙트 없음

	const vector<_wstring>& effectTags = it->second;

	if (bActive)
	{
		// 여러 개 이펙트를 동시에 등록
		for (const auto& effectTag : effectTags)
		{
			m_ActiveEffect.push_back({ effectTag, bIsOnce });
		}
	}
	else
	{
		for (auto itEff = m_ActiveEffect.begin(); itEff != m_ActiveEffect.end(); )
		{
			// 여러 개니까 전부 비교해서 삭제
			if (find(effectTags.begin(), effectTags.end(), itEff->first) != effectTags.end())
				itEff = m_ActiveEffect.erase(itEff);
			else
				++itEff;
		}
	}


	return S_OK;
}

HRESULT CFestivalLeader::Spawn_Effect() // 이펙트를 스폰 (대신 각각의 로직에 따라서 함수 호출)
{
	if (m_ActiveEffect.empty())
		return S_OK;

	for (auto it = m_ActiveEffect.begin(); it != m_ActiveEffect.end(); )
	{
		const _wstring EffectTag = it->first;
		ProcessingEffects(EffectTag);
		if (it->second) // 한번만 실행이면
		{
			it = m_ActiveEffect.erase(it);
		}
		else
		{
			++it;
		}
	}
	return S_OK;
}

HRESULT CFestivalLeader::Ready_Effect()
{

	return S_OK;
}

void CFestivalLeader::Ready_SoundEvents()
{

}

void CFestivalLeader::UpdatePatternWeight(_int iPattern)
{
	m_PatternCountMap[iPattern]++;
	if (m_PatternCountMap[iPattern] >= m_iPatternLimit)
	{
		m_PatternWeightMap[iPattern] *= (1.f - m_fWeightDecreaseRate); // 가중치 감소
		m_PatternWeightMap[iPattern] = max(m_PatternWeightMap[iPattern], m_fMinWeight); // 최소 가중치로 설정
		m_PatternCountMap[iPattern] = 0;

		for (auto& [pattern, weight] : m_PatternWeightMap)
		{
			if (pattern != iPattern)
			{
				weight += (m_fMaxWeight - weight) * m_fWeightIncreaseRate; // 가중치 증가
				weight = min(weight, m_fMaxWeight); // 최대 가중치로 제한
			}
		}
	}
}


void CFestivalLeader::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	
}

void CFestivalLeader::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{

	__super::On_CollisionStay(pOther, eColliderType, HitPos, HitNormal);

}

void CFestivalLeader::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CFestivalLeader::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{

}

void CFestivalLeader::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	ReceiveDamage(pOther, eColliderType);

	if (auto pPlayer = dynamic_cast<CPlayer*>(pOther))
	{

		_uint curNodeID = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->iNodeId;
		/*	switch (curNodeID)
			{

			default:
				break;
			}*/

	}

}

void CFestivalLeader::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}


CFestivalLeader* CFestivalLeader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFestivalLeader* pInstance = new CFestivalLeader(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CFestivalLeader");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CFestivalLeader::Clone(void* pArg)
{
	CFestivalLeader* pInstance = new CFestivalLeader(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CFestivalLeader");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CFestivalLeader::Free()
{
	__super::Free();
	Safe_Release(m_pSpringBoneSys);
	Safe_Release(m_pPhysXActorComForHammer);
	Safe_Release(m_pPhysXActorComForBasket);
	Safe_Release(m_pPhysXActorComForRightHand);
	Safe_Release(m_pPhysXActorComForLeftHand);
}
