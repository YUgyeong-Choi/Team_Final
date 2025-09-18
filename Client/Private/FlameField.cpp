#include "Player.h"
#include "FlameField.h"
#include "GameInstance.h"
#include "Effect_Manager.h"
#include "EffectContainer.h"
#include "PhysXDynamicActor.h"
#include "Client_Calculation.h"
#include <PhysX_IgnoreSelfCallback.h>
#include <Fuoco.h>
#include <Oil.h>

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
	pFlameFieldDesc->fSpeedPerSec = 0.f; // 불꽃 필드는 움직이지 않음
	lstrcpy(pFlameFieldDesc->szName, TEXT("FlameField"));
	m_fExpandRadius = pFlameFieldDesc->fExpandRadius; // 불꽃이 확장되는 반지름
	m_fInitialRadius = pFlameFieldDesc->fInitialRadius; // 불꽃의 초기 반지름
	m_fExpandTime = pFlameFieldDesc->fExpandTime; // 불꽃이 확장되는 시간
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pFlameFieldDesc->vPos), 1.f));

	if (FAILED(Ready_Components()))
		return E_FAIL;
	if (FAILED(Ready_Actor()))
		return E_FAIL;

	m_fExpandElapsedTime = 0.f;
	m_LastSpawnDist.resize(16, 0.f);
	m_SpawnEffectDistanceList.reserve(72);
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
	if (m_bEnterPlayer)
	{
		if (m_fDamgeElapsedTime >= m_fDamageInterval)
		{
			if (m_pPlayer)
			{
				_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
				_vector vFieldPos = m_vBegningRayPos;

				_vector vDir = vPlayerPos - vFieldPos;
				vDir = XMVectorSetY(vDir, 0.f);
				_float fDist = XMVectorGetX(XMVector3Length(vDir));

				_float fAngle = atan2f(XMVectorGetZ(vDir), XMVectorGetX(vDir));
				if (fAngle < 0) fAngle += XM_2PI;

				_float fAngleDegrees = XMConvertToDegrees(fAngle);
				_int iIndex = static_cast<_int>(fAngleDegrees / 5.0f + 0.5f) % 72;
				_int iPrevIndex = (iIndex - 1 + 72) % 72; // 이전 인덱스
				_int iNextIndex = (iIndex + 1) % 72;      // 다음 인덱스
				_float fMaxAllowedDist = max(m_SpawnEffectDistanceList[iPrevIndex], m_SpawnEffectDistanceList[iIndex]);
				fMaxAllowedDist = max(fMaxAllowedDist, m_SpawnEffectDistanceList[iNextIndex]);

				if (iIndex >= 0 && iIndex < m_SpawnEffectDistanceList.size())
				{

					if (fDist <= fMaxAllowedDist)
					{
						m_pPlayer->SetElementTypeWeight(EELEMENT::FIRE, 0.4f);
						m_pPlayer->SetHitMotion(HITMOTION::NONE_MOTION);
					}
				}
			}
			m_fDamgeElapsedTime = 0.f;
		}
		else
		{
			m_fDamgeElapsedTime += fTimeDelta;
		}
	}
#ifdef _DEBUG
	else
	{
		cout << "Player NOT in field" << endl;
	}
#endif
	m_fExpandElapsedTime += fTimeDelta;
	if (m_fExpandElapsedTime > m_fExpandTime + m_fRemainTime&& !m_bIsExpanded)
	{
		m_bEnterPlayer = false;
		m_bIsExpanded = true;
		m_fExpandElapsedTime = m_fExpandTime + m_fRemainTime; // 확장 완료
	}
	
	if (m_fExpandElapsedTime <= m_fExpandTime)
	{
		_float fExpandRatio = m_fExpandElapsedTime / m_fExpandTime;
		fExpandRatio = clamp(fExpandRatio, 0.f, 1.f);
		_float fCurrentRadius = LerpFloat(m_fInitialRadius, m_fExpandRadius, fExpandRatio);


		_float step = 4.f; // 이펙트 크기에 맞춰 조정

		// 12방향으로 퍼짐
		for (_int i = 0; i <12; i++)
		{
			_float maxDist = m_MergeDist[i];

			while (m_LastSpawnDist[i] + step <= fCurrentRadius && m_LastSpawnDist[i] + step <= maxDist)
			{
				m_LastSpawnDist[i] += step;

				_float angle = XMConvertToRadians(30.f * i);
				_vector vDir = XMVector3Normalize(XMVectorSet(cosf(angle), 0.f, sinf(angle), 0.f));
				_vector vSpawnPos = m_pTransformCom->Get_State(STATE::POSITION) + vDir * m_LastSpawnDist[i];

				Effect_FlameField(vSpawnPos);
			}
		}

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
	if (auto pOil = dynamic_cast<COil*>(pOther))
	{
		pOil->Explode_Oil();
	}

	if (eColliderType == COLLIDERTYPE::PLAYER)
	{
		if (!m_pPlayer)
		{
			m_pPlayer = dynamic_cast<CPlayer*>(pOther);
		}
		m_bEnterPlayer = true;
	}
}

void CFlameField::On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (eColliderType == COLLIDERTYPE::PLAYER)
	{
		m_bEnterPlayer = true;
	}
}

void CFlameField::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (eColliderType == COLLIDERTYPE::PLAYER)
	{
		m_bEnterPlayer = false;
	}
}

HRESULT CFlameField::Effect_FlameField(const _fvector& vSpawnPos)
{
	CEffectContainer::DESC desc = {};
	auto worldmat = m_pTransformCom->Get_WorldMatrix();

	_float3 pos;
	XMStoreFloat3(&pos, vSpawnPos);

	// 위치만 반영된 월드 매트릭스 생성
	_matrix matWorld = XMMatrixTranslation(pos.x, pos.y + 0.5f, pos.z);
	_int iLevelID = m_pGameInstance->GetCurrentLevelIndex();
	XMStoreFloat4x4(&desc.PresetMatrix, matWorld);
	if (nullptr == MAKE_EFFECT(ENUM_CLASS(iLevelID), TEXT("EC_Fuoco_FlameField_Imsi_P2"), &desc))
		MSG_BOX("이펙트 생성 실패함");
	return S_OK;
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
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY | WORLDFILTER::FILTER_MONSTERWEAPON;
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::BOSS_WEAPON);
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
	_float fOffSetY = 1.f; // 불꽃 필드의 Y 오프셋
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vOffsetPos = XMVectorSetY(vPos, XMVectorGetY(vPos) + fOffSetY);
	m_vBegningRayPos = vOffsetPos;
	PxVec3 origin(XMVectorGetX(vOffsetPos), XMVectorGetY(vOffsetPos), XMVectorGetZ(vOffsetPos));
	PxHitFlags hitFlags(PxHitFlag::eDEFAULT);
	PxQueryFilterData filterData;
	filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
	_int iLevelIndex = m_pGameInstance->GetCurrentLevelIndex();
	CFuoco* pFuoco = dynamic_cast<CFuoco*>(m_pGameInstance->Get_LastObject(iLevelIndex, TEXT("Layer_FireEater")));


	unordered_set<PxActor*> ignoreActors;
	if (pFuoco)
	{
		// Fuoco가 있는 경우, Fuoco의 Actor를 무시
		ignoreActors.insert(dynamic_cast<CPhysXActor*>(static_cast<CFuoco*>(pFuoco)->Get_Component(TEXT("Com_PhysX")))->Get_Actor());
		ignoreActors.insert(dynamic_cast<CPhysXActor*>(static_cast<CFuoco*>(pFuoco)->Get_Component(TEXT("Com_PhysX2")))->Get_Actor());
		ignoreActors.insert(dynamic_cast<CPhysXActor*>(static_cast<CFuoco*>(pFuoco)->Get_Component(TEXT("Com_PhysX3")))->Get_Actor());
	}

	ignoreActors.insert(m_pPhysXActorCom->Get_Actor());
	CIgnoreSelfCallback callback(ignoreActors);


	m_SpawnEffectDistanceList.clear();
	m_SpawnEffectDistanceList.reserve(72);

	for (_int i = 0; i < 72; i++)
	{
		_float fAngle = XMConvertToRadians(5.f * i);
		_vector vDir = XMVector3Normalize(XMVectorSet(cosf(fAngle), 0.f, sinf(fAngle), 0.f));
		_vector vSpawnPos = vOffsetPos + vDir * m_fExpandRadius;

		_vector vRay = vSpawnPos - vOffsetPos;
		_float fDistance = XMVectorGetX(XMVector3Length(vRay));
		_vector vRayDirNorm = XMVector3Normalize(vRay);
		PxVec3 vRayDir(XMVectorGetX(vRayDirNorm), XMVectorGetY(vRayDirNorm), XMVectorGetZ(vRayDirNorm));

		PxRaycastBuffer hit;
		_float finalDist = fDistance;

		if (m_pGameInstance->Get_Scene()->raycast(origin, vRayDir, fDistance, hit, hitFlags, filterData, &callback))
		{
			if (hit.hasBlock && hit.block.actor)
			{
				if (auto pHitActor = static_cast<CPhysXActor*>(hit.block.actor->userData))
				{
					if (pHitActor->Get_Owner() && pHitActor->Get_ColliderType() != COLLIDERTYPE::PLAYER
						&&pHitActor->Get_ColliderType() != COLLIDERTYPE::PLAYER_WEAPON
						&&pHitActor->Get_ColliderType() != COLLIDERTYPE::TRIGGER)
						finalDist = hit.block.distance;
					else
						finalDist = fDistance;
				}
			}
		}

		m_SpawnEffectDistanceList.push_back(finalDist);

#ifdef _DEBUG
		if (m_pGameInstance->Get_RenderCollider())
		{
			DEBUGRAY_DATA _data{};
			_data.vStartPos = origin;
			_data.vDirection = vRayDir;
			_data.fRayLength = finalDist;
			_data.bIsHit = hit.hasBlock;
			_data.vHitPos = hit.hasBlock ? hit.block.position : origin + vRayDir * finalDist;
			m_pPhysXActorCom->Add_RenderRay(_data);
		}
#endif
	}

	m_MergeDist.clear();
	m_MergeDist.resize(12, 0.f);

	for (_int i = 0; i < 12; i++)
	{
		_float fMinDist = FLT_MAX;


		for (_int j = -2; j <= 2; j++)
		{
			_int iIdx = (i * 6 + j + 72) % 72; // 72로 나누어 음수 인덱스 방지
			// 이전의 합친 그룹이랑도 같이 비교
			fMinDist = min(fMinDist, m_SpawnEffectDistanceList[iIdx]);
		}

		if (fMinDist == FLT_MAX)
			fMinDist = m_fExpandRadius;

		m_MergeDist[i] = fMinDist;
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
