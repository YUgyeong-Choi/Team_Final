#include "Buttler_Basic.h"

CButtler_Basic::CButtler_Basic(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)   
    :CMonster_Base{pDevice, pContext}
{
}

CButtler_Basic::CButtler_Basic(const CButtler_Basic& Prototype)
    :CMonster_Base{Prototype}
{
}

HRESULT CButtler_Basic::Initialize_Prototype()
{
    return E_NOTIMPL;
}

HRESULT CButtler_Basic::Initialize(void* pArg)
{
    return E_NOTIMPL;
}

void CButtler_Basic::Priority_Update(_float fTimeDelta)
{
}

void CButtler_Basic::Update(_float fTimeDelta)
{
}

void CButtler_Basic::Late_Update(_float fTimeDelta)
{
}

HRESULT CButtler_Basic::Render()
{
    return E_NOTIMPL;
}

void CButtler_Basic::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CButtler_Basic::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CButtler_Basic::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CButtler_Basic::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Basic::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Basic::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Basic::Update_State()
{
}

void CButtler_Basic::Attack(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Basic::AttackWithWeapon(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Basic::ReceiveDamage(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Basic::Calc_Pos(_float fTimeDelta)
{
}

void CButtler_Basic::Register_Events()
{
}

void CButtler_Basic::Block_Reaction()
{
}

void CButtler_Basic::Start_Fatal_Reaction()
{
}

void CButtler_Basic::Reset()
{
}

HRESULT CButtler_Basic::Ready_Weapon()
{
    return E_NOTIMPL;
}

CButtler_Basic* CButtler_Basic::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CGameObject* CButtler_Basic::Clone(void* pArg)
{
    return nullptr;
}

void CButtler_Basic::Free()
{
}
