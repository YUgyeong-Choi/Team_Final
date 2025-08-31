#include "Elite_Police.h"
#include "GameInstance.h"
#include "Weapon_Monster.h"
#include "LockOn_Manager.h"
#include "Client_Calculation.h"

CElite_Police::CElite_Police(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CEliteUnit{ pDevice, pContext }
{
}

CElite_Police::CElite_Police(const CElite_Police& Prototype)
	:CEliteUnit(Prototype)
{
	m_pAnimator = nullptr;
}

HRESULT CElite_Police::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CElite_Police::Initialize(void* pArg)
{
	m_fMaxRootMotionSpeed = 30.f;
	m_fRootMotionAddtiveScale = 1.f;
	m_fAttckDleay = 3.5f;
	m_fTooCloseDistance = 1.5f;
	m_fChasingDistance = 2.f;
	m_fMinimumTurnAngle = 120.f;
	m_bIsFirstAttack = false;
	if (pArg == nullptr)
	{
		UNIT_DESC UnitDesc{};
		UnitDesc.eMeshLevelID = LEVEL::KRAT_CENTERAL_STATION;
		UnitDesc.fRotationPerSec = XMConvertToRadians(270.f);
		UnitDesc.fSpeedPerSec = m_fWalkSpeed;
		lstrcpy(UnitDesc.szName, TEXT("Elite_Police"));
		UnitDesc.szMeshID = TEXT("Elite_Police");
		UnitDesc.InitPos = _float3(55.f, 0.f, -7.5f);
		//UnitDesc.InitPos = _float3(55.5f, 0.f, -7.5f);
		UnitDesc.InitScale = _float3(0.9f, 0.9f, 0.9f);

		if (FAILED(__super::Initialize(&UnitDesc)))
			return E_FAIL;
	}
	else
	{
		UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);

		lstrcpy(pDesc->szName, TEXT("Elite_Police"));
		pDesc->szMeshID = TEXT("Elite_Police");
		pDesc->fRotationPerSec = XMConvertToRadians(270.f);
		pDesc->fSpeedPerSec = m_fWalkSpeed;

		//UnitDesc.InitPos = _float3(55.f, 0.f, -7.5f);
		//UnitDesc.InitPos = _float3(55.5f, 0.f, -7.5f);
		//UnitDesc.InitScale = _float3(0.9f, 0.9f, 0.9f);

		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;
	}

	if (FAILED(Ready_Weapon()))
		return E_FAIL;


	m_fMaxHP = 150.f;
	m_fHP = m_fMaxHP;
	CUI_MonsterHP_Bar::HPBAR_DESC eDesc{};

	eDesc.fSizeX = 1.f;
	eDesc.fSizeY = 1.f;
	eDesc.fHeight = 2.5f;
	eDesc.pHP = &m_fHP;
	eDesc.pIsGroggy = &m_bGroggyActive;
	eDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();

	m_pHPBar = static_cast<CUI_MonsterHP_Bar*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT,
		ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_HPBar"), &eDesc));


	m_pHPBar->Set_MaxHp(m_fHP);

	m_iLockonBoneIndex = m_pModelCom->Find_BoneIndex("Bip001-Spine2");
	m_vRayOffset = { 0.f, 1.8f, 0.f, 0.f };

	auto pShape = m_pPhysXActorCom->Get_Shape();
	PxGeometryHolder geomHolder = pShape->getGeometry();
	PxBoxGeometry box = geomHolder.box();
	box.halfExtents = PxVec3(0.7f, 1.2f, 0.6f);
	pShape->setGeometry(box);

	PxTransform localPose = pShape->getLocalPose();
	localPose.p += PxVec3(0.f, -0.5f, 0.f);
	pShape->setLocalPose(localPose);


	Ready_AttackPatternWeight();
	m_bIsFirstAttack = false;
	return S_OK;
}

void CElite_Police::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
#ifdef _DEBUG
	if (KEY_DOWN(DIK_TAB))
	{
		cout << "현재 폴리스 상태 : ";

		switch (m_eCurrentState)
		{
		case Client::CEliteUnit::EEliteState::IDLE:
			cout << "IDLE" << endl;
			break;
		case Client::CEliteUnit::EEliteState::WALK:
			cout << "WALK" << endl;
			break;
		case Client::CEliteUnit::EEliteState::RUN:
			cout << "RUN" << endl;
			break;
		case Client::CEliteUnit::EEliteState::TURN:
			cout << "TURN" << endl;
			break;
		case Client::CEliteUnit::EEliteState::ATTACK:
			cout << "ATTACK" << endl;
			break;
		case Client::CEliteUnit::EEliteState::GROGGY:
			cout << "GROGGY" << endl;
			break;
		case Client::CEliteUnit::EEliteState::PARALYZATION:
			break;
		case Client::CEliteUnit::EEliteState::FATAL:
			break;
		case Client::CEliteUnit::EEliteState::DEAD:
			break;
		case Client::CEliteUnit::EEliteState::CUTSCENE:
			break;
		case Client::CEliteUnit::EEliteState::NONE:
			break;
		default:
			break;
		}
	}
#endif // _DEBUG

	if (m_bDead)
		m_pHPBar->Set_bDead();

	if (nullptr != m_pHPBar)
		m_pHPBar->Priority_Update(fTimeDelta);

	//if (m_strStateName.find("Dead") != m_strStateName.npos)
	//{
	//	if (m_pAnimator->IsFinished())
	//	{
	//		m_pWeapon->Set_bDead();
	//	}
	//}

}

void CElite_Police::Update(_float fTimeDelta)
{
	if (CalculateCurrentHpRatio() <= 0.f)
	{
		m_pAnimator->SetTrigger("Die");
		m_bUseLockon = false;
		Safe_Release(m_pHPBar);
	}
	__super::Update(fTimeDelta);

	if (nullptr != m_pHPBar)
		m_pHPBar->Update(fTimeDelta);
}

void CElite_Police::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
	if (nullptr != m_pHPBar)
		m_pHPBar->Late_Update(fTimeDelta);
}

HRESULT CElite_Police::Ready_Components(void* pArg)
{
	if (FAILED(__super::Ready_Components(pArg)))
		return E_FAIL;
	return S_OK;
}


void CElite_Police::Ready_BoneInformation()
{

}


HRESULT CElite_Police::Ready_Weapon()
{
 	CWeapon_Monster::MONSTER_WEAPON_DESC Desc{};
	Desc.eMeshLevelID = LEVEL::STATIC;
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.InitPos = { 0.125f, 0.f, 0.f };
	Desc.InitScale = {1.f,0.5f,1.f };
	Desc.iRender = 0;

	Desc.szMeshID = TEXT("Elite_Police_Weapon");
	lstrcpy(Desc.szName, TEXT("Elite_Police_Weapon"));
	Desc.vAxis = { 1.f,0.f,0.f,0.f };
	Desc.fRotationDegree = { -90.f };
	Desc.vLocalOffset = { -0.5f,0.f,0.f,1.f };
	Desc.vPhsyxExtent = { 0.4f, 0.2f, 0.2f };
	Desc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-R-Hand"));
	Desc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	Desc.pOwner = this;

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_Weapon"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Monster_Elite_Weapon"), &pGameObject, &Desc)))
		return E_FAIL;

	m_pWeapon = dynamic_cast<CWeapon_Monster*>(pGameObject);

	Safe_AddRef(m_pWeapon);


	return S_OK;
}


void CElite_Police::HandleMovementDecision(_float fDistance, _float fTimeDelta)
{
	if (m_bSpawned == false)
		return;

	if (fDistance < m_fTooCloseDistance) 
	{
		m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::BACK));
	}
	else if (fDistance <= m_fChasingDistance) // 공격 범위 근처
	{
		_vector vLook = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
		_vector vToPlayer = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) - m_pTransformCom->Get_State(STATE::POSITION);
		vToPlayer = XMVector3Normalize(XMVectorSetY(vToPlayer, 0.f));

		// 서로 락온이라서 외적으로 
		_vector vCross = XMVector3Cross(vLook, vToPlayer);
		_float fCrossY = XMVectorGetY(vCross);

		if (m_fChangeMoveDirCooldown <= 0.f)
		{
			if (fCrossY > 0.f) // 플레이어가 오른쪽에 있음
				m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::RIGHT));
			else               // 플레이어가 왼쪽에 있음
				m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::LEFT));

			m_fChangeMoveDirCooldown = 1.5f;
		}
		else
		{
			m_fChangeMoveDirCooldown -= fTimeDelta;
		}
	}
	else
	{
		if (m_fChangeMoveDirCooldown <= 0.f)
		{
			m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::FRONT));
			m_fChangeMoveDirCooldown = 0.5f;
		}
		else
		{
			m_fChangeMoveDirCooldown -= fTimeDelta;
		}
	}

	m_eCurrentState = EEliteState::WALK;
	m_pAnimator->SetFloat("Distance", abs(fDistance));
}

void CElite_Police::UpdateAttackPattern(_float fDistance, _float fTimeDelta)
{
	if (m_bPlayedDetect == false)
		return;

	if (fDistance < ATTACK_DISTANCE_CLOSE || fDistance > ATTACK_DISTANCE_MIDDLE)
		return; // 너무 가깝거나 너무 멀면 공격 안 함
	//if (m_bIsFirstAttack)
	//{
	//	m_pAnimator->SetTrigger("Attack");
	//	m_pAnimator->SetInt("AttackType", COMBO3);
	//	m_bIsFirstAttack = false;
	//	m_pAnimator->SetBool("Move", false);
	//	m_fAttackCooldown = m_fAttckDleay;
	//	SetTurnTimeDuringAttack(1.5f, 1.4f);
	//	return;
	//}

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


	if (IsTargetInFront(170.f) == false)
	{

		m_pAnimator->SetBool("Move", false);
		m_pAnimator->SetInt("AttackType", COMBO4);
		m_pAnimator->SetTrigger("Attack");
		m_ePrevState = m_eCurrentState;
		m_eCurrentState = EEliteState::ATTACK;
		m_fAttackCooldown = m_fAttckDleay;
		return;
	}
	EPoliceAttackPattern eAttackType = static_cast<EPoliceAttackPattern>(GetRandomAttackPattern(fDistance));



	SetupAttackByType(eAttackType);

	m_pAnimator->SetBool("Move", false);
	m_pAnimator->SetInt("AttackType", eAttackType);
	m_pAnimator->SetTrigger("Attack");
	m_ePrevState = m_eCurrentState;
	m_eCurrentState = EEliteState::ATTACK;
	m_fAttackCooldown = m_fAttckDleay;
	m_fChangeMoveDirCooldown = 0.f;
}

void CElite_Police::UpdateStateByNodeID(_uint iNodeID)
{
	m_ePrevState = m_eCurrentState;
	static _int iLastNodeID = -1;
	switch (iNodeID)
	{
	case ENUM_CLASS(EliteMonsterStateID::Idle):
		m_eCurrentState = EEliteState::IDLE;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Walk_B):
	case ENUM_CLASS(EliteMonsterStateID::Walk_F):
	case ENUM_CLASS(EliteMonsterStateID::Walk_R):
	case ENUM_CLASS(EliteMonsterStateID::Walk_L):
	{
		m_pTransformCom->SetfSpeedPerSec(m_fWalkSpeed);
		m_eCurrentState = EEliteState::WALK;
		break;
	}
	case ENUM_CLASS(EliteMonsterStateID::Run_F):
		m_pTransformCom->SetfSpeedPerSec(m_fRunSpeed);
		m_eCurrentState = EEliteState::RUN;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Atk_Combo1):
	case ENUM_CLASS(EliteMonsterStateID::Atk_Combo2):
	case ENUM_CLASS(EliteMonsterStateID::Atk_Combo3):
		m_eCurrentState = EEliteState::ATTACK;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Atk_Combo4):
		m_eCurrentState = EEliteState::ATTACK;

		break;
	case ENUM_CLASS(EliteMonsterStateID::Death_B):
		m_eCurrentState = EEliteState::DEAD;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Turn_L):
	case ENUM_CLASS(EliteMonsterStateID::Turn_R):
		m_eCurrentState = EEliteState::TURN;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Groggy_Start):
	case ENUM_CLASS(EliteMonsterStateID::Groggy_Loop):
	case ENUM_CLASS(EliteMonsterStateID::Groggy_End):
		m_eCurrentState = EEliteState::GROGGY;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Fatal_Hit_Start):
	case ENUM_CLASS(EliteMonsterStateID::Fatal_Hit_Ing):
	case ENUM_CLASS(EliteMonsterStateID::Fatal_Hit_End):
		m_eCurrentState = EEliteState::FATAL;
		break;
	case ENUM_CLASS(EliteMonsterStateID::Paralyzation_Start):
	case ENUM_CLASS(EliteMonsterStateID::Paralyzation_Loop):
	case ENUM_CLASS(EliteMonsterStateID::Paralyzation_End):
		m_eCurrentState = EEliteState::PARALYZATION;
		break;
	default:
		break;
	}
	iLastNodeID = iNodeID;
}

void CElite_Police::UpdateSpecificBehavior()
{
	if (m_pPlayer)
	{
		if (m_bPlayedDetect == false && Get_DistanceToPlayer() <= m_fDetectRange)
		{
			m_bPlayedDetect = true;
			m_pAnimator->SetTrigger("Detect");
		}

		if (m_eCurrentState == EEliteState::RUN || m_eCurrentState == EEliteState::WALK && m_eCurrentState != EEliteState::ATTACK && m_eCurrentState != EEliteState::TURN)
		{
			m_pTransformCom->LookAtWithOutY(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION));
		}

		//if (Get_DistanceToPlayer() <= m_fChasingDistance)
		//{
		//	m_pAnimator->SetBool("Move", false);
		//	m_ePrevState = m_eCurrentState;
		//	m_eCurrentState = EEliteState::IDLE;
		//}
	}
}

void CElite_Police::EnableColliders(_bool bEnable)
{
	__super::EnableColliders(bEnable);
}

void CElite_Police::Ready_EffectNames()
{
}

void CElite_Police::ProcessingEffects(const _wstring& stEffectTag)
{
}

HRESULT CElite_Police::EffectSpawn_Active(_int iPattern, _bool bActive, _bool bIsOnce)
{
	return S_OK;
}

HRESULT CElite_Police::Spawn_Effect()
{
	return S_OK;
}

HRESULT CElite_Police::Ready_Effect()
{
	return S_OK;
}

void CElite_Police::Register_Events()
{
	__super::Register_Events();

	m_pAnimator->RegisterEventListener("SetRootStep", [this]()
		{
			m_fMaxRootMotionSpeed = 1.f;
		});

	m_pAnimator->RegisterEventListener("ResetRootStep", [this]()
		{
			m_fMaxRootMotionSpeed = 30.f;
			m_fRootMotionAddtiveScale = 1.f;
		});

	m_pAnimator->RegisterEventListener("SetRootLargeStep", [this]()
		{
			m_fMaxRootMotionSpeed = 60.f;
			m_fRootMotionAddtiveScale = 1.5f;
		});

	m_pAnimator->RegisterEventListener("Spawned", [this]()
		{
			m_bSpawned = true;
		});
	m_pAnimator->RegisterEventListener("Turnnig", [this]()
		{
			SetTurnTimeDuringAttack(2.f, 1.4f);
		});

	m_pAnimator->RegisterEventListener("LargeTurnnig", [this]()
		{
			_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
			vLook = XMVectorSetY(vLook, 0.f);
			vLook = XMVector3Normalize(vLook);

			// 반대 방향
			_vector vDir = -vLook;
			m_pTransformCom->RotateToDirectionImmediately(vDir);
		});

}

void CElite_Police::Reset()
{
	__super::Reset();
	m_bPlayedDetect = false;
	m_bSpawned = false;
}

_int CElite_Police::GetRandomAttackPattern(_float fDistance)
{
	EPoliceAttackPattern ePattern = AP_NONE;
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
			ePattern = static_cast<EPoliceAttackPattern>(pattern);
			m_ePrevAttackPattern = m_eCurAttackPattern;
			m_eCurAttackPattern = ePattern;
			UpdatePatternWeight(ePattern);
			break;
		}
	}
	return ePattern;
}

void CElite_Police::UpdatePatternWeight(_int iPattern)
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

_bool CElite_Police::CanMove() const
{
	return (m_eCurrentState == EEliteState::IDLE ||
		m_eCurrentState == EEliteState::WALK ||
		m_eCurrentState == EEliteState::RUN) &&
		m_eCurrentState != EEliteState::GROGGY &&
		m_eCurrentState != EEliteState::DEAD &&
		m_eCurrentState != EEliteState::PARALYZATION &&
		m_eCurrentState != EEliteState::ATTACK &&
		m_eCurrentState != EEliteState::FATAL &&
		m_bPlayedDetect;
}



void CElite_Police::ChosePatternWeightByDistance(_float fDistance)
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
	else if (fDistance >= ATTACK_DISTANCE_MIDDLE && fDistance)
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
}

void CElite_Police::SetupAttackByType(_int iPattern)
{
	switch (iPattern)
	{
	case COMBO1:
		break;
	case COMBO2:
		break;
	case COMBO3:
		break;
	case COMBO4:
		break;
	default:
		break;
	}
}

void CElite_Police::Ready_AttackPatternWeight()
{
	m_PatternWeightMap.clear();
	m_PatternCountMap.clear();
	vector<EPoliceAttackPattern> m_vecBossPatterns = {
			COMBO1, COMBO2, COMBO3, COMBO4,COMBO5
	};

	for (const auto& pattern : m_vecBossPatterns)
	{
		m_PatternWeightMap[pattern] = m_fBasePatternWeight;
		m_PatternCountMap[pattern] = 0;
	}
}

void CElite_Police::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CElite_Police::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CElite_Police::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CElite_Police::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}


CElite_Police* CElite_Police::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CElite_Police* pInstance = new CElite_Police(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CElite_Police");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CElite_Police::Clone(void* pArg)
{
	CElite_Police* pInstance = new CElite_Police(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CElite_Police");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CElite_Police::Free()
{
	__super::Free();

	Safe_Release(m_pWeapon);

}
