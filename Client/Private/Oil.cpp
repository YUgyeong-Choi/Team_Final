#include "Oil.h"
#include "Player.h"
#include "FireBall.h"
#include "GameInstance.h"
#include "PhysXDynamicActor.h"
#include "Client_Calculation.h"
#include <FlameField.h>
#include "EffectContainer.h"
#include "Effect_Manager.h"
#include "Static_Decal.h"

COil::COil(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile(pDevice, pContext)
{
}

COil::COil(const COil& Prototype)
	: CProjectile(Prototype)
	, m_fDamge(Prototype.m_fDamge)
{
}

HRESULT COil::Initialize_Prototype()
{
	m_fDamge = 20.f; // 터졌을 때를 위해서
    return S_OK;
}

HRESULT COil::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_fLifeTime = 20.f; // 오일 시간 
	if (m_pPhysXActorCom)
	{
		PxFilterData filterData{};
		filterData.word0 = WORLDFILTER::FILTER_MONSTERWEAPON;
		filterData.word1 = WORLDFILTER::FILTER_MAP | WORLDFILTER::FILTER_FLOOR | WORLDFILTER::FILTER_FOOTSTEP
			| WORLDFILTER::FILTER_DYNAMICOBJ;
		m_pPhysXActorCom->Set_SimulationFilterData(filterData);
		m_pPhysXActorCom->Set_ShapeFlag(true, false, false);
		m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
	}

	_int iLevelIndex = m_pGameInstance->GetCurrentLevelIndex();
	m_pFuoco = m_pGameInstance->Get_LastObject(iLevelIndex, TEXT("Layer_Monster"));
	m_pSoundCom->Set3DState(0.f, 50.f);
	return S_OK;
}

void COil::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_bCanSpread&&!m_bIsSpreaded)
	{
		m_pPhysXActorCom->ReCreate_Shape(m_pPhysXActorCom->Get_Actor(), m_SpreadOilShape);
		m_pPhysXActorCom->Set_Kinematic(true);
		m_pPhysXActorCom->Set_ShapeFlag(true, false, false);
		m_bIsSpreaded = true;


		_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
		// 오일이 퍼지면 위치를 바꿔야함
		_float fRadius = GetRandomFloat(0.5f, 4.0f); // 반경
		_float fAngle = GetRandomFloat(0.f, XM_2PI); // 랜덤 각도
		_float fX = cosf(fAngle) * fRadius;
		_float fZ = sinf(fAngle) * fRadius;
		_float fY = 0.3f;

		const _float fGroundY = m_pFuoco ? XMVectorGetY(m_pFuoco->Get_TransfomCom()->Get_State(STATE::POSITION)) : fY;
		_vector vSpreadPos = XMVectorSet(
			XMVectorGetX(vPos) + fX,
			fGroundY,
			XMVectorGetZ(vPos) + fZ,
			1.f
		);


		m_pTransformCom->Set_State(STATE::POSITION, vSpreadPos);

		PxTransform pose = m_pPhysXActorCom->Get_Actor()->getGlobalPose();
		pose.p = PxVec3(XMVectorGetX(vSpreadPos),
			XMVectorGetY(vSpreadPos),
			XMVectorGetZ(vSpreadPos));
		pose.q = PxQuat(PxIdentity);
		m_pPhysXActorCom->Get_Actor()->setGlobalPose(pose);
		PxFilterData filterData{};
		filterData.word0 = WORLDFILTER::FILTER_MONSTERWEAPON;
		filterData.word1 = WORLDFILTER::FILTER_MONSTERWEAPON;
		m_pPhysXActorCom->Set_SimulationFilterData(filterData);
		_int iLevelIndex = m_pGameInstance->GetCurrentLevelIndex();
		auto pPlayer = GET_PLAYER(iLevelIndex);
		if (pPlayer)
		{
			pPlayer->Get_Controller()->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());	
			m_pPlayer = pPlayer;
		}
	
		Spawn_Decal(XMVectorSet(2.f, 0.5f, 2.f, 0));

	}
}

void COil::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (m_bSoundPlaying&& m_pSoundCom && !m_pSoundCom->IsPlaying("SE_NPC_SK_FX_FIre_Explo_Heavy_01"))
		
	{
		Set_bDead();
	}
}

void COil::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT COil::Render()
{
    return __super::Render();
}

void COil::Explode_Oil()
{
	if (m_bIsSpreaded)
	{
		if (m_pPlayer)
		{
			// 화염 처리만 히트 주고
			// 사각형 충돌 계산
			_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
			_vector vOilPos = m_pTransformCom->Get_State(STATE::POSITION);
			_float fDist = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPlayerPos, vOilPos)));
			// 내 콜라이더의 너비
			_float fColliderWidth = 2.f;
			if (fDist <= fColliderWidth)
			{
				_int iLevelIndex = m_pGameInstance->GetCurrentLevelIndex();
			
				// 살짝만 날라가게 나중에 처리하기
				m_pPlayer->SetHitMotion(HITMOTION::UP);
				m_pPlayer->SetElementTypeWeight(EELEMENT::FIRE, 1.f);
			}

			// 이펙트 생성

			CEffectContainer::DESC Desc = {};  


			CEffectContainer* pEffect = { nullptr };
			XMStoreFloat4x4(&Desc.PresetMatrix, m_pTransformCom->Get_WorldMatrix());
		

			pEffect = static_cast<CEffectContainer*>(MAKE_EFFECT(m_pGameInstance->GetCurrentLevelIndex(), TEXT("EC_GL_Explosion"), &Desc));

			m_pSoundCom->Play("SE_NPC_SK_FX_FIre_Explo_Heavy_01");
			m_bSoundPlaying = true;

		}
	}
}

HRESULT COil::Spawn_Decal(_fvector vDecalScale)
{
//	//푸오코 기름 데칼
///* For.Prototype_Component_Texture_FireEater_Oil_ARMT*/
//	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Texture_FireEater_Oil_ARMT"),
//		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_BloodSpot_38_ARMT.dds")))))
//		return E_FAIL;
//
//	/* For.Prototype_Component_Texture_FireEater_Oil_N*/
//	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Texture_FireEater_Oil_N"),
//		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_BloodClip_01_N_KMH.dds")))))
//		return E_FAIL;
//
//	/* For.Prototype_Component_Texture_FireEater_Oil_BC*/
//	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Texture_FireEater_Oil_BC"),
//		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Decal/T_Decal_Dust_01_BC.dds")))))
//		return E_FAIL;

#pragma region 영웅 데칼 생성코드
	CStatic_Decal::DECAL_DESC DecalDesc = {};
	DecalDesc.iLevelID = ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION);
	DecalDesc.PrototypeTag[ENUM_CLASS(CStatic_Decal::TEXTURE_TYPE::ARMT)] = TEXT("Prototype_Component_Texture_FireEater_Oil_ARMT");
	DecalDesc.PrototypeTag[ENUM_CLASS(CStatic_Decal::TEXTURE_TYPE::N)] = TEXT("Prototype_Component_Texture_FireEater_Oil_N");
	DecalDesc.PrototypeTag[ENUM_CLASS(CStatic_Decal::TEXTURE_TYPE::BC)] = TEXT("Prototype_Component_Texture_FireEater_Oil_BC");

	DecalDesc.bHasLifeTime = true;
	DecalDesc.fLifeTime = 20.f;

	// 플레이어의 월드 행렬
	_matrix WorldMatrix = m_pTransformCom->Get_WorldMatrix();

	// 위치만 추출
	_vector vPosition = WorldMatrix.r[3]; // 행렬의 4번째 행(translation)

	// 위치 행렬 생성
	_matrix PosMatrix = XMMatrixTranslationFromVector(vPosition);

	// 데칼 스케일 적용
	_matrix ScaleMatrix = XMMatrixScaling(
		XMVectorGetX(vDecalScale),
		XMVectorGetY(vDecalScale),
		XMVectorGetZ(vDecalScale));

	// 최종 월드 행렬 (스케일 + 위치)
	XMStoreFloat4x4(&DecalDesc.WorldMatrix, ScaleMatrix * PosMatrix);


	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_GameObject_Static_Decal"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Static_Decal"), &DecalDesc)))
	{
		return E_FAIL;
	}
#pragma endregion

	return S_OK;
}

void COil::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void COil::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (eColliderType == COLLIDERTYPE::ENVIRONMENT_CONVEX || eColliderType == COLLIDERTYPE::ENVIRONMENT_TRI
		|| eColliderType == COLLIDERTYPE::BREAKABLE_OBJECT || eColliderType == COLLIDERTYPE::PLAYER)
	{
		if (m_bIsSpreaded == false)
		{
			m_bCanSpread = true;
			m_pSoundCom->Play_Random("SE_NPC_Boss_Fire_Eater_SK_PJ_Oil_Hit_", 3);
		}
	}
}

void COil::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void COil::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void COil::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	//if (m_bIsSpreaded)
	//{
	//	if (m_pPlayer && eColliderType == COLLIDERTYPE::BOSS_WEAPON && dynamic_cast<CFlameField*>(pOther))
	//	{
	//		// 화염 처리만 히트 주고
	//		// 사각형 충돌 계산
	//		_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
	//		_vector vOilPos = m_pTransformCom->Get_State(STATE::POSITION);
	//		_float fDist = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPlayerPos, vOilPos)));
	//		_float fColliderWidth = 2.f;
	//		if (fDist <= fColliderWidth)
	//		{
	//			_int iLevelIndex = m_pGameInstance->GetCurrentLevelIndex();
	//			auto pPlayer = GET_PLAYER(iLevelIndex);
	//			if (pPlayer)
	//			{
	//				pPlayer->SetHitMotion(HITMOTION::NORMAL);
	//			}
	//			Set_bDead();
	//		}
	//	}
	//}
}

void COil::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT COil::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;

	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_FireEater"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
		return E_FAIL;
    return S_OK;
}


HRESULT COil::Ready_Effect()
{
	//CEffectContainer::DESC desc = {};
	//desc.pSocketMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	//XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixIdentity());
	//m_pEffect = dynamic_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_OilballProjectile_test_M1P1"), &desc));
	//if (nullptr == m_pEffect)
	//	MSG_BOX("이펙트 생성 실패함");

	return S_OK;
}

COil* COil::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	COil* pInstance = new COil(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : COil");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* COil::Clone(void* pArg)
{
	COil* pInstance = new COil(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : COil");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void COil::Free()
{
	__super::Free();

}
