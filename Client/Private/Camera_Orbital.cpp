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

	if (!m_bActive)
		return;

	if (m_pPlayer)
	{
		CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pPlayer);

		// �Է� ó��
		_long MouseMoveX = m_pGameInstance->Get_DIMouseMove(DIMM::X);
		_long MouseMoveY = m_pGameInstance->Get_DIMouseMove(DIMM::Y);
		_long iWheelDelta = m_pGameInstance->Get_DIMouseMove(DIMM::WHEEL);

		// �� �Ÿ� ����
		m_fDistance -= iWheelDelta * fTimeDelta * m_fZoomSpeed;
		m_fDistance = clamp(m_fDistance, 2.f, 5.5f);

		// ȸ���� ����
		m_fYaw += MouseMoveX * fTimeDelta * m_fMouseSensor;
		m_fPitch += MouseMoveY * fTimeDelta * m_fMouseSensor;
		if (pPlayer->Get_PlayerState() != EPlayerState::IDLE)
			m_fPitch = clamp(m_fPitch, XMConvertToRadians(-30.f), XMConvertToRadians(60.f));
		else
			m_fPitch = clamp(m_fPitch, XMConvertToRadians(-50.f), XMConvertToRadians(60.f));

		// ������ ��ġ ��� (�÷��̾� + ����)
		m_vPlayerPosition = static_cast<CTransform*>(m_pPlayer->Get_TransfomCom())->Get_State(STATE::POSITION);
		m_vPlayerPosition += XMVectorSet(0.f, 1.7f, 0.f, 0.f);
		
		// �����̶� ����ϰ�
		m_vPlayerPosition += XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;

		// ����Ż ī�޶� ���� ��� (spherical to cartesian)
		_float x = m_fDistance * cosf(m_fPitch) * sinf(m_fYaw);
		_float y = m_fDistance * sinf(m_fPitch);
		_float z = m_fDistance * cosf(m_fPitch) * cosf(m_fYaw);
		_vector vOffset = XMVectorSet(x, y, z, 0.f);

		// �⺻ ��ǥ ī�޶� ��ġ
		m_vTargetCamPos = m_vPlayerPosition + vOffset;

		// --- �������� Raycast ó�� ���� ---
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

		// ������ �ڱ� �ڽ� ���� ����
		PxRigidActor* actor = pPlayer->Get_Actor(pPlayer->Get_Controller());
		CIgnoreSelfCallback callback(actor);

		if (m_pGameInstance->Get_Scene()->raycast(origin, direction, fTargetDist, hit, hitFlags, filterData, &callback))
		{
			if (hit.hasBlock)
			{
				// �������� ���� �Ÿ�
				_float fHitDist = hit.block.distance - 0.3f; // ���� �Ÿ�
				fHitDist = max(fHitDist, 0.5f);              // �ʹ� ��������� �ʰ� ����

				// ������ ī�޶� ��ġ
				m_vTargetCamPos = m_vPlayerPosition + vRayDir * fHitDist;
			}
		}
		// --- �������� Raycast ó�� �� ---
		
		// ���� ī�޶� ��ġ
		_vector vCurrentPos = m_pTransformCom->Get_State(STATE::POSITION);

		// ��ġ ���� (LERP)
		_float fInterpSpeed = 5.0f;
		_vector vInterpolatedPos = XMVectorLerp(vCurrentPos, m_vTargetCamPos, fTimeDelta * fInterpSpeed);

		// ī�޶� ����
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

void CCamera_Orbital::Set_InitCam()
{
	if (m_pPlayer)
	{
		m_pTransformCom->Set_WorldMatrix(Get_OrbitalWorldMatrix(0.093079f, -1.587195f));
		Set_PitchYaw(0.093079f, -1.587195f);
	}
}

void CCamera_Orbital::Set_PitchYaw(_float pitch, _float yaw)
{
	m_fPitch = pitch;
	m_fYaw = yaw;
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

	// ��� ����
	_matrix matWorld;
	matWorld.r[0] = vRight;                             // Right 
	matWorld.r[1] = vUp;                                // Up
	matWorld.r[2] = vLook;                              // Look
	matWorld.r[3] = XMVectorSetW(vCamPos, 1.f);         // Position

	return matWorld;
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
