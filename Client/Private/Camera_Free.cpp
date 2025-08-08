#include "Camera_Free.h"
#include "GameInstance.h"

CCamera_Free::CCamera_Free(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{

}

CCamera_Free::CCamera_Free(const CCamera_Free& Prototype)
	: CCamera(Prototype)
{

}

HRESULT CCamera_Free::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_Free::Initialize(void* pArg)
{
	/*CCamera::CAMERA_DESC			Desc{};*/

	/*Desc.vEye = _float3(0.f, 20.f, -15.f);
	Desc.vAt = _float3(0.f, 0.f, 0.f);
	Desc.fFov = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 1000.f;
	Desc.fRotationPerSec = XMConvertToRadians(180.0f);
	Desc.fSpeedPerSec = 10.0f;
	lstrcpy(Desc.szName, TEXT("Camera"));*/

	m_fSensor = 0.1f;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CCamera_Free::Priority_Update(_float fTimeDelta)
{
	
	// 우클릭 눌러야 카메라 움직일 수 있음
	if (m_bMoveable && m_pGameInstance->Mouse_Pressing(DIM::RBUTTON))
	{
		// 달리기 여부 체크
		m_bSprint = (m_pGameInstance->Key_Pressing(DIK_LSHIFT)) != 0;
		m_bSlow = (m_pGameInstance->Key_Pressing(DIK_LALT)) != 0;
		// 속도 설정
		if (m_bSprint)
			m_pTransformCom->Set_SpeedPreSec(100.f);
		else if (m_bSlow)
			m_pTransformCom->Set_SpeedPreSec(1.f);
		else
			m_pTransformCom->Set_SpeedPreSec(10.f);

		if (m_pGameInstance->Key_Pressing(DIK_A))
		{
			m_pTransformCom->Go_Left(fTimeDelta);
		}
		if (m_pGameInstance->Key_Pressing(DIK_D))
		{
			m_pTransformCom->Go_Right(fTimeDelta);
		}
		if (m_pGameInstance->Key_Pressing(DIK_W))
		{
			m_pTransformCom->Go_Front(fTimeDelta);
		}
		if (m_pGameInstance->Key_Pressing(DIK_S))
		{
			m_pTransformCom->Go_Backward(fTimeDelta);
		}

		_long			MouseMove = {};

		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMM::X))
		{
			m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), MouseMove *0.008f * m_fSensor);
		}

		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMM::Y))
		{
			m_pTransformCom->Turn(m_pTransformCom->Get_State(STATE::RIGHT), MouseMove * 0.008f * m_fSensor);
		}
	}



	//__super::Bind_Matrices();
	__super::Priority_Update(fTimeDelta);

}

void CCamera_Free::Update(_float fTimeDelta)
{
	if (KEY_DOWN(DIK_CAPSLOCK))
		PrintMatrix("World", XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()));

	if (m_pGameInstance->Mouse_Down(DIM::LBUTTON)) {
		_vector pos = m_pTransformCom->Get_State(STATE::POSITION);
	}

	__super::Update(fTimeDelta);
}

void CCamera_Free::Late_Update(_float fTimeDelta)
{

}

HRESULT CCamera_Free::Render()
{

	return S_OK;
}

CCamera_Free* CCamera_Free::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Free* pInstance = new CCamera_Free(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCamera_Free::Clone(void* pArg)
{
	CCamera_Free* pInstance = new CCamera_Free(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CCamera_Free");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Free::Free()
{
	__super::Free();


}