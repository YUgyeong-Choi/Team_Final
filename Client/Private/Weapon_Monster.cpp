#include "Weapon_Monster.h"

#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "PhysX_IgnoreSelfCallback.h"
#include "Unit.h"
#include "EffectContainer.h"
#include "Effect_Manager.h"

CWeapon_Monster::CWeapon_Monster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CWeapon(pDevice, pContext)
{
}
CWeapon_Monster::CWeapon_Monster(const CWeapon_Monster& Prototype)
	: CWeapon(Prototype)
{
}
HRESULT CWeapon_Monster::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWeapon_Monster::Initialize(void* pArg)
{
	/* [ 데미지 설정 ] */
	m_fDamage = 18.f;

	MONSTER_WEAPON_DESC* pDesc = static_cast<MONSTER_WEAPON_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	_matrix MatHandle = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("BN_Handle")));

	m_pTransformCom->Set_WorldMatrix(MatHandle);
	
	m_pTransformCom->Rotation(XMLoadFloat4(&pDesc->vAxis), XMConvertToRadians(pDesc->fRotationDegree));

	m_vLocalOffset = pDesc->vLocalOffset;

	m_physxExtent = pDesc->vPhsyxExtent;

	// offset
	m_pTransformCom->Scaling(pDesc->InitScale);
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

	vPos += XMVectorSetW(XMLoadFloat3(&pDesc->InitPos), 0.f);
	 
	m_pTransformCom->Set_State(STATE::POSITION, vPos);

	m_fDurability = m_fMaxDurability;

	m_bIsActive = true;

	if (FAILED(Ready_Actor()))
		return E_FAIL;

	return S_OK;
}

void CWeapon_Monster::Priority_Update(_float fTimeDelta)
{
	

	__super::Priority_Update(fTimeDelta);


	

}
void CWeapon_Monster::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	

}

void CWeapon_Monster::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	Update_Collider();
}

HRESULT CWeapon_Monster::Render()
{
	__super::Render();


#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider() && m_pPhysXActorCom->Get_ReadyForDebugDraw()) {
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif



	return S_OK;
}

void CWeapon_Monster::Update_Collider()
{
	if (!m_isActive)
		return;

	// 1. 부모 행렬
	_matrix ParentWorld = {};  


	ParentWorld = XMLoadFloat4x4(m_pParentWorldMatrix);;
	

	// 2. Socket 월드 행렬 
	_matrix SocketMat = XMLoadFloat4x4(m_pSocketMatrix);

	for (size_t i = 0; i < 3; i++)
		SocketMat.r[i] = XMVector3Normalize(SocketMat.r[i]);

	// 3. Blade 본 월드 행렬
	_matrix HandleMat = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(m_iHandleIndex));

	for (size_t i = 0; i < 3; i++)
		HandleMat.r[i] = XMVector3Normalize(HandleMat.r[i]);

	_matrix WeaponWorld = HandleMat * SocketMat * ParentWorld;



	_vector localOffset = XMLoadFloat4(&m_vLocalOffset);
	_vector worldPos = XMVector4Transform(localOffset, WeaponWorld);

	// 6. PhysX 적용
	_vector finalPos = WeaponWorld.r[3];

	_vector finalRot = XMQuaternionRotationMatrix(WeaponWorld);

	PxVec3 physxPos(XMVectorGetX(worldPos), XMVectorGetY(worldPos), XMVectorGetZ(worldPos));
	PxQuat physxRot(XMVectorGetX(finalRot), XMVectorGetY(finalRot), XMVectorGetZ(finalRot), XMVectorGetW(finalRot));

	m_pPhysXActorCom->Set_Transform(PxTransform(physxPos, physxRot));
}

void CWeapon_Monster::Collider_FilterOff()
{
	m_pPhysXActorCom->Init_SimulationFilterData();
}

void CWeapon_Monster::Collider_ShapeOff()
{
	m_pPhysXActorCom->Set_ShapeFlag(false, false, true);
}

void CWeapon_Monster::SetisAttack(_bool isAttack)
{
	if (isAttack)
	{
		m_pPhysXActorCom->Set_SimulationFilterData(m_pPhysXActorCom->Get_FilterData());
	}
	else
	{
		m_pPhysXActorCom->Init_SimulationFilterData();
	}
}

//void CWeapon_Monster::Gravity_On()
//{
//	m_pPhysXActorCom->Set_Kinematic(false);
//}

HRESULT CWeapon_Monster::Ready_Components()
{
	/* [ 따로 추가할 컴포넌트가 있습니까? ] */

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CWeapon_Monster::Ready_Actor()
{
	// 3. Transform에서 S, R, T 분리
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 3-1. 스케일, 회전, 위치 변환
	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(m_physxExtent);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(false, true, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_MONSTERWEAPON;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY; 
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::MONSTER_WEAPON);
	m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

void CWeapon_Monster::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	if (eColliderType == COLLIDERTYPE::PLAYER)
	{
		_vector vDir = XMVector3Normalize(m_pOwner->Get_TransfomCom()->Get_State(STATE::POSITION) - pOther->Get_TransfomCom()->Get_State(STATE::POSITION));

		CUnit* pUnit = static_cast<CUnit*>(pOther);
		auto& vLockonPos = pUnit->Get_LockonPos();
		_float3 vModifiedPos = _float3(vLockonPos.x + vDir.m128_f32[0], vLockonPos.y + vDir.m128_f32[1], vLockonPos.z + vDir.m128_f32[2]);

		CEffectContainer::DESC desc = {};

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixScaling(2.f, 2.f, 2.f) * XMMatrixTranslation(vModifiedPos.x, vModifiedPos.y, vModifiedPos.z));

		CGameObject* pEffect = { nullptr };
		pEffect = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_PlayerHit_Basic_Spark_1_P1S3"), &desc);

		if (pEffect == nullptr)
			MSG_BOX("이펙트 생성 실패함");
	}
}

void CWeapon_Monster::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CWeapon_Monster::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CWeapon_Monster::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CWeapon_Monster::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (eColliderType == COLLIDERTYPE::PLAYER)
	{
	}
	else if (eColliderType == COLLIDERTYPE::PLAYER_WEAPON)
	{
	}
}

void CWeapon_Monster::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

CWeapon_Monster* CWeapon_Monster::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWeapon_Monster* pInstance = new CWeapon_Monster(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWeapon_Monster");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CWeapon_Monster::Clone(void* pArg)
{
	CWeapon_Monster* pInstance = new CWeapon_Monster(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CWeapon_Monster");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CWeapon_Monster::Free()
{
	__super::Free();

	Safe_Release(m_pPhysXActorCom);
}
