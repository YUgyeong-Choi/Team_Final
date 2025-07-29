#include "MeshEffect.h"

#include "GameInstance.h"

CMeshEffect::CMeshEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffectBase{ pDevice, pContext }
{

}

CMeshEffect::CMeshEffect(const CMeshEffect& Prototype)
	: CEffectBase(Prototype)
{

}

HRESULT CMeshEffect::Initialize_Prototype()
{
	m_KeyFrames.push_back(EFFKEYFRAME{});
	return S_OK;
}

HRESULT CMeshEffect::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	DESC* pDesc = static_cast<DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (!m_bTool)
	{
		if (FAILED(Ready_Components()))
			return E_FAIL;
	}
	return S_OK;
}

void CMeshEffect::Priority_Update(_float fTimeDelta)
{

}

void CMeshEffect::Update(_float fTimeDelta)
{

	__super::Update(fTimeDelta);

	return;
}

void CMeshEffect::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_BLEND, this);
}

HRESULT CMeshEffect::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (m_bTextureUsage[TU_DIFFUSE] == true)
		{
			if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0)))
				return E_FAIL;
		}
		if (m_bTextureUsage[TU_MASK1] == true)
		{
			if (FAILED(m_pMaskTextureCom[0]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture1", 0)))
				return E_FAIL;
		}
		if (m_bTextureUsage[TU_MASK2] == true)
		{
			if (FAILED(m_pMaskTextureCom[1]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture2", 0)))
				return E_FAIL;
		}
		//m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);

		//m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		m_pShaderCom->Begin(m_iShaderPass);

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CMeshEffect::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MeshEffect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Model_ToolMeshEffect"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Texture_T_SubUV_Explosion_01_8x8_SC_HJS"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;
	return S_OK;
}

HRESULT CMeshEffect::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTileSize", &m_fTileSize, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTileOffset", &m_fOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
		return E_FAIL;

	return S_OK;
}

CMeshEffect* CMeshEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeshEffect* pInstance = new CMeshEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeshEffect::Clone(void* pArg)
{
	CMeshEffect* pInstance = new CMeshEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMeshEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeshEffect::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}
