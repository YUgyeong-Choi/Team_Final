#include "Camera_Orbital.h"
#include "GameInstance.h"

#include "PhysX_IgnoreSelfCallback.h"
#include "Camera_Manager.h"
#include "Player.h"

#include "Client_Calculation.h"
#include "Unit.h"

// ---- Angle helpers (radians) ----
inline float WrapPi(float a) {
	a = fmodf(a + XM_PI, XM_2PI);
	if (a <= 0) a += XM_2PI;
	return a - XM_PI;
}
inline float ShortestDelta(float from, float to) {
	return WrapPi(to - from);
}
inline float MoveTargetNear(float current, float target) {
	return current + ShortestDelta(current, target);
}

CCamera_Orbital::CCamera_Orbital(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{
}

CCamera_Orbital::CCamera_Orbital(const CCamera_Orbital& Prototype)
	: CCamera(Prototype)
{
}

HRESULT CCamera_Orbital::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_Orbital::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	CAMERA_ORBITAL_DESC* pDesc = static_cast<CAMERA_ORBITAL_DESC*>(pArg);
	m_fMouseSensor = 0.6f;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CCamera_Orbital::Priority_Update(_float fTimeDelta)
{
	if (KEY_DOWN(DIK_T))
		m_bActive = !m_bActive;


	//if (KEY_DOWN(DIK_CAPSLOCK))
	//	PrintMatrix("OribitalCameraWold", XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()));

	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;
	__super::Priority_Update(fTimeDelta);
}

void CCamera_Orbital::Update(_float fTimeDelta)
{
	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;

	if (!m_pPlayer)
		return;

	Update_LerpDistacne(fTimeDelta);

	if (m_bActive)
		return;

	if (m_bLockOn)
	{
		Update_LockOnCameraLook(fTimeDelta);
	}
	else
	{
		if (m_bSetPitchYaw)
			Update_TargetCameraLook(fTimeDelta);
		else
			Update_CameraLook(fTimeDelta);
	}

	__super::Update(fTimeDelta);
}

void CCamera_Orbital::Late_Update(_float fTimeDelta)
{
	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;
}

HRESULT CCamera_Orbital::Render()
{
	return S_OK;
}

void CCamera_Orbital::Set_InitCam(_float fPitch, _float fYaw)
{
	if (m_pPlayer)
	{
		m_pTransformCom->Set_WorldMatrix(Get_OrbitalWorldMatrix(fPitch, fYaw));
		Set_PitchYaw(fPitch, fYaw); // Set_PitchYaw 내부에서 Wrap 적용

		m_vPlayerPosition = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
		m_vPlayerPosition += XMVectorSet(0.f, 1.7f, 0.f, 0.f);
		m_vPlayerPosition += XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;
		m_vPrevLookTarget = m_vPlayerPosition;
	}
}

void CCamera_Orbital::Set_PitchYaw(_float fPitch, _float fYaw)
{
	// Normalize incoming angles to prevent boundary glitches
	m_fPitch = WrapPi(fPitch);
	m_fYaw = WrapPi(fYaw);
}

void CCamera_Orbital::Set_TargetYawPitch(_vector vDir, _float fLerpSpeed, _bool bActivePitch)
{
	const _float bx = XMVectorGetX(vDir);
	const _float by = XMVectorGetY(vDir);
	const _float bz = XMVectorGetZ(vDir);

	// Ensure current angles are in [-pi, pi] before computing nearest target
	m_fYaw = WrapPi(m_fYaw);
	m_fPitch = WrapPi(m_fPitch);

	// Yaw target
	const _float rawTargetYaw = atan2f(bx, bz); // [-pi, pi]
	m_fTargetYaw = MoveTargetNear(m_fYaw, rawTargetYaw);
	m_fTargetYaw = WrapPi(m_fTargetYaw); // keep target normalized

	// Pitch target
	if (bActivePitch)
	{
		_float rawTargetPitch = atan2f(by, sqrtf(bx * bx + bz * bz)); // typically [-pi/2, pi/2]
		rawTargetPitch += XMConvertToRadians(10.f);
		m_fTargetPitch = MoveTargetNear(m_fPitch, rawTargetPitch);
	}
	else
	{
		m_fTargetPitch = m_fPitch;
	}

	m_fTargetLerpSpeed = fLerpSpeed;
	m_bSetPitchYaw = true;
}

void CCamera_Orbital::Set_LockOn(CGameObject* pTarget, _bool bActive)
{
	if (bActive)
	{
		m_pLockOnTarget = pTarget;
		m_bLockOn = true;
	}
	else
	{
		m_pLockOnTarget = pTarget;
		m_bLockOn = false;
	}
}

_matrix CCamera_Orbital::Get_OrbitalWorldMatrix(_float fPitch, _float fYaw)
{
	if (!m_pPlayer)
		return _matrix();

	m_vPlayerPosition = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.7f, 0.f, 0.f);
	m_vPlayerPosition += XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;

	_float x = m_fDistance * cosf(fPitch) * sinf(fYaw);
	_float y = m_fDistance * sinf(fPitch);
	_float z = m_fDistance * cosf(fPitch) * cosf(fYaw);
	_vector vOffset = XMVectorSet(x, y, z, 0.f);

	_vector vCamPos = m_vPlayerPosition + vOffset;

	_vector vLook = XMVector3Normalize(m_vPlayerPosition - vCamPos);
	_vector vRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook));
	_vector vUp = XMVector3Cross(vLook, vRight);

	// matrix
	_matrix matWorld;
	matWorld.r[0] = vRight;
	matWorld.r[1] = vUp;
	matWorld.r[2] = vLook;
	matWorld.r[3] = XMVectorSetW(vCamPos, 1.f);

	return matWorld;
}

void CCamera_Orbital::Set_ActiveTalk(_bool bActive, CGameObject* pTarget, _bool bCanMove, _float fTalkOffSet)
{
	if (bActive)
	{
		m_bTalkStart = true;
		m_bTalkEnd = false;
		m_bTalkActive = true;
		m_pNpcTalkTarget = pTarget;
		m_bCanMoveTalk = bCanMove;
		m_fTalkOffSet = fTalkOffSet;
	}
	else
	{
		m_bTalkStart = false;
		m_bTalkEnd = true;
		m_bTalkActive = false;
		m_pNpcTalkTarget = pTarget;
		m_bCanMoveTalk = true;
	}
}

void CCamera_Orbital::Start_DistanceLerp(_float fTargetLerpDistance, _float fDistanceStartTime, _float fDistanceEndTime, _float fDistanceDelayTime)
{
	m_bDistanceLerp = true;
	m_fDistanceTime = fDistanceStartTime + fDistanceEndTime;
	m_fDistanceTarget = fTargetLerpDistance;
	m_fDistnaceStartSpeed = fDistanceStartTime;
	m_fDistnaceEndSpeed = fDistanceEndTime;
	m_fDistanceDelayTime = fDistanceDelayTime;
	m_fDistanceLerpElapsed = {};
}

void CCamera_Orbital::Update_CameraLook(_float fTimeDelta)
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pPlayer);

	_long MouseMoveX = 0;
	_long MouseMoveY = 0;

	if (m_bCanMoveTalk)
	{
		MouseMoveX = m_pGameInstance->Get_DIMouseMove(DIMM::X);
		MouseMoveY = m_pGameInstance->Get_DIMouseMove(DIMM::Y);
	}

	//  Yaw Pitch
	_float fAfterYaw = m_fYaw;
	_float fAfterPitch = m_fPitch;

	fAfterYaw += MouseMoveX * 0.008f * m_fMouseSensor;
	fAfterPitch += MouseMoveY * 0.008f * m_fMouseSensor;

	// Pitch clamp
	if (pPlayer->Get_PlayerState() != EPlayerState::IDLE)
		fAfterPitch = clamp(fAfterPitch, XMConvertToRadians(-30.f), XMConvertToRadians(60.f));
	else
		fAfterPitch = clamp(fAfterPitch, XMConvertToRadians(-50.f), XMConvertToRadians(60.f));

	m_fYaw = LerpFloat(m_fYaw, fAfterYaw, m_fMouseSensor);
	m_fPitch = LerpFloat(m_fPitch, fAfterPitch, m_fMouseSensor);

	// Keep yaw normalized every frame
	m_fYaw = WrapPi(m_fYaw);

	// camera
	Update_CameraPos(fTimeDelta);

	_vector vTargetLookPos;
	if (m_bTalkActive)
	{
		_vector targetPos = m_pNpcTalkTarget->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, m_fTalkOffSet, 0.f, 0.f);
		vTargetLookPos = targetPos;
	}
	else
	{
		vTargetLookPos = m_vPlayerPosition;
	}

	// init
	if (!m_bPrevLookInit) {
		m_vPrevLookTarget = vTargetLookPos;
		m_bPrevLookInit = true;
	}

	// smoothing (exp decay)
	float alpha = 1.f - expf(-m_fLookLerpSpeed * fTimeDelta);
	m_vPrevLookTarget = XMVectorLerp(m_vPrevLookTarget, vTargetLookPos, alpha);

	m_pTransformCom->LookAt(m_vPrevLookTarget);
}

void CCamera_Orbital::Update_TargetCameraLook(_float fTimeDelta)
{
	// Yaw: move along shortest arc, normalize as we go
	float dyaw = ShortestDelta(m_fYaw, m_fTargetYaw);
	m_fYaw = WrapPi(m_fYaw + dyaw * (fTimeDelta * m_fTargetLerpSpeed));

	// Pitch: simple lerp toward target pitch (usually clamped elsewhere)
	m_fPitch = LerpFloat(m_fPitch, m_fTargetPitch, fTimeDelta * m_fTargetLerpSpeed);

	// camera
	Update_CameraPos(fTimeDelta);

	_vector vTargetLookPos;
	if (m_bTalkActive)
	{
		_vector targetPos = m_pNpcTalkTarget->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, m_fTalkOffSet, 0.f, 0.f);
		vTargetLookPos = targetPos;
	}
	else
	{
		vTargetLookPos = m_vPlayerPosition;
	}

	// init
	if (!m_bPrevLookInit) {
		m_vPrevLookTarget = vTargetLookPos;
		m_bPrevLookInit = true;
	}

	// smoothing
	float alpha = 1.f - expf(-m_fLookLerpSpeed * fTimeDelta);
	m_vPrevLookTarget = XMVectorLerp(m_vPrevLookTarget, vTargetLookPos, alpha);

	m_pTransformCom->LookAt(m_vPrevLookTarget);

	// use angular deltas for completion (wrap-safe)
	const float eps = 0.001f;
	if (fabsf(ShortestDelta(m_fYaw, m_fTargetYaw)) < eps && fabsf(m_fPitch - m_fTargetPitch) < eps)
	{
		m_bSetPitchYaw = false;
	}
}

void CCamera_Orbital::Update_LockOnCameraLook(_float fTimeDelta)
{
	_vector vTargetLookPos = XMLoadFloat4(&static_cast<CUnit*>(m_pLockOnTarget)->Get_LockonPos());

	// exp decay
	float alpha = 1.f - expf(-m_fLookLerpSpeed * fTimeDelta);
	m_vPrevLookTarget = XMVectorLerp(m_vPrevLookTarget, vTargetLookPos, alpha);
	m_pTransformCom->LookAt(m_vPrevLookTarget);

	_vector vCamLook = m_pTransformCom->Get_State(STATE::LOOK) * -1;

	const _float bx = XMVectorGetX(vCamLook);
	const _float by = XMVectorGetY(vCamLook);
	const _float bz = XMVectorGetZ(vCamLook);

	// derive angles from current camera orientation
	m_fYaw = WrapPi(atan2f(bx, bz));
	m_fPitch = atan2f(by, sqrtf(bx * bx + bz * bz));

	// distance-based pitch lift
	_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
	_float dist = XMVectorGetX(XMVector3Length(vTargetLookPos - vPlayerPos));
	const _float maxDist = 10.f;
	_float t = 1.f - clamp(dist / maxDist, 0.f, 1.f);
	_float pitchOffset = XMConvertToRadians(10.f * t);

	m_fPitch += pitchOffset;
	m_fPitch = clamp(m_fPitch, XMConvertToRadians(-20.f), XMConvertToRadians(30.f));

	Update_CameraPos(fTimeDelta);
}

void CCamera_Orbital::Update_CameraPos(_float fTimeDelta)
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pPlayer);

	// follow point (player + height + slight back)
	if (!m_bTalkActive)
	{
		m_vPlayerPosition = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
		m_vPlayerPosition += XMVectorSet(0.f, 1.7f, 0.f, 0.f);
		m_vPlayerPosition += XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;
	}

	// offset from yaw/pitch
	_float x = m_fDistance * cosf(m_fPitch) * sinf(m_fYaw);
	_float y = m_fDistance * sinf(m_fPitch);
	_float z = m_fDistance * cosf(m_fPitch) * cosf(m_fYaw);
	_vector vOffset = XMVectorSet(x, y, z, 0.f);

	// target cam pos
	m_vTargetCamPos = m_vPlayerPosition + vOffset;

	// --- spring-arm Raycast ---
	_vector vRayDir = XMVector3Normalize(vOffset);

	PxVec3 origin = VectorToPxVec3(m_vPlayerPosition);
	PxVec3 direction = VectorToPxVec3(vRayDir);

	PxRaycastBuffer hit;
	PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
	PxQueryFilterData filterData;
	filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	unordered_set<PxActor*> ignoreActors = pPlayer->Get_Controller()->Get_IngoreActors();
	CIgnoreSelfCallback callback(ignoreActors);

	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, m_fDistance, hit, hitFlags, filterData, &callback))
	{
		if (hit.hasBlock)
		{
			_float fHitDist = hit.block.distance - 0.3f; // margin
			fHitDist = max(fHitDist, 0.5f);              // min distance

			m_vTargetCamPos = m_vPlayerPosition + vRayDir * fHitDist;
		}
	}
	// --- end spring-arm Raycast ---

	// smooth position
	const _float kPos = 12.f;
	const _float a = 1.f - expf(-kPos * fTimeDelta);

	_vector vCurPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vNewPos = XMVectorLerp(vCurPos, m_vTargetCamPos, a);

	if (XMVectorGetX(XMVector3LengthSq(vNewPos - m_vTargetCamPos)) < 1e-6f)
		vNewPos = m_vTargetCamPos;

	m_pTransformCom->Set_State(STATE::POSITION, vNewPos);
}

void CCamera_Orbital::Update_LerpDistacne(_float fTimeDelta)
{
	if (m_bTalkStart)
	{
		m_fDistance = LerpFloat(m_fDistance, 1.7f, fTimeDelta * 2.f);

		if (fabsf(m_fDistance - 1.7f) < 0.01f)
		{
			m_fDistance = 1.7f;
			m_bTalkStart = false;
		}
	}

	if (m_bTalkEnd)
	{
		m_fDistance = LerpFloat(m_fDistance, 3.f, fTimeDelta * 2.f);

		if (fabsf(m_fDistance - 3.f) < 0.01f)
		{
			m_fDistance = 3.f;
			m_bTalkEnd = false;
		}
	}

	if (m_bDistanceLerp)
	{
		m_fDistanceLerpElapsed += fTimeDelta;

		// phase 1: 3.0 -> target
		if (m_fDistanceLerpElapsed <= m_fDistnaceStartSpeed)
		{
			_float t = m_fDistnaceStartSpeed <= 0.f ? 1.f : (m_fDistanceLerpElapsed / m_fDistnaceStartSpeed);
			m_fDistance = LerpFloat(3.f, m_fDistanceTarget, t);
		}
		// delay hold
		else if (m_fDistanceLerpElapsed <= (m_fDistnaceStartSpeed + m_fDistanceDelayTime))
		{
			m_fDistanceDelayElapsed += fTimeDelta;
			m_fDistance = m_fDistanceTarget;
		}
		// phase 2: target -> 3.0
		else if (m_fDistanceLerpElapsed <= (m_fDistnaceStartSpeed + m_fDistanceDelayTime + m_fDistnaceEndSpeed))
		{
			_float t = m_fDistnaceEndSpeed <= 0.f
				? 1.f
				: ((m_fDistanceLerpElapsed - (m_fDistnaceStartSpeed + m_fDistanceDelayTime)) / m_fDistnaceEndSpeed);

			m_fDistance = LerpFloat(m_fDistanceTarget, 3.f, t);
		}
		else
		{
			m_bDistanceLerp = false;
			m_fDistance = 3.f;
		}
	}
}


CCamera_Orbital* CCamera_Orbital::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Orbital* pGameInstance = new CCamera_Orbital(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CCamera_Orbital");
		Safe_Release(pGameInstance);
	}
	return pGameInstance;
}

CGameObject* CCamera_Orbital::Clone(void* pArg)
{
	CCamera_Orbital* pGameInstance = new CCamera_Orbital(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CCamera_Orbital");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CCamera_Orbital::Free()
{
	__super::Free();

}
