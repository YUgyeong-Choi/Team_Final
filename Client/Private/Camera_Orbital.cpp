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
	// 1) �÷��̾�/Ÿ�� ��ġ
	XMVECTOR vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.7f, 0.f, 0.f);
	XMVECTOR vTargetPos = m_pLockOnTarget->Get_TransfomCom()->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.3f, 0.f, 0.f);

	// 2) ����(0.5) + ����(F)
	XMVECTOR vMid = (vPlayerPos + vTargetPos) * 0.5f;

	XMVECTOR vF = XMVector3Normalize(vTargetPos - vPlayerPos);
	if (XMVector3Less(XMVector3LengthSq(vF), XMVectorReplicate(1e-6f))) {
		// ���� ���� ���� ��ġ�� �÷��̾� LOOK ���
		vF = XMVector3Normalize(m_pPlayer->Get_TransfomCom()->Get_State(STATE::LOOK));
	}

	// 3) �Ÿ� ��ǥ(dTarget)
	float span = XMVectorGetX(XMVector3Length(vTargetPos - vPlayerPos)); // �� ���� �Ÿ�
	const float kFrame = 0.85f;   // ȭ�� ����(�������� �� �ְ�)
	const float pad = 1.0f;     // ���� �е�
	const float dMin = 3.0f;     // �ּ�
	const float dMax = 6.0f;     // �ִ�(�⺻)

	float r = 0.5f * span + pad;                // ȭ�鿡 �� �� ��� ���� ������
	float dFov = (r / kFrame) / tanf(m_fFov * 0.5f);
	float dTarget = std::clamp(dFov, dMin, dMax);

	// 3-1) �÷��̾� �ּ� �̰�(����) 1�� ����: dTarget ���� ����
	{
		const float minPlayerDepth = 1.2f; // F�� �ּ� ����
		XMVECTOR vDesiredPre = vMid - vF * dTarget;
		float playerDepth = XMVectorGetX(XMVector3Dot(vPlayerPos - vDesiredPre, vF));
		if (playerDepth < minPlayerDepth) {
			dTarget += (minPlayerDepth - playerDepth);
			dTarget = std::min(dTarget, dMax); // �׷��� dMax�� ���� �ʰ�
		}
	}

	// 4) �Ÿ� ������(������ ���� ��������)
	static float dCurr = dMin; // ��������� ���� ��
	float aDist = 1.0f - expf(-8.0f * fTimeDelta); // ����ӵ� 8
	dCurr = dCurr + (dTarget - dCurr) * aDist;

	// 5) ��ǥ ī�޶� ��ġ = �������� -F�� dCurr
	XMVECTOR vDesired = vMid - vF * dCurr;

	// 6) ��������(�浹 ����)
	XMVECTOR vDir = XMVector3Normalize(vDesired - vMid);
	float    rayLen = XMVectorGetX(XMVector3Length(vDesired - vMid));

	XMFLOAT3 fC, fDir; XMStoreFloat3(&fC, vMid); XMStoreFloat3(&fDir, vDir);
	PxVec3 origin(fC.x, fC.y, fC.z), direction(fDir.x, fDir.y, fDir.z);

	PxRaycastBuffer   hit;
	PxHitFlags        hitFlags = PxHitFlag::eDEFAULT;
	PxQueryFilterData filter;    filter.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

	// �ڱ� �ڽ�/Ÿ�� ����
	unordered_set<PxActor*> ignore = static_cast<CPlayer*>(m_pPlayer)->Get_Controller()->Get_IngoreActors();
	CIgnoreSelfCallback cb(ignore);

	XMVECTOR vCamPos = vDesired;
	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, rayLen, hit, hitFlags, filter, &cb) && hit.hasBlock)
	{
		float pullPad = 0.3f;  // ǥ�� ����
		float minDist = 0.8f;  // �ʹ� ���� �ʱ�
		float hitDist = max(hit.block.distance - pullPad, minDist);
		vCamPos = vMid + vDir * hitDist;
	}

	// 6-1) ���浹 ���� ���� ���÷��̾� ���� ���̡� 2�� ���� (���Ⱑ �ٽ� �߰� ���)
	{
		// span�� Ŀ������ ������ ���� �� Ű���� ȭ�� �й� ��ȭ
		float minDepthNear = 1.2f;    // �ٰŸ� �⺻ ����
		float minDepthFar = 3.0f;    // ���Ÿ� �� ���� ����
		float spanFor0 = 2.0f * dMin; // �� ���� �� near ��
		float spanFor1 = 2.0f * dMax; // �� �̻� �� far ��

		float tSpan = 0.f;
		if (spanFor1 > spanFor0)
			tSpan = std::clamp((span - spanFor0) / (spanFor1 - spanFor0), 0.f, 1.f);

		float minDepth = minDepthNear + (minDepthFar - minDepthNear) * tSpan;

		// ���� ī�޶󿡼� �÷��̾���� F�� ����
		float currDepth = XMVectorGetX(XMVector3Dot(vPlayerPos - vCamPos, vF));
		if (currDepth < minDepth) {
			float needBack = minDepth - currDepth;
			float maxExtra = 2.0f; // dMax �ʰ� ��� �ѵ�(���ϰ� �ڷ� ������ �� ����)
			float extra = std::min(needBack, maxExtra);
			vCamPos -= vF * extra; // �ڷ�( -F ���� ) ��¦ �� ����
			// �ʿ��ϴٸ� ���⼭ �� �� �� ����ĳ��Ʈ�� ��Ȯ���ص� ��(�� ������ ����)
		}
	}

	// 7) ��ġ/�ü� ����
	XMVECTOR vCur = m_pTransformCom->Get_State(STATE::POSITION);
	float aPos = 1.0f - expf(-10.0f * fTimeDelta); // ��ġ ������
	XMVECTOR vPos = XMVectorLerp(vCur, vCamPos, aPos);

	// �ü��� ��������(���� ������)
	XMVECTOR Fcur = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	XMVECTOR Fdes = XMVector3Normalize(vMid - vPos);
	float aAim = 1.0f - expf(-12.0f * fTimeDelta);
	XMVECTOR F = XMVector3Normalize(XMVectorLerp(Fcur, Fdes, aAim));

	// 8) ���� ��� ����
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
