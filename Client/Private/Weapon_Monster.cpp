#include "Weapon_Monster.h"

#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "PhysX_IgnoreSelfCallback.h"


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
	MONSTER_WEAPON_DESC* pDesc = static_cast<MONSTER_WEAPON_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	_matrix MatHandle = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("BN_Handle")));

	m_pTransformCom->Set_WorldMatrix(MatHandle);
	
	m_pTransformCom->Rotation(XMLoadFloat4(&pDesc->vAxis), XMConvertToRadians(pDesc->fRotationDegree));

	// offset

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

	vPos += XMVectorSetW(XMLoadFloat3(&pDesc->InitPos), 0.f);
	 
	m_pTransformCom->Set_State(STATE::POSITION, vPos);

	m_iDurability = m_iMaxDurability;

	m_bIsActive = true;

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
}

HRESULT CWeapon_Monster::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CWeapon_Monster::Ready_Components()
{
	/* [ 따로 추가할 컴포넌트가 있습니까? ] */

	return S_OK;
}

void CWeapon_Monster::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CWeapon_Monster::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CWeapon_Monster::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CWeapon_Monster::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CWeapon_Monster::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
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
}
