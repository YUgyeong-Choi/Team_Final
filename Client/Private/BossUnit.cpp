#include "BossUnit.h"

#include "Player.h"
#include "UI_Manager.h"
#include "Static_Decal.h"
#include "GameInstance.h"
#include "UI_Container.h"
#include "SpringBoneSys.h"
#include "LockOn_Manager.h"
#include "SwordTrailEffect.h"
#include "Level_KratCentralStation.h"

CBossUnit::CBossUnit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEliteUnit(pDevice, pContext)
{
}

CBossUnit::CBossUnit(const CBossUnit& Prototype)
	: CEliteUnit(Prototype)
{
	m_eUnitType = EUnitType::BOSS;
}

HRESULT CBossUnit::Initialize(void* pArg)
{

	if (FAILED(__super::Initialize(pArg)))
 		return E_FAIL;

	m_fMaxHp = 1200.f;
	m_fHp = m_fMaxHp;


	m_pAnimator->Update(0.f);
	m_pModelCom->Update_Bones();
	m_pAnimator->SetPlaying(false);
	Ready_AttackPatternWeightForPhase1();
	m_fGroggyScale_Weak = 0.08f;
	m_fGroggyScale_Strong = 0.1f;
	m_fGroggyScale_Charge = 0.15f;
	m_ePrevState = EEliteState::CUTSCENE;
	m_eCurrentState = EEliteState::CUTSCENE;
	return S_OK;
}

void CBossUnit::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
	if (CalculateCurrentHpRatio() <= m_fPhase2HPThreshold && m_bIsPhase2 == false)
	{
		Ready_AttackPatternWeightForPhase2();
	}

	//if (KEY_DOWN(DIK_I))
	//{
	//	m_fHp -= 500.f;
	////	ReChallenge();
	//}
}

void CBossUnit::Update(_float fTimeDelta)
{
	if (CalculateCurrentHpRatio() <= 0.f)
	{
		// 죽음 처리
		m_fGroggyEndTimer = 0.f;
		m_fGroggyThreshold = 1.f;
		m_fGroggyGauge = 0.f;
		m_bGroggyActive = false;
		m_bUseLockon = false;
		if (!m_bDeathProcessed &&m_eCurrentState != EEliteState::DEAD && m_eCurrentState != EEliteState::FATAL)
		{
			m_bDeathProcessed = true;
			SetForDeath();
	

			if (auto pLevel = dynamic_cast<CLevel_KratCentralStation*>(m_pGameInstance->Get_CurrentLevel()))
			{
				pLevel->Apply_AreaBGM();
			}

		

			CUI_Container::UI_CONTAINER_DESC eDesc = {};

			eDesc.fDelay = 2.f;
			eDesc.strFilePath = TEXT("../Bin/Save/UI/Boss_Kill.json");

			eDesc.fLifeTime = 8.f;
			eDesc.useLifeTime = true;
			eDesc.strSoundTag = "SE_UI_AlertKill_02";

			if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"),
				ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster_UI_Death"), &eDesc)))
				return;

			_wstring strName = m_szMeshID;
			_wstring strFilePath = TEXT("../Bin/Save/UI/Popup/Boss_Drop_") + strName + TEXT(".json");
			eDesc.strFilePath = strFilePath; 
			eDesc.strSoundTag = {};

			if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"),
				ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Monster_UI_Death"), &eDesc)))
				return;

		
		}
		Safe_Release(m_pHPBar);
	}
	__super::Update(fTimeDelta);
	if (m_pSpringBoneSys)
	{
		m_pSpringBoneSys->Update(fTimeDelta);
	}
}

void CBossUnit::EnterCutScene()
{
	m_pAnimator->Get_CurrentAnimController()->SetStateToEntry();
	m_pAnimator->SetPlaying(true);
	m_bCutSceneOn = true;
	SwitchEmissive(true, 0.9f);
}

void CBossUnit::ReChallenge()
{
	m_eCurrentState = EEliteState::IDLE;
	m_ePrevState = EEliteState::IDLE;
	m_pAnimator->Get_CurrentAnimController()->SetState("Idle");
	m_pAnimator->SetPlaying(true);
	SwitchEmissive(true, 0.9f);
}

void CBossUnit::Reset()
{
	__super::Reset();
	// 재시작 때는 Idle 상태로 재도전이니까
	m_pAnimator->Get_CurrentAnimController()->SetState("Idle");
	m_bIsPhase2 = false;
	m_bStartPhase2 = false;
	m_bPlayerCollided = false;
	m_bCutSceneOn = false;
	m_pAnimator->Update(0.016f);
	m_pModelCom->Update_Bones();
	m_pAnimator->SetPlaying(false);
	Ready_AttackPatternWeightForPhase1();
	m_ActiveEffect.clear();
	m_pAnimator->SetPlayRate(1.f);
	m_fFirstChaseBeforeAttack = 2.5f;
	m_pAnimator->ResetTrigger("SpecialDie");
}

void CBossUnit::Ready_AttackPatternWeightForPhase1()
{
}

void CBossUnit::Ready_AttackPatternWeightForPhase2()
{
}

HRESULT CBossUnit::Spawn_Decal(CBone* pBone, const wstring& NormalTag, const wstring& MaskTag, _fvector vDecalScale)
{

#pragma region 영웅 데칼 생성코드
	CStatic_Decal::DECAL_DESC DecalDesc = {};
	DecalDesc.bNormalOnly = true;
	DecalDesc.iLevelID = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);
	DecalDesc.PrototypeTag[ENUM_CLASS(CStatic_Decal::TEXTURE_TYPE::N)] = NormalTag;
	DecalDesc.PrototypeTag[ENUM_CLASS(CStatic_Decal::TEXTURE_TYPE::MASK)] = MaskTag;
	DecalDesc.bHasLifeTime = true;
	DecalDesc.fLifeTime = 5.f;

	auto worldmat = XMLoadFloat4x4(pBone->Get_CombinedTransformationMatrix()) * m_pTransformCom->Get_WorldMatrix();

	// 기존 월드행렬
	_matrix World = XMLoadFloat4x4(pBone->Get_CombinedTransformationMatrix()) * XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr());

	// 스케일, 회전, 위치 분해
	_vector vScale, vRotQuat, vTrans;
	XMMatrixDecompose(&vScale, &vRotQuat, &vTrans, World);

	// 새 스케일 설정
	vScale = vDecalScale;

	// z파이팅 없애기 위해 스케일 y값 랜덤 조정 
	//_float fY = XMVectorGetY(vScale);
	//fY = m_pGameInstance->Compute_Random(fY * .9f, fY);
	//XMVectorSetY(vScale, fY);

	if (m_pNaviCom == nullptr)
		return E_FAIL;
	//네브메쉬 높이 값으로 변경
	_vector vNavPos = m_pNaviCom->SetUp_Height(m_pTransformCom->Get_State(STATE::POSITION));
	vTrans = XMVectorSetY(vTrans, XMVectorGetY(vNavPos));

	// Look 반대 방향
	_matrix ParentWorld = m_pTransformCom->Get_WorldMatrix();

	_vector vLook = XMVector3Normalize(-ParentWorld.r[2]);

	// Up은 기존 Up을 쓰고
	_vector vUp = XMVector3Normalize(ParentWorld.r[1]);

	// Right는 Look과 Up으로 다시 계산
	_vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));

	// Orthonormalize (보정)
	vUp = XMVector3Cross(vLook, vRight);

	// 회전 행렬 구성
	_matrix RotMat = XMMatrixIdentity();
	RotMat.r[0] = vRight;
	RotMat.r[1] = vUp;
	RotMat.r[2] = vLook;

	// 다시 합성
	_matrix NewWorld = XMMatrixScalingFromVector(vScale) *
		RotMat *
		XMMatrixTranslationFromVector(vTrans);

	// 적용
	XMStoreFloat4x4(&DecalDesc.WorldMatrix, NewWorld);

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Static_Decal"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Static_Decal"), &DecalDesc)))
	{
		return E_FAIL;
	}
#pragma endregion

	return S_OK;
}

void CBossUnit::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	__super::On_CollisionStay(pOther, eColliderType, HitPos, HitNormal);
}

void CBossUnit::SetForDeath()
{
	m_eCurrentState = EEliteState::DEAD;
	m_pAnimator->SetTrigger("SpecialDie");
	CLockOn_Manager::Get_Instance()->Set_Off(this);
	m_pAnimator->SetPlayRate(1.f);
	SwitchEmissive(false, 1.f);
	SwitchSecondEmissive(false, 1.f);
	SwitchFury(false, 1.f);
	m_ActiveEffect.clear();
	EnableColliders(false);
	if (m_pTrailEffect)
	{
		m_pTrailEffect->Set_TrailActive(false);
	}
}

CBossUnit* CBossUnit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBossUnit* pInstance = new CBossUnit(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBossUnit");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CBossUnit::Clone(void* pArg)
{
	CBossUnit* pInstance = new CBossUnit(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CBossUnit");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBossUnit::Free()
{
	__super::Free();
	Safe_Release(m_pSpringBoneSys);
}
