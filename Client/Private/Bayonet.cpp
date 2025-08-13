#include "Bayonet.h"

#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "PhysX_IgnoreSelfCallback.h"


CBayonet::CBayonet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CWeapon(pDevice, pContext)
{
}
CBayonet::CBayonet(const CBayonet& Prototype)
	: CWeapon(Prototype)
{
}
HRESULT CBayonet::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBayonet::Initialize(void* pArg)
{
	BAYONET_DESC* pDesc = static_cast<BAYONET_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	/* [ 바이오닛 위치 셋팅 ] */
	m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(90.f));
	m_pTransformCom->Scaling(_float3{0.4f,0.4f,0.4f});

	// 스킬 정보 세팅

	m_eSkillDesc[0].iManaCost = 300;
	m_eSkillDesc[0].iSkillType = 0;
	m_eSkillDesc[0].iCountCombo = 3;
	m_eSkillDesc[0].isCombo = true;

	m_eSkillDesc[1].iManaCost = 100;
	m_eSkillDesc[1].iSkillType = 1;
	m_eSkillDesc[1].iCountCombo = 0;
	m_eSkillDesc[1].isCombo = false;

	m_iDurability = m_iMaxDurability;

	if (FAILED(Ready_Actor()))
		return E_FAIL;

	return S_OK;
}

void CBayonet::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}
void CBayonet::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	
}

void CBayonet::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	//Update_Collider();
}

HRESULT CBayonet::Render()
{
	__super::Render();

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif

	return S_OK;
}

void CBayonet::Update_Collider()
{
	if (!m_isActive)
		return;

	// 1. 월드 행렬 가져오기
	_matrix worldMatrix = XMLoadFloat4x4(&m_CombinedWorldMatrix);

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
}

HRESULT CBayonet::Ready_Components()
{
	/* [ 따로 추가할 컴포넌트가 있습니까? ] */

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CBayonet::Ready_Actor()
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

	PxVec3 halfExtents = PxVec3(0.2f, 0.2f, 0.2f);
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_PLAYERWEAPON; 
	filterData.word1 = WORLDFILTER::FILTER_MONSTERBODY | FILTER_MONSTERWEAPON; // 일단 보류
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::PLAYER_WEAPON);
	m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

void CBayonet::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	// 내구도나 이런거 하면 될듯?
}

void CBayonet::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBayonet::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBayonet::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBayonet::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBayonet::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

CBayonet* CBayonet::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBayonet* pInstance = new CBayonet(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBayonet");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CBayonet::Clone(void* pArg)
{
	CBayonet* pInstance = new CBayonet(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBayonet");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBayonet::Free()
{
	__super::Free();

	Safe_Release(m_pPhysXActorCom);
}
