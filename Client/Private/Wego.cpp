#include "Wego.h"

#include "GameInstance.h"
CWego::CWego(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUnit(pDevice, pContext)
{
}

CWego::CWego(const CWego& Prototype)
	: CUnit(Prototype)
{
}

HRESULT CWego::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWego::Initialize(void* pArg)
{
	WEGO_DESC* pDesc = static_cast<WEGO_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	/* [ 초기화 위치값 ] */
	m_pTransformCom->Set_State(STATE::POSITION, _vector{ m_InitPos.x, m_InitPos.y, m_InitPos.z });
	//m_pTransformCom->Rotation(XMConvertToRadians(0.f), XMConvertToRadians(90.f), XMConvertToRadians(0.f));
	//m_pTransformCom->SetUp_Scale(pDesc->InitScale.x, pDesc->InitScale.y, pDesc->InitScale.z);
	m_pTransformCom->Scaling(_float3{ 0.4f,0.4f,0.4f });

	return S_OK;
}

void CWego::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CWego::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CWego::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CWego::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CWego::Ready_Components()
{
	return S_OK;
}

CWego* CWego::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWego* pInstance = new CWego(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWego");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CWego::Clone(void* pArg)
{
	CWego* pInstance = new CWego(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CWego");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CWego::Free()
{
	__super::Free();
}
