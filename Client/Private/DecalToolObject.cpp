#include "DecalToolObject.h"

CDecalToolObject::CDecalToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
{
}

CDecalToolObject::CDecalToolObject(const CDecalToolObject& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CDecalToolObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDecalToolObject::Initialize(void* pArg)
{
	return S_OK;
}

void CDecalToolObject::Priority_Update(_float fTimeDelta)
{
}

void CDecalToolObject::Update(_float fTimeDelta)
{
}

void CDecalToolObject::Late_Update(_float fTimeDelta)
{
}

HRESULT CDecalToolObject::Render()
{
	return S_OK;
}

HRESULT CDecalToolObject::Spawn_DecalToolObject()
{
	return S_OK;
}

CDecalToolObject* CDecalToolObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CDecalToolObject* pInstance = new CDecalToolObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CDecalToolObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDecalToolObject::Clone(void* pArg)
{
	CDecalToolObject* pInstance = new CDecalToolObject(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDecalToolObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CDecalToolObject::Free()
{
	__super::Free();

}
