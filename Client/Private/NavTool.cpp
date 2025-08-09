#include "GameInstance.h"

#include "NavTool.h"

CNavTool::CNavTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CYWTool(pDevice, pContext)
{
}

CNavTool::CNavTool(const CNavTool& Prototype)
	:CYWTool(Prototype)
{
}

HRESULT CNavTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNavTool::Initialize(void* pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CNavTool::Priority_Update(_float fTimeDelta)
{
}

void CNavTool::Update(_float fTimeDelta)
{
	m_pNavigationCom->Update(XMMatrixIdentity());


	Control(fTimeDelta);
}

void CNavTool::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CNavTool::Render()
{
#ifdef _DEBUG
	m_pNavigationCom->Render();
#endif

	return S_OK;
}

HRESULT	CNavTool::Render_ImGui()
{
	ImGui::Begin("Nav Tool", nullptr);

	ImGui::End();

	return S_OK;
}

void CNavTool::Control(_float fTimeDelta)
{
	if (GetForegroundWindow() != g_hWnd)
		return;

	//Ctrl + S ¸Ê ÀúÀå
	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_S))
	{
		
	}

}

HRESULT CNavTool::Ready_Components()
{
	/* For.Com_Navigation */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom))))
		return E_FAIL;

	return S_OK;
}


CNavTool* CNavTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CNavTool* pInstance = new CNavTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CNavTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNavTool::Clone(void* pArg)
{
	CNavTool* pInstance = new CNavTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CNavTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CNavTool::Free()
{
	__super::Free();

	Safe_Release(m_pNavigationCom);

}
