#include "TerrainBrush.h"

#include "GameInstance.h"

CTerrainBrush::CTerrainBrush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CTerrainBrush::CTerrainBrush(const CTerrainBrush& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CTerrainBrush::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTerrainBrush::Initialize(void* pArg)
{

	return S_OK;
}

void CTerrainBrush::Priority_Update(_float fTimeDelta)
{

}

EVENT CTerrainBrush::Update(_float fTimeDelta)
{

	return EVN_NONE;
}

void CTerrainBrush::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CTerrainBrush::Render()
{

	return S_OK;
}

CTerrainBrush* CTerrainBrush::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTerrainBrush* pInstance = new CTerrainBrush(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTerrainBrush::Clone(void* pArg)
{
	CTerrainBrush* pInstance = new CTerrainBrush(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTerrainBrush");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTerrainBrush::Free()
{
	__super::Free();

}
