#include "Camera_Orbital.h"
#include "GameInstance.h"

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
	m_fMouseSensor = 0.5f;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CCamera_Orbital::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CCamera_Orbital::Update(_float fTimeDelta)
{
	if (!m_bRockOn)
	{
		_long MouseMoveX = m_pGameInstance->Get_DIMouseMove(DIMM::X);
		_long MouseMoveY = m_pGameInstance->Get_DIMouseMove(DIMM::Y);
		_long iWheelDelta = m_pGameInstance->Get_DIMouseMove(DIMM::WHEEL);

		m_fDistance -= iWheelDelta * fTimeDelta * m_fZoomSpeed;
		m_fDistance = clamp(m_fDistance, 2.f, 7.5f);

		// ���콺 �Է¿� ���� ȸ���� ����
		m_fYaw += MouseMoveX * fTimeDelta * m_fMouseSensor;
		m_fPitch += MouseMoveY * fTimeDelta * m_fMouseSensor;

		// �÷��̾� ��ġ ��������
		m_vPlayerPosition = XMVectorSet(0.f, 0.f, 0.f, 1.f); + XMVectorSet(0.f, 3.f, 0.f, 0.f);

		// ������ǥ�� �� ������ǥ�� ��ȯ
		_float x = m_fDistance * cosf(m_fPitch) * sinf(m_fYaw);
		_float y = m_fDistance * sinf(m_fPitch);
		_float z = m_fDistance * cosf(m_fPitch) * cosf(m_fYaw);

		// ī�޶� ��ġ ���
		m_vTargetCamPos = m_vPlayerPosition + XMVectorSet(x, y, z, 0.f);

		// ī�޶� ��ġ ����
		m_pTransformCom->Set_State(STATE::POSITION, m_vTargetCamPos);
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

void CCamera_Orbital::ActiveDialogView(_vector NPCPos, _vector NPCLook)
{
	/* [ ī�޶� ������Ʈ�� ���߰� �ش� ���������� �ٰ�����. ] */
	m_bActiveDialogView = true;

	m_vDialogPostion = m_pTransformCom->Get_State(STATE::POSITION);

	//1. NPC �� �躤�͸� �������� ������������ NPC �� �ٶ󺸴� ����
	_vector vNPCPos = XMVectorSetY(NPCPos, XMVectorGetY(NPCPos) + 2.5f);
	_vector vNPCLook = XMVector3Normalize(NPCLook);
	_vector vTargetPos = vNPCPos + vNPCLook * 3.f;

	_vector vFinalCamPos = LERP(m_vDialogPostion, vTargetPos, 0.1f);
	m_pTransformCom->Set_State(STATE::POSITION, vFinalCamPos);
	m_pTransformCom->LookAt(vNPCPos);
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
