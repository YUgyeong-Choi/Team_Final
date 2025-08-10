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
		Update_CameraLockOnMatrix(fTimeDelta);
	}
	else
	{
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

void CCamera_Orbital::Set_PitchYaw(_float pitch, _float yaw)
{
	m_fPitch = pitch;
	m_fYaw = yaw;
}

void CCamera_Orbital::Set_LockOn(CGameObject* pTarget, _bool bActive)
{
	m_pLockOnTarget = pTarget;
	m_bLockOn = bActive;
}

_matrix CCamera_Orbital::Get_OrbitalWorldMatrix(_float pitch, _float yaw)
{
	if(!m_pPlayer)
		return _matrix();

	m_vPlayerPosition = static_cast<CTransform*>(m_pPlayer->Get_TransfomCom())->Get_State(STATE::POSITION);
	m_vPlayerPosition += XMVectorSet(0.f, 1.7f, 0.f, 0.f);
	m_vPlayerPosition += XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;

	_float x = m_fDistance * cosf(pitch) * sinf(yaw);
	_float y = m_fDistance * sinf(pitch);
	_float z = m_fDistance * cosf(pitch) * cosf(yaw);
	_vector vOffset = XMVectorSet(x, y, z, 0.f);

	_vector vCamPos = m_vPlayerPosition + vOffset;

	_vector vLook = XMVector3Normalize(m_vPlayerPosition - vCamPos);
	_vector vRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook));
	_vector vUp = XMVector3Cross(vLook, vRight);

	// 행렬 생성
	_matrix matWorld;
	matWorld.r[0] = vRight;                             // Right 
	matWorld.r[1] = vUp;                                // Up
	matWorld.r[2] = vLook;                              // Look
	matWorld.r[3] = XMVectorSetW(vCamPos, 1.f);         // Position

	return matWorld;
}

void CCamera_Orbital::Set_OrbitalPosBackLookFront()
{
	XMVECTOR cameraPos = XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -1;

	const _float bx = XMVectorGetX(cameraPos);
	const _float by = XMVectorGetY(cameraPos);
	const _float bz = XMVectorGetZ(cameraPos);

	m_fYaw = atan2f(bx, bz);                                   
	m_fPitch = atan2f(by, sqrtf(bx * bx + bz * bz));                 

	// 살짝 위에서 보이게 
	m_fPitch += XMConvertToRadians(10.f);
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

	_float afterYaw = m_fYaw;
	_float afterPitch = m_fPitch;

	// 회전값 누적
	afterYaw += MouseMoveX * fTimeDelta * m_fMouseSensor;
	afterPitch += MouseMoveY * fTimeDelta * m_fMouseSensor;

	// 카메라 Pitch 제한
	if (pPlayer->Get_PlayerState() != EPlayerState::IDLE)
		afterPitch = clamp(afterPitch, XMConvertToRadians(-30.f), XMConvertToRadians(60.f));
	else
		afterPitch = clamp(afterPitch, XMConvertToRadians(-50.f), XMConvertToRadians(60.f));

	m_fYaw = LerpFloat(m_fYaw, afterYaw, fTimeDelta * 5.f);
	m_fPitch = LerpFloat(m_fPitch, afterPitch, fTimeDelta * 5.f);

	// 기준점 위치 계산 (플레이어 + 높이)
	m_vPlayerPosition = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
	m_vPlayerPosition += XMVectorSet(0.f, 1.7f, 0.f, 0.f);

	// 플레이어가 보는 방향보다 조금 뒤에
	m_vPlayerPosition += XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;

	// 오비탈 카메라 방향 계산 (spherical to cartesian)
	_float x = m_fDistance * cosf(m_fPitch) * sinf(m_fYaw);
	_float y = m_fDistance * sinf(m_fPitch);
	_float z = m_fDistance * cosf(m_fPitch) * cosf(m_fYaw);
	_vector vOffset = XMVectorSet(x, y, z, 0.f);

	// 기본 목표 카메라 위치
	m_vTargetCamPos = m_vPlayerPosition + vOffset;

	// --- 스프링암 Raycast 처리 시작 ---
	_vector vRayDir = XMVector3Normalize(vOffset);
	_float fTargetDist = XMVectorGetX(XMVector3Length(vOffset));

	XMFLOAT3 fStart, fDir;
	XMStoreFloat3(&fStart, m_vPlayerPosition);
	XMStoreFloat3(&fDir, vRayDir);

	PxVec3 origin(fStart.x, fStart.y, fStart.z);
	PxVec3 direction(fDir.x, fDir.y, fDir.z);

	PxRaycastBuffer hit;
	PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
	PxQueryFilterData filterData;
	filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	PxRigidActor* actor = pPlayer->Get_Actor(pPlayer->Get_Controller());
	unordered_set<PxActor*> ignoreActors;
	ignoreActors.insert(actor);
	CIgnoreSelfCallback callback(ignoreActors);

	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, fTargetDist, hit, hitFlags, filterData, &callback))
	{
		if (hit.hasBlock)
		{
			// 스프링암 보정 거리
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

void CCamera_Orbital::Update_CameraLockOnMatrix(_float fTimeDelta)
{
	//_vector P = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.7f, 0.f, 0.f);
	//_vector T = m_pLockOnTarget->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.3f, 0.f, 0.f);

	// 플레이어와 타겟 위치
	XMVECTOR vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.7f, 0.f, 0.f);
	XMVECTOR vTargetPos = m_pLockOnTarget->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.3f, 0.f, 0.f);

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
