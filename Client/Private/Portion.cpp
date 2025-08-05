#include "Portion.h"
#include "GameInstance.h"

CPortion::CPortion(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CItem{pDevice, pContext}
{
}

CPortion::CPortion(const CPortion& Prototype)
	:CItem{Prototype}
{
}

HRESULT CPortion::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPortion::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_Portion");


	m_isRender = true;
	m_isConsumable = true;
	m_iUseCount = m_iMaxCount;



	Ready_Components();



	return S_OK;
}

void CPortion::Priority_Update(_float fTimeDelta)
{
}

void CPortion::Update(_float fTimeDelta)
{
}

void CPortion::Late_Update(_float fTimeDelta)
{
}

HRESULT CPortion::Render()
{
	return S_OK;
}

void CPortion::Activate()
{
	if(m_iUseCount > 0)
		--m_iUseCount;
}

ITEM_DESC CPortion::Get_ItemDesc()
{
	ITEM_DESC eDesc = {};

	eDesc.isConsumable = true;
	eDesc.iUseCount = m_iUseCount;
	eDesc.strPrototag = m_strProtoTag;
	if (m_iUseCount > 0)
		eDesc.isUsable = true;
	else
		eDesc.isUsable = false;

	_float fRatio = _float(m_iUseCount) / m_iMaxCount;
	
	if (fRatio >= 1.f)
		eDesc.iItemIndex = 0;
	else if (fRatio >= 0.5f)
		eDesc.iItemIndex = 1;
	else if (fRatio > 0.f)
		eDesc.iItemIndex = 2;
	else
		eDesc.iItemIndex = 3;



	return eDesc;
}

HRESULT CPortion::Ready_Components()
{
	return S_OK;
}


CPortion* CPortion::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPortion* pInstance = new CPortion(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPortion");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CPortion::Clone(void* pArg)
{
	CPortion* pInstance = new CPortion(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPortion");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPortion::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

}


