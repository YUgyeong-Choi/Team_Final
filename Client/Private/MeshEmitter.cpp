#include "MeshEmitter.h"

#include "GameInstance.h"

CMeshEmitter::CMeshEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CParticleEffect{ pDevice, pContext }
{

}

CMeshEmitter::CMeshEmitter(const CMeshEmitter& Prototype)
	: CParticleEffect(Prototype)
{

}


HRESULT CMeshEmitter::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeshEmitter::Initialize(void* pArg)
{
	return S_OK;
}

void CMeshEmitter::Priority_Update(_float fTimeDelta)
{
}

void CMeshEmitter::Update(_float fTimeDelta)
{
}

void CMeshEmitter::Late_Update(_float fTimeDelta)
{
}

HRESULT CMeshEmitter::Render()
{
	return E_NOTIMPL;
}

_float CMeshEmitter::Ready_Death()
{
	return _float();
}

HRESULT CMeshEmitter::Ready_Components()
{
	return E_NOTIMPL;
}

HRESULT CMeshEmitter::Bind_ShaderResources()
{
	return E_NOTIMPL;
}

CMeshEmitter* CMeshEmitter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeshEmitter* pInstance = new CMeshEmitter(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMeshEmitter");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeshEmitter::Clone(void* pArg)
{
	CMeshEmitter* pInstance = new CMeshEmitter(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMeshEmitter");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeshEmitter::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}

json CMeshEmitter::Serialize()
{
	return json();
}

void CMeshEmitter::Deserialize(const json& j)
{
}
