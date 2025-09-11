#include "MonsterToolObject.h"

#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "PhysX_IgnoreSelfCallback.h"
#include "PhysXController.h"
#include "Camera_Manager.h"


CMonsterToolObject::CMonsterToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_pAnimator(nullptr)
	, m_pShaderCom(nullptr)
{
}
CMonsterToolObject::CMonsterToolObject(const CMonsterToolObject& Prototype)
	: CGameObject(Prototype)
	, m_pAnimator(Prototype.m_pAnimator)
	, m_pShaderCom(Prototype.m_pShaderCom)
{
	Safe_AddRef(m_pAnimator);
	Safe_AddRef(m_pShaderCom);
}
HRESULT CMonsterToolObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonsterToolObject::Initialize(void* pArg)
{
	MONSTERTOOLOBJECT_DESC* pDesc = static_cast<MONSTERTOOLOBJECT_DESC*>(pArg);
	m_fSpeedPerSec = pDesc->fSpeedPerSec;
	m_fRotationPerSec = pDesc->fRotationPerSec;

	m_InitPos = pDesc->InitPos;
	m_InitScale = pDesc->InitScale;
	lstrcpy(m_szMeshID, pDesc->szMeshID);
	m_eMeshLevelID = pDesc->eMeshLevelID;
	m_iRender = pDesc->iRender;
	//m_szName = pDesc->szName;

	m_iID = pDesc->iID;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	LoadAnimDataFromJson();

	//Register_Events();

	_vector vInitPos = XMVectorSetW(XMLoadFloat3(&pDesc->InitPos), 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vInitPos);
	m_pTransformCom->Scaling(pDesc->InitScale);

	m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);

	m_eSpawnType = pDesc->eSpawnType;
	
	m_pAnimator->SetTrigger("Sit");

	return S_OK;
}

void CMonsterToolObject::Priority_Update(_float fTimeDelta)
{

}
void CMonsterToolObject::Update(_float fTimeDelta)
{



	/* [ 애니메이션 업데이트 ] */
	if (m_pAnimator)
		m_pAnimator->Update(fTimeDelta);

	if (m_pModelCom)
		m_pModelCom->Update_Bones();

}

void CMonsterToolObject::Late_Update(_float fTimeDelta)
{
	//m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_SHADOW, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CMonsterToolObject::Render()
{
	if (FAILED(Bind_Shader()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0);


		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

//HRESULT CMonsterToolObject::Render_Shadow()
//{
//	/* [ 월드 스페이스 넘기기 ] */
//	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
//		return E_FAIL;
//
//	SetCascadeShadow();
//	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Light_ViewMatrix(m_eShadow))))
//		return E_FAIL;
//	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Light_ProjMatrix(m_eShadow))))
//		return E_FAIL;
//
//	_int iCascadeCount = ENUM_CLASS(m_eShadow);
//	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
//
//	for (_uint i = 0; i < iNumMesh; i++)
//	{
//		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);
//
//		switch (iCascadeCount)
//		{
//		case 0: m_pShaderCom->Begin(3); break;
//		case 1: m_pShaderCom->Begin(4); break;
//		case 2: m_pShaderCom->Begin(5); break;
//		}
//
//		if (FAILED(m_pModelCom->Render(i)))
//			return E_FAIL;
//	}
//
//	return S_OK;
//}

//void CMonsterToolObject::SetCascadeShadow()
//{
//	if (m_pPhysXActorCom)
//	{
//		// 월드 AABB 구하기
//		AABBBOX tWorldAABB = GetWorldAABB();
//
//		// 뷰 행렬
//		_float4x4 tView = {};
//		XMStoreFloat4x4(&tView, XMLoadFloat4x4(m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW)));
//
//		// 8코너를 뷰공간으로 변환하여 Z범위 계산
//		_float fZMin = 1e9f, fZMax = -1e9f;
//		for (_uint i = 0; i < 8; ++i)
//		{
//			_float3 vCorner = ExtractAABBWorldCorner(tWorldAABB.vMin, tWorldAABB.vMax, i);
//			_vector vCornerVS = XMVector3TransformCoord(XMLoadFloat3(&vCorner), XMLoadFloat4x4(&tView));
//			_float  fZ = XMVectorGetZ(vCornerVS);
//			fZMin = min(fZMin, fZ);
//			fZMax = max(fZMax, fZ);
//		}
//
//		// 완전 포함 → 해당 캐스케이드
//		if (fZMax <= 5.f)        m_eShadow = SHADOW::SHADOWA;
//		else if (fZMin >= 5.f && fZMax <= 20.f) m_eShadow = SHADOW::SHADOWB;
//		else if (fZMin >= 20.f)  m_eShadow = SHADOW::SHADOWC;
//		else
//		{
//			// 걸침 → 최소 Z 기준으로만 분기
//			if (fZMin < 5.f)         m_eShadow = SHADOW::SHADOWA;
//			else if (fZMin < 20.f)   m_eShadow = SHADOW::SHADOWB;
//			else                     m_eShadow = SHADOW::SHADOWC;
//		}
//	}
//	else
//	{
//		// AABB 없으면 기존 방식
//		if (m_fViewZ < 5.f)         m_eShadow = SHADOW::SHADOWA;
//		else if (m_fViewZ < 20.f)   m_eShadow = SHADOW::SHADOWB;
//		else                        m_eShadow = SHADOW::SHADOWC;
//	}
//}

void CMonsterToolObject::LoadAnimDataFromJson()
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

HRESULT CMonsterToolObject::Bind_Shader()
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

	//아이디 출력
	_float fID = static_cast<_float>(m_iID);
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fID", &fID, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}
HRESULT CMonsterToolObject::Ready_Components()
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
HRESULT CMonsterToolObject::Ready_Collider()
{

	return S_OK;
}

CMonsterToolObject* CMonsterToolObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMonsterToolObject* pInstance = new CMonsterToolObject(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMonsterToolObject");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CMonsterToolObject::Clone(void* pArg)
{
	CMonsterToolObject* pInstance = new CMonsterToolObject(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonsterToolObject");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMonsterToolObject::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAnimator);
	Safe_Release(m_pShaderCom);
}
