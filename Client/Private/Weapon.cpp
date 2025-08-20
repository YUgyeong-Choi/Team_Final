#include "Weapon.h"

#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "Effect_Manager.h"
#include "SwordTrailEffect.h"
#include "PhysX_IgnoreSelfCallback.h"


CWeapon::CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_pAnimator(nullptr)
	, m_pShaderCom(nullptr)
{
}
CWeapon::CWeapon(const CWeapon& Prototype)
	: CGameObject(Prototype)
	, m_pAnimator(Prototype.m_pAnimator)
	, m_pShaderCom(Prototype.m_pShaderCom)
{
	Safe_AddRef(m_pAnimator);
	Safe_AddRef(m_pShaderCom);
}
HRESULT CWeapon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWeapon::Initialize(void* pArg)
{
	WEAPON_DESC* pDesc = static_cast<WEAPON_DESC*>(pArg);
	m_pParentWorldMatrix = pDesc->pParentWorldMatrix;
	m_pSocketMatrix = pDesc->pSocketMatrix;

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

	return S_OK;
}

void CWeapon::Priority_Update(_float fTimeDelta)
{

	if (KEY_DOWN(DIK_CAPSLOCK))
		PrintMatrix("World", XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()));
}

void CWeapon::Update(_float fTimeDelta)
{
	if (!m_bIsActive)
		return;

	///* [ 애니메이션 업데이트 ] */
	//if (m_pAnimator)
	//	m_pAnimator->Update(fTimeDelta);
	
	if (m_pModelCom)
		m_pModelCom->Update_Bones();

	// 상태에 따른 데미지 업데이트
}

void CWeapon::Late_Update(_float fTimeDelta)
{
	_matrix		SocketMatrix = XMLoadFloat4x4(m_pSocketMatrix);
	
	for (size_t i = 0; i < 3; i++)
		SocketMatrix.r[i] = XMVector3Normalize(SocketMatrix.r[i]);
	
	/* 무기 월드 1.f , 소켓 월드 , 부모 월드 0.02f */
	XMStoreFloat4x4(&m_CombinedWorldMatrix,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()) *
		SocketMatrix *
		XMLoadFloat4x4(m_pParentWorldMatrix));

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CWeapon::Render()
{
	if (!m_bIsActive)
		return S_OK;

	if (FAILED(Bind_Shader()))
		return E_FAIL;

	return S_OK;
}

void CWeapon::SetWeaponWorldMatrix(_float fTimeDelta)
{
	if (KEY_PRESSING(DIK_1)) // 점점 커지게
	{
		_vector vScale = m_pTransformCom->Get_Scale(); // 현재 스케일 (_vector)

		XMFLOAT3 f3Scale;
		XMStoreFloat3(&f3Scale, vScale); // _vector → _float3

		f3Scale.x *= 1.001f;
		f3Scale.y *= 1.001f;
		f3Scale.z *= 1.001f;

		m_pTransformCom->Scaling(f3Scale); // 최종 반영
	}

	if (KEY_PRESSING(DIK_2)) // 점점 작아지게
	{
		_vector vScale = m_pTransformCom->Get_Scale();

		XMFLOAT3 f3Scale;
		XMStoreFloat3(&f3Scale, vScale);

		f3Scale.x *= 0.999f;
		f3Scale.y *= 0.999f;
		f3Scale.z *= 0.999f;

		m_pTransformCom->Scaling(f3Scale);
	}

	const _float fRotateSpeed = 5.f;
	if (KEY_DOWN(DIK_3)) // Y축 좌회전
	{
		m_pTransformCom->RotationTimeDelta(fTimeDelta, XMVectorSet(0.f, 1.f, 0.f, 0.f), -fRotateSpeed);
	}
	if (KEY_DOWN(DIK_4)) // Y축 우회전
	{
		m_pTransformCom->RotationTimeDelta(fTimeDelta, XMVectorSet(0.f, 1.f, 0.f, 0.f), +fRotateSpeed);
	}

	if (KEY_DOWN(DIK_5)) // X축 회전
	{
		m_pTransformCom->RotationTimeDelta(fTimeDelta, XMVectorSet(1.f, 0.f, 0.f, 0.f), +fRotateSpeed);
	}
	if (KEY_DOWN(DIK_6)) // Z축 회전
	{
		m_pTransformCom->RotationTimeDelta(fTimeDelta, XMVectorSet(0.f, 0.f, 1.f, 0.f), +fRotateSpeed);
	}
}


HRESULT CWeapon::Bind_Shader()
{
	/* [ 월드 스페이스 넘기기 ] */
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
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
HRESULT CWeapon::Ready_Components()
{

	if (m_szMeshID == nullptr)
		return S_OK;

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

void CWeapon::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CWeapon::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CWeapon::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CWeapon::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CWeapon::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CWeapon::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}



void CWeapon::Calc_Durability(_int iDelta)
{
	m_fDurability -= iDelta;

	m_pGameInstance->Notify(L"Weapon_Status", L"Durablity", &m_fDurability);
}

_bool CWeapon::Find_CollisonObj(CGameObject* pObj)
{
	if (m_CollisonObjects.empty())
		return false;

	

	return find(m_CollisonObjects.begin(), m_CollisonObjects.end(), pObj) != m_CollisonObjects.end();
}

void CWeapon::Add_CollisonObj(CGameObject* pObj)
{
	m_CollisonObjects.push_back(pObj);
}

void CWeapon::Set_WeaponTrail_Active(_bool bActive)
{
	if (m_pWeaponTrailEffect)
		m_pWeaponTrailEffect->Set_TrailActive(bActive);
}

CWeapon* CWeapon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWeapon* pInstance = new CWeapon(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWeapon");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CWeapon::Clone(void* pArg)
{
	CWeapon* pInstance = new CWeapon(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CWeapon");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CWeapon::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAnimator);
	Safe_Release(m_pShaderCom);
}
