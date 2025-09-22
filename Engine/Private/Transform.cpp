#include "Transform.h"

#include "Shader.h"
#include "Navigation.h"
#include "PhysXController.h"
#include "PhysX_IgnoreSelfCallback.h"
CTransform::CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{
}

CTransform::CTransform(const CTransform& Prototype)
	: CComponent( Prototype )
	, m_WorldMatrix { Prototype.m_WorldMatrix }
{
}

_float3 CTransform::Get_Scaled()
{	
	return _float3(XMVectorGetX(XMVector3Length(Get_State(STATE::RIGHT))),
		XMVectorGetX(XMVector3Length(Get_State(STATE::UP))),
		XMVectorGetX(XMVector3Length(Get_State(STATE::LOOK))));		
}

HRESULT CTransform::Initialize_Prototype()
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	return S_OK;
}

HRESULT CTransform::Initialize(void* pArg)
{

	if (nullptr != pArg)
	{
		TRANSFORM_DESC* pDesc = static_cast<TRANSFORM_DESC*>(pArg);

		m_fSpeedPerSec = pDesc->fSpeedPerSec;
		m_fRotationPerSec = pDesc->fRotationPerSec;
	}

	

	return S_OK;
}

void CTransform::Scaling(_float fX, _float fY, _float fZ)
{
	Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * fX);
	Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * fY);
	Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * fZ);
}

void CTransform::Scaling(const _float3& vScale)
{
	Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * vScale.x);
	Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * vScale.y);
	Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * vScale.z);
}

void CTransform::Go_Backward(_float fTimeDelta, CPhysXController* pController, CNavigation* pNavigation)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	_vector		vLook = Get_State(STATE::LOOK) * -1;

	vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	if (pController)
	{
		PxVec3 moveDir = VectorToPxVec3(vLook);
		pController->Move(fTimeDelta, moveDir, m_fSpeedPerSec);
		PxVec3 pos = pController->Get_Actor()->getGlobalPose().p;
		Set_State(STATE::POSITION, PxVec3ToVector(pos));
	}
	else
	{
		if (nullptr == pNavigation || true == pNavigation->isMove(vPosition))
			Set_State(STATE::POSITION, vPosition);
	}
}

void CTransform::Go_Right(_float fTimeDelta, CPhysXController* pController, CNavigation* pNavigation)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	_vector		vRight = Get_State(STATE::RIGHT);

	vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

	if (pController)
	{
		PxVec3 moveDir = VectorToPxVec3(vRight);
		pController->Move(fTimeDelta, moveDir, m_fSpeedPerSec);
		PxVec3 pos = pController->Get_Actor()->getGlobalPose().p;
		Set_State(STATE::POSITION, PxVec3ToVector(pos));
	}
	else
	{
		if (nullptr == pNavigation || true == pNavigation->isMove(vPosition))
			Set_State(STATE::POSITION, vPosition);
	}
}

void CTransform::Go_Left(_float fTimeDelta, CPhysXController* pController, CNavigation* pNavigation)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	_vector		vRight = Get_State(STATE::RIGHT) * -1;

	vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

	if (pController)
	{
		PxVec3 moveDir = VectorToPxVec3(vRight);
		pController->Move(fTimeDelta, moveDir, m_fSpeedPerSec);
		PxVec3 pos = pController->Get_Actor()->getGlobalPose().p;
		Set_State(STATE::POSITION, PxVec3ToVector(pos));
	}
	else
	{
		if (nullptr == pNavigation || true == pNavigation->isMove(vPosition))
			Set_State(STATE::POSITION, vPosition);
	}
}

void CTransform::Go_Target(_float fTimeDelta, _fvector vTarget, _float fMinDistance, CPhysXController* pController, CNavigation* pNavigation)
{
	_vector		vMoveDir = vTarget - Get_State(STATE::POSITION);

	if (fMinDistance <= XMVectorGetX(XMVector3Length(vMoveDir)))
	{
		if (pController)
		{
			PxVec3 moveDir = VectorToPxVec3(XMVector3Normalize(vMoveDir));
			pController->Move(fTimeDelta, moveDir, m_fSpeedPerSec);
			PxVec3 pos = pController->Get_Actor()->getGlobalPose().p;
			Set_State(STATE::POSITION, PxVec3ToVector(pos));
		}
		else
		{
			_vector vNextPos = Get_State(STATE::POSITION) + XMVector3Normalize(vMoveDir) * m_fSpeedPerSec * fTimeDelta;
			if (nullptr == pNavigation || true == pNavigation->isMove(vNextPos))
				Set_State(STATE::POSITION, vNextPos);
		}
	}
}

void CTransform::Turn(_fvector vAxis, _float fTimeDelta)
{
	_matrix			RotationMatrix = XMMatrixRotationAxis(vAxis, m_fRotationPerSec * fTimeDelta);

	Set_State(STATE::RIGHT, XMVector4Transform(Get_State(STATE::RIGHT), RotationMatrix));
	Set_State(STATE::UP, XMVector4Transform(Get_State(STATE::UP), RotationMatrix));
	Set_State(STATE::LOOK, XMVector4Transform(Get_State(STATE::LOOK), RotationMatrix));
}

void CTransform::TurnAngle(_fvector vAxis, _float fAngle)
{
	_matrix RotationMatrix = XMMatrixRotationAxis(vAxis, fAngle);

	Set_State(STATE::RIGHT, XMVector4Transform(Get_State(STATE::RIGHT), RotationMatrix));
	Set_State(STATE::UP, XMVector4Transform(Get_State(STATE::UP), RotationMatrix));
	Set_State(STATE::LOOK, XMVector4Transform(Get_State(STATE::LOOK), RotationMatrix));
}

void CTransform::Rotation(_fvector vAxis, _float fRadian)
{
	_float3		vScaled = Get_Scaled();

	_vector		vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScaled.x;
	_vector		vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScaled.y;
	_vector		vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScaled.z;

	_matrix		RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);	

	Set_State(STATE::RIGHT, XMVector4Transform(vRight, RotationMatrix));
	Set_State(STATE::UP, XMVector4Transform(vUp, RotationMatrix));
	Set_State(STATE::LOOK, XMVector4Transform(vLook, RotationMatrix));
}

void CTransform::Rotation(_float fX, _float fY, _float fZ)
{
	_float3		vScaled = Get_Scaled();

	_vector		vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScaled.x;
	_vector		vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScaled.y;
	_vector		vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScaled.z;

	_vector		vQuaternion = XMQuaternionRotationRollPitchYaw(fX, fY, fZ);

	_matrix		RotationMatrix = XMMatrixRotationQuaternion(vQuaternion);

	Set_State(STATE::RIGHT, XMVector4Transform(vRight, RotationMatrix));
	Set_State(STATE::UP, XMVector4Transform(vUp, RotationMatrix));
	Set_State(STATE::LOOK, XMVector4Transform(vLook, RotationMatrix));
}

void CTransform::Move(const _vector& vDirectionVector, CPhysXController* pController)
{
	_vector vPos = Get_State(STATE::POSITION) + vDirectionVector;
	if(pController)
	{
		PxTransform pxTransform(VectorToPxVec3(vPos), PxQuat(PxIdentity)); // 회전은 기본값
		pController->Set_Transform(pxTransform);
		PxVec3 pos = pController->Get_Actor()->getGlobalPose().p;
		Set_State(STATE::POSITION, PxVec3ToVector(pos));
	}
	else
		Set_State(STATE::POSITION, vPos);
}

void CTransform::Go_Front(_float fTimeDelta, CPhysXController* pController, CNavigation* pNavigation)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	_vector		vLook = Get_State(STATE::LOOK);

	vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	if (pController)
	{
		pController->Move(fTimeDelta, VectorToPxVec3(vLook), m_fSpeedPerSec);
		PxVec3 pos = pController->Get_Actor()->getGlobalPose().p;
		Set_State(STATE::POSITION, PxVec3ToVector(pos));
	}
	else
	{
		if (nullptr == pNavigation || true == pNavigation->isMove(vPosition))
			Set_State(STATE::POSITION, vPosition);
	}
}

bool CTransform::Go_FrontByPosition(_float fTimeDelta, _vector vPosition, CPhysXController* pController, CNavigation* pNavigation)
{
	// 현재 위치
	_vector vMyPos = Get_State(STATE::POSITION);

	// 목표까지의 방향
	_vector vDir = XMVectorSubtract(vPosition, vMyPos);
	vDir = XMVector3Normalize(vDir);

	// 현재 바라보는 방향
	_vector vLook = Get_State(STATE::LOOK);
	vLook = XMVector3Normalize(vLook);

	// 목표 방향으로 조금이라도 틀어졌으면 정면을 보도록 회전
	_float fDot = XMVectorGetX(XMVector3Dot(vDir, vLook));
	if (fDot < 0.99f)
	{
		LookAtWithOutY(vPosition);
	}

	// 목표까지의 거리
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPosition, vMyPos)));
	if (fDistance <= 0.05f)
		return true;

	if (fDistance > 0.05f)
	{
		Go_Front(fTimeDelta, pController, pNavigation);
	}
	return false;
}

bool CTransform::Go_UpCustom(_float fTimeDelta, _float fSpeed, _float fMaxHight, CPhysXController* pController)
{
	_vector		vPosition = Get_State(STATE::POSITION);

	if (fMaxHight > XMVectorGetY(vPosition))
	{
		if (pController)
		{
			_vector moveDir = _vector{ 0.f, 1.f, 0.f, 0.f };
			pController->Move(fTimeDelta, VectorToPxVec3(moveDir), fSpeed);
			PxVec3 pos = pController->Get_Actor()->getGlobalPose().p;
			Set_State(STATE::POSITION, PxVec3ToVector(pos));
		}
		else
		{
			vPosition += _vector{ 0.f, 1.f, 0.f, 0.f } *fSpeed * fTimeDelta;
			Set_State(STATE::POSITION, vPosition);
		}
		return false;
	}
	else
	{
		return true;
	}
}

void CTransform::Go_DownCustom(_float fTimeDelta, _float fSpeed, CPhysXController* pController)
{
	_vector		vPosition = Get_State(STATE::POSITION);

	if (pController)
	{
		_vector moveDir = _vector{ 0.f, 1.f, 0.f, 0.f } * -1;
		pController->Move(fTimeDelta, VectorToPxVec3(moveDir), fSpeed);
		PxVec3 pos = pController->Get_Actor()->getGlobalPose().p;
		Set_State(STATE::POSITION, PxVec3ToVector(pos));
	}
	else
	{
		vPosition -= _vector{ 0.f, 1.f, 0.f, 0.f } *fSpeed * fTimeDelta;
		Set_State(STATE::POSITION, vPosition);
	}
}

void CTransform::Go_Dir(const _vector& vMoveDir, _float fTimeDelta, CPhysXController* pController, CNavigation* pNavigation)
{
	// 현재 위치 + 이동 방향 x 델타 타임 x 스피드퍼세크
	_vector vPos = Get_State(STATE::POSITION);
	_vector vNewPos = vPos + XMVector3Normalize(vMoveDir) * fTimeDelta * m_fSpeedPerSec;

	if (pController)
	{
		PxVec3 moveDir = VectorToPxVec3(XMVector3Normalize(vMoveDir));
		pController->Move(fTimeDelta, moveDir, m_fSpeedPerSec);
		PxVec3 pos = pController->Get_Actor()->getGlobalPose().p;
		Set_State(STATE::POSITION, PxVec3ToVector(pos));
	}
	else
	{
		if (pNavigation)
		{
			if(pNavigation->isMove(vNewPos))
			{
				Set_State(STATE::POSITION, vNewPos);
			}
			else
			{
				// 네비 밖 → 슬라이딩 방향 계산
				_vector vSlideDir = pNavigation->GetSlideDirection(vNewPos, XMVector3Normalize(vMoveDir));

				

				


				_vector vSlidePos = Get_State(STATE::POSITION) + vSlideDir * 0.5f  * m_fSpeedPerSec * fTimeDelta;

				if (pNavigation->isMove(vSlidePos))
					Set_State(STATE::POSITION, vSlidePos);
			}
		}
	}
}

bool CTransform::Move_Special(_float fTimeDelta, _float fTime, _vector& vMoveDir, _float fDistance, CPhysXController* pPhysXController, CNavigation* pNavigation)
{
	/* 특정 방향으로 Distance만큼 특정 시간안에 이동한다. */

	// 이동 중이 아니라면 초기화
	if (!m_bSpecialMoving)
	{
		m_bSpecialMoving = true;
		m_fSpecialMoveElapsed = 0.f;
		m_fSpecialMoveDuration = fTime;
		m_fSpecialMoveStartPos = Get_State(STATE::POSITION);
		m_vSpecialMoveOffset = XMVector3Normalize(vMoveDir) * fDistance;
	}

	// 매 프레임 델타 누적 + 보간 계산
	m_fSpecialMoveElapsed += fTimeDelta;
	_float t = m_fSpecialMoveElapsed / m_fSpecialMoveDuration;
	t = min(t, 1.f);

	_float smoothT = (sinf(t * XM_PI - XM_PIDIV2) + 1.f) * 0.5f; // 이징 In-Out
	_vector vNewPos = m_fSpecialMoveStartPos + m_vSpecialMoveOffset * smoothT;


	// 중력 적용
	constexpr _float fGravity = -9.81f;
	m_vGravityVelocity.y += fGravity * fTimeDelta;

	// 현재 위치
	_vector vCurPos = Get_State(STATE::POSITION);

	// 수평 델타
	_vector vDeltaH = vNewPos - XMVectorSetY(vCurPos, XMVectorGetY(vNewPos));
	_float dx = XMVectorGetX(vDeltaH);
	_float dz = XMVectorGetZ(vDeltaH);

	// 최종 이동 벡터 (m_vVelocity에 저장)
	PxVec3 pxMove = PxVec3(dx, m_vGravityVelocity.y * fTimeDelta, dz);

	if (pPhysXController)
	{
		CIgnoreSelfCallback filter(pPhysXController->Get_IngoreActors());
		PxControllerFilters filters;
		filters.mFilterCallback = &filter;

		_vector vCurPos = Get_State(STATE::POSITION);
		_vector vDelta = vNewPos - vCurPos;
		_float fX = XMVectorGetX(vDelta);
		_float fY = XMVectorGetY(vDelta);
		_float fZ = XMVectorGetZ(vDelta);

		PxControllerCollisionFlags flags = pPhysXController->Get_Controller()->move(pxMove, 0.001f, fTimeDelta, filters);

		// 지면 충돌 체크
		m_bOnGround = (flags & PxControllerCollisionFlag::eCOLLISION_DOWN);
		if (m_bOnGround)
			m_vGravityVelocity.y = 0.f;

		// 컨트롤러 → 트랜스폼 싱크
		const PxExtendedVec3 ex = pPhysXController->Get_Controller()->getPosition();
		Set_State(STATE::POSITION, XMVectorSet((float)ex.x, (float)ex.y, (float)ex.z, 1.f));
	}
	else if (pNavigation)
	{
		if (pNavigation->isMove(vNewPos))
		{
			_float NavigationY = pNavigation->Compute_NavigationY(vNewPos);
			vNewPos = XMVectorSetY(vNewPos, NavigationY);
			Set_State(STATE::POSITION, vNewPos);
		}
	}
	else
	{
		Set_State(STATE::POSITION, vNewPos);
	}

	// 끝났으면 초기화
	if (t >= 1.f)
	{
		m_bSpecialMoving = false;
		m_fSpecialMoveDuration = 0.f;
		return true;
	}

	return false;
}
bool CTransform::Move_SpecialB(_float fTimeDelta, _float fTime, _vector& vMoveDir, _float fDistance, CPhysXController* pPhysXController, CNavigation* pNavigation)
{
	/* 특정 방향으로 Distance만큼 특정 시간안에 이동한다. */

	// 이동 중이 아니라면 초기화
	if (!m_bSpecialMoving)
	{
		m_bSpecialMoving = true;
		m_fSpecialMoveElapsed = 0.f;
		m_fSpecialMoveDuration = fTime;
		m_fSpecialMoveStartPos = Get_State(STATE::POSITION);
		m_vSpecialMoveOffset = XMVector3Normalize(vMoveDir) * fDistance;
	}

	// 매 프레임 델타 누적 + 보간 계산
	m_fSpecialMoveElapsed += fTimeDelta;
	_float t = m_fSpecialMoveElapsed / m_fSpecialMoveDuration;
	t = min(t, 1.f);

	_float smoothT = 1.f - powf(2.f, -10.f * t);
	_vector vNewPos = m_fSpecialMoveStartPos + m_vSpecialMoveOffset * smoothT;

	if (pPhysXController)
	{
		PxControllerFilters filters;

		_vector vCurPos = Get_State(STATE::POSITION);
		_vector vDelta = vNewPos - vCurPos;
		_float fX = XMVectorGetX(vDelta);
		_float fY = XMVectorGetY(vDelta);
		_float fZ = XMVectorGetZ(vDelta);

		PxVec3 pxMove = { fX, fY, fZ };
		pPhysXController->Get_Controller()->move(pxMove, 0.001f, fTimeDelta, filters);
	}
	else if (pNavigation)
	{
		if (pNavigation->isMove(vNewPos))
		{
			_float NavigationY = pNavigation->Compute_NavigationY(vNewPos);
			vNewPos = XMVectorSetY(vNewPos, NavigationY);
			Set_State(STATE::POSITION, vNewPos);
		}
	}
	else
	{
		Set_State(STATE::POSITION, vNewPos);
	}

	// 끝났으면 초기화
	if (t >= 1.f)
	{
		m_bSpecialMoving = false;
		m_fSpecialMoveDuration = 0.f;
		return true;
	}

	return false;
}

bool CTransform::Scale_Special(_float fTimeDelta, _float fTime, _vector vTargetScale)
{
	// 시작 시 초기화
	if (!m_bScaling)
	{
		m_bScaling = true;
		m_fScaleElapsed = 0.f;
		m_fScaleDuration = fTime;
		m_vStartScale = Get_Scale();
		m_vTargetScale = vTargetScale;
	}

	// 시간 누적 및 보간 비율 계산
	m_fScaleElapsed += fTimeDelta;
	_float t = m_fScaleElapsed / m_fScaleDuration;
	t = min(t, 1.f);

	// Sine 이징 In-Out
	_float smoothT = (sinf(t * XM_PI - XM_PIDIV2) + 1.f) * 0.5f;

	_vector vNewScale = XMVectorLerp(m_vStartScale, m_vTargetScale, smoothT);

	// 스케일 적용
	SetUp_Scale(
		XMVectorGetX(vNewScale),
		XMVectorGetY(vNewScale),
		XMVectorGetZ(vNewScale)
	);

	// 완료 시 초기화
	if (t >= 1.f)
	{
		m_bScaling = false;
		m_fScaleDuration = 0.f;
		return true;
	}

	return false;
}

bool CTransform::Rotate_Special(_float fTimeDelta, _float fTime, _vector vAxis, _float fAngleDegree)
{
	// 최초 진입 시 회전 초기화
	if (!m_bSpecialRotating)
	{
		// 회전 처음 시작할 때 기준축을 저장
		m_vOriginalRight = Get_State(STATE::RIGHT);
		m_vOriginalUp = Get_State(STATE::UP);
		m_vOriginalLook = Get_State(STATE::LOOK);

		m_bSpecialRotating = true;
		m_fSpecialRotateElapsed = 0.f;
		m_fSpecialRotateDuration = fTime;

		m_fSpecialRotateAngle = XMConvertToRadians(fAngleDegree); // degree → radian
		m_vSpecialRotateAxis = XMVector3Normalize(vAxis);

		m_matSpecialRotateStart = XMLoadFloat4x4(&m_WorldMatrix);
	}

	// 진행 시간 누적
	m_fSpecialRotateElapsed += fTimeDelta;
	_float t = m_fSpecialRotateElapsed / m_fSpecialRotateDuration;
	t = min(t, 1.f);

	// Sine In-Out 보간
	_float smoothT = (sinf(t * XM_PI - XM_PIDIV2) + 1.f) * 0.5f;

	_float fCurrentAngle = m_fSpecialRotateAngle * smoothT;
	_matrix matRot = XMMatrixRotationAxis(m_vSpecialRotateAxis, fCurrentAngle);

	// 회전만 적용: 위치는 유지
	_vector vPosition = Get_State(STATE::POSITION);
	_float3 vScaled = Compute_Scaled();
	_vector vScale = XMLoadFloat3(&vScaled);

	// 원래 로컬 기준 회전 적용 (Right/Up/Look 축 재구성)
	_vector vRight = XMVector3TransformNormal(m_vOriginalRight, matRot);
	_vector vUp = XMVector3TransformNormal(m_vOriginalUp, matRot);
	_vector vLook = XMVector3TransformNormal(m_vOriginalLook, matRot);

	Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * XMVectorGetX(vScale));
	Set_State(STATE::UP, XMVector3Normalize(vUp) * XMVectorGetY(vScale));
	Set_State(STATE::LOOK, XMVector3Normalize(vLook) * XMVectorGetZ(vScale));
	Set_State(STATE::POSITION, vPosition); // 위치는 유지

	// 종료 처리
	if (t >= 1.f)
	{
		m_bSpecialRotating = false;
		return true;
	}

	return false;
}

bool CTransform::JumpToTarget(_float fTimeDelta, _vector vTargetPos, _float fJumpHeight, _float fJumpTime, CPhysXController* pController)
{
	if (!m_bSpecialMoving)
	{
		m_bSpecialMoving = true;
		m_fSpecialMoveElapsed = 0.f;
		m_fSpecialMoveDuration = fJumpTime;

		m_fSpecialMoveStartPos = Get_State(STATE::POSITION);
		m_vSpecialMoveTargetPos = vTargetPos;
	}

	m_fSpecialMoveElapsed += fTimeDelta;
	_float t = m_fSpecialMoveElapsed / m_fSpecialMoveDuration;
	t = min(t, 1.f);

	// X, Z는 선형 보간
	_float3 vStart = {};
	XMStoreFloat3(&vStart, m_fSpecialMoveStartPos);
	_float3 vEnd = {};
	XMStoreFloat3(&vEnd, m_vSpecialMoveTargetPos);

	_float3 vInterp = {};
	vInterp.x = vStart.x + (vEnd.x - vStart.x) * t;
	vInterp.z = vStart.z + (vEnd.z - vStart.z) * t;

	// Y는 시작과 끝의 보간값 + 점프 궤도 곡선
	_float fBaseY = vStart.y + (vEnd.y - vStart.y) * t;
	_float fArc = 4.f * fJumpHeight * powf(1.f - t, 2.5f) * t;

	vInterp.y = fBaseY + fArc;

	_vector vNewPos = XMVectorSet(vInterp.x, vInterp.y, vInterp.z, 1.f);
	if (pController)
	{
		PxTransform pxTransform(VectorToPxVec3(vNewPos), PxQuat(PxIdentity)); 
		pController->Set_Transform(pxTransform);
		PxVec3 pos = pController->Get_Actor()->getGlobalPose().p;
		Set_State(STATE::POSITION, PxVec3ToVector(pos));
	}
	else
	{
		Set_State(STATE::POSITION, vNewPos);
	}

	if (t >= 1.f)
	{
		m_bSpecialMoving = false;
		m_fSpecialMoveDuration = 0.f;
		return true;
	}

	return false;
}

void CTransform::RotationTimeDelta(_float fTimeDelta, _fvector Axis, _float fSpeedPerSec)
{
	// 1. 회전할 라디안 계산
	_float fRadian = fSpeedPerSec * fTimeDelta;

	// 2. 월드 Y축 기준 회전행렬
	_matrix RotationMatrix = XMMatrixRotationAxis(Axis, fRadian);

	// 3. 현재 상태에서 방향 벡터와 스케일 추출
	_vector vRight = Get_State(STATE::RIGHT);
	_vector vUp = Get_State(STATE::UP);
	_vector vLook = Get_State(STATE::LOOK);

	_float fScaleX = XMVectorGetX(XMVector3Length(vRight));
	_float fScaleY = XMVectorGetX(XMVector3Length(vUp));
	_float fScaleZ = XMVectorGetX(XMVector3Length(vLook));

	// 4. 단위 벡터로 정규화 후 회전
	_vector vRotatedRight = XMVector3TransformNormal(XMVector3Normalize(vRight), RotationMatrix);
	_vector vRotatedLook = XMVector3TransformNormal(XMVector3Normalize(vLook), RotationMatrix);

	// 5. 정직한 UP 벡터 구한 뒤 다시 Right도 재정렬
	_vector vRotatedUp = XMVector3Normalize(XMVector3Cross(vRotatedLook, vRotatedRight));
	vRotatedRight = XMVector3Normalize(XMVector3Cross(vRotatedUp, vRotatedLook));

	// 6. 스케일 다시 곱해서 원래 크기 유지
	vRotatedRight *= fScaleX;
	vRotatedUp *= fScaleY;
	vRotatedLook *= fScaleZ;

	// 7. 상태 저장
	Set_State(STATE::RIGHT, vRotatedRight);
	Set_State(STATE::UP, vRotatedUp);
	Set_State(STATE::LOOK, vRotatedLook);
}

bool CTransform::RotateToDirectionImmediately(const _fvector& vTargetDir)
{
	_float3 fScale = Compute_Scaled();
	_vector vScale = XMLoadFloat3(&fScale);
	_vector vLook = XMVector3Normalize(vTargetDir);

	// 수직 방향이랑 너무 가까우면 회전 불가능 → 실패 처리
	_vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vRight = XMVector3Normalize(XMVector3Cross(vWorldUp, vLook));
	if (XMVector3Equal(vRight, XMVectorZero()))
		return false;

	_vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

	// 방향 벡터 재설정
	Set_State(STATE::RIGHT, vRight * XMVectorGetX(vScale));
	Set_State(STATE::UP, vUp * XMVectorGetY(vScale));
	Set_State(STATE::LOOK, vLook * XMVectorGetZ(vScale));

	return true;
}

bool CTransform::RotateToDirectionSmoothly(const _fvector& vTargetDir, _float fTimeDelta)
{
	_float3 fScale = Compute_Scaled();
	_vector vScale = XMLoadFloat3(&fScale);

	// 현재 LOOK 방향
	_vector vCurrentLook = XMVector3Normalize(Get_State(STATE::LOOK));

	// 목표 방향
	_vector vTargetLook = XMVector3Normalize(vTargetDir);

	// 내적해서 각도 차이 확인
	_float fDot = XMVectorGetX(XMVector3Dot(vCurrentLook, vTargetLook));
	fDot = max(-1.f, min(1.f, fDot)); // Clamp

	_float fAngle = acosf(fDot); // 현재 각도 차이 (라디안)

	// 거의 정면이면 회전하지 않음
	if (fAngle < 0.01f)
		return true;

	// 회전 축 계산
	_vector vAxis = XMVector3Normalize(XMVector3Cross(vCurrentLook, vTargetLook));

	// 한 프레임 회전할 각도 계산
	//_float fStep = m_fSpeedPerSec * fTimeDelta;
	_float fStep = m_fRotationPerSec * fTimeDelta;
	fStep = min(fStep, fAngle); // 과회전 방지

	// 회전 행렬 생성
	_matrix matRot = XMMatrixRotationAxis(vAxis, fStep);

	// 현재 basis 벡터들 (Right, Up, Look)
	_vector vRight = Get_State(STATE::RIGHT);
	_vector vUp = Get_State(STATE::UP);
	_vector vLook = Get_State(STATE::LOOK);

	// 회전 적용
	vRight = XMVector3TransformNormal(vRight, matRot);
	vUp = XMVector3TransformNormal(vUp, matRot);
	vLook = XMVector3TransformNormal(vLook, matRot);

	// 회전 후에도 스케일 유지
	Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * XMVectorGetX(vScale));
	Set_State(STATE::UP, XMVector3Normalize(vUp) * XMVectorGetY(vScale));
	Set_State(STATE::LOOK, XMVector3Normalize(vLook) * XMVectorGetZ(vScale));

	return (fAngle - fStep) <= 0.001f;
}

void CTransform::Quaternion_Turn(const _vector& vAngle)
{
	_vector			vRight = Get_State(STATE::RIGHT);
	_vector			vUp = Get_State(STATE::UP);
	_vector			vLook = Get_State(STATE::LOOK);

	_vector Qur = XMQuaternionRotationRollPitchYaw(
		XMConvertToRadians(XMVectorGetX(vAngle)),
		XMConvertToRadians(XMVectorGetY(vAngle)),
		XMConvertToRadians(XMVectorGetZ(vAngle))
	);

	Set_State(STATE::RIGHT, XMVector3Rotate(vRight, Qur));
	Set_State(STATE::UP, XMVector3Rotate(vUp, Qur));
	Set_State(STATE::LOOK, XMVector3Rotate(vLook, Qur));
}

void CTransform::LookAtWithOutY(_fvector vAt)
{
	_vector vPosition = Get_State(STATE::POSITION);

	_vector vLook = vAt - vPosition;
	vLook = XMVectorSetY(vLook, 0.f);
	vLook = XMVector3Normalize(vLook);

	_vector vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	vRight = XMVector3Normalize(vRight);

	_vector vUp = XMVector3Cross(vLook, vRight);
	vUp = XMVector3Normalize(vUp);

	_matrix matWorld = XMLoadFloat4x4(&m_WorldMatrix);

	_float3 vScale;
	vScale.x = XMVectorGetX(XMVector3Length(matWorld.r[0]));
	vScale.y = XMVectorGetX(XMVector3Length(matWorld.r[1]));
	vScale.z = XMVectorGetX(XMVector3Length(matWorld.r[2]));

	Set_State(STATE::RIGHT, vRight * vScale.x);
	Set_State(STATE::UP, vUp * vScale.y);
	Set_State(STATE::LOOK, vLook * vScale.z);
}

bool CTransform::ChaseWithOutY(_vector& vTargetPos, _float fTimeDelta, _float fMinDistance, CPhysXController* pController,CNavigation* pNavigation)
{
	_vector		vPosition = Get_State(STATE::POSITION);
	vTargetPos = XMVectorSetY(vTargetPos, XMVectorGetY(vPosition));

	_vector		vMoveDir = vTargetPos - vPosition;
	_float fDist = XMVectorGetX(XMVector3Length(vMoveDir));
	//최소거리보다 길때는 포지션 갱신
	if (fDist >= fMinDistance)
	{
		Go_Dir(vMoveDir, fTimeDelta, pController, pNavigation);
		return true;  
	}
	else
	{
		return false;
	}
}

bool CTransform::ChaseCustom(const _fvector vTargetPos, _float fTimeDelta, _float fMinDistance, _float fSpeed, CPhysXController* pController)
{
	_vector vPosition = Get_State(STATE::POSITION);
	_vector vMoveDir = vTargetPos - vPosition;

	float fDistance = XMVectorGetX(XMVector3Length(vMoveDir)); // 거리 계산

	if (fDistance >= fMinDistance)
	{
		if (pController)
		{
			PxVec3 moveDir = VectorToPxVec3(XMVector3Normalize(vMoveDir));
			pController->Move(fTimeDelta, moveDir, m_fSpeedPerSec);
			PxVec3 pos = pController->Get_Actor()->getGlobalPose().p;
			Set_State(STATE::POSITION, PxVec3ToVector(pos));
		}
		else
		{
			_vector vMoveDirNorm = XMVector3Normalize(vMoveDir);
			vPosition += vMoveDirNorm * fSpeed * fTimeDelta;
			Set_State(STATE::POSITION, vPosition);
		}
	}
	else
	{
		return true; // 추적 완료
	}

	return false; // 아직 추적 중
}

void CTransform::BillboardToCameraY(_fvector vCameraPos)
{
	_vector vMyPos = Get_State(STATE::POSITION);

	// Y는 고정된 상태로 카메라를 바라보는 Look 벡터
	_vector vLook = - vMyPos + vCameraPos;
	vLook = XMVectorSetY(vLook, 0.f);  // Y 고정
	vLook = XMVector3Normalize(vLook);

	// 우측 벡터 (WorldUp × Look)
	_vector vRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook));
	_vector vUp = XMVector3Cross(vLook, vRight);

	_float3 vScale = Compute_Scaled();
	Set_State(STATE::RIGHT, vRight * vScale.x);
	Set_State(STATE::UP, vUp * vScale.y);
	Set_State(STATE::LOOK, vLook * vScale.z);
}

void CTransform::BillboardToCameraFull(_fvector vCameraPos)
{
	_vector vMyPos = Get_State(STATE::POSITION);
	_vector vLook = XMVector3Normalize(vMyPos - vCameraPos);

	// 기본적인 월드 업벡터 사용
	_vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	// Right = WorldUp × Look
	_vector vRight = XMVector3Normalize(XMVector3Cross(vWorldUp, vLook));

	// Up = Look × Right
	_vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

	_float3 vScale = Compute_Scaled();
	Set_State(STATE::RIGHT, vRight * vScale.x);
	Set_State(STATE::UP, vUp * vScale.y);
	Set_State(STATE::LOOK, vLook * vScale.z);
}

_float3 CTransform::Compute_Scaled()
{
	return _float3(
		XMVectorGetX(XMVector3Length(Get_State(STATE::RIGHT))),
		XMVectorGetX(XMVector3Length(Get_State(STATE::UP))),
		XMVectorGetX(XMVector3Length(Get_State(STATE::LOOK)))
	);
}
void CTransform::SetUp_Scale(_float fScaleX, _float fScaleY, _float fScaleZ)
{
	Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * fScaleX);
	Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * fScaleY);
	Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * fScaleZ);
}

_vector CTransform::Get_Scale() const
{
	_matrix matWorld = XMLoadFloat4x4(&m_WorldMatrix);

	_vector vRight = matWorld.r[0];
	_vector vUp = matWorld.r[1];
	_vector vLook = matWorld.r[2];

	_float fScaleX = XMVectorGetX(XMVector3Length(vRight));
	_float fScaleY = XMVectorGetX(XMVector3Length(vUp));
	_float fScaleZ = XMVectorGetX(XMVector3Length(vLook));

	return XMVectorSet(fScaleX, fScaleY, fScaleZ, 0.f);
}

void CTransform::Set_Orbit(_fvector vCenter, _fvector vAxis, _float fRadius, _float fSpeed)
{
	XMStoreFloat4(&m_vOrbitAxis, vAxis);
	XMStoreFloat4(&m_vOrbitCenter, vCenter);
	m_fOrbitRadius = fRadius;
	m_fSpeedPerSec = fSpeed;
}

void CTransform::UpdateOrbit(_float fTimeDelta)
{
	m_fOrbitAngle += m_fSpeedPerSec * fTimeDelta;

	// 000 기준
	_vector vCenter = XMLoadFloat4(&m_vOrbitCenter);
	_vector vAxis = XMLoadFloat4(&m_vOrbitAxis);

	// 축과 수직인 시작 벡터
	_vector vUpRef = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vStart = XMVector3Cross(vAxis, vUpRef);
	if (XMVector3Equal(vStart, XMVectorZero()))
	{
		vUpRef = XMVectorSet(1.f, 0.f, 0.f, 0.f);
		vStart = XMVector3Cross(vAxis, vUpRef);
	}
	vStart = XMVector3Normalize(vStart) * m_fOrbitRadius;

	// 현재 위치
	_matrix matRot = XMMatrixRotationAxis(vAxis, m_fOrbitAngle);
	_vector vCurPos = XMVector3TransformNormal(vStart, matRot) + vCenter;

	// 직전 위치 (접선 방향)
	_matrix matRotPrev = XMMatrixRotationAxis(vAxis, m_fOrbitAngle - m_fSpeedPerSec * fTimeDelta);
	_vector vPrevPos = XMVector3TransformNormal(vStart, matRotPrev) + vCenter;

	// 방향 벡터 (Look)
	_vector vLook = XMVector3Normalize(vCurPos - vPrevPos);
	_vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vRight = XMVector3Normalize(XMVector3Cross(vWorldUp, vLook));
	if (XMVector3Equal(vRight, XMVectorZero()))
	{
		vWorldUp = XMVectorSet(1.f, 0.f, 0.f, 0.f);
		vRight = XMVector3Normalize(XMVector3Cross(vWorldUp, vLook));
	}
	_vector vUp = XMVector3Cross(vLook, vRight);

	// 스케일 유지
	_float3 vScale = Compute_Scaled();
	Set_State(STATE::RIGHT, vRight * vScale.x);
	Set_State(STATE::UP, vUp * vScale.y);
	Set_State(STATE::LOOK, vLook * vScale.z);
	Set_State(STATE::POSITION, XMVectorSetW(vCurPos, 1.f));
}

void CTransform::LookAt(_fvector vAt)
{
	_float3		vScaled = Get_Scaled();

	_vector		vLook = vAt - Get_State(STATE::POSITION);
	_vector		vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	_vector		vUp = XMVector3Cross(vLook, vRight);

	Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * vScaled.x);
	Set_State(STATE::UP, XMVector3Normalize(vUp) * vScaled.y);
	Set_State(STATE::LOOK, XMVector3Normalize(vLook) * vScaled.z);
}


void CTransform::QuaternionRotate(_matrix matWorld)
{
	_vector vScale, vRotationQuat, vTranslation;
	XMMatrixDecompose(&vScale, &vRotationQuat, &vTranslation, matWorld);

	Set_Quaternion(vRotationQuat);
}

void CTransform::Set_Quaternion(_vector vQuaternion)
{
	// 정규화(안정성 확보) 후 저장
	m_vQuaternionRotation = XMQuaternionNormalize(vQuaternion);

	// 회전값이 바뀌었으니 월드 행렬도 갱신 필요
	Update_WorldMatrix();
}

void CTransform::Update_WorldMatrix()
{
	_float3 vScale = Compute_Scaled();
	_matrix matScale = XMMatrixScaling(vScale.x, vScale.y, vScale.z);

	_matrix matRotation = XMMatrixRotationQuaternion(m_vQuaternionRotation);

	_vector vPosition = Get_State(STATE::POSITION);
	_matrix matTranslation = XMMatrixTranslation(
		XMVectorGetX(vPosition),
		XMVectorGetY(vPosition),
		XMVectorGetZ(vPosition)
	);

	XMMATRIX matWorld = matScale * matRotation * matTranslation;
	XMStoreFloat4x4(&m_WorldMatrix, matWorld);

}

HRESULT CTransform::Bind_ShaderResource(CShader* pShader, const _char* pConstantName)
{
	return pShader->Bind_Matrix(pConstantName, &m_WorldMatrix);	
}

CTransform* CTransform::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTransform* pInstance = new CTransform(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTransform");
		Safe_Release(pInstance);
	}

	return pInstance;
}


CComponent* CTransform::Clone(void* pArg)
{
	CTransform* pInstance = new CTransform(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTransform");
		Safe_Release(pInstance);
	}

	return pInstance;

}

void CTransform::Free()
{
	__super::Free();

}
