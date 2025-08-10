#include "Camera_Orbital.h"
#include "GameInstance.h"

#include "PhysX_IgnoreSelfCallback.h"
#include "Camera_Manager.h"
#include "Player.h"

#include "Client_Calculation.h"

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
	_long iWheelDelta = m_pGameInstance->Get_DIMouseMove(DIMM::WHEEL);

	// 줌 거리 조정
	m_fDistance -= iWheelDelta * fTimeDelta * m_fZoomSpeed;
	m_fDistance = clamp(m_fDistance, 2.f, 5.5f);

	//  Yaw Pitch 설정
	_float fAfterYaw = m_fYaw;
	_float fAfterPitch = m_fPitch;

	fAfterYaw += MouseMoveX * fTimeDelta * m_fMouseSensor;
	fAfterPitch += MouseMoveY * fTimeDelta * m_fMouseSensor;

	// 카메라 Pitch 제한
	if (pPlayer->Get_PlayerState() != EPlayerState::IDLE)
		fAfterPitch = clamp(fAfterPitch, XMConvertToRadians(-30.f), XMConvertToRadians(60.f));
	else
		fAfterPitch = clamp(fAfterPitch, XMConvertToRadians(-50.f), XMConvertToRadians(60.f));

	m_fYaw = LerpFloat(m_fYaw, fAfterYaw, fTimeDelta * 5.f);
	m_fPitch = LerpFloat(m_fPitch, fAfterPitch, fTimeDelta * 5.f);

	// 카메라 설정
	Set_CameraMatrix();
}

void CCamera_Orbital::Update_TargetCameraMatrix(_float fTimeDelta)
{
	// Yaw 와 Pitch 설정
	m_fYaw = LerpFloat(m_fYaw, m_fTargetYaw, fTimeDelta * m_fTargetLerpSpeed);
	m_fPitch = LerpFloat(m_fPitch, m_fTargetPitch, fTimeDelta * m_fTargetLerpSpeed);

	// 카메라 설정
	Set_CameraMatrix();

	if (fabs(m_fYaw - m_fTargetYaw) < 0.001f && fabs(m_fPitch - m_fTargetPitch) < 0.001f)
	{
		m_bSetPitchYaw = false;
	}
}

void CCamera_Orbital::Update_LockOnCameraMatrix(_float fTimeDelta)
{
	XMVECTOR vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.7f, 0.f, 0.f);
	XMVECTOR vTargetPos = m_pLockOnTarget->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.3f, 0.f, 0.f);

	// 중점
	XMVECTOR vMid = (vPlayerPos + vTargetPos) * 0.5f;

	// 플레이어 타겟 방향
	XMVECTOR vF = XMVector3Normalize(vTargetPos - vPlayerPos);
	if (XMVector3Less(XMVector3LengthSq(vF), XMVectorReplicate(1e-6f))) {
		// 둘이 거의 같은 위치면 플레이어의 LOOK을 사용
		vF = XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK));
	}

	// ===== 거리 계산 =====
	float span = XMVectorGetX(XMVector3Length(vTargetPos - vPlayerPos)); // 둘 사이 거리
	const float kFrame = 0.85f;   // 화면 여유
	const float pad = 1.0f;    // 근접 패딩(둘이 붙어도 이만큼은 확보)
	const float dMin = 3.0f;    // 카메라 최솟값(너무 들이대지 않기)
	const float dMax = 8.0f;   // 최댓값

	// 세로 FOV 기반 필요 거리 (반지름span/2 + pad)
	float r = 0.5f * span + pad;
	float dFov = (r / kFrame) / tanf(m_fFov * 0.5f);

	// 최종 목표 거리: 바닥/최대 포함
	float dTarget = std::clamp(dFov, dMin, dMax);

	// 거리 스무딩(프레임 독립 지수보간)
	static float dCurr = dMin; // 멤버로 빼도 됨
	float aDist = 1.0f - expf(-8.0f * fTimeDelta); // 속도 계수 8
	dCurr = dCurr + (dTarget - dCurr) * aDist;

	// 목표 카메라 위치 = 중점에서 -F로 dCurr
	XMVECTOR vDesired = vMid - vF * dCurr;

	// ===== 스프링암(충돌 보정) =====
	XMVECTOR vDir = XMVector3Normalize(vDesired - vMid);
	float rayLen = XMVectorGetX(XMVector3Length(vDesired - vMid));

	XMFLOAT3 fC, fDir; XMStoreFloat3(&fC, vMid); XMStoreFloat3(&fDir, vDir);
	PxVec3 origin(fC.x, fC.y, fC.z), direction(fDir.x, fDir.y, fDir.z);

	PxRaycastBuffer hit;
	PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
	PxQueryFilterData filter; filter.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	// 자기 자신/타겟 무시
	unordered_set<PxActor*> ignore;
	ignore = static_cast<CPlayer*>(m_pPlayer)->Get_Controller()->Get_IngoreActors();
	CIgnoreSelfCallback cb(ignore);

	XMVECTOR vCamPos = vDesired;
	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, rayLen, hit, hitFlags, filter, &cb) && hit.hasBlock)
	{
		float pullPad = 0.3f;  // 표면 여유
		float minDist = 0.8f;  // 너무 붙지 않기
		float hitDist = max(hit.block.distance - pullPad, minDist);
		vCamPos = vMid + vDir * hitDist;
	}

	// ===== 위치/시선 보간 =====
	XMVECTOR vCur = m_pTransformCom->Get_State(STATE::POSITION);
	float aPos = 1.0f - expf(-10.0f * fTimeDelta); // 위치 스무스
	XMVECTOR vPos = XMVectorLerp(vCur, vCamPos, aPos);

	// 시선은 중점으로(조금 스무스)
	XMVECTOR Fcur = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	XMVECTOR Fdes = XMVector3Normalize(vMid - vPos);
	float aAim = 1.0f - expf(-12.0f * fTimeDelta);
	XMVECTOR F = XMVector3Normalize(XMVectorLerp(Fcur, Fdes, aAim));

	// 월드 행렬 세팅
	XMVECTOR UpW = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	XMVECTOR Rw = XMVector3Normalize(XMVector3Cross(UpW, F));
	XMVECTOR Uw = XMVector3Normalize(XMVector3Cross(F, Rw));

	_matrix W = XMMatrixIdentity();
	W.r[0] = XMVectorSetW(Rw, 0.f);
	W.r[1] = XMVectorSetW(Uw, 0.f);
	W.r[2] = XMVectorSetW(F, 0.f);
	W.r[3] = XMVectorSetW(vPos, 1.f);
	m_pTransformCom->Set_WorldMatrix(W);
}

void CCamera_Orbital::Set_CameraMatrix()
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
	m_pTransformCom->Set_State(STATE::POSITION, m_vTargetCamPos);
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
