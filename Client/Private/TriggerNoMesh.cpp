#include "TriggerNoMesh.h"
#include "GameInstance.h"

CTriggerNoMesh::CTriggerNoMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CTriggerBox{ pDevice, pContext }
{

}

CTriggerNoMesh::CTriggerNoMesh(const CTriggerNoMesh& Prototype)
	: CTriggerBox(Prototype)
{

}

HRESULT CTriggerNoMesh::Initialize_Prototype()
{
	/* �ܺ� �����ͺ��̽��� ���ؼ� ���� ä���. */

	return S_OK;
}

HRESULT CTriggerNoMesh::Initialize(void* pArg)
{
	CTriggerNoMesh::STATICTRIGGERNOMESH_DESC* TriggerNoMeshDESC = static_cast<STATICTRIGGERNOMESH_DESC*>(pArg);


	if (FAILED(__super::Initialize(TriggerNoMeshDESC)))
		return E_FAIL;


	return S_OK;
}

void CTriggerNoMesh::Priority_Update(_float fTimeDelta)
{

}

void CTriggerNoMesh::Update(_float fTimeDelta)
{
}

void CTriggerNoMesh::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTriggerNoMesh::Render()
{
	__super::Render();
	return S_OK;
}



CTriggerNoMesh* CTriggerNoMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTriggerNoMesh* pGameInstance = new CTriggerNoMesh(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTriggerNoMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CTriggerNoMesh::Clone(void* pArg)
{
	CTriggerNoMesh* pGameInstance = new CTriggerNoMesh(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTriggerNoMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CTriggerNoMesh::Free()
{
	__super::Free();
}
