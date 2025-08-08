#include "Monster_Test.h"
#include "GameInstance.h"

CMonster_Test::CMonster_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUnit{pDevice, pContext}
{
}

CMonster_Test::CMonster_Test(const CMonster_Test& Prototype)
    :CUnit{Prototype}
{
}

HRESULT CMonster_Test::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMonster_Test::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;


    
    //m_pAnimator->SetPlaying(true);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_InitPos), 1.f));

	_vector vDir = { 0.f,1.f,0.f,0.f };

	m_pTransformCom->Rotation(vDir, XMConvertToRadians(-90.f));

	LoadAnimDataFromJson();

	if (FAILED(Ready_Actor()))
		return E_FAIL;

    return S_OK;
}

void CMonster_Test::Priority_Update(_float fTimeDelta)
{
   
}

void CMonster_Test::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

	
	// 움직이고 부르기
	Update_Collider();
}

void CMonster_Test::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    Update_State();
}

HRESULT CMonster_Test::Render()
{

    __super::Render();

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif


    return S_OK;
}

void CMonster_Test::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	m_pAnimator->SetBool("Detect", true);
	printf("몬스터 충돌됨\n");
}


void CMonster_Test::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	printf("몬스터 충돌중\n");
}

void CMonster_Test::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	printf("몬스터 충돌 나감\n");
}

void CMonster_Test::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{

}

void CMonster_Test::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CMonster_Test::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT CMonster_Test::Ready_Components()
{
    // 무기 장착 시키기?
		/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;




    return S_OK;
}

HRESULT CMonster_Test::Ready_Actor()
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

	PxVec3 halfExtents = PxVec3(0.5f, 0.5f, 0.5f);
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

void CMonster_Test::RootMotionActive(_float fTimeDelta)
{

}

void CMonster_Test::Update_State()
{
	
    if (m_pAnimator->IsFinished())
    {
       // 나중에 생각
    }
}

void CMonster_Test::LoadAnimDataFromJson()
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

void CMonster_Test::Update_Collider()
{
	// 1. 월드 행렬 가져오기
	_matrix worldMatrix = m_pTransformCom->Get_WorldMatrix();

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

	// 무기 추가
}

CMonster_Test* CMonster_Test::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMonster_Test* pInstance = new CMonster_Test(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPlayer");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CMonster_Test::Clone(void* pArg)
{
	CMonster_Test* pInstance = new CMonster_Test(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonster_Test");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMonster_Test::Free()
{
    __super::Free();

	Safe_Release(m_pPhysXActorCom);
    
}
