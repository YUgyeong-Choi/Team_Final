#include "Camera.h"
#include "GameInstance.h"

CCamera::CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CCamera::CCamera(const CCamera& Prototype)
	: CGameObject( Prototype )
{
}

HRESULT CCamera::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CCamera::Initialize(void* pArg)
{
	CAMERA_DESC*		pDesc = static_cast<CAMERA_DESC*>(pArg);

	m_fFov = pDesc->fFov;
	m_fNear = pDesc->fNear;
	m_fFar = pDesc->fFar;

	D3D11_VIEWPORT			ViewportDesc{};
	_uint					iNumViewports = { 1 };
	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	m_fAspect = static_cast<_float>(ViewportDesc.Width) / ViewportDesc.Height;
	
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vEye), 1.f));
	m_pTransformCom->LookAt(XMVectorSetW(XMLoadFloat3(&pDesc->vAt), 1.f));

	return S_OK;
}

void CCamera::Priority_Update(_float fTimeDelta)
{
	if (m_bShake)    Update_Camera_Shake(fTimeDelta);
	if (m_bMoreRot)  Update_Camera_MoreRot(fTimeDelta);

	_vector vEye = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);   
	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);        

	// 1) look이 0이면 폴백: 이전 프레임 look 또는 기본 전방 (0,0,1)
	const _vector kEps = XMVectorReplicate(1e-8f);
	if (XMVector3LessOrEqual(XMVector3LengthSq(vLook), kEps)) {
		_vector vFallbackLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
		vLook = vFallbackLook;
	}

	// 2) look 정규화
	_vector vDir = XMVector3Normalize(vLook);

	// 3) up이 vDir과 거의 평행이면 대체 up 선택
	float dotLU = XMVectorGetX(XMVector3Dot(vDir, vUp));
	if (fabsf(dotLU) > 0.9990f) {
		_vector vAltUp = XMVectorSet(0.f, 0.f, 1.f, 0.f);
		if (fabsf(XMVectorGetX(XMVector3Dot(vDir, vAltUp))) > 0.9990f)
			vAltUp = XMVectorSet(1.f, 0.f, 0.f, 0.f);
		vUp = vAltUp;
	}

	_vector vAt = vEye + vDir;

	// 4) 최종 뷰/프로젝션
	XMMATRIX matViewLook = XMMatrixLookAtLH(vEye, vAt, vUp);
	XMStoreFloat4x4(&m_ViewMatrix, matViewLook);

	XMMATRIX matViewPerspective = XMMatrixPerspectiveFovLH(m_fFov, m_fAspect, m_fNear, m_fFar);
	XMStoreFloat4x4(&m_ProjMatrix, matViewPerspective);

}

void CCamera::Update(_float fTimeDelta)
{
}

void CCamera::Late_Update(_float fTimeDelta)
{
}


HRESULT CCamera::Render()
{
	return S_OK;
}

HRESULT CCamera::Render_DOF(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	if (!m_DOFDesc.bIsUse)
		return S_OK;

	// 카메라가 “값만” 바인딩한다. (MRT 시작/텍스처 바인딩은 렌더러가 함)
	if (FAILED(pShader->Bind_RawValue("g_fCloseIntensity", &m_DOFDesc.fCloseIntensity, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fFarIntensity", &m_DOFDesc.fFarIntensity, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fCleanRange", &m_DOFDesc.fCleanRange, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fFeatherPx", &m_DOFDesc.fFeatherPx, sizeof(_float))))
		return E_FAIL;

	// 패스 실행(렌더러가 Begin_MRT/텍스처 세팅을 끝내놓고 호출해줌)
	if (FAILED(pShader->Begin(ENUM_CLASS(DEFEREDPASS::DOF_ROUND))))
		return E_FAIL;

	pVIBuffer->Bind_Buffers();
	return pVIBuffer->Render();

	return S_OK;
}

HRESULT CCamera::Update_Camera()
{
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	XMStoreFloat4(&m_vPureCamPos, vPos);

	m_pTransformCom->Move(XMLoadFloat4(&m_vCurrentShakePos));
	m_pTransformCom->Quaternion_Turn(XMLoadFloat4(&m_vCurrentShakeRot));


	m_pGameInstance->Set_Transform(D3DTS::VIEW, m_pTransformCom->Get_WorldMatrix_Inverse());
	m_pGameInstance->Set_Transform(D3DTS::PROJ, XMMatrixPerspectiveFovLH(m_fFov, m_fAspect, m_fNear, m_fFar));
	
	//PrintMatrix("Real CameraWold", XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()));
	
	m_pTransformCom->Move(XMLoadFloat4(&m_vCurrentShakePos) * -1.f);
	m_pTransformCom->Quaternion_Turn(XMLoadFloat4(&m_vCurrentShakeRot) * -1.f);


	return S_OK;
}
_fvector CCamera::GetPosition()
{
	return m_pTransformCom->Get_State(STATE::POSITION);
}

_vector CCamera::GetRightVector()
{
	return m_pTransformCom->Get_State(STATE::RIGHT);
}

_vector CCamera::GetUpVector()
{
	return m_pTransformCom->Get_State(STATE::UP);;
}

_vector CCamera::GetLookVector()
{
	return m_pTransformCom->Get_State(STATE::LOOK);
}

void CCamera::StartShake(_float fIntensity, _float fDuration, _float fShakeFreqPos, _float fShakeFreqRot, _float fShakeUpdateInterval)
{
	m_fShakeIntensity = fIntensity;
	m_fShakeDuration = fDuration;
	m_fShakeFreqPos = fShakeFreqPos;
	m_fShakeFreqRot = fShakeFreqRot;

	m_fShakeUpdateInterval = fShakeUpdateInterval;
	m_fShakeUpdateAccum = 0.f;

	m_fShakeTime = 0.f;
	m_bShake = TRUE;
}

void CCamera::StartRot(_vector vRot, _float fDuration)
{
	m_fMoreRotDuration = fDuration;
	XMStoreFloat4(&m_vMoreRotFreq,vRot);

	m_fMoreRotTime = 0.f;
	m_bMoreRot = TRUE;
}

// 쉐이크 갱신 함수
void CCamera::Update_Camera_Shake(_float fTimedelta)
{
	m_fShakeTime += fTimedelta;

	if (m_fShakeTime >= m_fShakeDuration)
	{
		m_bShake = FALSE;
		m_vCurrentShakePos = { 0.f, 0.f, 0.f, 0.f };
		m_vCurrentShakeRot = { 0.f, 0.f, 0.f, 0.f };
		m_fShakeUpdateAccum = 0.f;
		return;
	}

	// 감쇠 적용
	_float decay = 1.f - (m_fShakeTime / m_fShakeDuration);
	_float shakeStrength = m_fShakeIntensity * decay;
	_float t = m_fShakeTime;

	// 누적 시간
	m_fShakeUpdateAccum += fTimedelta;

	// --- 일정 간격마다만 새 값 계산 ---
	if (m_fShakeUpdateAccum >= m_fShakeUpdateInterval)
	{
		// 3. 위치 흔들림
		_vector offsetPos = {
			sin(t * m_fShakeFreqPos) * shakeStrength * 0.5f,
			cos(t * m_fShakeFreqPos * 0.8f) * shakeStrength * 0.4f,
			sin(t * m_fShakeFreqPos * 1.2f) * shakeStrength * 0.3f
		};

		// 4. 회전 흔들림
		_vector offsetRot = {
			cos(t * m_fShakeFreqRot * 1.5f) * shakeStrength * 1.5f, // Pitch
			sin(t * m_fShakeFreqRot) * shakeStrength * 2.0f, // Yaw
			cos(t * m_fShakeFreqRot * 0.7f) * shakeStrength * 0.8f  // Roll
		};

		// 적용
		XMStoreFloat4(&m_vCurrentShakePos, offsetPos);
		XMStoreFloat4(&m_vCurrentShakeRot ,offsetRot);

		// 누적 시간 초기화
		m_fShakeUpdateAccum = 0.f;
	}
	else
	{
		// 간격 사이에는 새 계산 없이 기존 값 유지
		// (감쇠에 의한 스케일만 보정하고 싶으면 여기서 scale 적용 가능)
	}
}
void CCamera::Update_Camera_MoreRot(_float fTimedelta)
{
	m_fMoreRotTime += fTimedelta;

	if (m_fMoreRotTime >= m_fMoreRotDuration)
	{
		m_bMoreRot = FALSE;
		m_vCurrentShakeRot = { 0.f, 0.f, 0.f, 0.f };
		return;
	}

	float t = m_fMoreRotTime / m_fMoreRotDuration;
	float lerpFactor;
	if (t < 0.5f)
		lerpFactor = t / 0.5f;               // 0 → 1
	else
		lerpFactor = 1.f - (t - 0.5f) / 0.5f; // 1 → 0

	// 최종 회전 오프셋
	_vector moreRot = XMLoadFloat4(&m_vMoreRotFreq) * lerpFactor;

	if (m_bShake)
	{
		_vector vRot = moreRot - XMLoadFloat4(&m_vCurrentShakeRot);
		XMStoreFloat4(&m_vCurrentShakeRot, vRot);
	}
	else
	{
		XMStoreFloat4(&m_vCurrentShakeRot, moreRot);
	}
}

void CCamera::Bind_Matrices()
{
	m_pGameInstance->Set_Transform(D3DTS::VIEW, m_pTransformCom->Get_WorldMatrix_Inverse());
	m_pGameInstance->Set_Transform(D3DTS::PROJ, XMMatrixPerspectiveFovLH(m_fFov, m_fAspect, m_fNear, m_fFar));
}

void CCamera::Free()
{
	__super::Free();
}
