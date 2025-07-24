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
	if (m_bShake)
		Update_Camera_Shake(fTimeDelta);

	// Transform 컴포넌트에서 위치와 방향 정보 가져옴
	_vector vEye = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
	_vector vAt = vEye + vLook;
	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	// 뷰 행렬 갱신
	m_ViewMatrix = XMMatrixLookAtLH(vEye, vAt, vUp);

	// 투영 행렬 갱신
	m_ProjMatrix = XMMatrixPerspectiveFovLH(m_fFov, m_fAspect, m_fNear, m_fFar);
}

void CCamera::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_K))
		StartShake(0.4f, 0.4f);
}

void CCamera::Late_Update(_float fTimeDelta)
{
}


HRESULT CCamera::Render()
{
	return S_OK;
}

HRESULT CCamera::Update_Camera()
{
	m_vPureCamPos = m_pTransformCom->Get_State(STATE::POSITION);

	m_pTransformCom->Move(m_vCurrentShakePos);
	m_pTransformCom->Quaternion_Turn(m_vCurrentShakeRot);
	
	m_pGameInstance->Set_Transform(D3DTS::VIEW, m_pTransformCom->Get_WorldMatrix_Inverse());
	m_pGameInstance->Set_Transform(D3DTS::PROJ, XMMatrixPerspectiveFovLH(m_fFov, m_fAspect, m_fNear, m_fFar));

	m_pTransformCom->Move(-m_vCurrentShakePos);
	m_pTransformCom->Quaternion_Turn(-m_vCurrentShakeRot);


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

void CCamera::StartShake(_float fIntensity, _float fDuration, _float fShakeFreqPos, _float fShakeFreqRot)
{
	m_fShakeIntensity = fIntensity;
	m_fShakeDuration = fDuration;
	m_fShakeFreqPos = fShakeFreqPos;
	m_fShakeFreqRot = fShakeFreqRot;

	m_fShakeTime = 0.f;
	m_bShake = TRUE;
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
		return;
	}

	// 2. 감쇠 적용
	_float decay = 1.f - (m_fShakeTime / m_fShakeDuration);
	_float shakeStrength = m_fShakeIntensity * decay;

	_float t = m_fShakeTime;

	// 3. 부드러운 위치 흔들림 (sin/cos 기반)
	_vector offsetPos = {
		sin(t * m_fShakeFreqPos) * shakeStrength * 0.5f,
		cos(t * m_fShakeFreqPos * 0.8f) * shakeStrength * 0.4f,
		sin(t * m_fShakeFreqPos * 1.2f) * shakeStrength * 0.3f
	};

	// 4. 부드러운 회전 흔들림 (YawPitchRoll 순서 기준)
	_vector offsetRot = {
		cos(t * m_fShakeFreqRot * 1.5f) * shakeStrength * 1.5f, // Pitch (X)
		sin(t * m_fShakeFreqRot) * shakeStrength * 2.0f, // Yaw   (Y)
		cos(t * m_fShakeFreqRot * 0.7f) * shakeStrength * 0.8f  // Roll  (Z)
	};

	// 5. 적용
	m_vCurrentShakePos = offsetPos;
	m_vCurrentShakeRot = offsetRot;
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
