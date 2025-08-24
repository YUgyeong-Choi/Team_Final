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

	STATICMESHINSTANCE_DESC* InstanceDesc = static_cast<STATICMESHINSTANCE_DESC*>(pArg);

	m_iNumInstance = InstanceDesc->iNumInstance;

	m_szMeshID = InstanceDesc->szMeshID;

	m_iRender = InstanceDesc->iRender;

	m_iLightShape = InstanceDesc->iLightShape;

	m_wsMap = InstanceDesc->wsMap;

	InstanceDesc->fSpeedPerSec = 0.f;
	InstanceDesc->fRotationPerSec = 0.f;

	if (FAILED(CGameObject::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(InstanceDesc->WorldMatrix);

	//m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_InitPos.x, m_InitPos.y, m_InitPos.z, 1.f));
	//m_pTransformCom->SetUp_Scale(InstanceDesc->m_vInitScale.x, InstanceDesc->m_vInitScale.y, InstanceDesc->m_vInitScale.z);

	return S_OK;
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
	if (m_wsMap == TEXT("HOTEL")) //자신이 HOTEL맵에 속해있다면
	{
	}

	if (m_wsMap == TEXT("STATION"))//자신이 STATION맵에 속해있다면
	{
	}

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);

}

HRESULT CStaticMesh_Instance::Render()
{
	//인스턴싱으로 렌더

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_fEmissive = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &m_fEmissive, sizeof(_float))))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
		{
			return E_FAIL;
		}

		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
		{
			if (!m_bDoOnce)
			{
				/* Com_Texture */
				if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Texture_DefaultARM")),
					TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
					return E_FAIL;
				m_bDoOnce = true;
			}

			if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_ARMTexture", 0)))
				return E_FAIL;

		}

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);

	}

	return S_OK;
}

HRESULT CStaticMesh_Instance::SetEmissive()
{
	if (!m_bEmissive)
	{
		/* Com_Emissive */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Texture_Emissive")),
			TEXT("Com_Emissive"), reinterpret_cast<CComponent**>(&m_pEmissiveCom))))
			return E_FAIL;
		m_bEmissive = true;
	}

	if (FAILED(m_pEmissiveCom->Bind_ShaderResource(m_pShaderCom, "g_Emissive", 0)))
		return E_FAIL;

	m_fEmissive = 1.f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &m_fEmissive, sizeof(_float))))
		return E_FAIL;


	return S_OK;
}

HRESULT CStaticMesh_Instance::Ready_Components(void* pArg)
{
	STATICMESHINSTANCE_DESC* Desc = static_cast<STATICMESHINSTANCE_DESC*>(pArg);
	m_szMeshFullID = Desc->szModelPrototypeTag;

	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxMesh_Instance")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	CMesh_Instance::MESHINSTANCE_DESC ComDesc = {};
	ComDesc.iNumInstance = Desc->iNumInstance;
	ComDesc.pInstanceMatrixs = Desc->pInstanceMatrixs;
	
	/* Com_Model */
	if (FAILED(__super::Add_Component(Desc->iLevelID, Desc->szModelPrototypeTag/*_wstring(TEXT("Prototype_Component_Model_")) + m_szMeshID*/,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), &ComDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CStaticMesh_Instance::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
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

	Safe_Release(m_pModelCom);

}
