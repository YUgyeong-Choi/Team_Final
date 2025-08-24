#include "FlameField.h"

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
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	if (FAILED(Ready_Components()))
		return E_FAIL;
	if (FAILED(Ready_Actor()))
		return E_FAIL;
    return S_OK;
}

void CFlameField::Priority_Update(_float fTimeDelta)
{
}

void CFlameField::Update(_float fTimeDelta)
{
}

void CFlameField::Late_Update(_float fTimeDelta)
{
}

HRESULT CFlameField::Render()
{
    return E_NOTIMPL;
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
}

void CFlameField::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT CFlameField::Ready_Components()
{
	return S_OK;
}

HRESULT CFlameField::Ready_Actor()
{
	return S_OK;
}

CFlameField* CFlameField::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CGameObject* CFlameField::Clone(void* pArg)
{
    return nullptr;
}

void CFlameField::Free()
{
}
