#include "AStar.h"
#include "GameInstance.h"
#include "GameObject.h"

CAStar::CAStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)	
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
}

HRESULT CAStar::Initialize_Prototype()
{
	return E_NOTIMPL;
}

HRESULT CAStar::Initialize(void* pArg)
{
	return E_NOTIMPL;
}

HRESULT CAStar::Update()
{
	return E_NOTIMPL;
}

HRESULT CAStar::Render()
{
	return E_NOTIMPL;
}

_bool CAStar::Find_Route()
{
	return _bool();
}

void CAStar::Adjust_Route()
{
}

void CAStar::Add_Requester(CGameObject*)
{
}

CAStar* CAStar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAStar* pInstance = new CAStar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAStar");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAStar::Free()
{
	__super::Free();
}
