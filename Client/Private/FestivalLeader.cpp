#include "FestivalLeader.h"

#include "Bone.h"
#include <Oil.h>
#include <Player.h>
#include "Projectile.h"
#include "FlameField.h"
#include "GameInstance.h"
#include "Effect_Manager.h"
#include "LockOn_Manager.h"
#include "Camera_Manager.h"
#include "Client_Calculation.h"
#include <PhysX_IgnoreSelfCallback.h>
#include "UI_MonsterHP_Bar.h"
#include "Static_Decal.h"
#include "Weapon_Monster.h"

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
	/* [ 데미지 설정 ] */
	m_fDamage = 15.f;
	m_fAttckDleay = 1.5f;
	m_fChasingDistance = 4.f;
	m_iPatternLimit = 1;
	m_fMinimumTurnAngle = 50.f;
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

		//UnitDesc.InitPos = _float3(55.f, 0.f, -7.5f);
		//UnitDesc.InitPos = _float3(55.5f, 0.f, -7.5f);
		//UnitDesc.InitScale = _float3(0.9f, 0.9f, 0.9f);

		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;
	}

	// 체력 일단 각 객체에 

	// 0번 메시는 다리,1번은 몸통, 4번 양팔, 5번 머리
	// 2,3번은 바스켓
	//m_pModelCom->SetMeshVisible(5, false);
	//	m_pModelCom->SetMeshVisible(3, false);
	m_fMaxRootMotionSpeed = 18.f;

	if(FAILED(Ready_Weapon()))
		return E_FAIL;
	return S_OK;
}

void CFestivalLeader::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_bDead)
		m_pHPBar->Set_bDead();

#ifdef _DEBUG
	if (KEY_DOWN(DIK_Y))
	{
		//m_fHp -= 100;
	}
	if (KEY_DOWN(DIK_X))
	{

		//static _int i = 0;
		//static array<_int, 13> testArray{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,13 };


		///*cout << "현재 공격 인덱스 " << i << endl;*/
		//m_eCurAttackPattern = static_cast<EBossAttackPattern>(i + 1);
		//switch (m_eCurAttackPattern)
		//{
		//case CFestivalLeader::SlamCombo:
		//	cout << "SlamCombo" << endl;
		//	break;
		//case CFestivalLeader::Uppercut:
		//	cout << "Uppercut" << endl;
		//	break;
		//case CFestivalLeader::SwingAtkSeq:
		//	cout << "SwingAtkSeq" << endl;
		//	break;
		//case CFestivalLeader::SwingAtk:
		//	cout << "SwingAtk" << endl;
		//	break;
		//case CFestivalLeader::SlamFury:
		//	cout << "SlamFury" << endl;
		//	break;
		//case CFestivalLeader::FootAtk:
		//	cout << "FootAtk" << endl;
		//	break;
		//case CFestivalLeader::P2_FlameField:
		//	cout << "P2_FlameField" << endl;
		//	break;
		//case CFestivalLeader::SlamAtk:
		//	cout << "SlamAtk" << endl;
		//	break;
		//case CFestivalLeader::StrikeFury:
		//	cout << "StrikeFury" << endl;
		//	break;
		//case CFestivalLeader::P2_FireOil:
		//	cout << "P2_FireOil" << endl;
		//	break;
		//case CFestivalLeader::P2_FireBall:
		//	cout << "P2_FireBall" << endl;
		//	break;
		//case CFestivalLeader::P2_FireFlame:
		//	cout << "P2_FireFlame" << endl;
		//	break;
		//case CFestivalLeader::P2_FireBall_B:
		//	cout << "P2_FireBall_B" << endl;
		//	break;
		//default:
		//	cout << "Unknown" << endl;;
		//	break;
		//}
		//m_pAnimator->SetInt("SkillType", testArray[i++]);
		//if (i >= 13)
		//	i = 0;
		//m_pAnimator->SetTrigger("Paralyzation");
	//	m_pAnimator->SetTrigger("Fatal");
		//m_pAnimator->SetTrigger("Groggy");
		//if (m_bStartPhase2 == false)
		//	m_bStartPhase2 = true;
	//	m_fHp -= 10.f;
	}

	if (KEY_DOWN(DIK_C))
	{
		m_pAnimator->SetTrigger("Phase2Start");
	}

	if (KEY_DOWN(DIK_B))
	{
		EnterCutScene();
		m_bDebugMode = !m_bDebugMode;
	}

	if (KEY_PRESSING(DIK_LCONTROL))
	{
		if (KEY_DOWN(DIK_A))
		{


		}
		if (KEY_DOWN(DIK_S))
		{

		}
	}
#endif

	if (nullptr != m_pHPBar)
		m_pHPBar->Priority_Update(fTimeDelta);

	if (KEY_DOWN(DIK_B))
	{
		EnterCutScene();
	}

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

	if (nullptr != m_pHPBar)
		m_pHPBar->Update(fTimeDelta);

	if (m_pPlayer && static_cast<CUnit*>(m_pPlayer)->GetHP() <= 0 && m_pHPBar)
		m_pHPBar->Set_RenderTime(0.f);

//#ifdef _DEBUG
//
//	if (m_pHammer)
//	{
//		auto pTrans = m_pHammer->Get_TransfomCom();
//		auto pos = m_pHammer->GetLocalOffset();
//
//		static float rotX = 0.f;
//		static float rotY = 0.f;
//		static float rotZ = 0.f;
//
//		const _float fRotateSpeed = 1.f; // 도 단위
//
//		bool bChanged = false;
//
//		//// z는 280도 y는 5도
//		//// Y축 회전 (좌/우)
//		//if (KEY_DOWN(DIK_V)) {
//		//	rotY -= fRotateSpeed;
//		//	bChanged = true;
//		//}
//		//if (KEY_DOWN(DIK_G)) {
//		//	rotY += fRotateSpeed;
//		//	bChanged = true;
//		//}
//
//		//// X축 회전
//		//if (KEY_DOWN(DIK_5)) {
//		//	rotX += fRotateSpeed;
//		//	bChanged = true;
//		//}
//
//		//// Z축 회전
//		//if (KEY_DOWN(DIK_J)) {
//		//	rotZ += fRotateSpeed;
//		//	bChanged = true;
//		//}
//
//		//if (bChanged)
//		//{
//		//	// 누적된 회전값으로 행렬 생성
//		//	_matrix matRotX = XMMatrixRotationX(XMConvertToRadians(rotX));
//		//	_matrix matRotY = XMMatrixRotationY(XMConvertToRadians(rotY));
//		//	_matrix matRotZ = XMMatrixRotationZ(XMConvertToRadians(rotZ));
//
//		//	// 회전 순서 (필요에 맞게 바꿀 수 있음: Y→X→Z 등)
//		//	_matrix matFinalRot = matRotX * matRotY * matRotZ;
//
//		//	// 현재 위치 유지한 채 회전만 적용
//		//	_float4x4 worldMat;
//		//	XMStoreFloat4x4(&worldMat, matFinalRot);
//		//	worldMat._41 = pTrans->Get_State(STATE::POSITION).m128_f32[0];
//		//	worldMat._42 = pTrans->Get_State(STATE::POSITION).m128_f32[1];
//		//	worldMat._43 = pTrans->Get_State(STATE::POSITION).m128_f32[2];
//
//		//	pTrans->Set_WorldMatrix(worldMat);
//
//		//	// 디버그 출력
//		//	cout << "Hammer Rotation = (X:" << rotX
//		//		<< "°, Y:" << rotY
//		//		<< "°, Z:" << rotZ << "°)" << endl;
//		//}
//			if (KEY_DOWN(DIK_1)) { pos.y += 0.05f; bChanged = true; }  // 위
//			if (KEY_DOWN(DIK_2)) { pos.y -= 0.05f; bChanged = true; }  // 아래
//			if (KEY_DOWN(DIK_3)) { pos.x -= 0.05f; bChanged = true; }  // 왼쪽
//			if (KEY_DOWN(DIK_4)) { pos.x += 0.05f; bChanged = true; }  // 오른쪽
//			if (KEY_DOWN(DIK_5)) { pos.z += 0.05f; bChanged = true; }  // 앞
//			if (KEY_DOWN(DIK_6)) { pos.z -= 0.05f; bChanged = true; }  // 뒤
//
//			if (bChanged)
//			{
//				m_pHammer->SetLocalOffset(pos);
//
//				// 현재 오프셋 콘솔 출력
//				cout << "Hammer Offset = ("
//					<< pos.x << ", "
//					<< pos.y << ", "
//					<< pos.z << ")" << endl;
//			}
//	}
//#endif

}

void CFestivalLeader::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider())
	{
		if (m_pPhysXActorComForHammer->Get_ReadyForDebugDraw())
			m_pGameInstance->Add_DebugComponent(m_pPhysXActorComForHammer);
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
}

HRESULT CFestivalLeader::Ready_Components(void* pArg)
{
	if (FAILED(__super::Ready_Components(pArg)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX2"), reinterpret_cast<CComponent**>(&m_pPhysXActorComForHammer))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX3"), reinterpret_cast<CComponent**>(&m_pPhysXActorComForBasket))))
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
	_vector S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 부모꺼 부르면서 밑에꺼 부르기
	if (m_pHammerBone)
	{
		// 해머 본 → 월드 변환
		auto hammerLocalMatrix = m_pHammerBone->Get_CombinedTransformationMatrix();
		auto hammerWorldMatrix = XMLoadFloat4x4(hammerLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
		XMMatrixDecompose(&S, &R, &T, hammerWorldMatrix);

		// 해머 충돌체 변환
		PxQuat  hammerRotQ = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
		PxVec3  hammerPos = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
		PxTransform hammerPose(hammerPos, hammerRotQ);

		// 해머 구체 콜라이더
		PxSphereGeometry hammerGeom = m_pGameInstance->CookSphereGeometry(1.f);
		m_pPhysXActorComForHammer->Create_Collision(m_pGameInstance->GetPhysics(), hammerGeom, hammerPose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorComForHammer->Set_ShapeFlag(false, true, true);

		// 필터
		PxFilterData hammerFilter{};
		hammerFilter.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		hammerFilter.word1 = WORLDFILTER::FILTER_PLAYERBODY | WORLDFILTER::FILTER_MONSTERWEAPON;
		m_pPhysXActorComForHammer->Set_SimulationFilterData(hammerFilter);
		m_pPhysXActorComForHammer->Set_QueryFilterData(hammerFilter);

		// 소유/타입/키네마틱 설정
		m_pPhysXActorComForHammer->Set_Owner(this);
		m_pPhysXActorComForHammer->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
		m_pPhysXActorComForHammer->Set_Kinematic(true);

		// 씬 등록
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorComForHammer->Get_Actor());
	}

	if (m_pBasketBone)
	{
		auto basketLocalMatrix = m_pBasketBone->Get_CombinedTransformationMatrix();
		auto basketWorldMatrix = XMLoadFloat4x4(basketLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
		XMMatrixDecompose(&S, &R, &T, basketWorldMatrix);

		PxQuat  basketRotQ = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
		PxVec3  basketPos = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
		PxTransform basketPose(basketPos, basketRotQ);

		// 바스켓 구체 콜라이더
		PxSphereGeometry basketGeom = m_pGameInstance->CookSphereGeometry(1.0f);
		m_pPhysXActorComForBasket->Create_Collision(m_pGameInstance->GetPhysics(), basketGeom, basketPose, m_pGameInstance->GetMaterial(L"Default"));
		m_pPhysXActorComForBasket->Set_ShapeFlag(false, true, true);

		// 필터
		PxFilterData basketFilter{};
		basketFilter.word0 = WORLDFILTER::FILTER_MONSTERBODY;
		basketFilter.word1 = WORLDFILTER::FILTER_PLAYERBODY;
		m_pPhysXActorComForBasket->Set_SimulationFilterData(basketFilter);
		m_pPhysXActorComForBasket->Set_QueryFilterData(basketFilter);

		// 소유/타입/키네마틱 설정
		m_pPhysXActorComForBasket->Set_Owner(this);
		m_pPhysXActorComForBasket->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
		m_pPhysXActorComForBasket->Set_Kinematic(true);

		// 씬 등록
		m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorComForBasket->Get_Actor());
	}

	return S_OK;
}

HRESULT CFestivalLeader::Ready_Weapon()
{
	CWeapon_Monster::MONSTER_WEAPON_DESC Desc{};
	Desc.eMeshLevelID = LEVEL::STATIC;
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.InitPos = {0.f,0.f,0.f };
	Desc.InitScale = { 1.f,1.f,1.f };
	Desc.iRender = 0;

	Desc.szMeshID = TEXT("FestivalWeapon");
	lstrcpy(Desc.szName, TEXT("FestivalWeapon"));
	Desc.vAxis = { 0.f,0.f,1.f,0.f };
	Desc.fRotationDegree = { 0.f };
//	Desc.vLocalOffset = { 0.8f,0.2f,-0.2f,1.f };
	Desc.vLocalOffset = { 0.9f, 0.2f, -0.25f,1.f };
	Desc.vPhsyxExtent = { 0.1f, 0.1f, 0.1f };
	Desc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-R-Hand"));
	Desc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	Desc.pOwner = this;

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_Weapon"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("FestivalWeapon"), &pGameObject, &Desc)))
		return E_FAIL;

	m_pHammer = dynamic_cast<CWeapon_Monster*>(pGameObject);

	_matrix matRotX = XMMatrixRotationX(XMConvertToRadians(5.f));
	_matrix matRotY = XMMatrixRotationY(XMConvertToRadians(-12.f));
	_matrix matRotZ = XMMatrixRotationZ(XMConvertToRadians(279.f));

	auto pTrans = m_pHammer->Get_TransfomCom();
	_matrix matFinalRot = matRotX * matRotY * matRotZ;
	_float4x4 worldMat;
	XMStoreFloat4x4(&worldMat, matFinalRot);
	worldMat._41 = pTrans->Get_State(STATE::POSITION).m128_f32[0];
	worldMat._42 = pTrans->Get_State(STATE::POSITION).m128_f32[1];
	worldMat._43 = pTrans->Get_State(STATE::POSITION).m128_f32[2];

	pTrans->Set_WorldMatrix(worldMat);

	m_pHammer->SetisAttack(false);
	m_pHammer->Set_WeaponTrail_Active(true);
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
		m_pRightHandBone = *it;
	}
}

void CFestivalLeader::Update_Collider()
{
	__super::Update_Collider();

	auto hammerLocalMatrix = m_pHammerBone->Get_CombinedTransformationMatrix();
	auto hammerWorldMatrix = XMLoadFloat4x4(hammerLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
	_float4 hammerPos;
	XMStoreFloat4(&hammerPos, hammerWorldMatrix.r[3]);
	PxVec3 weaponPos(hammerPos.x, hammerPos.y, hammerPos.z);
	_vector boneQuatForHammer = XMQuaternionRotationMatrix(hammerWorldMatrix);
	_float4 fQuatForHammer;
	XMStoreFloat4(&fQuatForHammer, boneQuatForHammer);
	PxQuat hammerRot = PxQuat(fQuatForHammer.x, fQuatForHammer.y, fQuatForHammer.z, fQuatForHammer.w);
	m_pPhysXActorComForHammer->Set_Transform(PxTransform(weaponPos, hammerRot));

	auto basketLocalMatrix = m_pBasketBone->Get_CombinedTransformationMatrix();
	auto basketWorldMatrix = XMLoadFloat4x4(basketLocalMatrix) * m_pTransformCom->Get_WorldMatrix();
	_float4 baskeyPos;
	XMStoreFloat4(&baskeyPos, basketWorldMatrix.r[3]);
	PxVec3 basketPosVec(baskeyPos.x, baskeyPos.y, baskeyPos.z);
	_vector boneQuatForBasket = XMQuaternionRotationMatrix(basketWorldMatrix);
	_float4 fQuatForBasket;
	XMStoreFloat4(&fQuatForBasket, boneQuatForBasket);
	PxQuat basketRot = PxQuat(fQuatForBasket.x, fQuatForBasket.y, fQuatForBasket.z, fQuatForBasket.w);
	m_pPhysXActorComForBasket->Set_Transform(PxTransform(basketPosVec, basketRot));

}

void CFestivalLeader::UpdateAttackPattern(_float fDistance, _float fTimeDelta)
{
	if (m_fFirstChaseBeforeAttack >= 0.f)
	{
		m_fFirstChaseBeforeAttack -= fTimeDelta;
		//	m_pAnimator->SetBool("Move", true);
		//	m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::FRONT));
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
	//m_pSoundCom->Play_Random("VO_NPC_NHM_Boss_Fire_Eater_Attack_", 9);

}

void CFestivalLeader::UpdateStateByNodeID(_uint iNodeID)
{
	m_iPrevNodeID = m_iCurNodeID;
	m_iCurNodeID = iNodeID;
	m_ePrevState = m_eCurrentState;
	switch (iNodeID)
	{
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
	default:
		m_eCurrentState = EEliteState::ATTACK;
		break;
	}
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
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			if (auto pController = pPlayer->Get_Controller())
			{
				pController->Remove_IgnoreActors(m_pPhysXActorComForHammer->Get_Actor());
				pController->Remove_IgnoreActors(m_pPhysXActorComForBasket->Get_Actor());
			}
		}
	}
	else
	{
		m_pPhysXActorComForHammer->Init_SimulationFilterData();
		m_pPhysXActorComForBasket->Init_SimulationFilterData();
		if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
		{
			if (auto pController = pPlayer->Get_Controller())
			{
				pController->Add_IngoreActors(m_pPhysXActorComForHammer->Get_Actor());
				pController->Add_IngoreActors(m_pPhysXActorComForBasket->Get_Actor());
			}
		}
	}
}

_bool CFestivalLeader::CanProcessTurn()
{
	// 2페이즈이고 중앙으로 턴을 기다리는 중이면 false
	return !(m_bIsPhase2 && m_bWaitPhase2);
}


void CFestivalLeader::SetupAttackByType(_int iPattern)
{

	//switch (iPattern)
	//{
	//case Client::CFestivalLeader::SlamCombo:
	//{
	//	_bool bIsCombo = GetRandomInt(0, 1) == 1;
	//	m_pAnimator->SetBool("IsCombo", bIsCombo);
	//	if (bIsCombo)
	//	{
	//		_int iDir = GetYawSignFromDiretion();
	//		m_pAnimator->SetInt("Direction", iDir);
	//	}
	//}
	//break;
	//case Client::CFestivalLeader::SwingAtk:
	//{
	//	_bool bIsCombo = GetRandomInt(0, 1) == 1;
	//	m_pAnimator->SetBool("IsCombo", bIsCombo);
	//	if (bIsCombo)
	//	{
	//			_int iComboType;

	//			if (m_iLastComboType == -1) // 첫 시작은 랜덤
	//				iComboType = GetRandomInt(0, 1);
	//			else
	//				iComboType = 1 - m_iLastComboType; // 이전 값과 반대로

	//			m_pAnimator->SetInt("SwingCombo", iComboType);
	//			m_iLastComboType = iComboType;
	//	}
	//	m_eAttackType = EAttackType::KNOCKBACK;
	//}
	//break;
	//case Client::CFestivalLeader::SlamFury:
	//	SetTurnTimeDuringAttack(2.f, 1.5f);
	//	m_eAttackType = EAttackType::FURY_STAMP;
	//case Client::CFestivalLeader::FootAtk:
	//	m_eAttackType = EAttackType::AIRBORNE;
	//	break;
	//case Client::CFestivalLeader::SlamAtk:
	//	SetTurnTimeDuringAttack(1.f);
	//	m_eAttackType = EAttackType::STAMP;
	//	break;
	//case Client::CFestivalLeader::Uppercut:
	//	SetTurnTimeDuringAttack(1.f);
	//	m_eAttackType = EAttackType::NORMAL;
	//	break;
	//case Client::CFestivalLeader::StrikeFury:
	//	SetTurnTimeDuringAttack(1.2f);
	//	m_eAttackType = EAttackType::FURY_AIRBORNE;
	//	break;
	//case Client::CFestivalLeader::P2_FireFlame:
	//{
	//	_int iDir = GetYawSignFromDiretion();
	//	m_pAnimator->SetInt("Direction", iDir);
	//}
	//case Client::CFestivalLeader::P2_FireOil:
	//	m_eAttackType = EAttackType::NONE;
	//	break;
	//case Client::CFestivalLeader::P2_FireBall:
	//{
	//	_int iDir = GetRandomInt(0, 2);
	//	m_pAnimator->SetInt("Direction", iDir);
	//	m_eAttackType = EAttackType::KNOCKBACK;
	//}
	//break;
	//case Client::CFestivalLeader::P2_FireBall_B:
	//	m_eAttackType = EAttackType::KNOCKBACK;
	//	break;
	//default:
	//	break;
	//}
	//if (iPattern == Client::CFestivalLeader::SlamFury)
	//{
	//	m_bRootMotionClamped = true;
	//}
	//else
	//{
	//	m_bRootMotionClamped = false;
	//}
	static_cast<CPlayer*>(m_pPlayer)->SetHitedAttackType(m_eAttackType);
}

void CFestivalLeader::Register_Events()
{
	if (nullptr == m_pAnimator)
		return;

	CEliteUnit::Register_Events();


	m_pAnimator->RegisterEventListener("Turnning", [this]()
		{
			_bool bIsFront = IsTargetInFront(180.f);

			if (bIsFront == false)
			{
				SetTurnTimeDuringAttack(2.5f, 1.4f);
			}
			else
			{
				SetTurnTimeDuringAttack(1.3f);
			}

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
			if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
			{
				if (auto pController = pPlayer->Get_Controller())
				{
					pController->Remove_IgnoreActors(m_pPhysXActorComForBasket->Get_Actor());
				}
			}
		});
	m_pAnimator->RegisterEventListener("ColliderBasketOff", [this]()
		{
			m_pPhysXActorComForBasket->Init_SimulationFilterData();
			if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
			{
				if (auto pController = pPlayer->Get_Controller())
				{
					pController->Add_IngoreActors(m_pPhysXActorComForBasket->Get_Actor());
				}
			}
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
			m_fMaxRootMotionSpeed = 40.f;
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
				else
				{
					m_pModelCom->SetMeshVisible(2, true);
				}

				if (m_pModelCom->IsMeshVisible(3))
				{
					m_pModelCom->SetMeshVisible(3, false);
				}
				else
				{
					m_pModelCom->SetMeshVisible(3, true);
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
		m_PatternCountMap[pattern] = 0;
	}
}

void CFestivalLeader::Ready_AttackPatternWeightForPhase2()
{
	if (m_eCurrentState == EEliteState::FATAL
		|| m_eCurrentState == EEliteState::ATTACK)
		return;
	m_pAnimator->SetTrigger("Phase2Start");
	m_bStartPhase2 = true;
	vector<EBossAttackPattern> m_vecBossPatterns = {
		Slam  ,JumpAttack ,Strike ,Spin ,HalfSpin ,HammerSlam ,
		DashSwing ,Swing,FuryHammerSlam ,FurySwing ,FuryBodySlam
	};
	m_PatternWeightMap.clear();
	m_PatternCountMap.clear();
	//for (const auto& pattern : m_vecBossPatterns)
	//{
	//	if (pattern == P2_FireOil || pattern == P2_FireBall ||
	//		pattern == P2_FireFlame || pattern == P2_FireBall_B)
	//	{
	//		m_PatternWeightMap[pattern] = m_fBasePatternWeight * 2.2f;
	//	}
	//	else
	//	{
	//		m_PatternWeightMap[pattern] = m_fBasePatternWeight * 0.2f;
	//	}

	//	m_PatternCountMap[pattern] = 0;
	//}
	SwitchFury(false, 1.f);
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
				m_PatternWeighForDisttMap[pattern] *= 0.f;
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
				m_PatternWeighForDisttMap[pattern] *= 0.f;
			}
		}
	}

	// TODO
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
	if (pOther)
	{
		if (eColliderType == COLLIDERTYPE::PLAYER)
		{
			m_bPlayerCollided = true;
		}
	}
}

void CFestivalLeader::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{

	__super::On_CollisionStay(pOther, eColliderType, HitPos, HitNormal);
	if (pOther)
	{
		if (eColliderType == COLLIDERTYPE::PLAYER)
		{
			m_bPlayerCollided = true;
		}
	}
}

void CFestivalLeader::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (pOther)
	{
		if (eColliderType == COLLIDERTYPE::PLAYER)
		{
			m_bPlayerCollided = false;
		}
	}
}

void CFestivalLeader::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (eColliderType == COLLIDERTYPE::PLAYER)
	{
		cout << "플레이어 충돌" << endl;
	}
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
	//Safe_Release(m_pHammer);
	Safe_Release(m_pPhysXActorComForHammer);
	Safe_Release(m_pPhysXActorComForBasket);
}
