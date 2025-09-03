#include "EliteUnit.h"
#include <Player.h>
#include "Weapon.h"
#include "GameInstance.h"
#include "Camera_Manager.h"
#include "LockOn_Manager.h"
#include "Client_Calculation.h"
#include <PlayerFrontCollider.h>
#include <PhysX_IgnoreSelfCallback.h>


CEliteUnit::CEliteUnit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUnit(pDevice, pContext)
{
}

CEliteUnit::CEliteUnit(const CEliteUnit& Prototype)
    : CUnit(Prototype)
{
	m_eUnitType = EUnitType::ELITE_MONSTER;
}

HRESULT CEliteUnit::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEliteUnit::Initialize(void* pArg)
{

    // 이 때 이미 셰이더, 애니메이터, 모델은 로드된 상태
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (pArg != nullptr)
    {
        UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);
        //월드행렬로 소환
        m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);
        m_InitWorldMatrix = pDesc->WorldMatrix;
    }
    
	if (FAILED(LoadFromJson()))
		return E_FAIL;

    // 여기서는 네비와 기본 바디 콜라이더 클론
    if (FAILED(Ready_Components(pArg)))
        return E_FAIL;

    Ready_BoneInformation();
    // 바디 콜라이더 액터 준비 자식 안에서는 이걸 부르면서 자기가 따로 설정한 액터들 처리
    if (FAILED(Ready_Actor()))
        return E_FAIL;

    m_pPlayer = m_pGameInstance->Get_Object(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"), 0);
   
    if (m_pNaviCom)
    {
        m_pNaviCom->Select_Cell(m_pTransformCom->Get_State(STATE::POSITION));
        _float fY = m_pNaviCom->Compute_NavigationY(m_pTransformCom->Get_State(STATE::POSITION));
        m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION), fY));
    }

    // 래이캐스트 머리쪽에 할려고 둔 offset !!!!
    m_vRayOffset = { 0.f, 3.3f, 0.f, 0.f };
    m_bUseLockon = true;
    Ready_EffectNames();


    if (FAILED(Ready_Effect()))
        return E_FAIL;

    Ready_SoundEvents();
    return S_OK;
}

void CEliteUnit::Priority_Update(_float fTimeDelta)
{
#ifdef _DEBUG
    if (KEY_DOWN(DIK_TAB))
    {
        cout << "현재 플레이어와의 거리 : " << Get_DistanceToPlayer() << endl;
        cout << "현재 애니메이션 상태 : " << m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName << endl;
        cout << "현재 이동 방향 " << m_pAnimator->GetInt("MoveDir") << endl;
    }
#endif
}

void CEliteUnit::Update(_float fTimeDelta)
{
    UpdateState(fTimeDelta); // 상태 업데이트
    __super::Update(fTimeDelta); // 애니메이션 재생
    Update_Collider(); // 콜라이더 업데이트

    if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom))
    {
        CLockOn_Manager::Get_Instance()->Add_LockOnTarget(this);
        _matrix LockonMat = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(m_iLockonBoneIndex));

        _vector vLockonPos = XMVector3TransformCoord(LockonMat.r[3], m_pTransformCom->Get_WorldMatrix());

        XMStoreFloat4(&m_vLockonPos, vLockonPos);
    }
    Spawn_Effect();
}

void CEliteUnit::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

#ifdef _DEBUG
    if (m_pGameInstance->Get_RenderCollider())
    {
        m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
    }
#endif
}



HRESULT CEliteUnit::LoadAnimationEventsFromJson(const string& modelName)
{
    string path = "../Bin/Save/AnimationEvents/" + modelName + "_events.json";
    ifstream ifs(path);
    if (ifs.is_open())
    {
        json root;
        ifs >> root;
        if (root.contains("animations"))
        {
            auto& animationsJson = root["animations"];
            auto& clonedAnims = m_pModelCom->GetAnimations();

            for (const auto& animData : animationsJson)
            {
                const string& clipName = animData["ClipName"];

                for (auto& pAnim : clonedAnims)
                {
                    if (pAnim->Get_Name() == clipName)
                    {
                        pAnim->Deserialize(animData);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        MSG_BOX("Failed to open animation events file.");
        return E_FAIL;
    }
    return S_OK;
}

HRESULT CEliteUnit::LoadAnimationStatesFromJson(const string& modelName)
{
    string path = "../Bin/Save/AnimationStates/" + modelName + "_States.json";
    ifstream ifsStates(path);
    if (ifsStates.is_open())
    {
        json rootStates;
        ifsStates >> rootStates;
        m_pAnimator->Deserialize(rootStates);
    }
    else
    {
        MSG_BOX("Failed to open animation states file.");
        return E_FAIL;
    }
    return S_OK;
}

HRESULT CEliteUnit::LoadFromJson()
{
	string modelName = m_pModelCom->Get_ModelName();
	if (FAILED(LoadAnimationEventsFromJson(modelName)))
		return E_FAIL;
	if (FAILED(LoadAnimationStatesFromJson(modelName)))
		return E_FAIL;
    return S_OK;
}

HRESULT CEliteUnit::Ready_Components(void* pArg)
{
    /* For.Com_PhysX */
    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
        return E_FAIL;

    _int iLevelIndex = m_pGameInstance->GetCurrentLevelIndex();
    if (iLevelIndex == ENUM_CLASS(LEVEL::JW))
    {
        return S_OK;
    }

    if (pArg != nullptr)
    {
        UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);

        //네비게이션 가져오기
        wstring wsPrototypeTag = TEXT("Prototype_Component_Navigation_") + pDesc->wsNavName; //어떤 네비를 탈 것인가 STAION, HOTEL...
        if (FAILED(__super::Add_Component(iLevelIndex, wsPrototypeTag.c_str(),
            TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNaviCom))))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CEliteUnit::Ready_Actor()
{
    _vector S, R, T;
    XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

    PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
    PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
    PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

    PxVec3 offset(0.f, -0.5f, 0.f); // Y축으로 0.5만큼 밑으로 내림
    PxTransform pose(positionVec + offset, rotationQuat);


    PxVec3 halfExtents = PxVec3(scaleVec.x * 1.2f, scaleVec.y * 1.7f, scaleVec.z * 1.3f);
    PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
    m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
    m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

    PxFilterData filterData{};
    filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
    filterData.word1 = WORLDFILTER::FILTER_PLAYERWEAPON | FILTER_PLAYERBODY; // 일단 보류
    m_pPhysXActorCom->Set_SimulationFilterData(filterData);
    m_pPhysXActorCom->Set_QueryFilterData(filterData);
    m_pPhysXActorCom->Set_Owner(this);
    m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::MONSTER);
    m_pPhysXActorCom->Set_Kinematic(true);
    m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

    return S_OK;
}

void CEliteUnit::HandleMovementDecision(_float fDistance, _float fTimeDelta)
{   
 //   // 가까우면 
 //   if (fDistance < m_fChasingDistance)
 //   {
 //       if (m_fChangeMoveDirCooldown > 0.f)
 //       {
 //           m_fChangeMoveDirCooldown -= fTimeDelta;
 //           m_fChangeMoveDirCooldown = max(m_fChangeMoveDirCooldown, 0.f);
 //       }
 //       else
 //       {
 //           _int iMoveDir = GetRandomInt(1, 3);
 //           m_pAnimator->SetInt("MoveDir", iMoveDir);
 //           m_fChangeMoveDirCooldown = 5.f;
	//		cout << "현재 이동 방향 : " << iMoveDir << endl;
 //       }
 //   }
 //   else if (fDistance >= m_fChasingDistance)
 //   {
 //       m_pAnimator->SetInt("MoveDir", 0);
 //       cout << "현재 프론트로 바뀜" << endl;
 //   }
 ////   m_eCurrentState = EEliteState::WALK;
    m_pAnimator->SetFloat("Distance", abs(fDistance));
}

void CEliteUnit::Update_Collider()
{
    _matrix worldMatrix = m_pTransformCom->Get_WorldMatrix();

    _float4 vPos;
    XMStoreFloat4(&vPos, worldMatrix.r[3]);

    PxVec3 pos(vPos.x, vPos.y + 2.f, vPos.z);
    XMVECTOR boneQuat = XMQuaternionRotationMatrix(worldMatrix);
    XMFLOAT4 fQuat;
    XMStoreFloat4(&fQuat, boneQuat);
    PxQuat rot = PxQuat(fQuat.x, fQuat.y, fQuat.z, fQuat.w);

    m_pPhysXActorCom->Set_Transform(PxTransform(pos, rot));

}

void CEliteUnit::UpdateState(_float fTimeDelta)
{
    if (!m_pPlayer)
        return;

    _uint iCurrentAnimStateNodeID = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->iNodeId;

    UpdateStateByNodeID(iCurrentAnimStateNodeID); // 애니메이션 상태에 따라 현재 상태 업데이트
    if (m_eCurrentState == EEliteState::CUTSCENE)
    {
        return; // 컷신 중에는 상태 업데이트를 하지 않음
    }

    if (m_eCurrentState == EEliteState::DEAD)
        return;
    UpdateSpecificBehavior(fTimeDelta);
    _float fDistance = Get_DistanceToPlayer();
    if (m_bGroggyActive)
    {
        m_fGroggyEndTimer -= fTimeDelta;
        if (m_fGroggyEndTimer <= 0.f)
        {
            m_bGroggyActive = false; 
            m_fGroggyGauge = 0.f;
#ifdef _DEBUG
			cout << "그로기 가능 시간 종료" << endl;
#endif
        }
    }
    UpdateAttackPattern(fDistance, fTimeDelta);// 공격 패턴 업데이트
    UpdateMovement(fDistance, fTimeDelta);
}

void CEliteUnit::UpdateMovement(_float fDistance, _float fTimeDelta)
{
  	_bool bCanMove = CanMove();

	m_pAnimator->SetBool("Move", bCanMove);

    if (bCanMove)
    {
		HandleMovementDecision(fDistance, fTimeDelta);
      
    }
    _bool bIsRootMotion = m_pAnimator->GetCurrentAnim()->IsRootMotionEnabled();

    if (bIsRootMotion)
    {
        ApplyRootMotionDelta(fTimeDelta); // 루트 모션 적용
    }
    else
    {
          UpdateNormalMove(fTimeDelta);
    }

    _vector vDir = GetTargetDirection();
    if (XMVector3Equal(vDir, XMVectorZero()))
        return; // 방향이 0이면 회전하지 않음
    vDir = XMVectorSetY(vDir, 0.f);
    vDir = XMVector3Normalize(vDir);


  //   현재 상태에서 많이 회전했으면 애니메이터 Turn true
    _vector vCurrentLook = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
    _float fDot = XMVectorGetX(XMVector3Dot(vCurrentLook, vDir));
    fDot = clamp(fDot, -1.f, 1.f);
    _vector vCross = XMVector3Cross(vCurrentLook, vDir);
    _float fSign = (XMVectorGetY(vCross) < 0.f) ? -1.f : 1.f;
    _float fYaw = acosf(fDot) * fSign; // 회전 각도 (라디안 단위) -180~180


    _bool bIsTurn = abs(XMConvertToDegrees(fYaw)) > m_fMinimumTurnAngle && (m_eCurrentState == EEliteState::IDLE ||
        m_eCurrentState == EEliteState::WALK || m_eCurrentState == EEliteState::RUN);

    if (bIsTurn && m_eCurrentState != EEliteState::TURN)
    {
        m_pAnimator->SetTrigger("Turn");
        m_pAnimator->SetInt("TurnDir", (fYaw >= 0.f) ? 0 : 1); // 0: 오른쪽, 1: 왼쪽
        m_pAnimator->SetBool("Move", false); // 회전 중에는 이동하지 않음
        m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::FRONT));
    }

    if (m_eCurrentState == EEliteState::TURN && CanProcessTurn())
    {
        m_pTransformCom->RotateToDirectionSmoothly(vDir, fTimeDelta);
    }
}

void CEliteUnit::UpdateAttackPattern(_float fDistance, _float fTimeDelta)
{
}

void CEliteUnit::UpdateStateByNodeID(_uint iNodeID)
{
}


void CEliteUnit::EnableColliders(_bool bEnable)
{
    if (bEnable)
    {
        m_pPhysXActorCom->Set_SimulationFilterData(m_pPhysXActorCom->Get_FilterData());
        if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
        {
            if (auto pController = pPlayer->Get_Controller())
            {
                pController->Remove_IgnoreActors(m_pPhysXActorCom->Get_Actor());
            }
        }
    }
    else
    {
        m_pPhysXActorCom->Init_SimulationFilterData();
        if (auto pPlayer = dynamic_cast<CPlayer*>(m_pPlayer))
        {
            if (auto pController = pPlayer->Get_Controller())
            {
                pController->Add_IngoreActors(m_pPhysXActorCom->Get_Actor());
            }
        }
    }
}

_bool CEliteUnit::CanMove() const
{
    return (m_eCurrentState == EEliteState::IDLE ||
        m_eCurrentState == EEliteState::WALK ||
        m_eCurrentState == EEliteState::RUN) &&
        m_eCurrentState != EEliteState::GROGGY &&
        m_eCurrentState != EEliteState::DEAD &&
        m_eCurrentState != EEliteState::PARALYZATION &&
        m_eCurrentState != EEliteState::ATTACK &&
        m_eCurrentState != EEliteState::FATAL;
}

_bool CEliteUnit::IsTargetInFront(_float fDectedAngle) const
{
    if (!m_pPlayer)
        return false;

    _vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
    _vector vThisPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vForward = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
    _vector vToPlayerXZ = XMVectorSetY(vPlayerPos - vThisPos, 0.f);
    _float fDot = XMVectorGetX(XMVector3Dot(vForward, vToPlayerXZ));
    fDot = clamp(fDot, -1.f, 1.f); // -1 ~ 1 사이로 제한
    _float fAngle = cosf(XMConvertToRadians(fDectedAngle)); // 시야각 60도 기준

    return fDot > fAngle; // 앞쪽에 있으면 true
}

_bool CEliteUnit::UpdateTurnDuringAttack(_float fTimeDelta)
{
    if (m_fTurnTimeDuringAttack >= 0.001f)
    {
        _vector vDir = GetTargetDirection();
        vDir = XMVectorSetY(vDir, 0.f);
        vDir = XMVector3Normalize(vDir);
        m_pTransformCom->RotateToDirectionSmoothly(vDir, fTimeDelta * m_fAddtiveRotSpeed);
        m_fTurnTimeDuringAttack -= fTimeDelta;
        return false;
    }

    m_fAddtiveRotSpeed = 1.f;
    m_fTurnTimeDuringAttack = 0.f; // 초기화
    return true;
}

_float CEliteUnit::Get_DistanceToPlayer() const
{
    if (!m_pPlayer)
        return FLT_MAX;

    _vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
    _vector vThisPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vDiff = vPlayerPos - vThisPos;
    _float fDistance = XMVectorGetX(XMVector3Length(vDiff));

    return fDistance;
}

_vector CEliteUnit::GetTargetDirection() const
{
    if (!m_pPlayer)
        return XMVectorZero();
    _vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
    _vector vThisPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vToPlayer = vPlayerPos - vThisPos;
    return XMVector3Normalize(vToPlayer); // 플레이어 방향 벡터 반환
}

void CEliteUnit::ApplyRootMotionDelta(_float fTimeDelta)
{

    _float3 rootMotionDelta = m_pAnimator->GetRootMotionDelta();

    _vector vLocal = XMLoadFloat3(&rootMotionDelta);
    vLocal = XMVectorScale(vLocal, m_fRootMotionAddtiveScale);

    // 현재 트랜스폼 분해 (스케일, 회전, 위치)
    _vector vScale, vCurRotQuat, vTrans;
    XMMatrixDecompose(&vScale, &vCurRotQuat, &vTrans, m_pTransformCom->Get_WorldMatrix());


    _vector vWorldDelta = XMVector3Transform(vLocal, XMMatrixRotationQuaternion(vCurRotQuat));
    vWorldDelta = XMVectorSetY(vWorldDelta, 0.f);

    if (m_pPlayer && m_bRootMotionClamped && m_eCurrentState !=EEliteState::FATAL)
    {
        _float fDistToPlayer = Get_DistanceToPlayer();

        _float fFactor = clamp(fDistToPlayer / 2.5f, 0.f, 1.f);
        vWorldDelta *= fFactor;
    }

    _float fDeltaMag = XMVectorGetX(XMVector3Length(vWorldDelta));

    if (fDeltaMag < 1e-6f)
    {
        m_PrevWorldDelta = XMVectorZero();
        return;
    }

    _float fAnimSafeStep = m_fMaxRootMotionSpeed /60.f;
    if (fDeltaMag > fAnimSafeStep)
        vWorldDelta = XMVector3Normalize(vWorldDelta) * fAnimSafeStep;

    m_PrevWorldDelta = vWorldDelta;
    _vector vNext = XMVectorAdd(vTrans, vWorldDelta);

    // 네비 보정
    if (m_pNaviCom)
    {
        if (m_pNaviCom->isMove(vNext))
        {
            _float fY = m_pNaviCom->Compute_NavigationY(vNext);
            vTrans = XMVectorSetY(vNext, fY);
        }
        else
        {
            _vector vSlideDir = m_pNaviCom->GetSlideDirection(vNext, XMVector3Normalize(vWorldDelta));
            _vector vSlidePos = vTrans + vSlideDir * min(fDeltaMag, m_fSlideClamp);

            if (m_pNaviCom->isMove(vSlidePos))
            {
                _float fY = m_pNaviCom->Compute_NavigationY(vSlidePos);
                vTrans = XMVectorSetY(vSlidePos, fY);
            }
            else
            {
                vTrans = XMVectorSetY(vTrans, m_pNaviCom->Compute_NavigationY(vTrans));
            }
        }
    }

    _matrix newWorld =
        XMMatrixScalingFromVector(vScale) *
        XMMatrixRotationQuaternion(vCurRotQuat) *   // 회전은 그대로
        XMMatrixTranslationFromVector(vTrans);      // 위치만 적용

    m_pTransformCom->Set_WorldMatrix(newWorld);
}

void CEliteUnit::UpdateNormalMove(_float fTimeDelta)
{
    if (m_eCurrentState == EEliteState::WALK || m_eCurrentState == EEliteState::RUN)
    {
        _vector vTargetPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);

        _int iMoveDir = m_pAnimator->GetInt("MoveDir");
        if (m_fChangeMoveDirCooldown > 0.f)
            m_fChangeMoveDirCooldown -= fTimeDelta;
        switch (iMoveDir)
        {
        case ENUM_CLASS(EMoveDirection::FRONT):
        {
           _bool bCanChase =  m_pTransformCom->ChaseWithOutY(vTargetPos, fTimeDelta, m_fChasingDistance, nullptr, m_pNaviCom);
           if (bCanChase == false && m_fChangeMoveDirCooldown <= 0.f)
           {
               m_ePrevState = m_eCurrentState;
               // 랜덤 방향 전환
               int randDir = GetRandomInt(1, 3); // BACK, LEFT, RIGHT
               m_pAnimator->SetInt("MoveDir", randDir);

               m_eCurrentState = EEliteState::WALK;
               m_pAnimator->SetBool("Move", true);

               m_fChangeMoveDirCooldown = 1.5f; // 자주 안 바뀌게 쿨타임
               return;
           }
        }
            break;
        case ENUM_CLASS(EMoveDirection::BACK):
        case ENUM_CLASS(EMoveDirection::LEFT):
        case ENUM_CLASS(EMoveDirection::RIGHT):
			_bool bCanChase = Get_DistanceToPlayer() > m_fChasingDistance;
            if (bCanChase && m_fChangeMoveDirCooldown <= 0.f)
            {
                m_pAnimator->SetInt("MoveDir", ENUM_CLASS(EMoveDirection::FRONT));
                return;
            }
			if (iMoveDir == ENUM_CLASS(EMoveDirection::BACK))
				m_pTransformCom->Go_Backward(fTimeDelta, nullptr, m_pNaviCom);
			else if (iMoveDir == ENUM_CLASS(EMoveDirection::LEFT))
				m_pTransformCom->Go_Left(fTimeDelta, nullptr, m_pNaviCom);
			else if (iMoveDir == ENUM_CLASS(EMoveDirection::RIGHT))
				m_pTransformCom->Go_Right(fTimeDelta, nullptr, m_pNaviCom);
            break;
        }

        _float fY = m_pNaviCom->Compute_NavigationY(m_pTransformCom->Get_State(STATE::POSITION));
        m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION), fY));
    }
}

void CEliteUnit::Reset()
{
	m_fHp = m_fMaxHp;
    m_eCurrentState = EEliteState::IDLE;
	m_ePrevState = EEliteState::IDLE;
    m_bIsFirstAttack = true;
	m_fGroggyEndTimer = 0.f;
    m_fGroggyThreshold = 1.f;
	m_fGroggyGauge = 0.f;
	m_bGroggyActive = false;
    m_bUseLockon = true;
    m_fChangeMoveDirCooldown = 0.f;
	Safe_Release(m_pHPBar);
    m_pTransformCom->Set_WorldMatrix(m_InitWorldMatrix);
    if (m_pAnimator)
    {
        m_pAnimator->Reset();
      //  m_pAnimator->ResetParameters();
        auto pController = m_pAnimator->Get_CurrentAnimController();
        if (pController)
        {
            pController->SetState(pController->GetEntryNodeId());
        }
    }
    m_bRootMotionClamped = false;
    SwitchFury(false, 1.f);
    SwitchEmissive(false, 1.f);
    m_iCurNodeID = -1;
    m_iPrevNodeID = -1;
}

void CEliteUnit::Register_Events()
{
    m_pAnimator->RegisterEventListener("CameraShake",
        [this]()
        {
            CCamera_Manager::Get_Instance()->Shake_Camera(0.15f, 0.2f);
        });
    m_pAnimator->RegisterEventListener("ResetAnim", [this]()
        {
            m_pAnimator->GetCurrentAnim()->ResetTrack();
        });

    m_pAnimator->RegisterEventListener("SlowAnimSpeed", [this]() {
        m_pAnimator->SetPlayRate(0.4f);
        });
    m_pAnimator->RegisterEventListener("ResetAnimSpeed", [this]() {
        m_pAnimator->SetPlayRate(1.f);
        });

    m_pAnimator->RegisterEventListener("FastAnimSpeed", [this]() {
        m_pAnimator->SetPlayRate(1.3f);
        });

	m_pAnimator->RegisterEventListener("OnFury", [this]() {
        SwitchFury(true, 1.f);
		});
	m_pAnimator->RegisterEventListener("OffFury", [this]() {
		SwitchFury(false, 1.f);
		});
}

void CEliteUnit::EnterFatalHit()
{
    if (m_pAnimator && m_eCurrentState == EEliteState::GROGGY && m_eCurrentState != EEliteState::FATAL
        && m_eCurrentState != EEliteState::DEAD)
    {
		m_pAnimator->SetTrigger("Fatal");
		m_fGroggyEndTimer = 0.f;
		m_bGroggyActive = false;
		m_fGroggyGauge = 0.f;
        SwitchEmissive(false, 1.f);
#ifdef _DEBUG
        cout << "Elite Fatal Attack" << endl;
#endif
    }
}

void CEliteUnit::NotifyPerfectGuarded()
{
    m_fGroggyGauge += m_fGroggyScale_Charge;
    if (m_fGroggyGauge >= m_fGroggyThreshold && !m_bGroggyActive)
    {
        m_bGroggyActive = true;                  // 화이트 게이지 시작
        m_fGroggyEndTimer = m_fGroggyTimer;
        m_fGroggyGauge = m_fGroggyThreshold;
#ifdef _DEBUG
        cout << "그로기 가능" << endl;
#endif
        return;
    }
}

void CEliteUnit::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
    
}

void CEliteUnit::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
 
}

void CEliteUnit::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CEliteUnit::ReceiveDamage(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
    if (nullptr == pOther)
        return;
    if (m_eCurrentState == EEliteState::DEAD)
        return;
    if (COLLIDERTYPE::PLAYER_WEAPON == eColliderType)
    {
        auto pWeapon = static_cast<CWeapon*>(pOther);

        if (pWeapon->Find_CollisonObj(this, eColliderType))
            return;

        pWeapon->Add_CollisonObj(this);
		pWeapon->Calc_Durability(3);

        _float fDamage = 0.f;
		if (m_eUnitType == EUnitType::ELITE_MONSTER)
			fDamage = pWeapon->Get_CurrentDamage() * 0.25f;
		else
			fDamage = pWeapon->Get_CurrentDamage() * 0.15f;
        m_fHp -= fDamage;

        if (nullptr != m_pHPBar)
        {
            m_pHPBar->Add_Damage(fDamage);
            m_pHPBar->Set_RenderTime(3.f);
        }
         

		m_fHp = max(m_fHp, 0.f);
        cout << "몬스터 현재 체력 : " << m_fHp << endl;

		_int iLevelIndex = m_pGameInstance->GetCurrentLevelIndex();
        auto pPlayer = GET_PLAYER(iLevelIndex);

        if (pPlayer == nullptr)
            return;
        // cout << "플레이어 충돌" << endl;
        auto playerState = pPlayer->Get_PlayerState();

        switch (playerState)
        {
        case Client::EPlayerState::WEAKATTACKA:
        case Client::EPlayerState::WEAKATTACKB:
			m_fGroggyGauge += m_fGroggyScale_Weak;
			break;
        case Client::EPlayerState::STRONGATTACKA:
		case Client::EPlayerState::STRONGATTACKB:
            m_fGroggyGauge += m_fGroggyScale_Strong;
            break;
        case Client::EPlayerState::CHARGEA:
        case Client::EPlayerState::CHARGEB:
            m_fGroggyGauge += m_fGroggyScale_Charge;
            if (m_bGroggyActive)
            {
                SwitchEmissive(false, 1.f);
                m_pAnimator->SetTrigger("Groggy");
				m_eCurrentState = EEliteState::GROGGY;
                m_bGroggyActive = false;
                m_fGroggyGauge = 0.f;
                SwitchFury(false, 1.f);
            }
            break;
        case Client::EPlayerState::GARD:
            break;
        case Client::EPlayerState::USEITEM:
            break;
        case Client::EPlayerState::SHILD:
            break;
        case Client::EPlayerState::MAINSKILL:
            break;
        case Client::EPlayerState::ARMATTACKCHARGE:
            break;
        case Client::EPlayerState::ARMFAIL:
            break;
        case Client::EPlayerState::HITED:
            break;
        case Client::EPlayerState::FATAL:
            break;
        case Client::EPlayerState::END:
            break;
        default:
            break;
        }

        if (m_fGroggyGauge >= m_fGroggyThreshold && !m_bGroggyActive)
        {
            m_bGroggyActive = true;                  // 화이트 게이지 시작
            m_fGroggyEndTimer = m_fGroggyTimer;
            m_fGroggyGauge = m_fGroggyThreshold;
#ifdef _DEBUG
            cout << "그로기 가능" << endl;
#endif
            return;
        }

#ifdef _DEBUG
        cout << "그로기 게이지 : " << m_fGroggyGauge << endl;
        cout << "그로기 상태 : " << m_bGroggyActive << endl;
#endif
    }
}

void CEliteUnit::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CEliteUnit::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CEliteUnit::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

CGameObject* CEliteUnit::Clone(void* pArg)
{
	CEliteUnit* pInstance = new CEliteUnit(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CEliteUnit");
		Safe_Release(pInstance);
	}
	return pInstance;
}
void CEliteUnit::Free()
{
    __super::Free();
    Safe_Release(m_pNaviCom);
    Safe_Release(m_pHPBar);
}
