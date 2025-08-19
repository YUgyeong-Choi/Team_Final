#include "Player_Arm_Base.h"

CPlayer_Arm_Base::CPlayer_Arm_Base(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{pDevice, pContext}
{
}

CPlayer_Arm_Base::CPlayer_Arm_Base(const CPlayer_Arm_Base& Prototype)
	:CGameObject{Prototype}
{
}

HRESULT CPlayer_Arm_Base::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayer_Arm_Base::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	ARM_DESC* eDesc = static_cast<ARM_DESC*>(pArg);

	m_pSocketMatrix = eDesc->pSocketMatrix;
	m_pParentWorldMatrix = eDesc->pParentWorldMatrix;

	return S_OK;
}

void CPlayer_Arm_Base::Priority_Update(_float fTimeDelta)
{
}

void CPlayer_Arm_Base::Update(_float fTimeDelta)
{
}

void CPlayer_Arm_Base::Late_Update(_float fTimeDelta)
{
	_matrix		SocketMatrix = XMLoadFloat4x4(m_pSocketMatrix);

	for (size_t i = 0; i < 3; i++)
		SocketMatrix.r[i] = XMVector3Normalize(SocketMatrix.r[i]);

	/* 무기 월드 1.f , 소켓 월드 , 부모 월드 0.02f */
	XMStoreFloat4x4(&m_CombinedWorldMatrix,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()) *
		SocketMatrix *
		XMLoadFloat4x4(m_pParentWorldMatrix));
}

HRESULT CPlayer_Arm_Base::Render()
{
	return S_OK;
}

CGameObject* CPlayer_Arm_Base::Clone(void* pArg)
{
	return nullptr;
}

void CPlayer_Arm_Base::Free()
{
}
