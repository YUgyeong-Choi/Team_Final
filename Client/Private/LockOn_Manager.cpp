#include "LockOn_Manager.h"
#include "GameInstance.h"

#include "PhysX_IgnoreSelfCallback.h"
#include "Player.h"
#include "Unit.h"
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
    if (!m_pPlayer)
        return S_OK;

    if (m_bStartLockOn)
    {
        RemoveSomeTargets();
        CGameObject* pTarget = Find_ClosestToLookTarget();
        if (pTarget)
        {
            m_bActive = true;
            m_pBestTarget = pTarget;
            CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_LockOn(m_pBestTarget, true);
        }
        else
        {
            XMVECTOR backDir = XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -1;
            CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_TargetYawPitch(backDir,15.f);
        }
        m_bStartLockOn = false;
    }

    if (m_bActive && m_bCanChangeTarget)
    {
        CGameObject* pObj = Change_ToLookTarget();
        if (pObj)
        {
            m_bCanChangeTarget = false;
            m_pBestTarget = pObj;
            CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_LockOn(m_pBestTarget, true);
        }
    }

    if (!m_bCanChangeTarget)
    {
        m_fCoolChangeTarget += fTimeDelta;
        if (m_fCoolChangeTarget > 1.f)
        {
            printf("타겟 변경 가능\n");
            m_fCoolChangeTarget = 0.f;
            m_bCanChangeTarget = true;
        }
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
            CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_LockOn(m_pBestTarget, false);

            // Pitch Yaw 역계산
            XMVECTOR camerakDir = XMVector3Normalize(CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_TransfomCom()->Get_State(STATE::LOOK) * -1);
            const _float bx = XMVectorGetX(camerakDir);
            const _float by = XMVectorGetY(camerakDir);
            const _float bz = XMVectorGetZ(camerakDir);

            _float fYaw = atan2f(bx, bz);
            _float fPitch = atan2f(by, sqrtf(bx * bx + bz * bz));
            
            CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_PitchYaw(fPitch, fYaw);
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

void CLockOn_Manager::RemoveSomeTargets()
{
    _vector playerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) + _vector{ 0.f,0.5f,0.f,0.f };

    for (size_t i = 0; i < m_vecTarget.size(); )
    {
        CGameObject* pTarget = m_vecTarget[i];
        _vector targetPos = pTarget->Get_TransfomCom()->Get_State(STATE::POSITION) + _vector{ 0.f,0.5f,0.f,0.f };

        bool bRemove = false;

        // HP 0 이하 제거
        CUnit* pUnit = static_cast<CUnit*>(pTarget);
        if (!pUnit || !pUnit->Get_UseLockon()) {
            bRemove = true;
        }

        // 벽 뒤 체크
        if (!bRemove)
        {
            PxVec3 origin = VectorToPxVec3(playerPos);
            PxVec3 direction = VectorToPxVec3(targetPos - playerPos);
            direction.normalize();
            _float fRayLength = 7.f;

            PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
            PxRaycastBuffer hit;
            PxQueryFilterData filterData;
            filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

            CPlayer* pPlayer = static_cast<CPlayer*>(m_pPlayer);
            unordered_set<PxActor*> ignoreActors = pPlayer->Get_Controller()->Get_IngoreActors();
            CIgnoreSelfCallback callback(ignoreActors);

            if (m_pGameInstance->Get_Scene()->raycast(origin, direction, fRayLength, hit, hitFlags, filterData, &callback))
            {
                if (hit.hasBlock)
                {
                    PxRigidActor* hitActor = hit.block.actor;
                    CPhysXActor* pHitActor = static_cast<CPhysXActor*>(hitActor->userData);

                    if (pHitActor && pHitActor->Get_ColliderType() != COLLIDERTYPE::MONSTER)
                    {
                        // 벽 등 다른 오브젝트가 먼저 막음 → 제거
                        bRemove = true;
                    }
                }
                else
                {
                    // 아무것도 맞지 않음 → 거리 밖 등 → 제거
                    bRemove = true;
                }
            }
            else
            {
                bRemove = true;
            }
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
    _vector       vPlayerLook = CCamera_Manager::Get_Instance()->GetCurCam()->Get_TransfomCom()->Get_State(STATE::LOOK);
    vPlayerLook = XMVector3Normalize(vPlayerLook);

    // ===== 설정 =====
    const _float wAngle = 0.65f;                     // 각도 가중치(정면 우선)
    const _float wDist = 0.35f;                     // 거리 가중치(가까운 대상 우선)
    const _float cosHalfFov = cosf(XMConvertToRadians(60.f)); // 정면 ±45°

    // ===== 1) FOV(±45°) 안의 타깃만 대상으로 최대 거리 계산 =====
    _float maxDist2 = 0.f;
    int inFovCount = 0;
    for (auto& pTarget : m_vecTarget)
    {
        if (!pTarget) continue;

        const _vector vTargetPos = pTarget->Get_TransfomCom()->Get_State(STATE::POSITION);
        const _vector vDelta = vTargetPos - vPlayerPos;
        const _vector vToTarget = XMVector3Normalize(vDelta);

        const _float fDot = XMVectorGetX(XMVector3Dot(vPlayerLook, vToTarget)); // [-1,1]
        if (fDot < cosHalfFov)          // FOV 밖 → 스킵
            continue;

        const _float dist2 = XMVectorGetX(XMVector3LengthSq(vDelta));
        if (dist2 > maxDist2) maxDist2 = dist2;
        ++inFovCount;
    }

    if (inFovCount == 0)                // FOV 안에 아무도 없으면 락온 불가
        return nullptr;

    if (maxDist2 < 1e-12f)              // 안전 가드
        maxDist2 = 1e-12f;

    // ===== 2) 후보들 중 각도+거리 점수 최소 선택 =====
    CGameObject* pBestTarget = nullptr;
    _float bestScore = FLT_MAX;

    for (auto& pTarget : m_vecTarget)
    {
        if (!pTarget) continue;

        const _vector vTargetPos = pTarget->Get_TransfomCom()->Get_State(STATE::POSITION);
        const _vector vDelta = vTargetPos - vPlayerPos;
        const _vector vToTarget = XMVector3Normalize(vDelta);

        _float fDot = XMVectorGetX(XMVector3Dot(vPlayerLook, vToTarget));
        if (fDot < cosHalfFov)          // FOV 밖 → 스킵
            continue;

        fDot = clamp(fDot, -1.f, 1.f);

        // 각도 정규화: 0(정면) ~ 1(반대)
        const _float fAngle = acosf(fDot);          // [0, π]
        const _float angleNorm = fAngle / XM_PI;

        // 거리 정규화: 0(가깝) ~ 1(가장 멀리) (FOV 내 최대거리 기준)
        const _float dist2 = XMVectorGetX(XMVector3LengthSq(vDelta));
        const _float distNorm = sqrtf(dist2 / maxDist2);

        const _float score = wAngle * angleNorm + wDist * distNorm;

        if (score < bestScore) {
            bestScore = score;
            pBestTarget = pTarget;
        }
    }

    if (pBestTarget)
        wprintf(L"TargetName %s\n", pBestTarget->Get_Name().c_str());

    return pBestTarget;
}

CGameObject* CLockOn_Manager::Change_ToLookTarget()
{
    if (!m_pPlayer || m_vecTarget.empty())
        return nullptr;

    // 마우스 X 이동량으로 좌/우 선호 결정
    const _long mouseX = m_pGameInstance->Get_DIMouseMove(DIMM::X);
    const _float  kDeadZone = 50.f;
    int sidePref = 0;
    if (mouseX <= -kDeadZone) sidePref = +1;    // 왼쪽
    else if (mouseX >= kDeadZone) sidePref = -1; // 오른쪽
    else
        return nullptr; // 미세 이동이면 현재 대상 유지

    // 플레이어 기준 벡터
    const _vector P = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
    const _vector F = XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK));
    const _vector Up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    const _vector R = XMVector3Normalize(XMVector3Cross(Up, F));

    // 현재 락온 대상 제외
    CGameObject* pCurrent = m_pBestTarget;

    CGameObject* best = nullptr;
    _float bestAngle = XM_PI;

    for (auto* t : m_vecTarget)
    {
        if (t == pCurrent) continue;

        const _vector T = t->Get_TransfomCom()->Get_State(STATE::POSITION);
        const _vector V = XMVector3Normalize(T - P);

        // 좌/우 판정
        const _float side = XMVectorGetX(XMVector3Dot(V, R));
        if ((sidePref > 0 && side <= 0.f) || (sidePref < 0 && side >= 0.f))
            continue;

        // 각도 계산
        _float fDot = XMVectorGetX(XMVector3Dot(F, V));
        fDot = clamp(fDot, -1.f, 1.f);
        _float fAngle = acosf(fDot);

        if (fAngle < bestAngle)
        {
            bestAngle = fAngle;
            best = t;
        }
    }

    // 선호 방향에 후보가 없으면 현재 대상 유지
    return best ? best : nullptr;
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

void CLockOn_Manager::Set_Active()
{ 
    if (!m_bActive && !m_bStartLockOn)
    {
        m_bStartLockOn = true;
    }

    if (m_bActive)
    {
        m_bActive = false;
        m_pBestTarget = nullptr;
        CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_LockOn(m_pBestTarget, false);
    }
}

void CLockOn_Manager::Set_Off(CGameObject* pObj)
{
    if (m_pBestTarget != pObj)
        return;

    m_bActive = false;
    m_pBestTarget = nullptr;
    CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_LockOn(m_pBestTarget, false);

    // Pitch Yaw 역계산
    XMVECTOR camerakDir = XMVector3Normalize(CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_TransfomCom()->Get_State(STATE::LOOK) * -1);
    const _float bx = XMVectorGetX(camerakDir);
    const _float by = XMVectorGetY(camerakDir);
    const _float bz = XMVectorGetZ(camerakDir);

    _float fYaw = atan2f(bx, bz);
    _float fPitch = atan2f(by, sqrtf(bx * bx + bz * bz));

    CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_PitchYaw(fPitch, fYaw);
}



void CLockOn_Manager::Free()
{
    __super::Free();

    
}