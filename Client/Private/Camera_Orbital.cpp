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
	// 1) 플레이어/타겟 위치
	XMVECTOR vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.7f, 0.f, 0.f);
	XMVECTOR vTargetPos = m_pLockOnTarget->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.3f, 0.f, 0.f);

	// 2) 중점(0.5) + 전방(F)
	XMVECTOR vMid = (vPlayerPos + vTargetPos) * 0.5f;

	XMVECTOR vF = XMVector3Normalize(vTargetPos - vPlayerPos);
	if (XMVector3Less(XMVector3LengthSq(vF), XMVectorReplicate(1e-6f))) {
		// 둘이 거의 같은 위치면 플레이어 LOOK 사용
		vF = XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK));
	}

	// 3) 거리 목표(dTarget)
	float span = XMVectorGetX(XMVector3Length(vTargetPos - vPlayerPos)); // 둘 사이 거리
	const float kFrame = 0.85f;   // 화면 여유(작을수록 더 멀게)
	const float pad = 1.0f;     // 근접 패딩
	const float dMin = 3.0f;     // 최소
	const float dMax = 6.0f;     // 최대(기본)

	float r = 0.5f * span + pad;                // 화면에 둘 다 담기 위한 반지름
	float dFov = (r / kFrame) / tanf(m_fFov * 0.5f);
	float dTarget = std::clamp(dFov, dMin, dMax);

	// 3-1) 플레이어 최소 이격(깊이) 1차 보장: dTarget 산출 직후
	{
		const float minPlayerDepth = 1.2f; // F축 최소 깊이
		XMVECTOR vDesiredPre = vMid - vF * dTarget;
		float playerDepth = XMVectorGetX(XMVector3Dot(vPlayerPos - vDesiredPre, vF));
		if (playerDepth < minPlayerDepth) {
			dTarget += (minPlayerDepth - playerDepth);
			dTarget = std::min(dTarget, dMax); // 그래도 dMax는 넘지 않게
		}
	}

	// 4) 거리 스무딩(프레임 독립 지수보간)
	static float dCurr = dMin; // 멤버변수로 빼도 됨
	float aDist = 1.0f - expf(-8.0f * fTimeDelta); // 응답속도 8
	dCurr = dCurr + (dTarget - dCurr) * aDist;

	// 5) 목표 카메라 위치 = 중점에서 -F로 dCurr
	XMVECTOR vDesired = vMid - vF * dCurr;

	// 6) 스프링암(충돌 보정)
	XMVECTOR vDir = XMVector3Normalize(vDesired - vMid);
	float    rayLen = XMVectorGetX(XMVector3Length(vDesired - vMid));

	XMFLOAT3 fC, fDir; XMStoreFloat3(&fC, vMid); XMStoreFloat3(&fDir, vDir);
	PxVec3 origin(fC.x, fC.y, fC.z), direction(fDir.x, fDir.y, fDir.z);

	PxRaycastBuffer   hit;
	PxHitFlags        hitFlags = PxHitFlag::eDEFAULT;
	PxQueryFilterData filter;    filter.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	// 자기 자신/타겟 무시
	unordered_set<PxActor*> ignore = static_cast<CPlayer*>(m_pPlayer)->Get_Controller()->Get_IngoreActors();
	CIgnoreSelfCallback cb(ignore);

	XMVECTOR vCamPos = vDesired;
	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, rayLen, hit, hitFlags, filter, &cb) && hit.hasBlock)
	{
		float pullPad = 0.3f;  // 표면 여유
		float minDist = 0.8f;  // 너무 붙지 않기
		float hitDist = max(hit.block.distance - pullPad, minDist);
		vCamPos = vMid + vDir * hitDist;
	}

	// 6-1) ★충돌 보정 이후 ‘플레이어 여유 깊이’ 2차 보장 (여기가 핵심 추가 블록)
	{
		// span이 커질수록 여유를 조금 더 키워서 화면 압박 완화
		float minDepthNear = 1.2f;    // 근거리 기본 여유
		float minDepthFar = 3.0f;    // 원거리 때 여유 상한
		float spanFor0 = 2.0f * dMin; // 이 이하 → near 쪽
		float spanFor1 = 2.0f * dMax; // 이 이상 → far 쪽

		float tSpan = 0.f;
		if (spanFor1 > spanFor0)
			tSpan = std::clamp((span - spanFor0) / (spanFor1 - spanFor0), 0.f, 1.f);

		float minDepth = minDepthNear + (minDepthFar - minDepthNear) * tSpan;

		// 현재 카메라에서 플레이어까지 F축 깊이
		float currDepth = XMVectorGetX(XMVector3Dot(vPlayerPos - vCamPos, vF));
		if (currDepth < minDepth) {
			float needBack = minDepth - currDepth;
			float maxExtra = 2.0f; // dMax 초과 허용 한도(과하게 뒤로 빠지는 것 방지)
			float extra = std::min(needBack, maxExtra);
			vCamPos -= vF * extra; // 뒤로( -F 방향 ) 살짝 더 빼줌
			// 필요하다면 여기서 한 번 더 레이캐스트로 재확인해도 됨(맵 구조에 따라)
		}
	}

	// 7) 위치/시선 보간
	XMVECTOR vCur = m_pTransformCom->Get_State(STATE::POSITION);
	float aPos = 1.0f - expf(-10.0f * fTimeDelta); // 위치 스무딩
	XMVECTOR vPos = XMVectorLerp(vCur, vCamPos, aPos);

	// 시선은 중점으로(조금 스무스)
	XMVECTOR Fcur = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	XMVECTOR Fdes = XMVector3Normalize(vMid - vPos);
	float aAim = 1.0f - expf(-12.0f * fTimeDelta);
	XMVECTOR F = XMVector3Normalize(XMVectorLerp(Fcur, Fdes, aAim));

	// 8) 월드 행렬 세팅
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
