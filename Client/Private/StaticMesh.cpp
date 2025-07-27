#include "StaticMesh.h"
#include "GameInstance.h"

CStaticMesh::CStaticMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CStaticMesh::CStaticMesh(const CStaticMesh& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CStaticMesh::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CStaticMesh::Initialize(void* pArg)
{
	CStaticMesh::STATICMESH_DESC* StaicMeshDESC = static_cast<STATICMESH_DESC*>(pArg);

	m_eLevelID = StaicMeshDESC->m_eLevelID;

	m_szMeshID = StaicMeshDESC->szMeshID;

	m_iRender = StaicMeshDESC->iRender;

	StaicMeshDESC->fSpeedPerSec = 0.f;
	StaicMeshDESC->fRotationPerSec = 0.f;

	if (FAILED(__super::Initialize(StaicMeshDESC)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(StaicMeshDESC->WorldMatrix);

	//m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_InitPos.x, m_InitPos.y, m_InitPos.z, 1.f));
	//m_pTransformCom->SetUp_Scale(StaicMeshDESC->m_vInitScale.x, StaicMeshDESC->m_vInitScale.y, StaicMeshDESC->m_vInitScale.z);

	return S_OK;
}

void CStaticMesh::Priority_Update(_float fTimeDelta)
{

}

void CStaticMesh::Update(_float fTimeDelta)
{
}

void CStaticMesh::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CStaticMesh::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
			return E_FAIL;

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CStaticMesh::Ready_Components(void* pArg)
{
	CStaticMesh::STATICMESH_DESC* StaicMeshDESC = static_cast<STATICMESH_DESC*>(pArg);



	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(m_eLevelID), StaicMeshDESC->szModelPrototypeTag/*_wstring(TEXT("Prototype_Component_Model_")) + m_szMeshID*/,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CStaticMesh::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

CStaticMesh* CStaticMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStaticMesh* pGameInstance = new CStaticMesh(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CStaticMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CStaticMesh::Clone(void* pArg)
{
	CStaticMesh* pGameInstance = new CStaticMesh(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CStaticMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CStaticMesh::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pTextureCom);
}
