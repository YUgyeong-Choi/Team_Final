#include "Camera_Orbital.h"
#include "GameInstance.h"

#include "PhysX_IgnoreSelfCallback.h"
#include "Camera_Manager.h"
#include "Player.h"

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
	m_fMouseSensor = 0.3f;

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

	if (m_pPlayer)
	{
		// 입력 처리
		_long MouseMoveX = m_pGameInstance->Get_DIMouseMove(DIMM::X);
		_long MouseMoveY = m_pGameInstance->Get_DIMouseMove(DIMM::Y);
		_long iWheelDelta = m_pGameInstance->Get_DIMouseMove(DIMM::WHEEL);

		// 줌 거리 조정
		m_fDistance -= iWheelDelta * fTimeDelta * m_fZoomSpeed;
		m_fDistance = clamp(m_fDistance, 2.f, 5.5f);

		// 회전값 누적
		m_fYaw += MouseMoveX * fTimeDelta * m_fMouseSensor;
		m_fPitch += MouseMoveY * fTimeDelta * m_fMouseSensor;

		// 기준점 위치 계산 (플레이어 + 높이)
		m_vPlayerPosition = static_cast<CTransform*>(m_pPlayer->Get_TransfomCom())->Get_State(STATE::POSITION);
		m_vPlayerPosition += XMVectorSet(0.f, 1.5f, 0.f, 0.f);

		// 오비탈 카메라 방향 계산 (spherical to cartesian)
		_float x = m_fDistance * cosf(m_fPitch) * sinf(m_fYaw);
		_float y = m_fDistance * sinf(m_fPitch);
		_float z = m_fDistance * cosf(m_fPitch) * cosf(m_fYaw);
		_vector vOffset = XMVectorSet(x, y, z, 0.f);

		//printf("m_fPitch: %f, m_fYaw: %f\n", m_fPitch, m_fYaw, z);
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

		// 무시할 자기 자신 액터 설정
		PxRigidActor* actor = nullptr;
		if (CPlayer* obj = dynamic_cast<CPlayer*>(m_pPlayer))
			actor = obj->Get_Actor();
		CIgnoreSelfCallback callback(actor);

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

		// 현재 카메라 위치
		_vector vCurrentPos = m_pTransformCom->Get_State(STATE::POSITION);

		// 위치 보간 (LERP)
		_float fInterpSpeed = 8.0f;
		_vector vInterpolatedPos = XMVectorLerp(vCurrentPos, m_vTargetCamPos, fTimeDelta * fInterpSpeed);

		// 카메라 설정
		m_pTransformCom->Set_State(STATE::POSITION, vInterpolatedPos);
		m_pTransformCom->LookAt(m_vPlayerPosition);
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

void CCamera_Orbital::Set_PitchYaw(_float pitch, _float yaw)
{
	m_fPitch = pitch;
	m_fYaw = yaw;
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
