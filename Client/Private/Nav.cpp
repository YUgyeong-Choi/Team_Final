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
	if(FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	return S_OK;
}

void CNav::Priority_Update(_float fTimeDelta)
{
	if(m_pGameInstance->Key_Down(DIK_N))
		m_bVisible = !m_bVisible;
}

void CNav::Update(_float fTimeDelta)
{
	m_pNavigationCom->Update(XMMatrixIdentity());

}

void CNav::Late_Update(_float fTimeDelta)
{
	if(m_bVisible)
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
	wstring wsPrototypeTag = TEXT("Prototype_Component_Navigation");

	if (FAILED(__super::Add_Component(Desc->iLevelIndex, wsPrototypeTag.c_str(),
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
