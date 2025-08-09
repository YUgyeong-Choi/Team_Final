#include "GameInstance.h"

#include "Nav.h"

CNav::CNav(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
{
}

CNav::CNav(const CNav& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CNav::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNav::Initialize(void* pArg)
{
	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	return S_OK;
}

void CNav::Priority_Update(_float fTimeDelta)
{
}

void CNav::Update(_float fTimeDelta)
{
	m_pNavigationCom->Update(XMMatrixIdentity());

}

void CNav::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONLIGHT, this);
}

HRESULT CNav::Render()
{
#ifdef _DEBUG
	m_pNavigationCom->Render();
#endif

	return S_OK;
}

HRESULT CNav::Ready_Components(void* pArg)
{
	NAV_DESC* Desc = static_cast<NAV_DESC*>(pArg);

	/* For.Com_Navigation */
	if (FAILED(__super::Add_Component(Desc->iLevelIndex, TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom))))
		return E_FAIL;

	return S_OK;
}


CNav* CNav::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CNav* pInstance = new CNav(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CNav");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNav::Clone(void* pArg)
{
	CNav* pInstance = new CNav(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CNav");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CNav::Free()
{
	__super::Free();

	Safe_Release(m_pNavigationCom);

}
