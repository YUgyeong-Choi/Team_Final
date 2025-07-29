#include "ToolMeshEffect.h"

#include "GameInstance.h"

CToolMeshEffect::CToolMeshEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMeshEffect{ pDevice, pContext }
{

}

CToolMeshEffect::CToolMeshEffect(const CToolMeshEffect& Prototype)
	: CMeshEffect( Prototype )
{

}

HRESULT CToolMeshEffect::Initialize_Prototype()
{
	m_KeyFrames.push_back(EFFKEYFRAME{});
	return S_OK;
}

HRESULT CToolMeshEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_fThreshold = 0.9f;
	m_fIntensity = 3.0f;
	m_vCenterColor = {1.f, 1.f, 1.f, 1.f};
	//m_vCenterColor = {1.f, 1.f, 1.f, 1.f};
	m_bTextureUsage[TU_DIFFUSE] = true;
	m_bTextureUsage[TU_MASK1] = false;
	m_bTextureUsage[TU_MASK2] = false;

	m_iShaderPass = ENUM_CLASS(ME_UVMASK);

	return S_OK;
}

void CToolMeshEffect::Priority_Update(_float fTimeDelta)
{
	m_fTimeAcc += fTimeDelta;
}

void CToolMeshEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CToolMeshEffect::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_BLEND, this);
}

HRESULT CToolMeshEffect::Render()
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

HRESULT CToolMeshEffect::Change_Model(_wstring strModelName)
{
	Safe_Release(m_pModelCom);
	_wstring strModelTag = L"Prototype_Component_Model_" + strModelName;
	return Replace_Component(ENUM_CLASS(LEVEL::CY), strModelTag.c_str(),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom));
}

HRESULT CToolMeshEffect::Ready_Components()
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
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Texture_T_SubUV_Thunder_01_4x1_SC_GDH"), // 세로
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;
	/* For.Com_TextureMask1 */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Texture_T_Aura_04_C_LGS"), // 가로
		TEXT("Com_TextureMask1"), reinterpret_cast<CComponent**>(&m_pMaskTextureCom[0]))))
		return E_FAIL;
	/* For.Com_TextureMask2 */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Texture_T_Tile_Noise_81_C_GDH"), // 노이즈
		TEXT("Com_TextureMask2"), reinterpret_cast<CComponent**>(&m_pMaskTextureCom[1]))))
		return E_FAIL;

	return S_OK;
}

HRESULT CToolMeshEffect::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	/* dx9 : 장치에 뷰, 투영행렬을 저장해두면 렌더링시 알아서 정점에 Transform해주었다. */
	/* dx11 : 셰이더에 뷰, 투영행렬을 저장해두고 우리가 직접 변환해주어야한다. */
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bFlipUV", &m_bFlipUV, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTileSize", &m_fTileSize, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTileOffset", &m_fOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCenterColor", &m_vCenterColor, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fIntensity", &m_fIntensity, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fThreshold", &m_fThreshold, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fTimeAcc, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
		return E_FAIL;

	return S_OK;
}

CToolMeshEffect* CToolMeshEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CToolMeshEffect* pInstance = new CToolMeshEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CToolMeshEffect::Clone(void* pArg)
{
	CToolMeshEffect* pInstance = new CToolMeshEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CToolMeshEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CToolMeshEffect::Free()
{
	__super::Free();

}
