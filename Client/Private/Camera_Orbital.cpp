#include "Camera_Orbital.h"
#include "GameInstance.h"

#include "PhysX_IgnoreSelfCallback.h"
#include "Camera_Manager.h"
#include "Player.h"

#include "Client_Calculation.h"
#include "Unit.h"

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
	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;
	__super::Priority_Update(fTimeDelta);
}

void CCamera_Orbital::Update(_float fTimeDelta)
{
	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;

	//if (m_pGameInstance->Key_Down(DIK_T))
	//{
	//	m_bActive = !m_bActive;
	//	printf("Pitch %f, Yaw %f\n", m_fPitch, m_fYaw);
	//}

	if (m_pGameInstance->Key_Down(DIK_X))
	{
		m_fMouseSensor -= 0.1f;
		if (m_fMouseSensor <  0.f)
			m_fMouseSensor = 0.1f;
		printf("mouseSenor %f\n", m_fMouseSensor);
	}
		
	if (m_pGameInstance->Key_Down(DIK_C))
	{
		m_fMouseSensor += 0.1f;
		printf("mouseSenor %f\n", m_fMouseSensor);
	}

	if (!m_pPlayer)
		return;

	if (m_bLockOn)
	{
		Update_LockOnCameraMatrix(fTimeDelta);
	}
	else
	{
		if (m_bSetPitchYaw)
			Update_TargetCameraMatrix(fTimeDelta);
		else
			Update_CameraMatrix(fTimeDelta);
	}
	
	__super::Update(fTimeDelta);
}

void CCamera_Orbital::Late_Update(_float fTimeDelta)
{
}

HRESULT CCamera_Orbital::Render()
{
	return S_OK;
}

void CCamera_Orbital::Set_InitCam()
{
	if (m_pPlayer)
	{
		m_pTransformCom->Set_WorldMatrix(Get_OrbitalWorldMatrix(0.232652f, -1.561575f));
		Set_PitchYaw(0.232652f, -1.561575f);
	}
}

void CCamera_Orbital::Set_PitchYaw(_float fPitch, _float fYaw)
{
	m_fPitch = fPitch;
	m_fYaw = fYaw;
}

void CCamera_Orbital::Set_LockOn(CGameObject* pTarget, _bool bActive)
{
	m_pLockOnTarget = pTarget;
	m_bLockOn = bActive;
}

_matrix CCamera_Orbital::Get_OrbitalWorldMatrix(_float fPitch, _float fYaw)
{
	if(!m_pPlayer)
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

	// 행렬 생성
	_matrix matWorld;
	matWorld.r[0] = vRight;                          
	matWorld.r[1] = vUp;                              
	matWorld.r[2] = vLook;                            
	matWorld.r[3] = XMVectorSetW(vCamPos, 1.f);        

	return matWorld;
}

void CCamera_Orbital::Set_TargetYawPitch(_vector vDir, _float fLerpSpeed)
{
	const _float bx = XMVectorGetX(vDir);
	const _float by = XMVectorGetY(vDir);
	const _float bz = XMVectorGetZ(vDir);

	// Pitch Yaw 역계산
	m_fTargetYaw = atan2f(bx, bz);
	m_fTargetPitch = atan2f(by, sqrtf(bx * bx + bz * bz));

	// 살짝 위에서 보이게 
	m_fTargetPitch += XMConvertToRadians(10.f);

	m_fTargetLerpSpeed = fLerpSpeed;

	m_bSetPitchYaw = true;
}

void CCamera_Orbital::Update_CameraMatrix(_float fTimeDelta)
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pPlayer);

	_long MouseMoveX = m_pGameInstance->Get_DIMouseMove(DIMM::X);
	_long MouseMoveY = m_pGameInstance->Get_DIMouseMove(DIMM::Y);

	//  Yaw Pitch 설정
	_float fAfterYaw = m_fYaw;
	_float fAfterPitch = m_fPitch;

	fAfterYaw += MouseMoveX * 0.008f * m_fMouseSensor;
	fAfterPitch += MouseMoveY * 0.008f * m_fMouseSensor;

	// 카메라 Pitch 제한
	if (pPlayer->Get_PlayerState() != EPlayerState::IDLE)
		fAfterPitch = clamp(fAfterPitch, XMConvertToRadians(-30.f), XMConvertToRadians(60.f));
	else
		fAfterPitch = clamp(fAfterPitch, XMConvertToRadians(-50.f), XMConvertToRadians(60.f));

	m_fYaw = LerpFloat(m_fYaw, fAfterYaw, m_fMouseSensor);
	m_fPitch = LerpFloat(m_fPitch, fAfterPitch, m_fMouseSensor);

	// 카메라 설정
	Set_CameraMatrix(fTimeDelta);
}

void CCamera_Orbital::Update_TargetCameraMatrix(_float fTimeDelta)
{
	// Yaw 와 Pitch 설정
	m_fYaw = LerpFloat(m_fYaw, m_fTargetYaw, fTimeDelta * m_fTargetLerpSpeed);
	m_fPitch = LerpFloat(m_fPitch, m_fTargetPitch, fTimeDelta * m_fTargetLerpSpeed);

	// 카메라 설정
	Set_CameraMatrix(fTimeDelta);

	if (fabs(m_fYaw - m_fTargetYaw) < 0.001f && fabs(m_fPitch - m_fTargetPitch) < 0.001f)
	{
		m_bSetPitchYaw = false;
	}
}

void CCamera_Orbital::Update_LockOnCameraMatrix(_float fTimeDelta)
{
	/*
	const _vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION)+ XMVectorSet(0.f, 1.7f, 0.f, 0.f);
	const _vector vTargetPos = XMLoadFloat4(&static_cast<CUnit*>(m_pLockOnTarget)->Get_LockonPos());
	const _vector vCenter = (vPlayerPos + vTargetPos) * 0.5f;
	
	// 플레이어에서 타겟 방향, 너무 가까우면 플레이어 LOOK 사용
	_vector vCamForward = XMVector3Normalize(vTargetPos - vPlayerPos);
	const _float fSpanPT = XMVectorGetX(XMVector3Length(vTargetPos - vPlayerPos)); 
	if (fSpanPT < 0.0000001f)
		vCamForward = XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK));

	// 플레이어와 타겟이 사이의 반지름
	const _float fRadius = 0.5f * fSpanPT + m_fPadding;
	_float       fWantDist = (fRadius / m_fFrame) / tanf(m_fFov *   0.5f);
	fWantDist = clamp(fWantDist, m_fDistanceMin, m_fDistanceMax);

	// 플레이어와 카메라간의 최소 거리
	const _vector vDesiredPre = vCenter - vCamForward * fWantDist;
	const _float fPlayerDepthPre = XMVectorGetX(XMVector3Dot(vPlayerPos - vDesiredPre, vCamForward));
	if (fPlayerDepthPre < kMinPlayerDepth) {
		fWantDist += (kMinPlayerDepth - fPlayerDepthPre);
		fWantDist = min(fWantDist, m_fDistanceMax);
	}

	// 거리 보간 & 충돌 보정 전 카메라 위치
	_float fCurDistance = m_fDistanceMin;
	fCurDistance = SmoothExp(fCurDistance, fWantDist, 8.0f, fTimeDelta);
	const _vector vDesired = vCenter - vCamForward * fCurDistance;

#pragma region 스프링암
	// 스프링암 충돌 보정
	XMVECTOR vDir = XMVector3Normalize(vDesired - vCenter);     
	float    fRayLen = XMVectorGetX(XMVector3Length(vDesired - vCenter));

	XMFLOAT3 fC, fD; XMStoreFloat3(&fC, vCenter); XMStoreFloat3(&fD, vDir);
	PxVec3 origin(fC.x, fC.y, fC.z), direction(fD.x, fD.y, fD.z);

	PxRaycastBuffer   hit;
	PxHitFlags        hitFlags = PxHitFlag::eDEFAULT;
	PxQueryFilterData filter; filter.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	unordered_set<PxActor*> ignore = static_cast<CPlayer*>(m_pPlayer)->Get_Controller()->Get_IngoreActors();
	CIgnoreSelfCallback cb(ignore);

	XMVECTOR vCamPos = vDesired;
	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, fRayLen, hit, hitFlags, filter, &cb) && hit.hasBlock)
	{
		const float kPullPad = 0.3f;  // 표면 여유
		const float kMinDist = 0.8f;  // 너무 붙지 않기
		const float hitDist = max(hit.block.distance - kPullPad, kMinDist);
		vCamPos = vCenter + vDir * hitDist;
	}
#pragma endregion

	// ===== 5) 플레이어 여유 깊이(2차) 보장 =====
	{
		const float minDepthNear = 3.f;
		const float minDepthFar = 6.0f;
		const float span0 = 2.0f * m_fDistanceMin;
		const float span1 = 2.0f * m_fDistanceMax;

		float tSpan = 0.f;
		if (span1 > span0)
			tSpan = std::clamp((fSpanPT - span0) / (span1 - span0), 0.f, 1.f);

		const float minDepth = minDepthNear + (minDepthFar - minDepthNear) * tSpan;

		const float currDepth = XMVectorGetX(XMVector3Dot(vPlayerPos - vCamPos, vCamForward));
		if (currDepth < minDepth) {
			const float needBack = minDepth - currDepth;
			const float maxExtra = 2.0f;
			const float extra = std::min(needBack, maxExtra);
			vCamPos -= vCamForward * extra; // 뒤로(-F)
			// 필요하면 여기서 재-레이캐스트
		}
	}

	// ===== 6) 위치/시선 스무딩 =====
	const XMVECTOR vPosCurr = m_pTransformCom->Get_State(STATE::POSITION);
	const XMVECTOR vPosNext = XMVectorLerp(vPosCurr, vCamPos, (1.f - expf(-10.f * fTimeDelta)));

	const XMVECTOR Fcur = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	const XMVECTOR Fdes = XMVector3Normalize(vCenter - vPosNext);
	const XMVECTOR F = XMVector3Normalize(XMVectorLerp(Fcur, Fdes, (1.f - expf(-12.f * fTimeDelta))));

	// ===== 7) 월드 행렬 구성 =====
	const XMVECTOR Up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	const XMVECTOR R = XMVector3Normalize(XMVector3Cross(Up, F));
	const XMVECTOR U = XMVector3Normalize(XMVector3Cross(F, R));

	_matrix W = XMMatrixIdentity();
	W.r[0] = XMVectorSetW(R, 0.f);
	W.r[1] = XMVectorSetW(U, 0.f);
	W.r[2] = XMVectorSetW(F, 0.f);
	W.r[3] = XMVectorSetW(vPosNext, 1.f);

	m_pTransformCom->Set_WorldMatrix(W);
	*/

	// 플레이어와 타겟 위치
	XMVECTOR vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.7f, 0.f, 0.f);
	XMVECTOR vTargetPos = XMLoadFloat4(&static_cast<CUnit*>(m_pLockOnTarget)->Get_LockonPos());

	// 두 점 중점 계산
	XMVECTOR vMid = (vPlayerPos + vTargetPos) * 0.5f;

	// 두 캐릭터 간 거리
	float fDistance = XMVectorGetX(XMVector3Length(vTargetPos - vPlayerPos));

	// m_fFov는 세로 기준이므로 세로 FOV로 거리 계산
	// 삼각함수로 거리 확보:  tan(FOV/2) = (높이/2) / 거리
	float fRequiredDist = (fDistance * 0.5f) / tanf(m_fFov * 0.5f);

	// 카메라 방향(플레이어-타겟 바라보는 방향 반대)
	XMVECTOR vForward = XMVector3Normalize(vMid - vPlayerPos);
	XMVECTOR vCamDir = -vForward;

	// 최종 카메라 위치 = 중점 + 뒤로 뺀 값 + 살짝 위로 올리기
	XMVECTOR vCamPos = vMid + vCamDir * fRequiredDist + XMVectorSet(0.f, 1.5f, 0.f, 0.f);

	// Up 벡터
	XMVECTOR vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	// 월드 행렬 구성
	XMMATRIX matView = XMMatrixLookAtLH(vCamPos, vMid, vUp);
	XMMATRIX matWorld = XMMatrixInverse(nullptr, matView);

	// 트랜스폼에 적용
	m_pTransformCom->Set_WorldMatrix(matWorld);

}
void CCamera_Orbital::Set_CameraMatrix(_float fTimeDelta)
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pPlayer);

	// 기준점 위치 계산 (플레이어 + 높이 + 조금 뒤에)
	m_vPlayerPosition = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
	m_vPlayerPosition += XMVectorSet(0.f, 1.7f, 0.f, 0.f);
	m_vPlayerPosition += XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;

	// 방향 계산
	_float x = m_fDistance * cosf(m_fPitch) * sinf(m_fYaw);
	_float y = m_fDistance * sinf(m_fPitch);
	_float z = m_fDistance * cosf(m_fPitch) * cosf(m_fYaw);
	_vector vOffset = XMVectorSet(x, y, z, 0.f);

	// 목표 카메라 위치
	m_vTargetCamPos = m_vPlayerPosition + vOffset;

	// --- 스프링암 Raycast 처리 시작 ---
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
			_float fHitDist = hit.block.distance - 0.3f; // 여유 거리
			fHitDist = max(fHitDist, 0.5f);              // 너무 가까워지지 않게 제한

			// 보정된 카메라 위치
			m_vTargetCamPos = m_vPlayerPosition + vRayDir * fHitDist;
		}
	}
	// --- 스프링암 Raycast 처리 끝 ---

	// 카메라 설정
	// ===== 위치만 보간 =====
	const float dt = fTimeDelta; // 혹은 인자로 받기
	const float kPos = 12.f;                           // 응답 속도(높을수록 빠름)
	const float a = 1.f - expf(-kPos * dt);         // 프레임 독립 보간 계수

	_vector vCurPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vNewPos = XMVectorLerp(vCurPos, m_vTargetCamPos, a);

	// 아주 가까워지면 스냅
	if (XMVectorGetX(XMVector3LengthSq(vNewPos - m_vTargetCamPos)) < 1e-6f)
		vNewPos = m_vTargetCamPos;

	// 카메라 설정
	m_pTransformCom->Set_State(STATE::POSITION, vNewPos);
	m_pTransformCom->LookAt(m_vPlayerPosition);
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
