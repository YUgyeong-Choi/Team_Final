#include "AStar_Manager.h"
#include "GameInstance.h"
#include "Navigation.h"


CAStar_Manager::CAStar_Manager()
	:m_pGameInstance{ CGameInstance::Get_Instance() }
{
}

HRESULT CAStar_Manager::Initialize(LEVEL eLevel)
{
	return E_NOTIMPL;
}

HRESULT CAStar_Manager::Priority_Update(_float fTimeDelta)
{
	return E_NOTIMPL;
}

HRESULT CAStar_Manager::Update(_float fTimeDelta)
{
	return E_NOTIMPL;
}

HRESULT CAStar_Manager::Late_Update(_float fTimeDelta)
{
	return E_NOTIMPL;
}

HRESULT CAStar_Manager::Render()
{
	return E_NOTIMPL;
}

void CAStar_Manager::Free()
{
	__super::Free();


}
