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

	if (FAILED(m_pShaderCom->Begin(0)))
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
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_Texture_Splash"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CSpriteEffect::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	/* dx9 : 장치에 뷰, 투영행렬을 저장해두면 렌더링시 알아서 정점에 Transform해주었다. */
	/* dx11 : 셰이더에 뷰, 투영행렬을 저장해두고 우리가 직접 변환해주어야한다. */

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

void CSpriteEffect::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
