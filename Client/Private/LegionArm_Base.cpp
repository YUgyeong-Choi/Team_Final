#include "LegionArm_Base.h"

CLegionArm_Base::CLegionArm_Base(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CWeapon{pDevice, pContext}
{
}

CLegionArm_Base::CLegionArm_Base(const CLegionArm_Base& Prototype)
	:CWeapon{Prototype}
{
}

HRESULT CLegionArm_Base::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLegionArm_Base::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	/*ARM_DESC* eDesc = static_cast<ARM_DESC*>(pArg);

	m_pSocketMatrix = eDesc->pSocketMatrix;
	m_pParentWorldMatrix = eDesc->pParentWorldMatrix;*/

	return S_OK;
}

void CLegionArm_Base::Priority_Update(_float fTimeDelta)
{
}

void CLegionArm_Base::Update(_float fTimeDelta)
{
}

void CLegionArm_Base::Late_Update(_float fTimeDelta)
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

HRESULT CLegionArm_Base::Render()
{
	return S_OK;
}

void CLegionArm_Base::Free()
{
	__super::Free();
}
