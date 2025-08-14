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

	// ��� ����
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

	// Pitch Yaw �����
	m_fTargetYaw = atan2f(bx, bz);
	m_fTargetPitch = atan2f(by, sqrtf(bx * bx + bz * bz));

	// ��¦ ������ ���̰� 
	m_fTargetPitch += XMConvertToRadians(10.f);

	m_fTargetLerpSpeed = fLerpSpeed;

	m_bSetPitchYaw = true;
}

void CCamera_Orbital::Update_CameraMatrix(_float fTimeDelta)
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pPlayer);

	_long MouseMoveX = m_pGameInstance->Get_DIMouseMove(DIMM::X);
	_long MouseMoveY = m_pGameInstance->Get_DIMouseMove(DIMM::Y);

	//  Yaw Pitch ����
	_float fAfterYaw = m_fYaw;
	_float fAfterPitch = m_fPitch;

	fAfterYaw += MouseMoveX * 0.008f * m_fMouseSensor;
	fAfterPitch += MouseMoveY * 0.008f * m_fMouseSensor;

	// ī�޶� Pitch ����
	if (pPlayer->Get_PlayerState() != EPlayerState::IDLE)
		fAfterPitch = clamp(fAfterPitch, XMConvertToRadians(-30.f), XMConvertToRadians(60.f));
	else
		fAfterPitch = clamp(fAfterPitch, XMConvertToRadians(-50.f), XMConvertToRadians(60.f));

	m_fYaw = LerpFloat(m_fYaw, fAfterYaw, m_fMouseSensor);
	m_fPitch = LerpFloat(m_fPitch, fAfterPitch, m_fMouseSensor);

	// ī�޶� ����
	Set_CameraMatrix(fTimeDelta);
}

void CCamera_Orbital::Update_TargetCameraMatrix(_float fTimeDelta)
{
	// Yaw �� Pitch ����
	m_fYaw = LerpFloat(m_fYaw, m_fTargetYaw, fTimeDelta * m_fTargetLerpSpeed);
	m_fPitch = LerpFloat(m_fPitch, m_fTargetPitch, fTimeDelta * m_fTargetLerpSpeed);

	// ī�޶� ����
	Set_CameraMatrix(fTimeDelta);

	if (fabs(m_fYaw - m_fTargetYaw) < 0.001f && fabs(m_fPitch - m_fTargetPitch) < 0.001f)
	{
		m_bSetPitchYaw = false;
	}
}

void CCamera_Orbital::Update_LockOnCameraMatrix(_float fTimeDelta)
{
	// �÷��̾�/Ÿ�� ��ġ
	_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.7f, 0.f, 0.f);
	_vector vTargetPos = XMLoadFloat4(&static_cast<CUnit*>(m_pLockOnTarget)->Get_LockonPos());
	_vector vCenter = (vPlayerPos + vTargetPos) * 0.5f;

	// ���� ���� ���� ��ġ�� �÷��̾� LOOK ���
	_vector vCamLook = XMVector3Normalize(vTargetPos - vPlayerPos);
	if (XMVector3Less(XMVector3LengthSq(vCamLook), XMVectorReplicate(1e-6f))) {
		vCamLook = XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK));
	}

	// ȭ�鿡 �� �� ��� ���� ������
	_float fSpan = XMVectorGetX(XMVector3Length(vTargetPos - vPlayerPos)); // �� ���� �Ÿ�
	_float fRadius = 0.5f * fSpan + m_fPadding;                
	_float dFov = (fRadius / m_fFrame) / tanf(m_fFov * 0.5f);
	_float fTargetDistance = clamp(dFov, m_fDistanceMin, m_fDistanceMax);

	// �÷��̾� & ī�޶� �ּ� ���� (ī�޶� �ʹ� ������ �����ʰ�)
	_vector vDesiredPre = vCenter - vCamLook * fTargetDistance;
	_float playerDepth = XMVectorGetX(XMVector3Dot(vPlayerPos - vDesiredPre, vCamLook));
	if (playerDepth < fMinPlayerDepth) {
		fTargetDistance += (fMinPlayerDepth - playerDepth);
		fTargetDistance = min(fTargetDistance, m_fDistanceMax); 
	}

	// ī�޶� �Ÿ� & ������ ī�޶� ��ġ
	m_fCurDistance = SmoothExp(m_fCurDistance, fTargetDistance, 8.0f, fTimeDelta);
	_vector vDesired = vCenter - vCamLook * m_fCurDistance;

#pragma region ��������
	_vector vDir = XMVector3Normalize(vDesired - vCenter);
	_float    fRayLength = XMVectorGetX(XMVector3Length(vDesired - vCenter));

	_float3 fC, fDir; XMStoreFloat3(&fC, vCenter); XMStoreFloat3(&fDir, vDir);
	PxVec3 origin(fC.x, fC.y, fC.z), direction(fDir.x, fDir.y, fDir.z);

	PxRaycastBuffer   hit;
	PxHitFlags        hitFlags = PxHitFlag::eDEFAULT;
	PxQueryFilterData filter;    filter.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	unordered_set<PxActor*> ignore = static_cast<CPlayer*>(m_pPlayer)->Get_Controller()->Get_IngoreActors();
	CIgnoreSelfCallback callbackFilter(ignore);

	XMVECTOR vCamPos = vDesired;
	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, fRayLength, hit, hitFlags, filter, &callbackFilter) && hit.hasBlock)
	{
		// ǥ�� ���� , �ʹ� ���� �ʰ�
		_float fHitDistance = max(hit.block.distance - 0.3f, 0.8f);
		vCamPos = vCenter + vDir * fHitDistance;
	}
#pragma endregion

	// �÷��̾� & ī�޶� �ĺ��� (�������� ��)
	_float fMinDepthNear = 1.2f;    // �ٰŸ� �⺻ ����
	_float fMinDepthFar = 3.0f;    // ���Ÿ� �� ���� ����
	_float fSpanNearLimit = 2.0f * m_fDistanceMin; // �� ���� �� �ٰŸ��� ���
	_float fSpanFarLimit = 2.0f * m_fDistanceMax; // �� �̻� �� ���Ÿ��� ���

	// �÷��̾� & ī�޶� �ּ� ����
	_float tSpan = 0.f;
	if (fSpanFarLimit > fSpanNearLimit)
		tSpan = clamp((fSpan - fSpanNearLimit) / (fSpanFarLimit - fSpanNearLimit), 0.f, 1.f);
	_float minDepth = LerpFloat(fMinDepthNear, fMinDepthFar, tSpan);

	// �ּұ��̺��� ���� ���̰� ���� �� ����
	_float fCurDepth = XMVectorGetX(XMVector3Dot(vPlayerPos - vCamPos, vCamLook));
	if (fCurDepth < minDepth) {
		_float needBack = minDepth - fCurDepth;
		// 2.0f ���� �ڷ� ���� ��(���ϰ� �ڷ� ������ �� ����)
		_float extra = min(needBack, 2.0f);
		vCamPos -= vCamLook * extra; 
	}

	// ��ġ ����
	_vector vCur = m_pTransformCom->Get_State(STATE::POSITION);
	_float aPos = 1.0f - expf(-10.0f * fTimeDelta); // ��ġ ������
	_vector vPos = XMVectorLerp(vCur, vCamPos, aPos);

	// ȸ�� ����
	_vector Fcur = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	_vector Fdes = XMVector3Normalize(vCenter - vPos);
	_float aAim = 1.0f - expf(-12.0f * fTimeDelta);
	_vector vLook = XMVector3Normalize(XMVectorLerp(Fcur, Fdes, aAim));

	// ���� ��� ����
	_vector UpW = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vRight = XMVector3Normalize(XMVector3Cross(UpW, vLook));
	_vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

	_matrix worldMatrix = XMMatrixIdentity();
	worldMatrix.r[0] = XMVectorSetW(vRight, 0.f);
	worldMatrix.r[1] = XMVectorSetW(vUp, 0.f);
	worldMatrix.r[2] = XMVectorSetW(vLook, 0.f);
	worldMatrix.r[3] = XMVectorSetW(vPos, 1.f);

	m_pTransformCom->Set_WorldMatrix(worldMatrix);
}

void CCamera_Orbital::Set_CameraMatrix(_float fTimeDelta)
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pPlayer);

	// ������ ��ġ ��� (�÷��̾� + ���� + ���� �ڿ�)
	m_vPlayerPosition = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
	m_vPlayerPosition += XMVectorSet(0.f, 1.7f, 0.f, 0.f);
	m_vPlayerPosition += XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;

	// ���� ���
	_float x = m_fDistance * cosf(m_fPitch) * sinf(m_fYaw);
	_float y = m_fDistance * sinf(m_fPitch);
	_float z = m_fDistance * cosf(m_fPitch) * cosf(m_fYaw);
	_vector vOffset = XMVectorSet(x, y, z, 0.f);

	// ��ǥ ī�޶� ��ġ
	m_vTargetCamPos = m_vPlayerPosition + vOffset;

	// --- �������� Raycast ó�� ���� ---
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
			_float fHitDist = hit.block.distance - 0.3f; // ���� �Ÿ�
			fHitDist = max(fHitDist, 0.5f);              // �ʹ� ��������� �ʰ� ����

			// ������ ī�޶� ��ġ
			m_vTargetCamPos = m_vPlayerPosition + vRayDir * fHitDist;
		}
	}
	// --- �������� Raycast ó�� �� ---

	// ī�޶� ����
	// ===== ��ġ�� ���� =====
	const float dt = fTimeDelta; // Ȥ�� ���ڷ� �ޱ�
	const float kPos = 12.f;                           // ���� �ӵ�(�������� ����)
	const float a = 1.f - expf(-kPos * dt);         // ������ ���� ���� ���

	_vector vCurPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vNewPos = XMVectorLerp(vCurPos, m_vTargetCamPos, a);

	// ���� ��������� ����
	if (XMVectorGetX(XMVector3LengthSq(vNewPos - m_vTargetCamPos)) < 1e-6f)
		vNewPos = m_vTargetCamPos;

	// ī�޶� ����
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
