#include "FestivalLeader.h"

#include "Bone.h"
#include "Player.h"
#include "Static_Decal.h"
#include "AnimatedProp.h"
#include "GameInstance.h"
#include "SpringBoneSys.h"
#include "LockOn_Manager.h"
#include "UI_MonsterHP_Bar.h"
#include "Client_Calculation.h"
#include "PhysX_IgnoreSelfCallback.h"

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
	m_bIsDissolve = true;
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
		m_HeadLocalInit = m_BoneRefs[EBossBones::Hammer]->Get_TransformationMatrix_Float4x4();
		m_pAnimator->PlayClip(m_pModelCom->GetAnimationClipByName("AS_Idle_C_1"), false);
		m_pAnimator->Update(0.016f);
		m_pModelCom->Update_Bones();

		CSpringBoneSys::SpringInitParams springParams;
		springParams.restDirBiasEnable = true;
		m_pSpringBoneSys = CSpringBoneSys::Create(m_pModelCom, vector<string>{"Hair", "Head_Rope", "Coat"}, springParams);
		if (m_pSpringBoneSys == nullptr)
			return E_FAIL;
		if (m_BoneRefs[EBossBones::Hammer])
		{
			_matrix cmHead = XMLoadFloat4x4(m_BoneRefs[EBossBones::Hammer]->Get_CombinedTransformationMatrix()); // 머리 컴바인드
			_matrix cmHand = XMLoadFloat4x4(m_pRightWeaponBone->Get_CombinedTransformationMatrix()); // 오른손 무기 소켓 컴바인드

			//  손 기준 로컬 오프셋
			_matrix attachOffset = cmHead * XMMatrixInverse(nullptr, cmHand);
			XMStoreFloat4x4(&m_StoredHeadLocalMatrix, attachOffset);
		}
		m_pAnimator->Get_CurrentAnimController()->SetStateToEntry();

	}
	// 0번 메시는 다리,1번은 몸통, 4번 양팔, 5번 머리
	// 2,3번은 바스켓

	m_fMaxHp = 1300.f;
	m_fHp = m_fMaxHp;
	m_fDamage = 15.f;
	m_fAttckDleay = 1.5f;
	m_iPatternLimit = 1;
	m_fChasingDistance = 3.5f;
	m_fMinimumTurnAngle = 45.f;
	m_fPhase2HPThreshold = 0.65f;
	m_fMaxRootMotionSpeed = 18.f;
	m_fChangeMoveDirCooldown = 2.5f;
	m_fRootMotionClampDist = 4.5f;

	m_fGroggyScale_Weak = 0.07f;
	m_fGroggyScale_Strong = 0.09f;
	m_fGroggyScale_Charge = 0.12f;
	m_eCurrentState = EEliteState::CUTSCENE;
	m_ePrevState = EEliteState::CUTSCENE;
	return S_OK;
}

void CFestivalLeader::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_bDead)
		m_pHPBar->Set_bDead();
	if (KEY_DOWN(DIK_B))
	{
		EnterCutScene();
	}

	if (KEY_DOWN(DIK_V))
	{
		BreakPanel();
	}
	if (KEY_DOWN(DIK_C))
	{
		EnterNextCutScene();
	}

	if (KEY_DOWN(DIK_I))
	{
		m_fHp -= 500.f;
		//	ReChallenge();
	}
#ifdef _DEBUG


	

	if (KEY_PRESSING(DIK_LALT))
	{
		if (KEY_DOWN(DIK_MINUS))
		{
			m_fTimeScale = 0.5f;
		}
		if (KEY_DOWN(DIK_EQUALS))
		{
			m_fTimeScale = 1.f;
		}
		if (KEY_DOWN(DIK_Q))
		{

			// 오른손 왼손 회전 상태 뭔가 이상한 것 같은데 나중에 확인함
			CEffectContainer::DESC desc = {};

			desc.pSocketMatrix = nullptr;
			desc.pParentMatrix = nullptr;
			const _float4x4* socketPtr = m_BoneRefs[LeftHand]->Get_CombinedTransformationMatrix();
			const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
			_matrix socket = XMLoadFloat4x4(socketPtr);
			_matrix parent = XMLoadFloat4x4(parentPtr);

			_matrix comb = socket * parent;

			_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1] + 0.5f);
			_vector pos, rot, scale;
			XMMatrixDecompose(&scale, &rot, &pos, comb);

			rot = XMQuaternionMultiply(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-90.f)), rot);

			XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixRotationQuaternion(rot) * XMMatrixTranslationFromVector(position));
			MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Fes_Scratch"), &desc);

			socketPtr = m_BoneRefs[RightHand]->Get_CombinedTransformationMatrix();
			socket = XMLoadFloat4x4(socketPtr);
			comb = socket * parent;
			position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1] + 0.5f);

			XMMatrixDecompose(&scale, &rot, &pos, comb);

			rot = XMQuaternionMultiply(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-90.f)), rot);
			XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixRotationQuaternion(rot) * XMMatrixTranslationFromVector(position));

			CGameObject* pEC = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Fes_Scratch"), &desc);
			if (pEC == nullptr)
				MSG_BOX("이펙트 생성 실패함");
		}
		if (KEY_DOWN(DIK_W))
		{
			CEffectContainer::DESC desc = {};
			desc.pSocketMatrix = nullptr;
			desc.pParentMatrix = nullptr;
			const _float4x4* socketPtr = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-Spine1"));
			const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
			_matrix socket = XMLoadFloat4x4(socketPtr);
			_matrix parent = XMLoadFloat4x4(parentPtr);

			_matrix comb = socket * parent;

			//_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);

			XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(comb.r[3]));
			CGameObject* pEC = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_NormalExplosions3"), &desc);
			if (pEC == nullptr)
				MSG_BOX("이펙트 생성 실패함");
		}
		if (KEY_DOWN(DIK_E))
		{
			CEffectContainer::DESC desc = {};
			desc.pSocketMatrix = nullptr;
			desc.pParentMatrix = nullptr;
			const _float4x4* socketPtr = m_BoneRefs[HeadJaw]->Get_CombinedTransformationMatrix();
			const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
			_matrix socket = XMLoadFloat4x4(socketPtr);
			_matrix parent = XMLoadFloat4x4(parentPtr);

			_matrix comb = socket * parent;

			_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);

			XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(comb.r[3]));
			CGameObject* pEC = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Fes_P2Start"), &desc);
			if (pEC == nullptr)
				MSG_BOX("이펙트 생성 실패함");
		}
	}
#endif

	if (nullptr != m_pHPBar)
		m_pHPBar->Priority_Update(fTimeDelta);
}

void CFestivalLeader::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_bSwitchHeadSpace)
	{
		ApplyHeadSpaceSwitch(fTimeDelta);
	}
	if (nullptr != m_pHPBar)
		m_pHPBar->Update(fTimeDelta);

	if (m_pPlayer && static_cast<CUnit*>(m_pPlayer)->GetHP() <= 0 && m_pHPBar)
		m_pHPBar->Set_RenderTime(0.f);
	Update_Collider();

	if (KEY_DOWN(DIK_L))
	{
		Spawn_Decal(m_pRightWeaponBone,
			TEXT("Prototype_Component_Texture_FireEater_Slam_Normal"),
			TEXT("Prototype_Component_Texture_FireEater_Slam_Mask"),
			XMVectorSet(5.f, 0.5f, 5.f, 0));
	}

}

void CFestivalLeader::Late_Update(_float fTimeDelta)
{

	if (!m_bDissolveSwitch && m_fDissolve <= 0.003f)
	{
		if (m_pModelCom->IsMeshVisible(2))
			m_pModelCom->SetMeshVisible(2, false);
		if (m_pModelCom->IsMeshVisible(3))
			m_pModelCom->SetMeshVisible(3, false);
	}

	__super::Late_Update(fTimeDelta);

	if (nullptr != m_pHPBar)
		m_pHPBar->Late_Update(fTimeDelta);
#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider())
	{
		for (const auto& pPhysX : m_Colliders)
		{
			if (pPhysX->Get_ReadyForDebugDraw())
				m_pGameInstance->Add_DebugComponent(pPhysX);
		}
	}
#endif

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
	m_pAnimator->CancelOverrideAnimController();
	m_pModelCom->SetMeshVisible(2, true);
	m_pModelCom->SetMeshVisible(3, true);
	m_pModelCom->SetMeshVisible(5, true);

	m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->clip->SetCurrentTrackPosition(120.f);
	m_bSwitchHeadSpace = false;

	m_pModelCom->Update_Bones();                      // 뼈 재계산
	Update_Collider();                                // 콜라이더도 같은 프레임에 동기화

}

HRESULT CFestivalLeader::Ready_Components(void* pArg)
{
	if (FAILED(__super::Ready_Components(pArg)))
		return E_FAIL;

	for (_int i = 0; i < static_cast<_int>(m_Colliders.size()); i++)
	{
		auto& pPhysXCom = m_Colliders[i];
		_wstring comName = TEXT("Com_PhysX") + to_wstring(i + 2);
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), comName, reinterpret_cast<CComponent**>(&pPhysXCom))))
			return E_FAIL;
	}

	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_FestivalLeader"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CFestivalLeader::Ready_Actor()
{
	if (FAILED(__super::Ready_Actor()))
		return E_FAIL;


	PxFilterData Filter{};
	Filter.word0 = WORLDFILTER::FILTER_MONSTERWEAPON;
	Filter.word1 = WORLDFILTER::FILTER_PLAYERBODY;

	struct ColliderSetup
	{
		EBossBones type;
		PxGeometryHolder geom;
		_vector offset = XMVectorZero();
	};

	vector<ColliderSetup> setupList = {
		{EBossBones::Hammer,      PxBoxGeometry(2.2f, 0.9f, 0.9f), XMVectorSet(-2.5f, 0.f, 0.f, 1.f)},
		{EBossBones::Basket,      PxSphereGeometry(1.0f),          XMVectorZero()},
		{EBossBones::LeftHand,    PxSphereGeometry(1.1f),          XMVectorZero()},
		{EBossBones::RightHand,   PxSphereGeometry(0.96f),          XMVectorZero()},
		{EBossBones::LeftForearm, PxSphereGeometry(1.4f),          XMVectorZero()},
		{EBossBones::RightForearm,PxSphereGeometry(0.96f),          XMVectorZero()}
	};

	for (auto& setup : setupList)
	{
		auto idx = static_cast<_int>(setup.type);
		if (!m_Colliders[idx] || !m_BoneRefs[idx])
			continue;

		_matrix matOffset = XMMatrixTranslationFromVector(setup.offset);
		const PxTransform pose = XMVectorGetX(setup.offset) != 0.f ?
			GetBonePose(m_BoneRefs[idx], &matOffset) :
			GetBonePose(m_BoneRefs[idx]);

		m_Colliders[idx]->Create_Collision(
			m_pGameInstance->GetPhysics(),
			setup.geom.any(),
			pose,
			m_pGameInstance->GetMaterial(L"Default"));

		m_Colliders[idx]->Set_ShapeFlag(false, true, true);
		m_Colliders[idx]->Set_SimulationFilterData(Filter);
		m_Colliders[idx]->Set_QueryFilterData(Filter);
		m_Colliders[idx]->Set_Owner(this);
		m_Colliders[idx]->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
		m_Colliders[idx]->Set_Kinematic(true);

		m_pGameInstance->Get_Scene()->addActor(*m_Colliders[idx]->Get_Actor());
		m_pPhysXActorCom->Add_IngoreActors(m_Colliders[idx]->Get_Actor());
	}

	m_pPhysXActorCom->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
	return S_OK;
}

void CFestivalLeader::Ready_BoneInformation()
{
	m_iLockonBoneIndex = m_pModelCom->Find_BoneIndex("Bip001-Spine1");

	auto findBone = [&](const _char* name, EBossBones type)
		{
			auto it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
				[&](CBone* pBone) { return !strcmp(pBone->Get_Name(), name); });
			if (it != m_pModelCom->Get_Bones().end())
				m_BoneRefs[static_cast<_int>(type)] = *it;
		};

	findBone("Bn_Head_Weapon", EBossBones::Hammer);
	findBone("Bn_Basket_Body", EBossBones::Basket);
	findBone("Ref_Bip001-L-Hand", EBossBones::LeftHand);
	findBone("Bip001-R-Hand", EBossBones::RightHand);
	findBone("Ref_Bip001-L-Forearm", EBossBones::LeftForearm);
	findBone("Skin_Bip001-R-Forearm", EBossBones::RightForearm);
	findBone("Bip001-L-Clavicle", EBossBones::LeftShoulder);
	findBone("Bip001-R-Clavicle", EBossBones::RightShoulder);
	findBone("Bip001-L-Calf", EBossBones::LeftKnee);
	findBone("Bip001-R-Calf", EBossBones::RightKnee);
	findBone("Bip001-L-Finger2Nub", EBossBones::LeftMiddleFinger);
	findBone("Bip001-R-Finger2Nub", EBossBones::RightMiddleFinger);
	findBone("Ref_Bip001-L-Finger21", EBossBones::LeftMiddleFingerStart);
	findBone("Skin_Bip001-R-Finger21", EBossBones::RightMiddleFingerStart);
	findBone("Bn_Head_Jaw_02", EBossBones::HeadJaw);
	findBone("Bip001-Neck", EBossBones::Neck);

	auto it = find_if(m_pModelCom->Get_Bones().begin(), m_pModelCom->Get_Bones().end(),
		[](CBone* pBone) { return !strcmp(pBone->Get_Name(), "BN_Weapon_R"); });
	if (it != m_pModelCom->Get_Bones().end())
		m_pRightWeaponBone = *it;
}

void CFestivalLeader::Update_Collider()
{
	__super::Update_Collider();
	// static 변수로 오프셋 값을 선언하여 키 입력에 따라 값이 유지되도록 함
	static _float fOffsetX = 0.f;
	static _float fOffsetY = 0.f;
	static _float fOffsetZ = 0.f;

	if (KEY_DOWN(DIK_W)) { // W 키: Z축 증가 (Up)
		fOffsetZ += 0.1f;
	}
	if (KEY_DOWN(DIK_S)) { // S 키: Z축 감소 (Down)
		fOffsetZ -= 0.1f;
	}
	if (KEY_DOWN(DIK_A)) { // A 키: X축 감소 (Left)
		fOffsetX -= 0.1f;
	}
	if (KEY_DOWN(DIK_D)) { // D 키: X축 증가 (Right)
		fOffsetX += 0.1f;
	}
	if (KEY_DOWN(DIK_E)) { // E 키: Y축 증가 (Page Up)
		fOffsetY += 0.1f;
	}
	if (KEY_DOWN(DIK_Q)) { // Q 키: Y축 감소 (Page Down)
		fOffsetY -= 0.1f;
	}

	for (_int i = 0; i < EBossBones::Collider_Count; i++)
	{
		if (!m_Colliders[i] || !m_BoneRefs[i]) continue;

		if (i == EBossBones::Hammer)
		{
			//_vector vWorldOffset = XMVectorSet(fOffsetX, fOffsetY, fOffsetZ, 1.f);
			_vector vWorldOffset = XMVectorSet(1.5f, 0.f, 1.49011612e-08f, 1.f);
			_matrix matWorldOffset = XMMatrixTranslationFromVector(vWorldOffset);
			m_Colliders[i]->Set_Transform(GetBonePose(m_BoneRefs[i], &matWorldOffset));
		}
		else
		{
			m_Colliders[i]->Set_Transform(GetBonePose(m_BoneRefs[i]));
		}
	}
}

void CFestivalLeader::UpdateAttackPattern(_float fDistance, _float fTimeDelta)
{
	if (m_eCurrentState == EEliteState::DEAD || m_bDead
		|| m_eCurrentState == EEliteState::CUTSCENE
		|| m_eCurrentState == EEliteState::GROGGY || 
		 m_eCurrentState == EEliteState::PARALYZATION ||
		m_eCurrentState == EEliteState::FATAL)
		return;
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
		{
			pPlayer->SetHitedAttackType(EAttackType::AIRBORNE);
			pPlayer->SetHitMotion(HITMOTION::UP);
		}
		m_bRootMotionClamped = true;
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
		m_pTransformCom->Set_SpeedPerSec(m_fWalkSpeed);
		m_eCurrentState = EEliteState::WALK;
	}
	break;
	case ENUM_CLASS(BossStateID::Run_F):
		m_pTransformCom->Set_SpeedPerSec(m_fRunSpeed);
		m_eCurrentState = EEliteState::RUN;
		break;
	case ENUM_CLASS(BossStateID::Groggy_Start):
	case ENUM_CLASS(BossStateID::Groggy_Loop):
		m_eCurrentState = EEliteState::GROGGY;
		break;
	case ENUM_CLASS(BossStateID::Groggy_End):
		m_bHeadSpark = false;
		m_eCurrentState = EEliteState::GROGGY;
		break;
	case ENUM_CLASS(BossStateID::Special_Die):
		m_eCurrentState = EEliteState::DEAD;
		EFFECT_MANAGER->Set_Active_Effect(TEXT("Fes_P2_HeadSmoke_L"), false);
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
		if (m_iPrevNodeID != m_iCurNodeID)
		{
			if (m_iSwingComboCount == 0 && m_bInSwingCombo == false) // 이전 콤보가 끝난 상태
			{
				m_iSwingComboLimit = GetRandomInt(0, 1); 
				m_bInSwingCombo = true;                  
			}
		}
		break;

	case ENUM_CLASS(BossStateID::Atk_Swing_End):
	case ENUM_CLASS(BossStateID::Atk_DashSwing_End):
		if (m_iPrevNodeID != m_iCurNodeID)
		{
			m_iSwingComboCount++;
			if (m_iSwingComboCount >= m_iSwingComboLimit)
			{
				m_pAnimator->SetBool("IsCombo", false);
				m_iSwingComboCount = 0;
				m_bInSwingCombo = false;
			}
		}
		break;
	case ENUM_CLASS(BossStateID::Atk_HalfSpin_Start):
		break;
	case ENUM_CLASS(BossStateID::Atk_FuryBodySlam_Start):
		m_eAttackType = EAttackType::FURY_STAMP;
		break;
	case ENUM_CLASS(BossStateID::Atk_FurySwing_Start):
		m_eAttackType = EAttackType::KNOCKBACK;
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


	if (m_eCurrentState == EEliteState::GROGGY || m_eCurrentState == EEliteState::FATAL
		|| m_eCurrentState == EEliteState::DEAD)
	{
		EffectSpawn_Active(EF_GROUND_SPARK, false);
	}
}

void CFestivalLeader::EnableColliders(_bool bEnable)
{
	__super::EnableColliders(bEnable);

	for (auto* pCol : m_Colliders)
	{
		if (!pCol) continue;
		if (bEnable)
			pCol->Set_SimulationFilterData(pCol->Get_FilterData());
		else
			pCol->Init_SimulationFilterData();
	}

	if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
	{
		if (auto pController = pPlayer->Get_Controller())
		{
			for (auto* pCol : m_Colliders)
			{
				if (!pCol)
					continue;
				if (bEnable)
					pController->Remove_IgnoreActors(pCol->Get_Actor());
				else
					pController->Add_IngoreActors(pCol->Get_Actor());
			}
		}
	}
}

_bool CFestivalLeader::CanProcessTurn()
{
	// 2페이즈
	return !(m_bIsPhase2 && m_bWaitPhase2);
}

void CFestivalLeader::ApplyHeadSpaceSwitch(_float fTimeDelta)
{
	// 거꾸로 뒤집혀 있어서 x축으로 90도 회전
	_matrix rotExtra = XMMatrixRotationX(XMConvertToRadians(90.f));

	// 로컬 = 추가회전과 저장해둔 부착 오프셋 회전 후에 위치
	_matrix L = rotExtra * XMLoadFloat4x4(&m_StoredHeadLocalMatrix);

	m_BoneRefs[EBossBones::Hammer]->Set_TransformationMatrix(L);

	// 뼈 정보 반영
	m_pModelCom->Update_Bones();
}


void CFestivalLeader::SetupAttackByType(_int iPattern)
{
	_bool bIsCombo = static_cast<_int>((GetRandomFloat(0.f, 1.f) < 0.85f));
	switch (iPattern)
	{
	case Client::CFestivalLeader::Slam:
	{

		m_pAnimator->SetBool("IsCombo", bIsCombo);
		if (m_bIsPhase2)
		{
			m_pAnimator->SetBool("IsCombo", false);
		}
		//m_eAttackType = EAttackType::NORMAL;
	}
	break;
	case Client::CFestivalLeader::CrossSlam:
	{
		m_pAnimator->SetBool("IsCombo", bIsCombo);
		//	m_eAttackType = EAttackType::KNOCKBACK;
	}
	break;
	case Client::CFestivalLeader::JumpAttack:
		m_bRootMotionClamped = true;
		break;
	case Client::CFestivalLeader::Strike:
	{
		m_bRootMotionClamped = true;
		_int iStrikeCombo = static_cast<_int>((GetRandomFloat(0.f, 1.f) < 0.85f));
		m_pAnimator->SetInt("IsCombo", bIsCombo);
		m_pAnimator->SetInt("StrikeCombo", iStrikeCombo);
		//	m_eAttackType = EAttackType::AIRBORNE;
	}
	break;
	case Client::CFestivalLeader::AlternateSmash:
	{
		_int iSmashCount = GetRandomInt(0, 2);
		if (m_fCanSmashDistance >= Get_DistanceToPlayer())
			m_pAnimator->SetInt("SmashCount", iSmashCount);
		m_pAnimator->SetInt("IsCombo", bIsCombo);
		//	m_eAttackType = EAttackType::STAMP;
	}
	break;
	case Client::CFestivalLeader::Spin:
		//	m_eAttackType = EAttackType::NORMAL;
		break;
	case Client::CFestivalLeader::HalfSpin:
		m_pAnimator->SetBool("IsCombo", bIsCombo);
		//	m_eAttackType = EAttackType::FURY_AIRBORNE;
		break;
	case Client::CFestivalLeader::HammerSlam:
	{

	}
	break;
	case Client::CFestivalLeader::DashSwing:
		//	m_eAttackType = EAttackType::KNOCKBACK;
		break;
	case Client::CFestivalLeader::Swing:
	{
		m_pAnimator->SetInt("IsCombo", bIsCombo);
		//	m_eAttackType = EAttackType::KNOCKBACK;
	}
	break;
	case Client::CFestivalLeader::FuryHammerSlam:
		//	m_eAttackType = EAttackType::FURY_STAMP;
		break;
	case Client::CFestivalLeader::FurySwing:
		//	m_eAttackType = EAttackType::KNOCKBACK;
		m_pAnimator->SetBool("IsCombo", bIsCombo);
		break;
	case Client::CFestivalLeader::FuryBodySlam:
		//	m_eAttackType = EAttackType::FURY_STAMP;
		break;
	default:
		break;
	}
	if (iPattern == Client::CFestivalLeader::Strike
		|| iPattern == Client::CFestivalLeader::JumpAttack)
	{
		m_bRootMotionClamped = true;
	}
	else
	{
		m_bRootMotionClamped = false;
	}
	//static_cast<CPlayer*>(m_pPlayer)->SetHitedAttackType(m_eAttackType);
}

void CFestivalLeader::Register_Events()
{
	if (nullptr == m_pAnimator)
		return;

	CEliteUnit::Register_Events();


	m_pAnimator->RegisterEventListener("Turnning", [this]()
		{
			SetTurnTimeDuringAttack(1.2f, 1.5f);
		});

	m_pAnimator->RegisterEventListener("ActiveHpBar", [this]()
		{			
			SwitchEmissive(true, 0.9f);
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
		m_bPlayerCollided = false;
		});
	m_pAnimator->RegisterEventListener("CollidersOn", [this]() {
		EnableColliders(true);

		});

	m_pAnimator->RegisterEventListener("ColliderHammerOn", [this]()
		{
			m_Colliders[EBossBones::Hammer]->Set_SimulationFilterData(m_Colliders[EBossBones::Hammer]->Get_FilterData());
		});
	m_pAnimator->RegisterEventListener("ColliderHammerOff", [this]()
		{
			m_Colliders[EBossBones::Hammer]->Init_SimulationFilterData();
			m_bPlayerCollided = false;
		});

	m_pAnimator->RegisterEventListener("ColliderBaskettOn", [this]()
		{
			m_Colliders[EBossBones::Basket]->Set_SimulationFilterData(m_Colliders[EBossBones::Basket]->Get_FilterData());
		});
	m_pAnimator->RegisterEventListener("ColliderBasketOff", [this]()
		{
			m_Colliders[EBossBones::Basket]->Init_SimulationFilterData();	
			m_bPlayerCollided = false;
		});

	m_pAnimator->RegisterEventListener("ColliderRightHandOn", [this]()
		{

			m_Colliders[EBossBones::RightHand]->Set_SimulationFilterData(m_Colliders[EBossBones::RightHand]->Get_FilterData());
			m_Colliders[EBossBones::RightForearm]->Set_SimulationFilterData(m_Colliders[EBossBones::RightForearm]->Get_FilterData());
		});
	m_pAnimator->RegisterEventListener("ColliderRightHandOff", [this]()
		{
			m_Colliders[EBossBones::RightHand]->Init_SimulationFilterData();
			m_Colliders[EBossBones::RightForearm]->Init_SimulationFilterData();
			m_bPlayerCollided = false;

		});
	m_pAnimator->RegisterEventListener("ColliderLeftHandOn", [this]()
		{
			m_Colliders[EBossBones::LeftHand]->Set_SimulationFilterData(m_Colliders[EBossBones::LeftHand]->Get_FilterData());
			m_Colliders[EBossBones::LeftForearm]->Set_SimulationFilterData(m_Colliders[EBossBones::LeftForearm]->Get_FilterData());
		});
	m_pAnimator->RegisterEventListener("ColliderLeftHandOff", [this]()
		{
			m_Colliders[EBossBones::LeftHand]->Init_SimulationFilterData();
			m_Colliders[EBossBones::LeftForearm]->Init_SimulationFilterData();
			m_bPlayerCollided = false;
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



	m_pAnimator->RegisterEventListener("SetRootStep", [this]()
		{
			m_fMaxRootMotionSpeed = 37.f;
			m_fRootMotionAddtiveScale = 4.f;
		});

	m_pAnimator->RegisterEventListener("ResetRootStep", [this]()
		{
			m_fMaxRootMotionSpeed = 18.f;
			m_fRootMotionAddtiveScale = 1.2f;
		});


	m_pAnimator->RegisterEventListener("Phase2InvisibledModel", [this]()
		{
			if (m_pAnimator)
			{
				m_pAnimator->ApplyOverrideAnimController("Phase2");
				m_pAnimator->SetInt("SkillType", DashSwing);
				m_pAnimator->SetTrigger("Attack");
				m_ePrevState = m_eCurrentState;
				m_eCurrentState = EEliteState::ATTACK;
			}

			if (m_BoneRefs[Hammer])
			{
				if (m_iOriginBoneIndex == -1)
					m_iOriginBoneIndex = m_BoneRefs[Hammer]->Get_ParentBoneIndex();

				m_iNewParentIndex = m_pModelCom->Find_BoneIndex("BN_Weapon_R");
				if (m_iNewParentIndex >= 0)
				{
					m_BoneRefs[Hammer]->Set_ParentBoneIndex(m_iNewParentIndex);
					m_bSwitchHeadSpace = true;
				}
			}
			EFFECT_MANAGER->Set_Active_Effect(TEXT("Fes_P2_HeadSmoke_L"), true);
		});

	m_pAnimator->RegisterEventListener("DissolveOn", [this]()
		{
			SwitchDissolve(false, 0.7f, _float3{ 0.75f, 0.8f, 0.9f }, vector<_uint>{ 2, 3 });
		});
	m_pAnimator->RegisterEventListener("RestoreHeadBoneIndex", [this]()
		{
			if (!m_pModelCom || !m_BoneRefs[Hammer] || m_iOriginBoneIndex < 0)
				return;
			// 부모 교체
			m_BoneRefs[Hammer]->Set_ParentBoneIndex(m_iOriginBoneIndex);
			m_BoneRefs[Hammer]->Set_TransformationMatrix(XMLoadFloat4x4(&m_HeadLocalInit));
			m_pModelCom->Update_Bones();
			m_bSwitchHeadSpace = false;
			m_bPlayerCollided = false;
		});

	m_pAnimator->RegisterEventListener("EndPhase2Change", [this]()
		{
			if (!m_pModelCom || !m_BoneRefs[Hammer] || m_iOriginBoneIndex < 0 || m_bSwitchHeadSpace == false)
				return;
			m_BoneRefs[Hammer]->Set_ParentBoneIndex(m_iOriginBoneIndex);
			m_bSwitchHeadSpace = false;
			m_bPlayerCollided = false;
		});


	m_pAnimator->RegisterEventListener("OneHandSlamEffect", [this]()
		{
			if (m_iCurNodeID == ENUM_CLASS(BossStateID::Atk_AlternateSmash_Loop)
				|| m_iCurNodeID == ENUM_CLASS(BossStateID::Atk_AlternateSmash_Loop3)
				|| m_iCurNodeID == ENUM_CLASS(BossStateID::Atk_FuryBodySlam_Loop))
			{
				m_bLeftHand = m_bLeftHand ? false : true;
			}
			else
			{
				m_bLeftHand = false;
			}

			EffectSpawn_Active(EF_ONE_HANDSLAM, true);

			if (m_bLeftHand)
			{
				Spawn_Decal(m_BoneRefs[LeftHand],
					TEXT("Prototype_Component_Texture_FireEater_Slam_Normal"),
					TEXT("Prototype_Component_Texture_FireEater_Slam_Mask"),
					XMVectorSet(5.f, 0.5f, 5.f, 0));
			}
			else
			{
				Spawn_Decal(m_pRightWeaponBone,
					TEXT("Prototype_Component_Texture_FireEater_Slam_Normal"),
					TEXT("Prototype_Component_Texture_FireEater_Slam_Mask"),
					XMVectorSet(5.f, 0.5f, 5.f, 0));
			}
		});

	m_pAnimator->RegisterEventListener("LeftScratchEffect", [this]()
		{
			m_bLeftHand = true;
			EffectSpawn_Active(EF_SCRATCH, true);
		});

	m_pAnimator->RegisterEventListener("RightScratchEffect", [this]()
		{
			m_bLeftHand = false;
			EffectSpawn_Active(EF_SCRATCH, true);

		});

	m_pAnimator->RegisterEventListener("SlamNoSmokeEffect", [this]()
		{
			m_bLeftHand = true;
			EffectSpawn_Active(EF_DEFAULT_SLAM_NOSMOKE, true);

			Spawn_Decal(m_pRightWeaponBone,
				TEXT("Prototype_Component_Texture_FireEater_Slam_Normal"),
				TEXT("Prototype_Component_Texture_FireEater_Slam_Mask"),
				XMVectorSet(5.f, 0.5f, 5.f, 0));
		});

	m_pAnimator->RegisterEventListener("SmokeEffect", [this]()
		{
			m_bLeftHand = true;
			EffectSpawn_Active(EF_SMOKE, true);
		});

	m_pAnimator->RegisterEventListener("KneeEffect", [this]()
		{
			m_bLeftHand = true;
			EffectSpawn_Active(EF_NOSMOKE_KNEE, true);
		});

	m_pAnimator->RegisterEventListener("LeftFallingEffect", [this]()
		{
			m_bLeftHand = true;
			m_bLeftKnee = true;
			EffectSpawn_Active(EF_NOSMOKE_KNEE, true);
			EffectSpawn_Active(EF_DEFAULT_SLAM_NOSMOKE, true);
		});

	m_pAnimator->RegisterEventListener("RightFallingEffect", [this]()
		{
			m_bLeftHand = false;
			m_bLeftKnee = false;
			EffectSpawn_Active(EF_NOSMOKE_KNEE, true);
			EffectSpawn_Active(EF_DEFAULT_SLAM_NOSMOKE, true);
		});

	m_pAnimator->RegisterEventListener("LShooulderSparkEffect", [this]()
		{	
			EffectSpawn_Active(EF_LShoulder_SPARK, true);
	});

	m_pAnimator->RegisterEventListener("RShooulderSparkEffect", [this]()
		{
			EffectSpawn_Active(EF_RShoulder_SPARK, true);
		});

	m_pAnimator->RegisterEventListener("LHandSparkEffect", [this]()
		{
			EffectSpawn_Active(EF_LHand_SPARK, true);
		});

	m_pAnimator->RegisterEventListener("LForearmSparkEffect", [this]()
		{
			EffectSpawn_Active(EF_LForearm_SPARK, true);
		});

	m_pAnimator->RegisterEventListener("OnGroundScratchEffect", [this]()
		{
			EffectSpawn_Active(EF_GROUND_SPARK, true, false);

		});

	m_pAnimator->RegisterEventListener("OffGroundScratchEffect", [this]()
		{
			EffectSpawn_Active(EF_GROUND_SPARK, false);
		});

	m_pAnimator->RegisterEventListener("ShockWaveEffect", [this]()
		{
			EffectSpawn_Active(EF_HAMMER_SLAM, true);
			Spawn_Decal(m_pRightWeaponBone,
				TEXT("Prototype_Component_Texture_FireEater_Slam_Normal"),
				TEXT("Prototype_Component_Texture_FireEater_Slam_Mask"),
				XMVectorSet(5.f, 0.5f, 5.f, 0));
		});

	m_pAnimator->RegisterEventListener("P2_StartEffect", [this]()
		{
			EffectSpawn_Active(EF_P2_START, true);
		});

	m_pAnimator->RegisterEventListener("P2_BaskeEffect", [this]()
		{
			EffectSpawn_Active(EF_BASKET_SLAM, true);
		});

	m_pAnimator->RegisterEventListener("HammerSlamEndEffect", [this]()
		{
			EffectSpawn_Active(EF_HAMMER_SLAM_END, true);
		});

	m_pAnimator->RegisterEventListener("GroggyEffect", [this]()
		{
			m_bHeadSpark = true;
			EffectSpawn_Active(EF_GROGGY, true);
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
			m_PatternWeightMap[pattern] *= 1.5f; // 번갈아 공격을 좀 더 많이
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
		m_bGroggyActive = false;
		m_fGroggyGauge = 0.f;
		m_pAnimator->SetTrigger("Phase2Start");
		m_pAnimator->SetBool("Phase2Combo", true);
		m_pAnimator->SetPlayRate(1.f);
		m_bIsPhase2 = true;
		static_cast<CPlayer*>(m_pPlayer)->SetHitedAttackType(EAttackType::STAMP); // 바스켓에 충돌했을 때를 생각해서 
		m_bStartPhase2 = true;
		vector<EBossAttackPattern> m_vecBossPatterns = {
			DashSwing,Slam, JumpAttack ,Strike ,Spin ,HalfSpin ,HammerSlam ,
			Swing,FuryHammerSlam ,FurySwing
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
				m_PatternWeighForDisttMap[pattern] *= 0.05f;

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

	m_EffectMap[EF_ONE_HANDSLAM].emplace_back(TEXT("EC_Fes_OnehandSlam"));
	m_EffectMap[EF_SCRATCH].emplace_back(TEXT("EC_Fes_Scratch"));
	m_EffectMap[EF_SMOKE].emplace_back(TEXT("EC_Fes_Falling_Smoke_P1"));

	m_EffectMap[EF_LShoulder_SPARK].emplace_back(TEXT("EC_OldSparkDrop_Big_LClavicle"));
	m_EffectMap[EF_RShoulder_SPARK].emplace_back(TEXT("EC_OldSparkDrop_Big_RClavicle"));
	m_EffectMap[EF_LHand_SPARK].emplace_back(TEXT("EC_OldSparkDrop_Big_LHand"));
	m_EffectMap[EF_LForearm_SPARK].emplace_back(TEXT("EC_OldSparkDrop_Small_LForearm"));

	// Phase 2
	m_EffectMap[EF_DEFAULT_SLAM_NOSMOKE].emplace_back(TEXT("EC_Fes_DefaultSlam_NoSmoke_P2"));
	m_EffectMap[EF_NOSMOKE_KNEE].emplace_back(TEXT("EC_Fes_DefaultSlam_NoSmoke_P2_Knee"));

	m_EffectMap[EF_GROUND_SPARK].emplace_back(TEXT("EC_Fuoco_Spin3_FloorFountain_P5"));
	m_EffectMap[EF_HAMMER_SLAM].emplace_back(TEXT("EC_Fes_P2_HammerSlam"));
	m_EffectMap[EF_HAMMER_SLAM].emplace_back(TEXT("EC_Fes_HammerSmallSlam"));
	m_EffectMap[EF_P2_START].emplace_back(TEXT("EC_Fes_P2Start"));
	m_EffectMap[EF_BASKET_SLAM].emplace_back(TEXT("EC_Fes_P2Start_CageSlam"));

	m_EffectMap[EF_HAMMER_SLAM_END].emplace_back(TEXT("EC_Fes_HammerSmallSlam"));


	m_EffectMap[EF_GROGGY].emplace_back(TEXT("EC_OldSparkDrop_Big_LHand"));
	m_EffectMap[EF_GROGGY].emplace_back(TEXT("EC_OldSparkDrop_Big_LClavicle"));
	m_EffectMap[EF_GROGGY].emplace_back(TEXT("EC_OldSparkDrop_Big_RClavicle"));
}


void CFestivalLeader::ProcessingEffects(const _wstring& stEffectTag)
{
	if (m_BoneRefs[Hammer] == nullptr || m_BoneRefs[Basket] == nullptr)
		return;



	CEffectContainer::DESC desc = {};

	// P1
	if (stEffectTag == TEXT("EC_Fes_OnehandSlam")) // 한 손 바닥 찍기
	{
		_uint iHand = m_bLeftHand ? LeftHand : RightHand;

		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;
		const _float4x4* socketPtr = m_BoneRefs[iHand]->Get_CombinedTransformationMatrix();
		const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
		_matrix socket = XMLoadFloat4x4(socketPtr);
		_matrix parent = XMLoadFloat4x4(parentPtr);

		_matrix comb = socket * parent;

		_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(position));
	}
	else if (stEffectTag == TEXT("EC_Fes_Scratch")) // 바닥 긁기, 손마다 개별 생성이므로 두번 호출
	{   
		_uint iFinger = m_bLeftHand ? LeftMiddleFingerStart : RightMiddleFingerStart;

		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;

		const _float4x4* socketPtr = m_BoneRefs[iFinger]->Get_CombinedTransformationMatrix();
		const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
		_matrix socket = XMLoadFloat4x4(socketPtr);
		_matrix parent = XMLoadFloat4x4(parentPtr);

		_matrix comb = socket * parent;

		_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(position));
	}
	else if (stEffectTag == TEXT("EC_Fes_DefaultSlam_NoSmoke_P2")) // 넘어지며 양 손 스파크
	{
		_uint iHand = m_bLeftHand ? LeftHand : RightHand;

		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;
		const _float4x4* socketPtr = m_BoneRefs[iHand]->Get_CombinedTransformationMatrix();
		const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
		_matrix socket = XMLoadFloat4x4(socketPtr);
		_matrix parent = XMLoadFloat4x4(parentPtr);

		_matrix comb = socket * parent;

		_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(position));

	}
	else if (stEffectTag == TEXT("EC_Fes_DefaultSlam_NoSmoke_P2_Knee")) // 넘어지며 양 무릎 스파크
	{
		_uint iKnee = m_bLeftKnee ? LeftKnee : RightKnee;

		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;
		const _float4x4* socketPtr = m_BoneRefs[iKnee]->Get_CombinedTransformationMatrix();

		const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
		_matrix socket = XMLoadFloat4x4(socketPtr);
		_matrix parent = XMLoadFloat4x4(parentPtr);

		_matrix comb = socket * parent;

		_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(position));

	}
	else if (stEffectTag == TEXT("EC_Fes_Falling_Smoke_P1")) // 넘어지며 몸통 먼지
	{
		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;
		const _float4x4* socketPtr = m_pModelCom->Get_CombinedTransformationMatrix(m_iLockonBoneIndex);
		const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
		_matrix socket = XMLoadFloat4x4(socketPtr);
		_matrix parent = XMLoadFloat4x4(parentPtr);

		_matrix comb = socket * parent;

		_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(position));
	}
	else if (stEffectTag == TEXT("EC_OldSparkDrop_Big_LClavicle")) // 왼쪽 어깨 스파크
	{
		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;

		_matrix socket = XMLoadFloat4x4(m_BoneRefs[LeftShoulder]->Get_CombinedTransformationMatrix());
		_matrix parent = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr());

		_matrix comb = socket * parent;

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(comb.r[3]));
	}
	else if (stEffectTag == TEXT("EC_OldSparkDrop_Big_LHand")) // 왼손 스파크
	{
		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;

		_matrix socket = XMLoadFloat4x4(m_BoneRefs[LeftHand]->Get_CombinedTransformationMatrix());
		_matrix parent = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr());

		_matrix comb = socket * parent;

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(comb.r[3]));
	}
	else if (stEffectTag == TEXT("EC_OldSparkDrop_Big_RClavicle")) // 오른쪽 어깨 스파크 (그로기 때 머리 스팤크로도 사용)
	{
		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;
		EBossBones bone = m_bHeadSpark ? HeadJaw : RightShoulder;
		_matrix socket = XMLoadFloat4x4(m_BoneRefs[bone]->Get_CombinedTransformationMatrix());
		_matrix parent = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr());

		_matrix comb = socket * parent;

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(comb.r[3]));
	}
	else if (stEffectTag == TEXT("EC_OldSparkDrop_Small_LForearm")) // 왼쪽 팔꿈치 스파크
	{
		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;

		_matrix socket = XMLoadFloat4x4(m_BoneRefs[LeftForearm]->Get_CombinedTransformationMatrix());
		_matrix parent = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr());

		_matrix comb = socket * parent;

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(comb.r[3]));
	}

	// P2
	else if (stEffectTag == TEXT("EC_Fuoco_Spin3_FloorFountain_P5")) 
	{
		auto worldmat = XMLoadFloat4x4(m_BoneRefs[Hammer]->Get_CombinedTransformationMatrix()) * m_pTransformCom->Get_WorldMatrix();
		_vector rot, trans, scale;
		XMMatrixDecompose(&scale, &rot, &trans, worldmat);

		_vector finalRot = XMQuaternionMultiply(XMQuaternionInverse(rot), XMQuaternionRotationAxis(XMVectorSet(0.f, 0.f, 1.f, 0.f), XMConvertToRadians(-60.f)));

		XMStoreFloat4x4(&desc.PresetMatrix, /*XMMatrixRotationQuaternion(finalRot)**/
			XMMatrixTranslation(worldmat.r[3].m128_f32[0],
				m_pTransformCom->Get_State(STATE::POSITION).m128_f32[1],
				worldmat.r[3].m128_f32[2]));
	}
	else if (stEffectTag == TEXT("EC_Fes_P2_HammerSlam")) //
	{
		const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
		_matrix parent = XMLoadFloat4x4(parentPtr);

		_matrix comb = XMLoadFloat4x4(m_BoneRefs[Hammer]->Get_CombinedTransformationMatrix()) * m_pTransformCom->Get_WorldMatrix();

		_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(position));
	}
	else if (stEffectTag == TEXT("EC_Fes_P2Start"))
	{
		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;
		const _float4x4* socketPtr = m_BoneRefs[Neck]->Get_CombinedTransformationMatrix();
		const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
		_matrix socket = XMLoadFloat4x4(socketPtr);
		_matrix parent = XMLoadFloat4x4(parentPtr);

		_matrix comb = socket * parent;

		_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(comb.r[3]));
	}
	else if (stEffectTag == TEXT("EC_Fes_HammerSmallSlam"))
	{
		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;
		const _float4x4* socketPtr = m_BoneRefs[Hammer]->Get_CombinedTransformationMatrix();
		const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
		_matrix socket = XMLoadFloat4x4(socketPtr);
		_matrix parent = XMLoadFloat4x4(parentPtr);

		_matrix comb = socket * parent;

		_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(position));
	}
	else if (stEffectTag == TEXT("EC_Fes_P2Start_CageSlam"))
	{
		desc.pSocketMatrix = nullptr;
		desc.pParentMatrix = nullptr;
		const _float4x4* socketPtr = m_BoneRefs[Basket]->Get_CombinedTransformationMatrix();
		const _float4x4* parentPtr = m_pTransformCom->Get_WorldMatrix_Ptr();
		_matrix socket = XMLoadFloat4x4(socketPtr);
		_matrix parent = XMLoadFloat4x4(parentPtr);

		_matrix comb = socket * parent;

		_vector position = XMVectorSetY(comb.r[3], parent.r[3].m128_f32[1]);

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslationFromVector(position));
	}
	if (MAKE_EFFECT(ENUM_CLASS(m_iLevelID), stEffectTag, &desc) == nullptr)
		MSG_BOX("이펙트 생성 실패함");
}

HRESULT CFestivalLeader::EffectSpawn_Active(_int iEffectId, _bool bActive, _bool bIsOnce) // 어떤 이펙트를 스폰할지 결정
{
	auto it = m_EffectMap.find(iEffectId);
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
	{
		Reset_EffectFlags();
		return S_OK;
	}

	for (auto it = m_ActiveEffect.begin(); it != m_ActiveEffect.end(); )
	{
		const _wstring& EffectTag = it->first;
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
	CGameObject* pEC = { nullptr };
	CEffectContainer::DESC P2HeadSmokeDesc = {};
	P2HeadSmokeDesc.pSocketMatrix = m_BoneRefs[Neck]->Get_CombinedTransformationMatrix();
	P2HeadSmokeDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	XMStoreFloat4x4(&P2HeadSmokeDesc.PresetMatrix, XMMatrixIdentity());
	pEC = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Fes_P2_HeadSmoke"), &P2HeadSmokeDesc);
	if (pEC == nullptr)
		MSG_BOX("이펙트 생성 실패함");

	EFFECT_MANAGER->Store_EffectContainer(TEXT("Fes_P2_HeadSmoke_L"), static_cast<CEffectContainer*>(pEC));
	EFFECT_MANAGER->Set_Active_Effect(TEXT("Fes_P2_HeadSmoke_L"), false);
	

	/**************************************/
	XMStoreFloat4x4(&P2HeadSmokeDesc.PresetMatrix, XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(90.f)));
	pEC = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Fes_P2_HeadSmoke"), &P2HeadSmokeDesc);
	if (pEC == nullptr)
		MSG_BOX("이펙트 생성 실패함");

	EFFECT_MANAGER->Store_EffectContainer(TEXT("Fes_P2_HeadSmoke_R"), static_cast<CEffectContainer*>(pEC));
	EFFECT_MANAGER->Set_Active_Effect(TEXT("Fes_P2_HeadSmoke_R"), false);



	//EFFECT_MANAGER->Set_Dead_EffectContainer(TEXT("Fuoco_HeadSmoke2")); 삭제시
	return S_OK;
}

void CFestivalLeader::Reset_EffectFlags()
{
	m_bLeftHand = false;
	m_bLeftKnee = false;
	m_bFullbodyEffect = false;
}

void CFestivalLeader::Ready_SoundEvents()
{
	if (m_pSoundCom)
	{
		m_pSoundCom->Set_AllVolume(1.f);
	}
	m_pAnimator->RegisterEventListener("DeadSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play_Random("VO_NPC_FestivalLeader_Dead_Post_0",4,1);
			}
		});

	m_pAnimator->RegisterEventListener("WalkSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Stop("SE_NPC_FS_FestivalLeader_Mud_01");
				m_pSoundCom->Play("SE_NPC_FS_FestivalLeader_Mud_01");
			}
		});

	m_pAnimator->RegisterEventListener("StandUpSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_MT_Standup_1");
			}
		});

	m_pAnimator->RegisterEventListener("PhaseStartSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("Dialog_CH01_Spawn_text_2");
			}
		});

	m_pAnimator->RegisterEventListener("SparkSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->SetVolume("SE_PC_SK_FX_Spark_M_01", 0.6f);
				m_pSoundCom->Play("SE_PC_SK_FX_Spark_M_01");
			}
		});


	m_pAnimator->RegisterEventListener("RustleSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_MT_Rustle_01");
			}
		});

	m_pAnimator->RegisterEventListener("RattleSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_MT_Rattle_07");
		
			}
		});


	m_pAnimator->RegisterEventListener("MovementSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_Boss_Fire_Eater_MT_Movement_0");

			}
		});

	m_pAnimator->RegisterEventListener("SlashSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_SK_WS_Blunt_L_Slash");

			}
		});

	m_pAnimator->RegisterEventListener("AttackLSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play_Random("VO_NPC_FestivalLeader_Attack_L_0",5,1);
			}
		});

	m_pAnimator->RegisterEventListener("AttackMSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play_Random("VO_NPC_FestivalLeader_Attack_M_0", 6, 1);
			}
		});

	m_pAnimator->RegisterEventListener("AttackSSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play_Random("VO_NPC_FestivalLeader_Attack_S_0", 5, 1);
			}
		});

	m_pAnimator->RegisterEventListener("RoarSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play_Random("VO_NPC_FestivalLeader_Roar_0", 5, 1);
			}
		});

	m_pAnimator->RegisterEventListener("WooshSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_TrueOgre_Whoosh_03");
			}
		});


	m_pAnimator->RegisterEventListener("GroundImpactSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_Boss_Judge_SK_Impact_Ground_Heavy_01");
			}
		});

	m_pAnimator->RegisterEventListener("CreakSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_MT_Plate_Creak_01");
			}
		});


	m_pAnimator->RegisterEventListener("GroundExpSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_SK_FX_Ground_L_Exp_01");
			}
		});

	m_pAnimator->RegisterEventListener("MechanicSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_Osis_Mechanic_01");
			}
		});

	m_pAnimator->RegisterEventListener("FuryStartSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_MT_Mechanic_M_Land_01");
			}
		});
	

	m_pAnimator->RegisterEventListener("DmgLSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play_Random("VO_NPC_FestivalLeader_Dmg_L_0",5,1);
			}
		});

	m_pAnimator->RegisterEventListener("DmgMSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play_Random("VO_NPC_FestivalLeader_Dmg_M_0", 5, 1);
			}
		});

	m_pAnimator->RegisterEventListener("DmgSSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play_Random("VO_NPC_FestivalLeader_Dmg_S_0", 5, 1);
			}
		});

	m_pAnimator->RegisterEventListener("FistSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_Seed_Clown_SK_Fist_01");
			}
		});

	m_pAnimator->RegisterEventListener("RockDebrisSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_SK_FX_Rock_Debris_L_01");
			}
		});

	m_pAnimator->RegisterEventListener("DropBasketSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_PC_SK_GetHit_M_Parry_04");
			}
		});

	m_pAnimator->RegisterEventListener("PullOutHeadSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_Boss_Judge_SK_Dead_Explo_01");
				m_pSoundCom->Play("SE_PC_SK_GetHit_M_Sword_PerfectGuard_03");
			}
		});

	m_pAnimator->RegisterEventListener("PullOutFireSound", [this]()
		{
			if (m_pSoundCom)
			{
				m_pSoundCom->Play("SE_NPC_SK_FX_Bomb");
			}
		});
}

void CFestivalLeader::EnterCutScene()
{
	m_pAnimator->Get_CurrentAnimController()->SetStateToEntry();
	m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->clip->SetCurrentTrackPosition(120.f);
	m_pAnimator->SetPlaying(true);
	m_bCutSceneOn = true;
}

void CFestivalLeader::Calc_WeaponDir()
{
	if (m_bIsPhase2)
	{
		m_vPrevWeaponPos = m_vCurWeaponPos;
		
		auto CurMat = XMLoadFloat4x4(m_BoneRefs[Hammer]->Get_CombinedTransformationMatrix()) * m_pTransformCom->Get_WorldMatrix();
		_vector CurPos = CurMat.r[3];

		XMStoreFloat3(&m_vCurWeaponPos, CurPos);
		XMStoreFloat3(&m_vWeaponDir, CurPos - XMLoadFloat3(&m_vPrevWeaponPos));
	}
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


void CFestivalLeader::BreakPanel()
{
	auto panel = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_AnimPropPanel"));

	if (auto pPanel = dynamic_cast<CAnimatedProp*>(panel))
	{
		pPanel->NotifyPlayAnimation(true);
	}
}

void CFestivalLeader::EnterNextCutScene()
{
	if (m_pAnimator)
	{
		m_pAnimator->SetTrigger("NextCut");
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


void CFestivalLeader::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	ReceiveDamage(pOther, eColliderType);

	if (m_bPlayerCollided)
	{
		cout << "Already Collided" << endl;
		return;
	}
	if (auto pPlayer = dynamic_cast<CPlayer*>(pOther))
	{
		_uint curNodeID = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->iNodeId;
		switch (curNodeID)
		{
		case ENUM_CLASS(BossStateID::Atk_Jump_Start):
		case ENUM_CLASS(BossStateID::Atk_Jump_Loop):
			pPlayer->SetfReceiveDamage(DAMAGE_MEDIUM);
			pPlayer->SetHitMotion(HITMOTION::STAMP);
			pPlayer->SetHitedAttackType(EAttackType::STAMP);
			break;
		case ENUM_CLASS(BossStateID::Atk_Slam_Start):
		case ENUM_CLASS(BossStateID::Atk_Slam_Loop):
			pPlayer->SetfReceiveDamage(DAMAGE_MEDIUM);
			pPlayer->SetHitMotion(HITMOTION::NORMAL);
			pPlayer->SetHitedAttackType(EAttackType::NORMAL);
			break;
		case ENUM_CLASS(BossStateID::Atk_Strike_Start):
		case ENUM_CLASS(BossStateID::Atk_Strike_Loop):
			pPlayer->SetfReceiveDamage(DAMAGE_HEAVY);
			pPlayer->SetHitMotion(HITMOTION::UP);
			pPlayer->SetHitedAttackType(EAttackType::AIRBORNE);
			break;

		case ENUM_CLASS(BossStateID::Atk_FurySwing_Start):
		case ENUM_CLASS(BossStateID::Atk_FurySwing_Loop):
			pPlayer->SetfReceiveDamage(DAMAGE_FURY);
			pPlayer->SetHitMotion(HITMOTION::KNOCKBACK);
			pPlayer->SetHitedAttackType(EAttackType::FURY_KNOCKBACK);
			break;

		case ENUM_CLASS(BossStateID::Atk_FuryBodySlam_Start):
		case ENUM_CLASS(BossStateID::Atk_FuryBodySlam_Loop):
			pPlayer->SetfReceiveDamage(DAMAGE_FURY);
			pPlayer->SetHitMotion(HITMOTION::STAMP);
			pPlayer->SetHitedAttackType(EAttackType::FURY_STAMP);
			break;

		case ENUM_CLASS(BossStateID::Atk_CrossSlam_Start):
		case ENUM_CLASS(BossStateID::Atk_CrossSlam_Loop):
		case ENUM_CLASS(BossStateID::Atk_AlternateSmash_Start):
		case ENUM_CLASS(BossStateID::Atk_AlternateSmash_Loop):
		case ENUM_CLASS(BossStateID::Atk_AlternateSmash_Start2):
		case ENUM_CLASS(BossStateID::Atk_AlternateSmash_Loop2):
		case ENUM_CLASS(BossStateID::Atk_AlternateSmash_Start3):
		case ENUM_CLASS(BossStateID::Atk_AlternateSmash_Loop3):
			pPlayer->SetfReceiveDamage(DAMAGE_MEDIUM);
			pPlayer->SetHitMotion(HITMOTION::KNOCKBACK);
			pPlayer->SetHitedAttackType(EAttackType::KNOCKBACK);
			break;

		case ENUM_CLASS(BossStateID::Atk_HalfSpin_Start):
		case ENUM_CLASS(BossStateID::Atk_HalfSpin_Loop):
		case ENUM_CLASS(BossStateID::Atk_Spin):
		case ENUM_CLASS(BossStateID::Atk_DashSwing_Loop):
		case ENUM_CLASS(BossStateID::Atk_Swing_Loop):
			pPlayer->SetfReceiveDamage(DAMAGE_MEDIUM);
			pPlayer->SetHitMotion(HITMOTION::KNOCKBACK);
			pPlayer->SetHitedAttackType(EAttackType::KNOCKBACK);
			break;

		case ENUM_CLASS(BossStateID::Atk_HammerSlam_Loop):
			pPlayer->SetfReceiveDamage(DAMAGE_HEAVY);
			pPlayer->SetHitMotion(HITMOTION::STAMP);
			pPlayer->SetHitedAttackType(EAttackType::STAMP);
			break;
		case ENUM_CLASS(BossStateID::Atk_FuryHammerSlam_Loop):
			pPlayer->SetfReceiveDamage(DAMAGE_FURY);
			pPlayer->SetHitMotion(HITMOTION::STAMP);
			pPlayer->SetHitedAttackType(EAttackType::FURY_STAMP);
			break;
		case ENUM_CLASS(BossStateID::Atk_Phase2Start):
			pPlayer->SetfReceiveDamage(DAMAGE_MEDIUM);
			pPlayer->SetHitMotion(HITMOTION::STAMP);
			pPlayer->SetHitedAttackType(EAttackType::STAMP);
			break;
		default:
			break;
		}
		m_bPlayerCollided = true;
	}
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
	//	Safe_Release(m_pSpringBoneSys);
	for (auto& pCol : m_Colliders)
		Safe_Release(pCol);
}
