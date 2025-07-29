#include "PBRMesh.h"
#include "GameInstance.h"

CPBRMesh::CPBRMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CPBRMesh::CPBRMesh(const CPBRMesh& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CPBRMesh::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CPBRMesh::Initialize(void* pArg)
{
	CPBRMesh::STATICMESH_DESC* StaicMeshDESC = static_cast<STATICMESH_DESC*>(pArg);

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

	m_pTransformCom->Set_State(STATE::POSITION, 
		XMVectorSet(
			StaicMeshDESC->InitPos.x,
			StaicMeshDESC->InitPos.y,
			StaicMeshDESC->InitPos.z,
			1.f));

	m_pTransformCom->SetUp_Scale(
		StaicMeshDESC->InitScale.x,
		StaicMeshDESC->InitScale.y,
		StaicMeshDESC->InitScale.z
	);

	return S_OK;
}

void CPBRMesh::Priority_Update(_float fTimeDelta)
{

}

void CPBRMesh::Update(_float fTimeDelta)
{
}

void CPBRMesh::Late_Update(_float fTimeDelta)
{
	_vector	vTemp = m_pTransformCom->Get_State(STATE::POSITION);
	CGameObject::Compute_ViewZ(&vTemp);

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_SHADOW, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CPBRMesh::Render()
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
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
		{
			if (!m_bDoOnce)
			{
				/* Com_Texture */
				if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Texture_DefaultARM")),
					TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
					return E_FAIL;

				if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_ARMTexture", 0)))
					return E_FAIL;
				m_bDoOnce = true;
			}
		}

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CPBRMesh::Render_Shadow()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	SetCascadeShadow();
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Light_ViewMatrix(m_eShadow))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Light_ProjMatrix(m_eShadow))))
		return E_FAIL;

	_int iCascadeCount = ENUM_CLASS(m_eShadow);
	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		switch (iCascadeCount)
		{
		case 0: m_pShaderCom->Begin(2); break;
		case 1: m_pShaderCom->Begin(3); break;
		case 2: m_pShaderCom->Begin(4); break;
		}

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CPBRMesh::SetCascadeShadow()
{
	if (m_fViewZ < 20.f)
		m_eShadow = SHADOW::SHADOWA;
	else if (m_fViewZ < 40.f)
		m_eShadow = SHADOW::SHADOWB;
	else
		m_eShadow = SHADOW::SHADOWC;
}

HRESULT CPBRMesh::Ready_Components(void* pArg)
{
	CPBRMesh::STATICMESH_DESC* StaicMeshDESC = static_cast<STATICMESH_DESC*>(pArg);



	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(m_eLevelID), _wstring(TEXT("Prototype_Component_Model_")) + m_szMeshID,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CPBRMesh::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

CPBRMesh* CPBRMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPBRMesh* pGameInstance = new CPBRMesh(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CPBRMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CPBRMesh::Clone(void* pArg)
{
	CPBRMesh* pGameInstance = new CPBRMesh(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CPBRMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CPBRMesh::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pTextureCom);
}
