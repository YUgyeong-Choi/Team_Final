#include "TriggerItem.h"

#include "GameInstance.h"
CTriggerItem::CTriggerItem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CTriggerItem::CTriggerItem(const CTriggerItem& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CTriggerItem::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTriggerItem::Initialize(void* pArg)
{
	CTriggerItem::TRIGGERITEM_DESC* pDesc = static_cast<TRIGGERITEM_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg))) {
		return E_FAIL;
	}

	m_pTransformCom->Set_WorldMatrix(pDesc->triggerWorldMatrix);
	_vector offSetPos = m_pTransformCom->Get_State(STATE::POSITION) + pDesc->vOffSetObj;
	m_pTransformCom->Set_State(STATE::POSITION, offSetPos);
	m_pTransformCom->SetUp_Scale(XMVectorGetX(pDesc->vScaleObj),XMVectorGetY(pDesc->vScaleObj),XMVectorGetZ(pDesc->vScaleObj));

	return S_OK;
}

void CTriggerItem::Priority_Update(_float fTimeDelta)
{
}

void CTriggerItem::Update(_float fTimeDelta)
{
}

void CTriggerItem::Late_Update(_float fTimeDelta)
{
}

HRESULT CTriggerItem::Render()
{
	return S_OK;
}


CTriggerItem* CTriggerItem::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTriggerItem* pInstance = new CTriggerItem(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTriggerItem");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTriggerItem::Clone(void* pArg)
{
	CTriggerItem* pInstance = new CTriggerItem(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTriggerItem");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTriggerItem::Free()
{
	__super::Free();
}
