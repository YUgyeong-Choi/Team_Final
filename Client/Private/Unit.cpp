#include "Unit.h"

#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "PhysX_IgnoreSelfCallback.h"
#include "PhysXController.h"
#include "Camera_Manager.h"


CUnit::CUnit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_pAnimator(nullptr)
	, m_pShaderCom(nullptr)
{
}
CUnit::CUnit(const CUnit& Prototype)
	: CGameObject(Prototype)
	, m_pAnimator(nullptr)
	, m_pShaderCom(Prototype.m_pShaderCom)
{
	Safe_AddRef(m_pAnimator);
	Safe_AddRef(m_pShaderCom);
}
HRESULT CUnit::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUnit::Initialize(void* pArg)
{
	UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);
	m_fSpeedPerSec = pDesc->fSpeedPerSec;
	m_fRotationPerSec = pDesc->fRotationPerSec;

	m_InitPos = pDesc->InitPos;
	m_InitScale = pDesc->InitScale;
	m_szMeshID = pDesc->szMeshID;
	m_eMeshLevelID = pDesc->eMeshLevelID;
	m_iRender = pDesc->iRender;
	m_szName = pDesc->szName;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	Register_Events();

	m_pCamera_Orbital = CCamera_Manager::Get_Instance()->GetOrbitalCam();

	_vector vInitPos = XMVectorSetW(XMLoadFloat3(&pDesc->InitPos), 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vInitPos);
	m_pTransformCom->Scaling(pDesc->InitScale);
	return S_OK;
}

void CUnit::Priority_Update(_float fTimeDelta)
{
	if (!m_pPlayer)
		m_pPlayer = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"));
}
void CUnit::Update(_float fTimeDelta)
{
	/* [ 애니메이션 업데이트 ] */
	if (m_pAnimator)
		m_pAnimator->Update(fTimeDelta);

	if (m_pModelCom)
		m_pModelCom->Update_Bones();

}

void CUnit::Late_Update(_float fTimeDelta)
{
	_vector	vTemp = m_pTransformCom->Get_State(STATE::POSITION);

	if (m_pPlayer)
	{
		_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
		CGameObject::Compute_ViewZ(vPlayerPos, &vTemp);
	}
	else
	{
		_vector vCam = m_pCamera_Orbital->GetPosition();
		CGameObject::Compute_ViewZ(vCam, &vTemp);
	}

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_SHADOW, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CUnit::Render()
{
	if (FAILED(Bind_Shader()))
		return E_FAIL;

	return S_OK;
}

HRESULT CUnit::Render_Shadow()
{
	/* [ 월드 스페이스 넘기기 ] */
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;

	SetCascadeShadow();
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Light_ViewMatrix(m_eShadow))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Light_ProjMatrix(m_eShadow))))
		return E_FAIL;

	_int iCascadeCount = ENUM_CLASS(m_eShadow);
	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		switch (iCascadeCount)
		{
		case 0: m_pShaderCom->Begin(3); break;
		case 1: m_pShaderCom->Begin(4); break;
		case 2: m_pShaderCom->Begin(5); break;
		}

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CUnit::SetCascadeShadow()
{
	if (m_pPhysXActorCom && !m_bIsPlayer)
	{
		// 월드 AABB 구하기
		AABBBOX tWorldAABB = GetWorldAABB();

		// 뷰 행렬
		_float4x4 tView = {};
		XMStoreFloat4x4(&tView, XMLoadFloat4x4(m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW)));

		// 8코너를 뷰공간으로 변환하여 Z범위 계산
		_float fZMin = 1e9f, fZMax = -1e9f;
		for (_uint i = 0; i < 8; ++i)
		{
			_float3 vCorner = ExtractAABBWorldCorner(tWorldAABB.vMin, tWorldAABB.vMax, i);
			_vector vCornerVS = XMVector3TransformCoord(XMLoadFloat3(&vCorner), XMLoadFloat4x4(&tView));
			_float  fZ = XMVectorGetZ(vCornerVS);
			fZMin = min(fZMin, fZ);
			fZMax = max(fZMax, fZ);
		}

		// 완전 포함 → 해당 캐스케이드
		if (fZMax <= 5.f)        m_eShadow = SHADOW::SHADOWA;
		else if (fZMin >= 5.f && fZMax <= 20.f) m_eShadow = SHADOW::SHADOWB;
		else if (fZMin >= 20.f)  m_eShadow = SHADOW::SHADOWC;
		else
		{
			// 걸침 → 최소 Z 기준으로만 분기
			if (fZMin < 5.f)         m_eShadow = SHADOW::SHADOWA;
			else if (fZMin < 20.f)   m_eShadow = SHADOW::SHADOWB;
			else                     m_eShadow = SHADOW::SHADOWC;
		}
	}
	else
	{
		// AABB 없으면 기존 방식
		if (m_fViewZ < 5.f)         m_eShadow = SHADOW::SHADOWA;
		else if (m_fViewZ < 20.f)   m_eShadow = SHADOW::SHADOWB;
		else                        m_eShadow = SHADOW::SHADOWC;
	}
}

HRESULT CUnit::Bind_Shader()
{
	/* [ 월드 스페이스 넘기기 ] */
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;

	/* [ 뷰 , 투영 스페이스 넘기기 ] */
	_float4x4 ViewMatrix, ProjViewMatrix;
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjViewMatrix)))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0);
		

		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}
HRESULT CUnit::Ready_Components()
{
	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(m_eMeshLevelID), _wstring(TEXT("Prototype_Component_Model_")) + m_szMeshID,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	m_pAnimator = CAnimator::Create(m_pDevice, m_pContext);
	if (nullptr == m_pAnimator)
		return E_FAIL;
	if (FAILED(m_pAnimator->Initialize(m_pModelCom)))
		return E_FAIL;

	return S_OK;
}
HRESULT CUnit::Ready_Collider()
{

	return S_OK;
}

PxRigidActor* CUnit::Get_Actor(CPhysXActor* actor)
{
	return actor->Get_Actor();
}


void CUnit::RayCast(CPhysXActor* actor)
{
	PxVec3 origin = actor->Get_Actor()->getGlobalPose().p;
	XMFLOAT3 fLook;
	XMStoreFloat3(&fLook, m_pTransformCom->Get_State(STATE::LOOK));
	PxVec3 direction = PxVec3(fLook.x, fLook.y, fLook.z);
	direction.normalize();
	_float fRayLength = 10.f;

	PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
	PxRaycastBuffer hit;
	PxQueryFilterData filterData;
	filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	unordered_set<PxActor*> ignoreActors = actor->Get_IngoreActors();
	CIgnoreSelfCallback callback(ignoreActors);

	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, fRayLength, hit, hitFlags, filterData, &callback))
	{
		if (hit.hasBlock)
		{
			PxRigidActor* hitActor = hit.block.actor;

			if (hitActor == actor->Get_Actor())
			{
				printf(" Ray hit myself  skipping\n");
				return;
			}
			PxVec3 hitPos = hit.block.position;
			PxVec3 hitNormal = hit.block.normal;

			CPhysXActor* pHitActor = static_cast<CPhysXActor*>(hitActor->userData);

			if (pHitActor)
			{
				if (nullptr == pHitActor->Get_Owner())
					return;
				pHitActor->Get_Owner()->On_Hit(this, actor->Get_ColliderType());
			}

			//printf("RayHitPos X: %f, Y: %f, Z: %f\n", hitPos.x, hitPos.y, hitPos.z);
			//printf("RayHitNormal X: %f, Y: %f, Z: %f\n", hitNormal.x, hitNormal.y, hitNormal.z);
			m_bRayHit = true;
			m_vRayHitPos = hitPos;
		}
	}

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		DEBUGRAY_DATA _data{};
		_data.vStartPos = actor->Get_Actor()->getGlobalPose().p;
		XMFLOAT3 fLook;
		XMStoreFloat3(&fLook, m_pTransformCom->Get_State(STATE::LOOK));
		_data.vDirection = PxVec3(fLook.x, fLook.y, fLook.z);
		_data.fRayLength = 10.f;
		_data.bIsHit = m_bRayHit;
		_data.vHitPos = m_vRayHitPos;
		actor->Add_RenderRay(_data);

		m_bRayHit = false;
		m_vRayHitPos = {};
	}
#endif
}

void CUnit::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CUnit::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CUnit::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CUnit::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CUnit::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CUnit::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}


AABBBOX CUnit::GetWorldAABB() const
{
	PxBounds3 wb = m_pPhysXActorCom->Get_Actor()->getWorldBounds();
	AABBBOX worldBox{ {wb.minimum.x, wb.minimum.y, wb.minimum.z},
					  {wb.maximum.x, wb.maximum.y, wb.maximum.z} };

	return worldBox;
}

CUnit* CUnit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUnit* pInstance = new CUnit(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUnit");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUnit::Clone(void* pArg)
{
	CUnit* pInstance = new CUnit(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUnit");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUnit::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAnimator);
	Safe_Release(m_pShaderCom);
}
