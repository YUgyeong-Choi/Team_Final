#include "ToolSprite.h"

#include "GameInstance.h"

CToolSpirte::CToolSpirte(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CSpriteEffect{ pDevice, pContext }
{

}

CToolSpirte::CToolSpirte(const CToolSpirte& Prototype)
	: CSpriteEffect( Prototype )
{

}

HRESULT CToolSpirte::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CToolSpirte::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;


	return S_OK;
}

void CToolSpirte::Priority_Update(_float fTimeDelta)
{

}

void CToolSpirte::Update(_float fTimeDelta)
{
	m_pTransformCom->Get_Scaled();
	m_pTransformCom->Scaling();
	return ;
}

void CToolSpirte::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_BLEND, this);
}

HRESULT CToolSpirte::Render()
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

HRESULT CToolSpirte::Ready_Components()
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

HRESULT CToolSpirte::Bind_ShaderResources()
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

CToolSpirte* CToolSpirte::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CToolSpirte* pInstance = new CToolSpirte(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CToolSpirte::Clone(void* pArg)
{
	CToolSpirte* pInstance = new CToolSpirte(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CToolSpirte");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CToolSpirte::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
