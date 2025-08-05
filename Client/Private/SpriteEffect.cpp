#include "SpriteEffect.h"

#include "GameInstance.h"

CSpriteEffect::CSpriteEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffectBase{ pDevice, pContext }
{

}

CSpriteEffect::CSpriteEffect(const CSpriteEffect& Prototype)
	: CEffectBase( Prototype )
{

}

HRESULT CSpriteEffect::Initialize_Prototype()
{
	//m_KeyFrames.push_back(EFFKEYFRAME{});
	return S_OK;
}

HRESULT CSpriteEffect::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	DESC* pDesc = static_cast<DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	//if (!m_bTool)
	//{
	//	if (FAILED(Ready_Components()))
	//		return E_FAIL;
	//}


	m_fTileSize.x = 1.0f / _float(m_iTileX);
	m_fTileSize.y = 1.0f / _float(m_iTileY);
	m_iTileCnt = m_iTileX * m_iTileY;

	return S_OK;
}

void CSpriteEffect::Priority_Update(_float fTimeDelta)
{

}

void CSpriteEffect::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

}

void CSpriteEffect::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_BLEND, this);
}

HRESULT CSpriteEffect::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSpriteEffect::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SpriteEffect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	_wstring TextureTag = TEXT("Prototype_Component_Texture_");
	if (m_bTextureUsage[TU_DIFFUSE] == true){
		/* For.Com_Texture */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_DIFFUSE], 
			TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_DIFFUSE]))))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK1] == true){
		/* For.Com_TextureMask1 */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_MASK1],
			TEXT("Com_TextureMask1"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_MASK1]))))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK2] == true){
		/* For.Com_TextureMask2 */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_MASK2],
			TEXT("Com_TextureMask2"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_MASK2]))))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK3] == true){
		/* For.Com_TextureMask3 */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_MASK3],
			TEXT("Com_TextureMask3"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_MASK3]))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CSpriteEffect::Bind_ShaderResources()
{
	if (m_pSocketMatrix != nullptr)
	{
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;
	}
	/* dx9 : 장치에 뷰, 투영행렬을 저장해두면 렌더링시 알아서 정점에 Transform해주었다. */
	/* dx11 : 셰이더에 뷰, 투영행렬을 저장해두고 우리가 직접 변환해주어야한다. */
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTileSize", &m_fTileSize, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTileOffset", &m_fOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	if (m_bTextureUsage[TU_DIFFUSE] == true) {
		if (FAILED(m_pTextureCom[TU_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0)))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK1] == true) {
		if (FAILED(m_pTextureCom[TU_MASK1]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture1", 0)))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK2] == true) {
		if (FAILED(m_pTextureCom[TU_MASK2]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture2", 0)))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK3] == true) {
		if (FAILED(m_pTextureCom[TU_MASK3]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture3", 0)))
			return E_FAIL;
	}

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
		return E_FAIL;

	return S_OK;
}

CSpriteEffect* CSpriteEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSpriteEffect* pInstance = new CSpriteEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSpriteEffect::Clone(void* pArg)
{
	CSpriteEffect* pInstance = new CSpriteEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CSpriteEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSpriteEffect::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	//Safe_Release(m_pShaderCom);
	//Safe_Release(m_pTextureCom);
}

json CSpriteEffect::Serialize()
{
	json j = __super::Serialize();


	return j;
}

void CSpriteEffect::Deserialize(const json& j)
{
	__super::Deserialize(j);
}
