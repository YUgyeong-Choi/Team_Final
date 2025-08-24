#include "Oil.h"
#include "Player.h"
#include "FireBall.h"
#include "PhysXDynamicActor.h"
#include "Client_Calculation.h"
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
	m_fDamge = 5.f; // 터졌을 때를 위해서


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
		filterData.word1 = WORLDFILTER::FILTER_MAP;
		m_pPhysXActorCom->Set_SimulationFilterData(filterData);
		m_pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Object(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"), 0));
		if (m_pPlayer)
		{
			m_pPlayer->Get_Controller()->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
		}
	}

	return S_OK;
}

void COil::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_bCanSpread&&!m_bIsSpreaded)
	{
		m_pPhysXActorCom->ReCreate_Shape(m_pPhysXActorCom->Get_Actor(), m_SpreadOilShape);
		m_pPhysXActorCom->Set_Kinematic(true);
		m_pPhysXActorCom->Set_ShapeFlag(false, true, false);
		m_bIsSpreaded = true;
		_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
		// 오일이 퍼지면 위치를 바꿔야함
		_float fRadius = GetRandomFloat(0.5f, 4.0f); // 반경
		_float fAngle = GetRandomFloat(0.f, XM_2PI); // 랜덤 각도
		_float fX = cosf(fAngle) * fRadius;
		_float fZ = sinf(fAngle) * fRadius;

		_vector vSpreadPos = vPos + XMVectorSet(fX, 0.f, fZ, 0.f);

		m_pTransformCom->Set_State(STATE::POSITION, vSpreadPos);

		PxTransform pose = m_pPhysXActorCom->Get_Actor()->getGlobalPose();
		pose.p = PxVec3(XMVectorGetX(vSpreadPos),
			XMVectorGetY(vPos),  
			XMVectorGetZ(vSpreadPos));
		m_pPhysXActorCom->Get_Actor()->setGlobalPose(pose);
		PxFilterData filterData{};
		filterData.word0 = WORLDFILTER::FILTER_MONSTERWEAPON;
		filterData.word1 = WORLDFILTER::FILTER_MAP | WORLDFILTER::FILTER_MONSTERWEAPON;
		m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	
	}
}

void COil::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void COil::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT COil::Render()
{
    return __super::Render();
}

void COil::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (eColliderType == COLLIDERTYPE::ENVIRONMENT_CONVEX || eColliderType == COLLIDERTYPE::ENVIRONMENT_TRI)
	{
		m_bCanSpread = true;
	}
}

void COil::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void COil::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void COil::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void COil::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (m_bIsSpreaded)
	{
		// 나중에 터질 때 처리(플레이어로 테스트 처리, 나중에 화염 발사랑 만나면)
		if (m_pPlayer && eColliderType == COLLIDERTYPE::MONSTER_WEAPON && dynamic_cast<CFireBall*>(pOther))
		{
			// 화염 처리만 히트 주고
			// 사각형 충돌 계산
			_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
			_vector vOilPos = m_pTransformCom->Get_State(STATE::POSITION);
			_float fDist = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPlayerPos, vOilPos)));
			// 내 콜라이더의 너비
			_float fColliderWidth =2.f;
			if (fDist <= fColliderWidth)
			{
				m_pPlayer->ReceiveDamage(this, COLLIDERTYPE::MONSTER_WEAPON);
				m_pPlayer->Get_Animator()->SetTrigger("Hited");
				Set_bDead();
			}
		}
	}
}

void COil::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT COil::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;
	// 나중에 모델 불러오기
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
