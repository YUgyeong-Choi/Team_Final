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

	//// 화면 중앙으로 마우스 위치 고정
	//RECT rcClient;
	//GetClientRect(g_hWnd, &rcClient);

	//POINT ptCenter;
	//ptCenter.x = (rcClient.right - rcClient.left) / 2;
	//ptCenter.y = (rcClient.bottom - rcClient.top) / 2;

	//// 클라이언트 좌표 -> 스크린 좌표로 변환
	//ClientToScreen(g_hWnd, &ptCenter);

	//// 마우스 커서 이동
	//SetCursorPos(ptCenter.x, ptCenter.y);

	//if (m_pGameInstance->Key_Down(DIK_T))
	//{
	//	m_bActive = !m_bActive;
	//	printf("Pitch %f, Yaw %f\n", m_fPitch, m_fYaw);
	//}

	//if (m_pGameInstance->Key_Down(DIK_X))
	//{
	//	m_fMouseSensor -= 0.1f;
	//	if (m_fMouseSensor <  0.f)
	//		m_fMouseSensor = 0.1f;
	//	printf("mouseSenor %f\n", m_fMouseSensor);
	//}
	//	
	//if (m_pGameInstance->Key_Down(DIK_C))
	//{
	//	m_fMouseSensor += 0.1f;
	//	printf("mouseSenor %f\n", m_fMouseSensor);
	//}

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
		Set_PitchYaw(fPitch, fYaw);

		m_vPlayerPosition = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
		m_vPlayerPosition += XMVectorSet(0.f, 1.7f, 0.f, 0.f);
		m_vPlayerPosition += XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;
		m_vPrevLookTarget = m_vPlayerPosition;
	}
}

void CCamera_Orbital::Set_PitchYaw(_float fPitch, _float fYaw)
{
	m_fPitch = fPitch;
	m_fYaw = fYaw;
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

void CCamera_Orbital::Start_DistanceLerp(_float fTargetLerpDistance, _float fDistanceLerpSpeed)
{
	m_bLerpDistanceStart = true;
	m_fSaveDistance = m_fDistance;
	m_fTargetLerpDistance = fTargetLerpDistance;
	m_fDistanceLerpSpeed = fDistanceLerpSpeed;
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
	Update_CameraPos(fTimeDelta);

	_vector vTargetLookPos;
	if (m_bTalkActive)
	{
		_vector targetPos = m_pNpcTalkTarget->Get_TransfomCom()->Get_State(STATE::POSITION)+ XMVectorSet(0.f, m_fTalkOffSet, 0.f, 0.f);
		vTargetLookPos = targetPos;
	}
	else
	{
		vTargetLookPos = m_vPlayerPosition;
	}

	// 초기화 (첫 프레임)
	if (!m_bPrevLookInit) {
		m_vPrevLookTarget = vTargetLookPos;
		m_bPrevLookInit = true;
	}

	// 보간
	float alpha = 1.f - expf(-m_fLookLerpSpeed * fTimeDelta);
	m_vPrevLookTarget = XMVectorLerp(m_vPrevLookTarget, vTargetLookPos, alpha);

	m_pTransformCom->LookAt(m_vPrevLookTarget);
}

void CCamera_Orbital::Update_TargetCameraLook(_float fTimeDelta)
{
	// Yaw 와 Pitch 설정
	m_fYaw = LerpFloat(m_fYaw, m_fTargetYaw, fTimeDelta * m_fTargetLerpSpeed);
	m_fPitch = LerpFloat(m_fPitch, m_fTargetPitch, fTimeDelta * m_fTargetLerpSpeed);

	// 카메라 설정
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

	// 초기화 (첫 프레임)
	if (!m_bPrevLookInit) {
		m_vPrevLookTarget = vTargetLookPos;
		m_bPrevLookInit = true;
	}

	// 보간
	float alpha = 1.f - expf(-m_fLookLerpSpeed * fTimeDelta);
	m_vPrevLookTarget = XMVectorLerp(m_vPrevLookTarget, vTargetLookPos, alpha);

	m_pTransformCom->LookAt(m_vPrevLookTarget);

	if (fabs(m_fYaw - m_fTargetYaw) < 0.001f && fabs(m_fPitch - m_fTargetPitch) < 0.001f)
	{
		m_bSetPitchYaw = false;
	}
}

void CCamera_Orbital::Update_LockOnCameraLook(_float fTimeDelta)
{
	_vector vTargetLookPos = XMLoadFloat4(&static_cast<CUnit*>(m_pLockOnTarget)->Get_LockonPos());

	// 보간량 (지수 감쇠)
	float alpha = 1.f - expf(-m_fLookLerpSpeed * fTimeDelta);

	// 보간된 목표 계산
	m_vPrevLookTarget = XMVectorLerp(m_vPrevLookTarget, vTargetLookPos, alpha);

	// 보간된 위치를 바라보도록 회전
	m_pTransformCom->LookAt(m_vPrevLookTarget);


	_vector vCamLook = m_pTransformCom->Get_State(STATE::LOOK) * -1;

	const _float bx = XMVectorGetX(vCamLook);
	const _float by = XMVectorGetY(vCamLook);
	const _float bz = XMVectorGetZ(vCamLook);

	// Pitch Yaw 역계산
	m_fYaw = atan2f(bx, bz);
	m_fPitch = atan2f(by, sqrtf(bx * bx + bz * bz));

	// 거리가 가까워 질 수록 카메라 조금 위로
	_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
	_float dist = XMVectorGetX(XMVector3Length(vTargetLookPos - vPlayerPos));
	const _float maxDist = 10.f; // 원하는 거리값으로 조정
	_float t = 1.f - clamp(dist / maxDist, 0.f, 1.f); // 0~1
	_float pitchOffset = XMConvertToRadians(10.f * t);

	m_fPitch += pitchOffset;
	m_fPitch = clamp(m_fPitch, XMConvertToRadians(-20.f), XMConvertToRadians(30.f));

	Update_CameraPos(fTimeDelta);
}

void CCamera_Orbital::Update_CameraPos(_float fTimeDelta)
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pPlayer);

	// 기준점 위치 계산 (플레이어 + 높이 + 조금 뒤에)
	if (!m_bTalkActive)
	{
		m_vPlayerPosition = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
		m_vPlayerPosition += XMVectorSet(0.f, 1.7f, 0.f, 0.f);
		m_vPlayerPosition += XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;
	}

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
	const _float kPos = 12.f; // 응답 속도(높을수록 빠름)
	const _float a = 1.f - expf(-kPos * fTimeDelta);         

	_vector vCurPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vNewPos = XMVectorLerp(vCurPos, m_vTargetCamPos, a);

	// 아주 가까워지면 스냅
	if (XMVectorGetX(XMVector3LengthSq(vNewPos - m_vTargetCamPos)) < 1e-6f)
		vNewPos = m_vTargetCamPos;

	// 카메라 설정
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


	if (m_bLerpDistanceStart)
	{
		m_fDistance = LerpFloat(m_fDistance, m_fTargetLerpDistance, fTimeDelta * m_fDistanceLerpSpeed);

		if (fabsf(m_fDistance - m_fTargetLerpDistance) < 0.01f)
		{
			m_fDistance = m_fTargetLerpDistance;
			m_bLerpDistanceStart = false;
			m_bLerpDistanceEnd = true;
		}
	}

	if (m_bLerpDistanceEnd)
	{
		m_fDistance = LerpFloat(m_fDistance, m_fSaveDistance, fTimeDelta * m_fDistanceLerpSpeed);

		if (fabsf(m_fDistance - m_fSaveDistance) < 0.01f)
		{
			m_fDistance = m_fSaveDistance;
			m_bLerpDistanceEnd = false;
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
