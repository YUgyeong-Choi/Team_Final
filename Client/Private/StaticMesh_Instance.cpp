#include "StaticMesh_Instance.h"
#include "GameInstance.h"

CStaticMesh_Instance::CStaticMesh_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CStaticMesh{ pDevice, pContext }
{

}

CStaticMesh_Instance::CStaticMesh_Instance(const CStaticMesh_Instance& Prototype)
	: CStaticMesh(Prototype)
{

}

HRESULT CStaticMesh_Instance::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CStaticMesh_Instance::Initialize(void* pArg)
{
	//인스턴싱으로 이니셜라이즈

	return __super::Initialize(pArg);
}

void CStaticMesh_Instance::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CStaticMesh_Instance::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CStaticMesh_Instance::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CStaticMesh_Instance::Render()
{
	//인스턴싱으로 렌더

	return __super::Render();
}

HRESULT CStaticMesh_Instance::Ready_Components(void* pArg)
{
	return __super::Ready_Components(pArg);
}

HRESULT CStaticMesh_Instance::Bind_ShaderResources()
{
	return __super::Bind_ShaderResources();
}

CStaticMesh_Instance* CStaticMesh_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStaticMesh_Instance* pGameInstance = new CStaticMesh_Instance(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CStaticMesh_Instance");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CStaticMesh_Instance::Clone(void* pArg)
{
	CStaticMesh_Instance* pGameInstance = new CStaticMesh_Instance(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CStaticMesh_Instance");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CStaticMesh_Instance::Free()
{
	__super::Free();
}
