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

    if (FAILED(Ready_Effect()))
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

    CEffectContainer::DESC desc = {};
    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixTranslation(vPos.m128_f32[0], vPos.m128_f32[1], vPos.m128_f32[2]));
    if (nullptr == MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Projectile_Gun_Hit_P3"), &desc))
        MSG_BOX("이펙트 생성 실패함");
    
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
    CEffectContainer::DESC desc = {};
    desc.pSocketMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
    XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixIdentity());
    m_pEffect = dynamic_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_Projectile_Gun_Trail_P1"), &desc));
    if (nullptr == m_pEffect)
        MSG_BOX("이펙트 생성 실패함");

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
