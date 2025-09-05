#include "YWTool.h"

_int CYWTool::m_iID = -1;

CYWTool::CYWTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
{
}

CYWTool::CYWTool(const CYWTool& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CYWTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CYWTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CYWTool::Priority_Update(_float fTimeDelta)
{
}

void CYWTool::Update(_float fTimeDelta)
{
}

void CYWTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CYWTool::Render()
{
	return S_OK;
}

HRESULT	CYWTool::Render_ImGui()
{
	return S_OK;
}

HRESULT CYWTool::Save(const _char* Map)
{
	return S_OK;
}

HRESULT CYWTool::Load(const _char* Map)
{
	return S_OK;
}

void CYWTool::Free()
{
	__super::Free();

}
