#include "Player.h"
#include "FlameField.h"
#include "GameInstance.h"
#include "PhysXDynamicActor.h"
#include "Client_Calculation.h"
#include <PhysX_IgnoreSelfCallback.h>
#include <Fuoco.h>

CFlameField::CFlameField(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{

}
CFlameField::CFlameField(const CFlameField& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CFlameField::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CFlameField::Initialize(void* pArg)
{

	FLAMEFIELD_DESC* pFlameFieldDesc = static_cast<FLAMEFIELD_DESC*>(pArg);
	pFlameFieldDesc->fRotationPerSec = XMConvertToRadians(140.f);
	pFlameFieldDesc->fSpeedPerSec = 0.f; // �Ҳ� �ʵ�� �������� ����
	lstrcpy(pFlameFieldDesc->szName, TEXT("FlameField"));
	m_fExpandRadius = pFlameFieldDesc->fExpandRadius; // �Ҳ��� Ȯ��Ǵ� ������
	m_fInitialRadius = pFlameFieldDesc->fInitialRadius; // �Ҳ��� �ʱ� ������
	m_fExpandTime = pFlameFieldDesc->fExpandTime; // �Ҳ��� Ȯ��Ǵ� �ð�
	if (FAILED(__super::Initialize(&pArg)))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pFlameFieldDesc->vPos), 1.f));

	if (FAILED(Ready_Components()))
		return E_FAIL;
	if (FAILED(Ready_Actor()))
		return E_FAIL;

	m_fExpandElapsedTime = 0.f;

	Check_SpawnEffectDistance();
    return S_OK;
}

void CFlameField::Priority_Update(_float fTimeDelta)
{
	if (m_bDead)
	{
		m_pPhysXActorCom->RemovePhysX();
	}
}

void CFlameField::Update(_float fTimeDelta)
{
	m_fExpandElapsedTime += fTimeDelta;
	if (m_fExpandElapsedTime > m_fExpandTime + m_fRemainTime&& !m_bIsExpanded)
	{
		m_bIsExpanded = true;
		m_fExpandElapsedTime = m_fExpandTime + m_fRemainTime; // Ȯ�� �Ϸ�
	}
	
	if (m_fExpandElapsedTime <= m_fExpandTime)
	{
		_float fExpandRatio = m_fExpandElapsedTime / m_fExpandTime;
		fExpandRatio = clamp(fExpandRatio, 0.f, 1.f);
		_float fCurrentRadius = LerpFloat(m_fInitialRadius, m_fExpandRadius, fExpandRatio);
		m_ModifyFlame.halfExtents = PxVec3(fCurrentRadius, 0.5f, fCurrentRadius);
		m_ModifyFlame = m_pGameInstance->CookBoxGeometry(m_ModifyFlame.halfExtents);
		m_pPhysXActorCom->Modify_Shape(m_ModifyFlame);
	}

	if (auto pActor = m_pPhysXActorCom->Get_Actor())
	{
		PxTransform pose = pActor->getGlobalPose();
		_vector vPos = XMVectorSet(pose.p.x, pose.p.y, pose.p.z, 1.f);
		m_pTransformCom->Set_State(STATE::POSITION, vPos);
	}
}

void CFlameField::Late_Update(_float fTimeDelta)
{
	if (m_bIsExpanded)
	{
		Set_bDead();
	}

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()&& m_pPhysXActorCom->Get_ReadyForDebugDraw())
	{
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif
}

HRESULT CFlameField::Render()
{
    return S_OK;
}

void CFlameField::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	int i = 0;
}

void CFlameField::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CFlameField::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CFlameField::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFlameField::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	int i = 0;
}

void CFlameField::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT CFlameField::Ready_Components()
{
	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;
	return S_OK;
}

HRESULT CFlameField::Ready_Actor()
{
	_vector S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);

	PxVec3 halfExtents = PxVec3(0.5f, 0.5f, 0.5f);
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(false, true, false);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_MONSTERWEAPON;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::MONSTER_WEAPON);
	m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	
	if (auto pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Object(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"), 0)))
	{
		pPlayer->Get_Controller()->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
	}
	return S_OK;
}

void CFlameField::Check_SpawnEffectDistance()
{
	_float fOffSetY = 1.f; // �Ҳ� �ʵ��� Y ������
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vOffsetPos = XMVectorSetY(vPos, XMVectorGetY(vPos) + fOffSetY);
	PxVec3 origin(XMVectorGetX(vOffsetPos), XMVectorGetY(vOffsetPos), XMVectorGetZ(vOffsetPos));
	PxHitFlags hitFlags(PxHitFlag::eDEFAULT);
	PxQueryFilterData filterData;
	filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	_int iLevelIndex = m_pGameInstance->GetCurrentLevelIndex();
	auto pFuoco = m_pGameInstance->Get_Object(iLevelIndex, TEXT("Layer_Monster"), 0);
	unordered_set<PxActor*> ignoreActors;
	if (dynamic_cast<CFuoco*>(pFuoco))
	{
		// Fuoco�� �ִ� ���, Fuoco�� Actor�� ����
		ignoreActors.insert(dynamic_cast<CPhysXActor*>(static_cast<CFuoco*>(pFuoco)->Get_Component(TEXT("Com_PhysX")))->Get_Actor());
		ignoreActors.insert(dynamic_cast<CPhysXActor*>(static_cast<CFuoco*>(pFuoco)->Get_Component(TEXT("Com_PhysX2")))->Get_Actor());
		ignoreActors.insert(dynamic_cast<CPhysXActor*>(static_cast<CFuoco*>(pFuoco)->Get_Component(TEXT("Com_PhysX3")))->Get_Actor());
	}

	ignoreActors.insert(m_pPhysXActorCom->Get_Actor());
	CIgnoreSelfCallback callback(ignoreActors);



	for (_int i = 0; i < 12; i++)
	{
		_float fAngle = XMConvertToRadians(30.f * i); // 12�������� ������
		_vector vDir = XMVector3Normalize(XMVectorSet(cosf(fAngle), 0.f, sinf(fAngle), 0.f));
		_vector vSpawnPos = vOffsetPos + vDir * m_fExpandRadius;


		_vector vRay = vSpawnPos - vOffsetPos;                  // ����+�Ÿ�
		_float fDistance = XMVectorGetX(XMVector3Length(vRay)); // ����
		_vector vRayDirNorm = XMVector3Normalize(vRay);         // ���⸸ ����ȭ
		PxVec3 vRayDir(XMVectorGetX(vRayDirNorm), XMVectorGetY(vRayDirNorm), XMVectorGetZ(vRayDirNorm));

		PxRaycastBuffer hit;
		if (m_pGameInstance->Get_Scene()->raycast(origin, vRayDir, fDistance, hit, hitFlags, filterData, &callback))
		{
			PxRigidActor* hitActor = hit.block.actor;
			PxVec3 hitPos = hit.block.position;
			PxVec3 hitNormal = hit.block.normal;
			if (hit.hasBlock)
			{

				CPhysXActor* pHitActor = static_cast<CPhysXActor*>(hitActor->userData);

				if (pHitActor && pHitActor->Get_Owner())
				{
					if (nullptr == pHitActor->Get_Owner())
						return;
					// ������ ��� ���߿� �����غ���
					pHitActor->Get_Owner()->On_Hit(this, COLLIDERTYPE::MONSTER_WEAPON);
				}
				wcout << L"Hit: " << hit.block.actor << endl;
				for (auto* p : ignoreActors) {
					wcout << L"Ignore: " << p << endl;
				}
				wcout << pHitActor->Get_Owner()->Get_Name() << L" Hit by FlameField" << endl;
#ifdef _DEBUG
				if (m_pGameInstance->Get_RenderCollider()) {
					DEBUGRAY_DATA _data{};
					_data.vStartPos = origin;
					_data.vDirection = vRayDir;
					_data.fRayLength = fDistance;
					_data.bIsHit = hit.hasBlock;
					_data.vHitPos = hitPos;
					m_pPhysXActorCom->Add_RenderRay(_data);
				}
#endif
			}


		}

		m_SpawnEffectDistanceList.push_back(fDistance);
	}
}

CFlameField* CFlameField::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFlameField* pInstance = new CFlameField(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CFlameField");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CFlameField::Clone(void* pArg)
{
	CFlameField* pInstance = new CFlameField(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CFlameField");
		Safe_Release(pInstance);
	}
	return pInstance;
}
void CFlameField::Free()
{
	Safe_Release(m_pPhysXActorCom);
	__super::Free();
}
