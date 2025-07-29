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
	m_KeyFrames.push_back(EFFKEYFRAME{});
	return S_OK;
}

HRESULT CSpriteEffect::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	DESC* pDesc = static_cast<DESC*>(pArg);

	m_bAnimation = pDesc->bAnimation;


	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (!m_bTool)
	{
		if (FAILED(Ready_Components()))
			return E_FAIL;
	}
	return S_OK;
}

void CSpriteEffect::Priority_Update(_float fTimeDelta)
{

}

void CSpriteEffect::Update(_float fTimeDelta)
{
	//if (m_bAnimation)
	//{
	//	m_fFrame += m_fMaxFrame * fTimeDelta;

	//	if (m_fFrame >= m_fMaxFrame)
	//		m_fFrame = 0.f;
	//}
	__super::Update(fTimeDelta);

	return ;
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

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Texture_T_SubUV_Explosion_01_8x8_SC_HJS"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CSpriteEffect::Bind_ShaderResources()
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

	if (FAILED(m_pTextureCom[TU_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

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
	json j;

	// Basic Effect Preferences
	j["Color"] = { m_vColor.x, m_vColor.y, m_vColor.z, m_vColor.w };
	j["LifeTime"] = m_fLifeTime;
	j["Billboard"] = m_bBillboard;
	j["Animation"] = m_bAnimation;
	j["ShaderPass"] = m_iShaderPass;

	// Texture Usage
	json textureUsage;
	for (int i = 0; i < TU_END; ++i)
		textureUsage.push_back(m_bTextureUsage[i]);
	j["TextureUsage"] = textureUsage;

	// Track Positions
	j["Duration"] = m_iDuration;
	j["StartTrack"] = m_iStartTrackPosition;
	j["EndTrack"] = m_iEndTrackPosition;
	j["TickPerSecond"] = m_fTickPerSecond;

	// KeyFrames
	j["NumKeyFrames"] = m_iNumKeyFrames;

	json keyFramesJson = json::array();
	for (auto& key : m_KeyFrames)
		keyFramesJson.push_back(key.Serialize()); // EFFKEYFRAME에 Serialize() 함수 필요
	j["KeyFrames"] = keyFramesJson;

	// UV Grid
	j["TileX"] = m_iTileX;
	j["TileY"] = m_iTileY;
	j["FlipUV"] = m_bFlipUV;

	return j;
}

void CSpriteEffect::Deserialize(const json& j)
{
}
