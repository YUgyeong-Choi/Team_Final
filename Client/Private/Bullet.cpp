#include "Bullet.h"
#include "GameInstance.h"
#include "Player.h"

CBullet::CBullet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CProjectile{pDevice, pContext}
{
}

CBullet::CBullet(const CBullet& Prototype)
    :CProjectile{Prototype}
{
}

HRESULT CBullet::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBullet::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_fDamge = 10.f;

    // 이펙트?
    // 모르겟음 필요하면 넣기

    return S_OK;
}

void CBullet::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CBullet::Update(_float fTimeDelta)
{
    if (m_bDead)
        return;

    __super::Update(fTimeDelta);
}

void CBullet::Late_Update(_float fTimeDelta)
{
    if (m_bDead)
        return;

    __super::Late_Update(fTimeDelta);
}

HRESULT CBullet::Render()
{
    __super::Render();

    return S_OK;
}

void CBullet::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{

    if (eColliderType == COLLIDERTYPE::PLAYER)
    {
        CPlayer* pPlayer = static_cast<CPlayer*>(pOther);
        pPlayer->SetHitMotion(HITMOTION::NORMAL);
        pPlayer->SetfReceiveDamage(m_fDamge);
    }

   
    
    Set_bDead();

    if (m_pEffect)
        m_pEffect->End_Effect();
}

void CBullet::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CBullet::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CBullet::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBullet::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBullet::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT CBullet::Ready_Components()
{
    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
        return E_FAIL;

    return S_OK;
}

HRESULT CBullet::Ready_Effect()
{
    return S_OK;
}

CBullet* CBullet::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBullet* pInstance = new CBullet(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CBullet");
        Safe_Release(pInstance);
    }
    return pInstance;
}
CGameObject* CBullet::Clone(void* pArg)
{
    CBullet* pInstance = new CBullet(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Created : CBullet");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBullet::Free()
{
    __super::Free();
}
