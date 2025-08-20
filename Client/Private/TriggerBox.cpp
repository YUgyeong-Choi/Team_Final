#include "TriggerBox.h"
#include "GameInstance.h"

CTriggerBox::CTriggerBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CTriggerBox::CTriggerBox(const CTriggerBox& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CTriggerBox::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CTriggerBox::Initialize(void* pArg)
{
	CTriggerBox::TRIGGERBOX_DESC* TriggerBoxDESC = static_cast<TRIGGERBOX_DESC*>(pArg);

	if (FAILED(__super::Initialize(TriggerBoxDESC)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION, TriggerBoxDESC->vPos);
	m_pTransformCom->Rotation(XMConvertToRadians(TriggerBoxDESC->Rotation.x), XMConvertToRadians(TriggerBoxDESC->Rotation.y), XMConvertToRadians(TriggerBoxDESC->Rotation.z));

	if (FAILED(Ready_Trigger()))
		return E_FAIL;

	return S_OK;
}

void CTriggerBox::Priority_Update(_float fTimeDelta)
{
}

void CTriggerBox::Update(_float fTimeDelta)
{
}

void CTriggerBox::Late_Update(_float fTimeDelta)
{
	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXTriggerCom))
	{
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
	}
}

HRESULT CTriggerBox::Render()
{
#ifdef _DEBUG

	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXTriggerCom))
	{
		if (m_pGameInstance->Get_RenderCollider())
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXTriggerCom)))
				return E_FAIL;
		}
	}

#endif

	return S_OK;
}


HRESULT CTriggerBox::Ready_Components(void* pArg)
{
	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysXTrigger"), reinterpret_cast<CComponent**>(&m_pPhysXTriggerCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CTriggerBox::Ready_Trigger()
{
	return E_NOTIMPL;
}

CTriggerBox* CTriggerBox::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTriggerBox* pGameInstance = new CTriggerBox(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTriggerBox");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CTriggerBox::Clone(void* pArg)
{
	CTriggerBox* pGameInstance = new CTriggerBox(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTriggerBox");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CTriggerBox::Free()
{
	__super::Free();

	Safe_Release(m_pPhysXTriggerCom);
}
