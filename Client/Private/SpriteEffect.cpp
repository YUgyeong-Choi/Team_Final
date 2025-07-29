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
	json j = __super::Serialize();


	return j;
}

void CSpriteEffect::Deserialize(const json& j)
{
	// Basic Effect Preferences
	if (j.contains("Color") && j["Color"].is_array() && j["Color"].size() == 4)
		m_vColor = { j["Color"][0].get<_float>(), j["Color"][1].get<_float>(), j["Color"][2].get<_float>(), j["Color"][3].get<_float>() };

	if (j.contains("LifeTime"))
		m_fLifeTime = j["LifeTime"].get<_float>();

	if (j.contains("Billboard"))
		m_bBillboard = j["Billboard"].get<_bool>();

	if (j.contains("Animation"))
		m_bAnimation = j["Animation"].get<_bool>();

	if (j.contains("ShaderPass"))
		m_iShaderPass = j["ShaderPass"].get<_uint>();

	// Texture Usage
	if (j.contains("TextureUsage") && j["TextureUsage"].is_array())
	{
		for (int i = 0; i < TU_END && i < j["TextureUsage"].size(); ++i)
			m_bTextureUsage[i] = j["TextureUsage"][i].get<_bool>();
	}

	if (j.contains("TextureTags") && j["TextureTags"].is_array())
	{
		for (int i = 0; i < TU_END && i < j["TextureTags"].size(); ++i)
			m_TextureTag[i] = StringToWString(j["TextureTags"][i].get<std::string>());
	}

	// Track Positions
	if (j.contains("Duration"))
		m_iDuration = j["Duration"].get<_int>();

	if (j.contains("StartTrack"))
		m_iStartTrackPosition = j["StartTrack"].get<_int>();

	if (j.contains("EndTrack"))
		m_iEndTrackPosition = j["EndTrack"].get<_int>();

	if (j.contains("TickPerSecond"))
		m_fTickPerSecond = j["TickPerSecond"].get<_float>();

	// KeyFrames
	if (j.contains("NumKeyFrames"))
		m_iNumKeyFrames = j["NumKeyFrames"].get<_uint>();

	if (j.contains("KeyFrames") && j["KeyFrames"].is_array())
	{
		m_KeyFrames.clear();
		for (const auto& keyJson : j["KeyFrames"])
		{
			tagEffectKeyFrame key;
			key.Deserialize(keyJson);
			m_KeyFrames.push_back(key);
		}
	}

	// UV Grid
	if (j.contains("TileX"))
		m_iTileX = j["TileX"].get<_int>();

	if (j.contains("TileY"))
		m_iTileY = j["TileY"].get<_int>();

	if (j.contains("FlipUV"))
		m_bFlipUV = j["FlipUV"].get<_bool>();
}
