#include "BossUnit.h"
#include <Player.h>
#include "Weapon.h"
#include "LockOn_Manager.h"
#include "Client_Calculation.h"
#include <PhysX_IgnoreSelfCallback.h>

CBossUnit::CBossUnit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUnit(pDevice, pContext)
{
}

CBossUnit::CBossUnit(const CBossUnit& Prototype)
    : CUnit(Prototype)
{
	m_eUnitType = EUnitType::BOSS;
}

HRESULT CBossUnit::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBossUnit::Initialize(void* pArg)
{

    // 이 때 이미 셰이더, 애니메이터, 모델은 로드된 상태
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (pArg != nullptr)
    {
        UNIT_DESC* pDesc = static_cast<UNIT_DESC*>(pArg);
        //월드행렬로 소환
        m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);
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

    Ready_AttackPatternWeightForPhase1();

    // 래이캐스트 머리쪽에 할려고 둔 offset !!!!
    m_vRayOffset = { 0.f, 3.3f, 0.f, 0.f };
    m_bUseLockon = true;
    Ready_EffectNames();


    if (FAILED(Ready_Effect()))
        return E_FAIL;

    // 컷씬 시작 전 대기
    m_pAnimator->Update(0.f);
    m_pAnimator->SetPlaying(false);
    m_pModelCom->Update_Bones();
    return S_OK;
}

void CBossUnit::Priority_Update(_float fTimeDelta)
{
#ifdef _DEBUG
    if (KEY_DOWN(DIK_TAB))
    {
        cout << "현재 플레이어와의 거리 : " << Get_DistanceToPlayer() << endl;
        cout << "현재 애니메이션 상태 : " << m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName << endl;
        cout << "현재 이동 방향 " << m_pAnimator->GetInt("MoveDir") << endl;
    }
#endif

    if (CalculateCurrentHpRatio() <= m_fPhase2HPThreshold && m_bIsPhase2 == false)
    {
        Ready_AttackPatternWeightForPhase2();
    }
}

void CBossUnit::Update(_float fTimeDelta)
{
    UpdateBossState(fTimeDelta); // 상태 업데이트
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

void CBossUnit::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

#ifdef _DEBUG
    if (m_pGameInstance->Get_RenderCollider())
    {
        m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
    }
#endif
}



HRESULT CBossUnit::LoadAnimationEventsFromJson(const string& modelName)
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

HRESULT CBossUnit::LoadAnimationStatesFromJson(const string& modelName)
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

HRESULT CBossUnit::LoadFromJson()
{
	string modelName = m_pModelCom->Get_ModelName();
	if (FAILED(LoadAnimationEventsFromJson(modelName)))
		return E_FAIL;
	if (FAILED(LoadAnimationStatesFromJson(modelName)))
		return E_FAIL;
    return S_OK;
}

HRESULT CBossUnit::Ready_Components(void* pArg)
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

HRESULT CBossUnit::Ready_Actor()
{
    _vector S, R, T;
    XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

    PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
    PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
    PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

    PxTransform pose(positionVec, rotationQuat);

    PxVec3 halfExtents = PxVec3(scaleVec.x * 1.2f, scaleVec.y * 1.7f, scaleVec.z * 1.5f);
    PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
    m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
    m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

    PxFilterData filterData{};
    filterData.word0 = WORLDFILTER::FILTER_MONSTERBODY;
    filterData.word1 = WORLDFILTER::FILTER_PLAYERWEAPON; // 일단 보류
    m_pPhysXActorCom->Set_SimulationFilterData(filterData);
    m_pPhysXActorCom->Set_QueryFilterData(filterData);
    m_pPhysXActorCom->Set_Owner(this);
    m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::MONSTER);
    m_pPhysXActorCom->Set_Kinematic(true);
    m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

    return S_OK;
}

void CBossUnit::Update_Collider()
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

void CBossUnit::UpdateBossState(_float fTimeDelta)
{
    if (!m_pPlayer)
        return;

    _uint iCurrentAnimStateNodeID = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->iNodeId;

    UpdateStateByNodeID(iCurrentAnimStateNodeID); // 애니메이션 상태에 따라 현재 상태 업데이트
    if (m_eCurrentState == EBossState::CUTSCENE)
    {
        return; // 컷신 중에는 상태 업데이트를 하지 않음
    }

    if (m_eCurrentState == EBossState::DEAD)
        return;
    UpdateSpecificBehavior();
    _float fDistance = Get_DistanceToPlayer();

    UpdateAttackPattern(fDistance, fTimeDelta);// 공격 패턴 업데이트
    UpdateMovement(fDistance, fTimeDelta);
}

void CBossUnit::UpdateMovement(_float fDistance, _float fTimeDelta)
{
	_bool bCanMove = CanMove();

	m_pAnimator->SetBool("Move", bCanMove);

    if (bCanMove)
    {
        // 가까우면 
        if (fDistance < m_fChasingDistance)
        {
            if (m_fChangeMoveDirCooldown > 0.f)
            {
                m_fChangeMoveDirCooldown -= fTimeDelta;
                m_fChangeMoveDirCooldown = max(m_fChangeMoveDirCooldown, 0.f);
            }
            else
            {
                _int iMoveDir = GetRandomInt(1, 3);
                m_pAnimator->SetInt("MoveDir", iMoveDir);
                m_fChangeMoveDirCooldown = 5.f;
            }
        }
        else if (fDistance >= m_fChasingDistance)
        {
            m_pAnimator->SetInt("MoveDir", 0);
        }
        m_eCurrentState = EBossState::WALK;
        m_pAnimator->SetFloat("Distance", abs(fDistance));
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


    _bool bIsTurn = abs(XMConvertToDegrees(fYaw)) > MINIMUM_TURN_ANGLE && (m_eCurrentState == EBossState::IDLE ||
        m_eCurrentState == EBossState::WALK || m_eCurrentState == EBossState::RUN);

    if (bIsTurn && m_eCurrentState != EBossState::TURN)
    {
        m_pAnimator->SetTrigger("Turn");
        m_pAnimator->SetInt("TurnDir", (fYaw >= 0.f) ? 0 : 1); // 0: 오른쪽, 1: 왼쪽
        m_pAnimator->SetBool("Move", false); // 회전 중에는 이동하지 않음
    }

    if (m_eCurrentState == EBossState::TURN)
    {
        m_pTransformCom->RotateToDirectionSmoothly(vDir, fTimeDelta);
    }
}

void CBossUnit::UpdateAttackPattern(_float fDistance, _float fTimeDelta)
{
}

void CBossUnit::UpdateStateByNodeID(_uint iNodeID)
{
}

_bool CBossUnit::CanMove() const
{
    return (m_eCurrentState == EBossState::IDLE ||
        m_eCurrentState == EBossState::WALK ||
        m_eCurrentState == EBossState::RUN) &&
        m_eCurrentState != EBossState::GROGGY &&
        m_eCurrentState != EBossState::DEAD &&
        m_eCurrentState != EBossState::PARALYZATION &&
        m_eCurrentState != EBossState::ATTACK &&
        m_eCurrentState != EBossState::FATAL &&
        !m_bIsFirstAttack;
}

_bool CBossUnit::IsTargetInFront(_float fDectedAngle) const
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

_bool CBossUnit::UpdateTurnDuringAttack(_float fTimeDelta)
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

_float CBossUnit::Get_DistanceToPlayer() const
{
    if (!m_pPlayer)
        return FLT_MAX;

    _vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
    _vector vThisPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vDiff = vPlayerPos - vThisPos;
    _float fDistance = XMVectorGetX(XMVector3Length(vDiff));

    return fDistance;
}

_vector CBossUnit::GetTargetDirection() const
{
    if (!m_pPlayer)
        return XMVectorZero();
    _vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
    _vector vThisPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vToPlayer = vPlayerPos - vThisPos;
    return XMVector3Normalize(vToPlayer); // 플레이어 방향 벡터 반환
}

void CBossUnit::ApplyRootMotionDelta(_float fTimeDelta)
{
  //  _float3	 rootMotionDelta = m_pAnimator->GetRootMotionDelta();
  //  _float4  rootMotionQuat = m_pAnimator->GetRootRotationDelta();

  //  _vector vLocal = XMLoadFloat3(&rootMotionDelta);
  //  vLocal = XMVectorScale(vLocal, m_fRootMotionAddtiveScale);
  //  _vector vRotQuat = XMQuaternionNormalize(XMLoadFloat4(&rootMotionQuat));

  //  _vector vScale, vCurRotQuat, vTrans;
  //  XMMatrixDecompose(&vScale, &vCurRotQuat, &vTrans, m_pTransformCom->Get_WorldMatrix());
  //  _vector vNewRotQut = XMQuaternionNormalize(XMQuaternionMultiply(vRotQuat, vCurRotQuat));

  //  _vector vWorldDelta = XMVector3Transform(vLocal, XMMatrixRotationQuaternion(vNewRotQut));
  //  vWorldDelta = XMVectorSetY(vWorldDelta, 0.f);
  //  _float fDeltaMag = XMVectorGetX(XMVector3Length(vWorldDelta));

  //  _vector finalDelta = vWorldDelta;
  //  float fLenSq = XMVectorGetX(XMVector3LengthSq(finalDelta));
  //  if (fLenSq < 1e-6f) {
  //      m_PrevWorldDelta = XMVectorZero();
  //      return; // 루트모션 적용하지 않고 그대로 둠
  //  }
  //  //if (XMVectorGetX(XMVector3Length(m_PrevWorldDelta)) > m_fSmoothThreshold)
  //  //{
  //  //    //// 이전 프레임 이동이 있으면 현재 이동과 보간
  //  //    //_float alpha = clamp(fTimeDelta * m_fSmoothSpeed, 0.f, 1.f);
  //  //    //finalDelta = XMVectorLerp(m_PrevWorldDelta, vWorldDelta, alpha);
  //  //    return;
  //  //    m_PrevWorldDelta = finalDelta;
  //  //}

  //  _float fAnimSafeStep = 12.f * fTimeDelta; //
  //  if (XMVectorGetX(XMVector3Length(finalDelta)) > fAnimSafeStep)
  //      finalDelta = XMVector3Normalize(finalDelta) * fAnimSafeStep;
  //  m_PrevWorldDelta = finalDelta;
  //  _float fLen = XMVectorGetX(XMVector3Length(finalDelta));
  //  if (fLen < 1e-6f)
  //      cout << "PrevWorldDelta = ZERO" << endl;
  //  else
  //      cout << "PrevWorldDelta = " << fLen << endl;
  //  _vector vNext = XMVectorAdd(vTrans, finalDelta);

  //  if (m_pNaviCom)
  //  {
  //      if (m_pNaviCom->isMove(vNext))
  //      {
  //          // 갈 수 있으면 Y만 네비로 보정
  //          _float fY = m_pNaviCom->Compute_NavigationY(vNext);
  //          vTrans = XMVectorSetY(vNext, fY);
		////	cout << "일반 이동" << endl;
  //      }
  //      else
  //      {
  //          _vector vSlideDir = m_pNaviCom->GetSlideDirection(vNext, XMVector3Normalize(finalDelta));
  //          _vector vSlidePos = vTrans + vSlideDir * min(fDeltaMag, m_fSlideClamp);

  //          if (m_pNaviCom->isMove(vSlidePos))
  //          {
		//		//cout << "슬라이드 이동" << endl;
  //              _float fY = m_pNaviCom->Compute_NavigationY(vSlidePos);
  //              vTrans = XMVectorSetY(vSlidePos, fY);
  //          }
  //          else
  //          {
		//		//cout << "이동 불가" << endl;
  //              vTrans = XMVectorSetY(vTrans, m_pNaviCom->Compute_NavigationY(vTrans));
  //          }
  //      }
  //  }

  //  // 최종 매트릭스 갱신
  //  _matrix newWorld =
  //      XMMatrixScalingFromVector(vScale) *
  //      XMMatrixRotationQuaternion(vNewRotQut) *
  //      XMMatrixTranslationFromVector(vTrans);

  //  m_pTransformCom->Set_WorldMatrix(newWorld);

    _float3 rootMotionDelta = m_pAnimator->GetRootMotionDelta();

    _vector vLocal = XMLoadFloat3(&rootMotionDelta);
    vLocal = XMVectorScale(vLocal, m_fRootMotionAddtiveScale);

    // 현재 트랜스폼 분해 (스케일, 회전, 위치)
    _vector vScale, vCurRotQuat, vTrans;
    XMMatrixDecompose(&vScale, &vCurRotQuat, &vTrans, m_pTransformCom->Get_WorldMatrix());


    _vector vWorldDelta = XMVector3Transform(vLocal, XMMatrixRotationQuaternion(vCurRotQuat));
    vWorldDelta = XMVectorSetY(vWorldDelta, 0.f);

    _float fDeltaMag = XMVectorGetX(XMVector3Length(vWorldDelta));

    if (fDeltaMag < 1e-6f)
    {
        m_PrevWorldDelta = XMVectorZero();
        return;
    }

    _float fAnimSafeStep = 13.f * fTimeDelta; 
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

void CBossUnit::UpdateNormalMove(_float fTimeDelta)
{
    if (m_eCurrentState == EBossState::WALK || m_eCurrentState == EBossState::RUN)
    {
        _vector vTargetPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);

        _int iMoveDir = m_pAnimator->GetInt("MoveDir");

        switch (iMoveDir)
        {
        case ENUM_CLASS(EMoveDirection::FRONT):
            m_pTransformCom->ChaseWithOutY(vTargetPos, fTimeDelta, m_fChasingDistance, nullptr, m_pNaviCom);
            break;
        case ENUM_CLASS(EMoveDirection::BACK):
            m_pTransformCom->Go_Backward(fTimeDelta, nullptr, m_pNaviCom);
            break;
        case ENUM_CLASS(EMoveDirection::LEFT):
            m_pTransformCom->Go_Left(fTimeDelta, nullptr, m_pNaviCom);
            break;
        case ENUM_CLASS(EMoveDirection::RIGHT):
            m_pTransformCom->Go_Right(fTimeDelta, nullptr, m_pNaviCom);
            break;
        }

        _float fY = m_pNaviCom->Compute_NavigationY(m_pTransformCom->Get_State(STATE::POSITION));
        m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION), fY));
    }
}

void CBossUnit::Ready_AttackPatternWeightForPhase1()
{
}

void CBossUnit::Ready_AttackPatternWeightForPhase2()
{
}

void CBossUnit::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CBossUnit::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CBossUnit::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CBossUnit::ReceiveDamage(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
    if (nullptr == pOther)
        return;
    if (m_eCurrentState == EBossState::DEAD)
        return;
    if (COLLIDERTYPE::PLAYER_WEAPON == eColliderType)
    {
        auto pWeapon = static_cast<CWeapon*>(pOther);

        if (pWeapon->Find_CollisonObj(this, eColliderType))
            return;

        pWeapon->Add_CollisonObj(this);
		pWeapon->Calc_Durability(3);

        m_fHP -= pWeapon->Get_CurrentDamage() * 0.03f;
		m_fHP = max(m_fHP, 0.f);
        cout << "보스 현재 체력 : " << m_fHP << endl;
    }
}

void CBossUnit::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBossUnit::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBossUnit::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

CGameObject* CBossUnit::Clone(void* pArg)
{
	CBossUnit* pInstance = new CBossUnit(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CBossUnit");
		Safe_Release(pInstance);
	}
	return pInstance;
}
void CBossUnit::Free()
{
    __super::Free();
    Safe_Release(m_pNaviCom);
}
