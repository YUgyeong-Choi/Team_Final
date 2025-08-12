#include "ToolTrail.h"

#include "GameInstance.h"

CToolTrail::CToolTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CTrailEffect { pDevice, pContext }
{

}

CToolTrail::CToolTrail(const CToolTrail& Prototype)
	: CTrailEffect( Prototype )
{

}

HRESULT CToolTrail::Initialize_Prototype()
{
	m_KeyFrames.push_back(EFFKEYFRAME{});
	return S_OK;
}

HRESULT CToolTrail::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_bTextureUsage[TU_DIFFUSE] = true;
	m_bTextureUsage[TU_MASK1] = true;
	m_bTextureUsage[TU_MASK2] = true;
	m_bTextureUsage[TU_MASK3] = false;

	m_iShaderPass = 0;
	return S_OK;
}

void CToolTrail::Priority_Update(_float fTimeDelta)
{

}

void CToolTrail::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CToolTrail::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CToolTrail::Render()
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

void CToolTrail::Update_Tool(_float fTimeDelta, _float fCurFrame)
{
	m_fCurrentTrackPosition = fCurFrame;
	if (m_fCurrentTrackPosition > static_cast<_float>(m_iDuration))
		m_fCurrentTrackPosition = 0;

	Update_Keyframes();

	if (m_bAnimation)
		m_iTileIdx = static_cast<_int>(m_fCurrentTrackPosition);
	else
		m_iTileIdx = 0;

	if (m_iTileX == 0)
		m_iTileX = 1;
	if (m_iTileY == 0)
		m_iTileY = 1;

	m_fTileSize.x = 1.0f / _float(m_iTileX);
	m_fTileSize.y = 1.0f / _float(m_iTileY);
	m_fOffset.x = (m_iTileIdx % m_iTileX) * m_fTileSize.x;
	m_fOffset.y = (m_iTileIdx / m_iTileX) * m_fTileSize.y;
}

HRESULT CToolTrail::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_TrailEffect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Trail"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Texture_T_Mask_27_C_GDH"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_DIFFUSE]))))
		return E_FAIL;
	
	/* For.Com_TextureMask1 */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Texture_T_Trail_01_C_GDH"),
		TEXT("Com_TextureMask1"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_MASK1]))))
		return E_FAIL;
	
	/* For.Com_TextureMask2 */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Texture_T_Slash_01_C_RSW"),
		TEXT("Com_TextureMask2"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_MASK2]))))
		return E_FAIL;

	return S_OK;
}

HRESULT CToolTrail::Bind_ShaderResources()
{
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &m_fEmissiveIntensity, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	if (m_bTextureUsage[TU_DIFFUSE] == true){
		if (FAILED(m_pTextureCom[TU_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0)))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK1] == true){
		if (FAILED(m_pTextureCom[TU_MASK1]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture1", 0)))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK2] == true){
		if (FAILED(m_pTextureCom[TU_MASK2]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture2", 0)))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK3] == true){
		if (FAILED(m_pTextureCom[TU_MASK3]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture3", 0)))
			return E_FAIL;
	}

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
		return E_FAIL;

	return S_OK;
}

CToolTrail* CToolTrail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CToolTrail* pInstance = new CToolTrail(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CToolTrail::Clone(void* pArg)
{
	CToolTrail* pInstance = new CToolTrail(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CToolTrail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CToolTrail::Free()
{
	__super::Free();

}
