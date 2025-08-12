#include "Monster_Base.h"
#include "GameInstance.h"
#include "LockOn_Manager.h"
#include "PhysX_ControllerReport.h"
#include "PhysX_IgnoreSelfCallback.h"

CMonster_Base::CMonster_Base(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUnit{pDevice, pContext}
{
}

CMonster_Base::CMonster_Base(const CMonster_Base& Prototype)
	:CUnit{Prototype}
{
}

HRESULT CMonster_Base::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonster_Base::Initialize(void* pArg)
{

	MONSTER_BASE_DESC* pDesc = static_cast<MONSTER_BASE_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pAnimator->SetPlaying(true);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_InitPos), 1.f));

	_vector vDir = { 0.f,1.f,0.f,0.f };

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_InitPos), 1.f));
	m_pTransformCom->Scaling(pDesc->InitScale);

	LoadAnimDataFromJson();

	if (FAILED(Ready_Actor(pArg)))
		return E_FAIL;

	if (FAILED(Ready_PartObject()))
		return E_FAIL;

	m_pPlayer = Find_Player(m_pGameInstance->GetCurrentLevelIndex());
	
	m_fHeight = pDesc->fHeight;

	m_pNaviCom->Select_Cell(m_pTransformCom->Get_State(STATE::POSITION));

	m_pAnimator->SetBool("Detect", false);

	

	return S_OK;
}

void CMonster_Base::Priority_Update(_float fTimeDelta)
{
	// 죽는 조건 만들어서 다 같이 쓰기

	if (m_strStateName.find("Dead") != m_strStateName.npos)
	{
		if (m_pAnimator->IsFinished())
		{
			// 렉돌 나중에?
			Set_bDead();
		}
	}


}

void CMonster_Base::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	
	float fY = m_pNaviCom->Compute_NavigationY(m_pTransformCom->Get_State(STATE::POSITION));

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

	vPos.m128_f32[1] = fY;

	m_pTransformCom->Set_State(STATE::POSITION, vPos);

	// 움직이고 부르기
	Update_Collider();

	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom)) {
		CLockOn_Manager::Get_Instance()->Add_LockOnTarget(this);
	}

	if (m_isLookAt)
	{
		m_pTransformCom->LookAtWithOutY(m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION));
	}

	m_pHPBar->Update(fTimeDelta);
}

void CMonster_Base::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	// 더 좋은 방법 있으면 바꾸기
	if (this == CLockOn_Manager::Get_Instance()->Get_Target())
		m_pHPBar->Set_RenderTime(2.f);


	m_pHPBar->Late_Update(fTimeDelta);

	
}

HRESULT CMonster_Base::Render()
{
	__super::Render();

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif


	return S_OK;
}

void CMonster_Base::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CMonster_Base::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CMonster_Base::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CMonster_Base::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CMonster_Base::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CMonster_Base::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT CMonster_Base::Ready_Components()
{
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), 
		TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	m_pGameInstance->GetCurrentLevelIndex();

	wstring wsPrototypeTag = TEXT("Prototype_Component_Navigation_");

	switch (m_pGameInstance->GetCurrentLevelIndex())
	{
	case ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION):
		wsPrototypeTag += TEXT("STATION");
		break;
	case ENUM_CLASS(LEVEL::KRAT_HOTEL):
		wsPrototypeTag += TEXT("HOTEL");
		break;
	default:
		return E_FAIL;
		break;
	}

	if (FAILED(__super::Add_Component(m_pGameInstance->GetCurrentLevelIndex(), wsPrototypeTag.c_str(),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNaviCom))))
		return E_FAIL;
	

	return S_OK;
}

HRESULT CMonster_Base::Ready_Actor(void* pArg)
{
	MONSTER_BASE_DESC* pDesc = static_cast<MONSTER_BASE_DESC*>(pArg);

	// 3. Transform에서 S, R, T 분리
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 3-1. 스케일, 회전, 위치 변환
	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxVec3 halfExtents = VectorToPxVec3(XMLoadFloat3(&pDesc->vExtent));
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY | FILTER_PLAYERWEAPON; // 일단 보류
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::MONSTER);
	m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

HRESULT CMonster_Base::Ready_PartObject()
{
	// 체력바 넣기

	CUI_MonsterHP_Bar::HPBAR_DESC eDesc{};

	eDesc.fSizeX = 1.f;
	eDesc.fSizeY = 1.f;
	eDesc.fHeight = 2.25f;
	eDesc.pHP = &m_iHP;
	eDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();

	m_pHPBar = static_cast<CUI_MonsterHP_Bar*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, 
											   ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_HPBar"), &eDesc));

	//Safe_AddRef(m_pHPBar);

	return S_OK;
}



void CMonster_Base::RootMotionActive(_float fTimeDelta)
{
	CAnimation* pCurAnim = m_pAnimator->GetCurrentAnim();
	_bool        bUseRoot = (pCurAnim && pCurAnim->IsRootMotionEnabled());

	if (bUseRoot)
	{
		_float3			rootMotionDelta = m_pAnimator->GetRootMotionDelta();
		_float4 		rootMotionQuat = m_pAnimator->GetRootRotationDelta();
		XMVECTOR vLocal = XMLoadFloat3(&rootMotionDelta);

		_vector vScale, vRotQuat, vTrans;
		XMMatrixDecompose(&vScale, &vRotQuat, &vTrans, m_pTransformCom->Get_WorldMatrix());

		XMVECTOR vWorldDelta = XMVector3Transform(vLocal, XMMatrixRotationQuaternion(vRotQuat));



		_float fDeltaMag = XMVectorGetX(XMVector3Length(vWorldDelta));
		_vector finalDelta;
		if (fDeltaMag > m_fSmoothThreshold)
		{
			_float alpha = clamp(fTimeDelta * m_fSmoothSpeed, 0.f, 1.f);
			finalDelta = XMVectorLerp(m_PrevWorldDelta, vWorldDelta, alpha);
		}
		else
		{
			finalDelta = vWorldDelta;
		}
		m_PrevWorldDelta = finalDelta;

		m_PrevWorldDelta.m128_f32[3] = 0;

		vTrans += m_PrevWorldDelta;

		// 네비 이동 가능 여부 체크 후 위치 재설정
		if (!m_pNaviCom->isMove(vTrans))
		{
			vTrans -= m_PrevWorldDelta;
			m_pTransformCom->Set_State(STATE::POSITION, vTrans);
		}

		// 이제 충돌 테스트 해서 충돌하면 돌아가게



		// 회전 보정
		_vector vRotDelta = XMLoadFloat4(&rootMotionQuat);
		_vector vNewRot = XMQuaternionMultiply(vRotDelta, vRotQuat);
	

		// 월드 행렬 재생성 및 세팅
		_matrix newWorld =
			XMMatrixScalingFromVector(vScale) *
			XMMatrixRotationQuaternion(vNewRot) *
			XMMatrixTranslationFromVector(vTrans);
		m_pTransformCom->Set_WorldMatrix(newWorld);

	}
}



void CMonster_Base::LoadAnimDataFromJson()
{
	string path = "../Bin/Save/AnimationEvents/" + m_pModelCom->Get_ModelName() + "_events.json";
	ifstream ifs(path);
	if (ifs.is_open())
	{
		json root;
		ifs >> root;
		if (root.contains("animations"))
		{
			auto& animationsJson = root["animations"];
			auto& clonedAnims = m_pModelCom->GetAnimations();

			for (const auto& animData : animationsJson)
			{
				const string& clipName = animData["ClipName"];

				for (auto& pAnim : clonedAnims)
				{
					if (pAnim->Get_Name() == clipName)
					{
						pAnim->Deserialize(animData);
						break;
					}
				}
			}
		}
	}

	path = "../Bin/Save/AnimationStates/" + m_pModelCom->Get_ModelName() + "_States.json";
	ifstream ifsStates(path);
	if (ifsStates.is_open())
	{
		json rootStates;
		ifsStates >> rootStates;
		m_pAnimator->Deserialize(rootStates);
	}
}

void CMonster_Base::Update_Collider()
{
	// 1. 월드 행렬 가져오기
	_matrix worldMatrix = m_pTransformCom->Get_WorldMatrix();

	// 2. 위치 추출
	_float4 vPos;
	XMStoreFloat4(&vPos, worldMatrix.r[3]);

	PxVec3 pos(vPos.x, vPos.y, vPos.z);
	pos.y += m_fHeight;

	// 3. 회전 추출
	XMVECTOR boneQuat = XMQuaternionRotationMatrix(worldMatrix);
	XMFLOAT4 fQuat;
	XMStoreFloat4(&fQuat, boneQuat);
	PxQuat rot = PxQuat(fQuat.x, fQuat.y, fQuat.z, fQuat.w);

	// 4. PhysX Transform 적용
	m_pPhysXActorCom->Set_Transform(PxTransform(pos, rot));

}

CGameObject* CMonster_Base::Find_Player(_int iLevel)
{
	auto pPlayer = m_pGameInstance->Get_LastObject((iLevel), TEXT("Layer_Player"));

	if (nullptr == pPlayer)
		return nullptr;

	Safe_AddRef(pPlayer);

	return pPlayer;
}

_bool CMonster_Base::Check_Detect()
{
	if (nullptr == m_pPlayer)
		return false;

	_vector vDir = {};
	vDir = m_pTransformCom->Get_State(STATE::POSITION) - m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);

	if (XMVectorGetX(XMVector3Length(vDir)) < m_fDetectDist)
	{
		m_isDetect = true;
		m_pAnimator->SetBool("Detect", m_isDetect);
		return true;
	}
	

	return false;
}

CMonster_Base::MONSTER_DIR CMonster_Base::Calc_HitDir(_vector vOtherPos)
{
	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
	vLook = XMVector3Normalize(vLook);

	_vector vDir = vOtherPos - m_pTransformCom->Get_State(STATE::POSITION);
	vDir = XMVector3Normalize(vDir);

	if(XMVectorGetX(XMVector3Dot(vLook, vDir)) < 0)
		return MONSTER_DIR::B;
	else
	{
		// 외적해서 왼쪽 오른쪽, 기준은 vLook

		if (XMVectorGetY(XMVector3Cross(vLook, vDir)) < 0)
			return MONSTER_DIR::L; 
		else
			return MONSTER_DIR::R; 
	}
	


	return MONSTER_DIR::END;
}

CMonster_Base* CMonster_Base::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMonster_Base* pInstance = new CMonster_Base(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPlayer");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CMonster_Base::Clone(void* pArg)
{
	CMonster_Base* pInstance = new CMonster_Base(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonster_Base");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMonster_Base::Free()
{
	__super::Free();

	Safe_Release(m_pPhysXActorCom);
	Safe_Release(m_pPlayer);
	Safe_Release(m_pNaviCom);
	Safe_Release(m_pHPBar);
}