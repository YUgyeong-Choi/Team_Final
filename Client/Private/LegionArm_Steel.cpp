#include "LegionArm_Steel.h"
#include "GameInstance.h"

#include "EffectContainer.h"
#include "Effect_Manager.h"
#include "Player.h"

CLegionArm_Steel::CLegionArm_Steel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CLegionArm_Base{pDevice, pContext}
{
}

CLegionArm_Steel::CLegionArm_Steel(const CLegionArm_Steel& Prototype)
	:CLegionArm_Base(Prototype)
{
}

HRESULT CLegionArm_Steel::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLegionArm_Steel::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Actor()))
		return E_FAIL;

	m_szMeshID = L"LegionArm_Steel";
	
	m_isAttack = true;

	m_fDamage = 100.f;

	m_fBaseDamage = m_fDamage;
	//


	return S_OK;
}

void CLegionArm_Steel::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_bHitRegActive)
		Update_HitReg(fTimeDelta);
}

void CLegionArm_Steel::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLegionArm_Steel::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	Update_Collider();

#ifdef _DEBUG
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
#endif
}

HRESULT CLegionArm_Steel::Render()
{


#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider() && m_pActorCom->Get_ReadyForDebugDraw()) {
		m_pGameInstance->Add_DebugComponent(m_pActorCom);
	}
#endif

	return S_OK;
}


void CLegionArm_Steel::Activate()
{

	m_isAttack = !m_isAttack;

}

void CLegionArm_Steel::Update_Collider()
{
	
	_float4 vlocalOffset =  {0.f,0.f,0.f,1.f};
	_vector localOffset = XMLoadFloat4(&vlocalOffset);
	_vector worldPos = XMVector4Transform(localOffset, XMLoadFloat4x4(&m_CombinedWorldMatrix));

	_vector finalRot = XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_CombinedWorldMatrix));

	PxVec3 physxPos(XMVectorGetX(worldPos), XMVectorGetY(worldPos), XMVectorGetZ(worldPos));
	PxQuat physxRot(XMVectorGetX(finalRot), XMVectorGetY(finalRot), XMVectorGetZ(finalRot), XMVectorGetW(finalRot));

	m_pActorCom->Set_Transform(PxTransform(physxPos, physxRot));
}

void CLegionArm_Steel::SetisAttack(_bool isAttack)
{
	if (isAttack)
	{
		m_pActorCom->Set_SimulationFilterData(m_pActorCom->Get_FilterData());

	}
	else
	{
		m_pActorCom->Init_SimulationFilterData();
	}
}

void CLegionArm_Steel::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (eColliderType == COLLIDERTYPE::MONSTER)
	{
		// 히트랙
		if (static_cast<CPlayer*>(m_pOwner)->GetAnimCategory() == CPlayer::eAnimCategory::ARM_ATTACKCHARGE)
			StartHitReg(0.01f, 0.075f, 0.075f);

		// m_pOwner 설정 후 주석 풀 것 
		_vector vPlayerPos = XMVectorSetY(m_pOwner->Get_TransfomCom()->Get_State(STATE::POSITION), 0.f);
		_vector vOtherPos = XMVectorSetY(pOther->Get_TransfomCom()->Get_State(STATE::POSITION), 0.f);
		_vector vDir = XMVector3Normalize(vPlayerPos - vOtherPos);

		CUnit* pUnit = static_cast<CUnit*>(pOther);

		auto& vLockonPos = pUnit->Get_LockonPos();
		_float3 vModifiedPos = _float3(vLockonPos.x + vDir.m128_f32[0], vLockonPos.y + vDir.m128_f32[1], vLockonPos.z + vDir.m128_f32[2]);

		_vector vLook = XMVector3Normalize(vDir);           // Look
		_vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f); // 고정 Up
		_vector vRight = XMVector3Normalize(XMVector3Cross(vWorldUp, vLook));
		_vector vUp = XMVector3Cross(vLook, vRight);

		_matrix mAlign = XMMATRIX(vRight, vUp, vLook, XMVectorSet(vModifiedPos.x, vModifiedPos.y, vModifiedPos.z, 1.f));

		CEffectContainer::DESC desc = {};

		CGameObject* pEffect = { nullptr };
		CPlayer::eAnimCategory eCategory = dynamic_cast<CPlayer*>(m_pOwner)->GetAnimCategory();

		// 막타일 때 
		if (pUnit->GetHP() <= 0.f)
		{
			XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixScaling(2.f, 2.f, 2.f) * mAlign);
			if (MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_AttackHit_Basic_Spark_1_P2S4"), &desc) == nullptr)
				return;
		}
		else
		{
			XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixScaling(2.f, 2.f, 2.f) * mAlign);
			if (MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_AttackHit_LeftArm_Spark_1_P2S1"), &desc) == nullptr)
				return;
		}

		if (pEffect == nullptr)
			return;

	

	}

	return ;

}

HRESULT CLegionArm_Steel::Ready_Actor()
{

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pActorCom))))
		return E_FAIL;

	// 3. Transform에서 S, R, T 분리
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 3-1. 스케일, 회전, 위치 변환
	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);
	
	PxVec3 halfExtents = PxVec3(0.3f, 0.3f, 0.3f);
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pActorCom->Set_ShapeFlag(false, true, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_PLAYERWEAPON;
	filterData.word1 = WORLDFILTER::FILTER_MONSTERBODY | FILTER_MONSTERWEAPON; // 일단 보류
	m_pActorCom->Set_SimulationFilterData(filterData);
	m_pActorCom->Set_QueryFilterData(filterData);
	m_pActorCom->Set_Owner(this);
	m_pActorCom->Set_ColliderType(COLLIDERTYPE::PLAYER_WEAPON);
	m_pActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pActorCom->Get_Actor());

	return S_OK;
}

CLegionArm_Steel* CLegionArm_Steel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLegionArm_Steel* pInstance = new CLegionArm_Steel(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLegionArm_Steel");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CLegionArm_Steel::Clone(void* pArg)
{
	CLegionArm_Steel* pInstance = new CLegionArm_Steel(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLegionArm_Steel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CLegionArm_Steel::Free()
{

	__super::Free();

	Safe_Release(m_pActorCom);
}
