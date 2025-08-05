#include "Item.h"

CItem::CItem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{pDevice, pContext}
{
}

CItem::CItem(const CItem& Prototype)
	:CGameObject{Prototype}
{
}

HRESULT CItem::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CItem::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	

	return S_OK;
}

void CItem::Priority_Update(_float fTimeDelta)
{
}

void CItem::Update(_float fTimeDelta)
{
}

void CItem::Late_Update(_float fTimeDelta)
{
}

HRESULT CItem::Render()
{
	return S_OK;
}


void CItem::Free()
{
	__super::Free();
}
