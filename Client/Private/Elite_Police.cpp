#include "Elite_Police.h"

CElite_Police::CElite_Police(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMonster_Base{pDevice, pContext}
{
}

CElite_Police::CElite_Police(const CElite_Police& Prototype)
	:CMonster_Base{Prototype}
{
}

HRESULT CElite_Police::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CElite_Police::Initialize(void* pArg)
{
	return E_NOTIMPL;
}

void CElite_Police::Priority_Update(_float fTimeDelta)
{
}

void CElite_Police::Update(_float fTimeDelta)
{
}

void CElite_Police::Late_Update(_float fTimeDelta)
{
}

HRESULT CElite_Police::Render()
{
	return E_NOTIMPL;
}

void CElite_Police::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::Update_State()
{
}
