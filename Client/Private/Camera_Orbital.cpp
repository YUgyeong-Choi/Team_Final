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
	_long iWheelDelta = m_pGameInstance->Get_DIMouseMove(DIMM::WHEEL);

	// �� �Ÿ� ����
	m_fDistance -= iWheelDelta * fTimeDelta * m_fZoomSpeed;
	m_fDistance = clamp(m_fDistance, 2.f, 5.5f);

	//  Yaw Pitch ����
	_float fAfterYaw = m_fYaw;
	_float fAfterPitch = m_fPitch;

	fAfterYaw += MouseMoveX * fTimeDelta * m_fMouseSensor;
	fAfterPitch += MouseMoveY * fTimeDelta * m_fMouseSensor;

	// ī�޶� Pitch ����
	if (pPlayer->Get_PlayerState() != EPlayerState::IDLE)
		fAfterPitch = clamp(fAfterPitch, XMConvertToRadians(-30.f), XMConvertToRadians(60.f));
	else
		fAfterPitch = clamp(fAfterPitch, XMConvertToRadians(-50.f), XMConvertToRadians(60.f));

	m_fYaw = LerpFloat(m_fYaw, fAfterYaw, fTimeDelta * 5.f);
	m_fPitch = LerpFloat(m_fPitch, fAfterPitch, fTimeDelta * 5.f);

	// ī�޶� ����
	Set_CameraMatrix();
}

void CCamera_Orbital::Update_TargetCameraMatrix(_float fTimeDelta)
{
	// Yaw �� Pitch ����
	m_fYaw = LerpFloat(m_fYaw, m_fTargetYaw, fTimeDelta * m_fTargetLerpSpeed);
	m_fPitch = LerpFloat(m_fPitch, m_fTargetPitch, fTimeDelta * m_fTargetLerpSpeed);

	// ī�޶� ����
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

	// ����
	XMVECTOR vMid = (vPlayerPos + vTargetPos) * 0.5f;

	// �÷��̾� Ÿ�� ����
	XMVECTOR vF = XMVector3Normalize(vTargetPos - vPlayerPos);
	if (XMVector3Less(XMVector3LengthSq(vF), XMVectorReplicate(1e-6f))) {
		// ���� ���� ���� ��ġ�� �÷��̾��� LOOK�� ���
		vF = XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK));
	}

	// ===== �Ÿ� ��� =====
	float span = XMVectorGetX(XMVector3Length(vTargetPos - vPlayerPos)); // �� ���� �Ÿ�
	const float kFrame = 0.85f;   // ȭ�� ����
	const float pad = 1.0f;    // ���� �е�(���� �پ �̸�ŭ�� Ȯ��)
	const float dMin = 3.0f;    // ī�޶� �ּڰ�(�ʹ� ���̴��� �ʱ�)
	const float dMax = 8.0f;   // �ִ�

	// ���� FOV ��� �ʿ� �Ÿ� (������span/2 + pad)
	float r = 0.5f * span + pad;
	float dFov = (r / kFrame) / tanf(m_fFov * 0.5f);

	// ���� ��ǥ �Ÿ�: �ٴ�/�ִ� ����
	float dTarget = std::clamp(dFov, dMin, dMax);

	// �Ÿ� ������(������ ���� ��������)
	static float dCurr = dMin; // ����� ���� ��
	float aDist = 1.0f - expf(-8.0f * fTimeDelta); // �ӵ� ��� 8
	dCurr = dCurr + (dTarget - dCurr) * aDist;

	// ��ǥ ī�޶� ��ġ = �������� -F�� dCurr
	XMVECTOR vDesired = vMid - vF * dCurr;

	// ===== ��������(�浹 ����) =====
	XMVECTOR vDir = XMVector3Normalize(vDesired - vMid);
	float rayLen = XMVectorGetX(XMVector3Length(vDesired - vMid));

	XMFLOAT3 fC, fDir; XMStoreFloat3(&fC, vMid); XMStoreFloat3(&fDir, vDir);
	PxVec3 origin(fC.x, fC.y, fC.z), direction(fDir.x, fDir.y, fDir.z);

	PxRaycastBuffer hit;
	PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
	PxQueryFilterData filter; filter.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	// �ڱ� �ڽ�/Ÿ�� ����
	unordered_set<PxActor*> ignore;
	ignore = static_cast<CPlayer*>(m_pPlayer)->Get_Controller()->Get_IngoreActors();
	CIgnoreSelfCallback cb(ignore);

	XMVECTOR vCamPos = vDesired;
	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, rayLen, hit, hitFlags, filter, &cb) && hit.hasBlock)
	{
		float pullPad = 0.3f;  // ǥ�� ����
		float minDist = 0.8f;  // �ʹ� ���� �ʱ�
		float hitDist = max(hit.block.distance - pullPad, minDist);
		vCamPos = vMid + vDir * hitDist;
	}

	// ===== ��ġ/�ü� ���� =====
	XMVECTOR vCur = m_pTransformCom->Get_State(STATE::POSITION);
	float aPos = 1.0f - expf(-10.0f * fTimeDelta); // ��ġ ������
	XMVECTOR vPos = XMVectorLerp(vCur, vCamPos, aPos);

	// �ü��� ��������(���� ������)
	XMVECTOR Fcur = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	XMVECTOR Fdes = XMVector3Normalize(vMid - vPos);
	float aAim = 1.0f - expf(-12.0f * fTimeDelta);
	XMVECTOR F = XMVector3Normalize(XMVectorLerp(Fcur, Fdes, aAim));

	// ���� ��� ����
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
