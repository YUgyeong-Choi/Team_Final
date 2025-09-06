#include "Lamp.h"
#include "GameInstance.h"

CLamp::CLamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CItem{pDevice, pContext}
{
}

CLamp::CLamp(const CLamp& Prototype)
    :CItem{Prototype}
{
}

HRESULT CLamp::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CLamp::Initialize(void* pArg)
{
    if(FAILED(__super::Initialize(pArg)))
        return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_Lamp");
	

	m_isRender = true;

    if(FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_Light()))
		return E_FAIL;



	



	m_isActive = false;

    return S_OK;
}

void CLamp::Priority_Update(_float fTimeDelta)
{

	
	
}

void CLamp::Update(_float fTimeDelta)
{
   
}

void CLamp::Late_Update(_float fTimeDelta)
{
 
	
}

HRESULT CLamp::Render()
{
	return S_OK;
}


void CLamp::Use()
{
	m_isActive = !m_isActive;



	
}

ITEM_DESC CLamp::Get_ItemDesc()
{
	ITEM_DESC eDesc = {};

	if (m_isActive)
		eDesc.iItemIndex = 1;
	else
		eDesc.iItemIndex = 0;
	eDesc.strPrototag = m_strProtoTag;
	eDesc.isUsable = true;
	eDesc.isConsumable = false;


	return eDesc;
}

HRESULT CLamp::Ready_Components()
{

	return S_OK;

}

HRESULT CLamp::Ready_Light()
{
	

	return S_OK;
}

HRESULT CLamp::Bind_ShaderResources()
{


	return S_OK;
}

CLamp* CLamp::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLamp* pInstance = new CLamp(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLamp");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CLamp::Clone(void* pArg)
{
	CLamp* pInstance = new CLamp(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLamp");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CLamp::Free()
{
	__super::Free();


}
