#include "LockOn_Manager.h"
#include "GameInstance.h"

#include "PhysX_IgnoreSelfCallback.h"
#include "Player.h"

#include "Camera_Manager.h"

IMPLEMENT_SINGLETON(CLockOn_Manager)

CLockOn_Manager::CLockOn_Manager() 
    :m_pGameInstance{ CGameInstance::Get_Instance() }
{
}

HRESULT CLockOn_Manager::Initialize(LEVEL eLevel)
{

    return S_OK;
}

HRESULT CLockOn_Manager::Priority_Update(_float fTimeDelta)
{
    return S_OK;
}

HRESULT CLockOn_Manager::Update(_float fTimeDelta)
{
    if (m_bStartLockOn)
    {
        RemoveBehindWallTargets();
        CGameObject* pTarget = Find_ClosestToLookTarget();
        if (pTarget)
        {
            m_bActive = true;
            m_pBestTarget = pTarget;
            // 타겟 락온 카메라에 넘겨주기 ?
        }
        else
        {
            CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_OrbitalPosBackLookFront();
            // 오비탈 카메라 플레이어 시야에 맞게
        }
        m_bStartLockOn = false;
    }

    PxVec3 hitPos = PxVec3();
    _bool bHit = false;

    if (m_bActive)
    {
        //wprintf(L"LockOnTarget: %s\n", m_pBestTarget->Get_Name().c_str());

        _vector playerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) + _vector{ 0.f,0.5f,0.f,0.f };

        _vector targetPos = m_pBestTarget->Get_TransfomCom()->Get_State(STATE::POSITION) + _vector{ 0.f,0.5f,0.f,0.f };

        PxVec3 origin = VectorToPxVec3(playerPos);
        PxVec3 direction = VectorToPxVec3(targetPos - playerPos);
        direction.normalize(); // 방향 벡터 정규화
        _float fRayLength = 10.f;

        PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
        PxRaycastBuffer hit;
        PxQueryFilterData filterData;
        filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

        CPlayer* pPlayer = static_cast<CPlayer*>(m_pPlayer);
        unordered_set<PxActor*> ignoreActors = pPlayer->Get_Controller()->Get_IngoreActors();
        CIgnoreSelfCallback callback(ignoreActors);

        _bool bRemove = false;
        // 레이캐스트 수행
        if (m_pGameInstance->Get_Scene()->raycast(origin, direction, fRayLength, hit, hitFlags, filterData, &callback))
        {
            if (hit.hasBlock)
            {
                PxRigidActor* hitActor = hit.block.actor;
                CPhysXActor* pHitActor = static_cast<CPhysXActor*>(hitActor->userData);

                if (pHitActor && pHitActor->Get_ColliderType() != COLLIDERTYPE::MONSTER)
                {
                    // 다른 오브젝트(벽 등)가 레이에 먼저 걸림 → 타겟에서 제거
                    bRemove = true;
                }

                bHit = true;
                hitPos = hit.block.position;
            }
            else
            {
                // 레이에 아무것도 맞지 않음 → 거리 밖으로 간 것 → 제거
                bRemove = true;
            }
        }
        else
        {
            bRemove = true;
        }

        if (bRemove)
        {
            m_bActive = false;
            m_pBestTarget = nullptr;
        }

#ifdef _DEBUG
        if (m_pGameInstance->Get_RenderCollider()) {
            DEBUGRAY_DATA _data{};

            _data.vStartPos = origin;
            _data.vDirection = direction;
            _data.fRayLength = 10.f;
            _data.bIsHit = bHit;
            _data.vHitPos = hitPos;
            pPlayer->Get_Controller()->Add_RenderRay(_data);
        }

#endif
    }

    return S_OK;
}

void CLockOn_Manager::RemoveBehindWallTargets()
{
    _vector playerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) + _vector{ 0.f,0.5f,0.f,0.f };

    for (size_t i = 0; i < m_vecTarget.size(); )
    {
        CGameObject* pTarget = m_vecTarget[i];
        _vector targetPos = pTarget->Get_TransfomCom()->Get_State(STATE::POSITION) + _vector{ 0.f,0.5f,0.f,0.f };

        PxVec3 origin = VectorToPxVec3(playerPos);
        PxVec3 direction = VectorToPxVec3(targetPos - playerPos);
        direction.normalize(); // 방향 벡터 정규화
        _float fRayLength = 10.f;

        PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
        PxRaycastBuffer hit;
        PxQueryFilterData filterData;
        filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

        CPlayer* pPlayer = static_cast<CPlayer*>(m_pPlayer);
        unordered_set<PxActor*> ignoreActors = pPlayer->Get_Controller()->Get_IngoreActors();
        CIgnoreSelfCallback callback(ignoreActors);

        bool bRemove = false;

        // 레이캐스트 수행
        if (m_pGameInstance->Get_Scene()->raycast(origin, direction, fRayLength, hit, hitFlags, filterData, &callback))
        {
            if (hit.hasBlock)
            {
                PxRigidActor* hitActor = hit.block.actor;
                CPhysXActor* pHitActor = static_cast<CPhysXActor*>(hitActor->userData);

                if (pHitActor && pHitActor->Get_Owner()->Get_Name() != L"Elite_Police")
                {
                    // 다른 오브젝트(벽 등)가 레이에 먼저 걸림 → 타겟에서 제거
                    bRemove = true;
                }
            }
            else
            {
                // 레이에 아무것도 맞지 않음 → 거리 밖으로 간 것 → 제거
                bRemove = true;
            }
        }
        else
        {
            bRemove = true;
        } 

        if (bRemove)
            m_vecTarget.erase(m_vecTarget.begin() + i);
        else
            ++i;
    }
}

CGameObject* CLockOn_Manager::Find_ClosestToLookTarget()
{
    if (!m_pPlayer || m_vecTarget.empty())
        return nullptr;

    const _vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
    const _vector vPlayerLook = m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK);

    CGameObject* pBestTarget = nullptr;
    float fMinAngle = XM_PI; // 180도

    for (auto& pTarget : m_vecTarget)
    {
        const _vector vTargetPos = pTarget->Get_TransfomCom()->Get_State(STATE::POSITION);
        const _vector vToTarget = XMVector3Normalize(vTargetPos - vPlayerPos);

        float fDot = XMVectorGetX(XMVector3Dot(vPlayerLook, vToTarget));
        fDot = clamp(fDot, -1.f, 1.f); 
        float fAngle = acosf(fDot);   

        if (fAngle < fMinAngle)
        {
            fMinAngle = fAngle;
            pBestTarget = pTarget;
        }
    }

    return pBestTarget;
}

HRESULT CLockOn_Manager::Late_Update(_float fTimeDelta)
{
    m_vecTarget.clear();
    return S_OK;
}

HRESULT CLockOn_Manager::Render()
{
    return S_OK;
}


void CLockOn_Manager::SetPlayer(CGameObject* pPlayer)
{
    m_pPlayer = pPlayer;
}

void CLockOn_Manager::Add_LockOnTarget(CGameObject* pTarget)
{
    m_vecTarget.push_back(pTarget);
}



void CLockOn_Manager::Free()
{
    __super::Free();

}