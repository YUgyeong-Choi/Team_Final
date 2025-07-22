#include "ImGuiTool.h"
#include "GameInstance.h"

CImGuiTool::CImGuiTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CImGuiTool::CImGuiTool(const CImGuiTool& Prototype)
	: CGameObject( Prototype )
{

}

HRESULT CImGuiTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CImGuiTool::Initialize(void* pArg)
{
	m_pWindowData = static_cast<IMGUIWINDATA*>(pArg);
	return S_OK;
}

void CImGuiTool::Priority_Update(_float fTimeDelta)
{
}

void CImGuiTool::Update(_float fTimeDelta)
{

}

void CImGuiTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CImGuiTool::Render()
{
	return S_OK;
}

void CImGuiTool::Free()
{
	__super::Free();

}
