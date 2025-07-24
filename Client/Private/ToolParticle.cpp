#include "ToolParticle.h"

#include "GameInstance.h"

CToolParticle::CToolParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffectBase { pDevice, pContext }
{

}

CToolParticle::CToolParticle(const CToolParticle& Prototype)
	: CEffectBase( Prototype )
{

}

HRESULT CToolParticle::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CToolParticle::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CToolParticle::Priority_Update(_float fTimeDelta)
{

}

void CToolParticle::Update(_float fTimeDelta)
{

	return ;
}

void CToolParticle::Late_Update(_float fTimeDelta)
{
	/* WeightBlend */	
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONLIGHT, this);
}

HRESULT CToolParticle::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CToolParticle::Set_Loop(_bool isLoop)
{
	m_pVIBufferCom->Set_Loop(isLoop);
}

HRESULT CToolParticle::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPosInstance"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::CY), TEXT("Prototype_Component_VIBuffer_ToolParticle"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Smoke"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	return S_OK;
}

CToolParticle* CToolParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CToolParticle* pInstance = new CToolParticle(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CToolParticle::Clone(void* pArg)
{
	CToolParticle* pInstance = new CToolParticle(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CToolParticle");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CToolParticle::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
