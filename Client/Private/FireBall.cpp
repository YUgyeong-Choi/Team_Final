#include "FireBall.h"

CFireBall::CFireBall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile(pDevice, pContext)
{
}

CFireBall::CFireBall(const CFireBall& Prototype)
	: CProjectile(Prototype)
	, m_fDamge(Prototype.m_fDamge)
{
}

HRESULT CFireBall::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CFireBall::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	return S_OK;
}

void CFireBall::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CFireBall::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CFireBall::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CFireBall::Render()
{
    return __super::Render();
}

void CFireBall::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFireBall::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFireBall::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFireBall::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFireBall::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFireBall::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT CFireBall::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;
	// 나중에 모델 불러오기
    return S_OK;
}

CFireBall* CFireBall::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFireBall* pInstance = new CFireBall(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CFireBall");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CFireBall::Clone(void* pArg)
{
	CFireBall* pInstance = new CFireBall(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CFireBall");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CFireBall::Free()
{
	__super::Free();
}
