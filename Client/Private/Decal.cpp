#include "GameInstance.h"

#include "Decal.h"

CDecal::CDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
{
}

CDecal::CDecal(const CDecal& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CDecal::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDecal::Initialize(void* pArg)
{
	DECAL_DESC* pDesc = static_cast<DECAL_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);

	return S_OK;
}

void CDecal::Priority_Update(_float fTimeDelta)
{
}

void CDecal::Update(_float fTimeDelta)
{
}

void CDecal::Late_Update(_float fTimeDelta)
{
	//컬링? 인스턴싱?

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_DECAL, this);
}

HRESULT CDecal::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

#ifdef _DEBUG
	//if (FAILED(m_pShaderCom->Begin(1)))
	//	return E_FAIL;

	//if (FAILED(m_pVIBufferCom->Bind_Buffers()))
	//	return E_FAIL;

	//if (FAILED(m_pVIBufferCom->Render()))
	//	return E_FAIL;
#endif // _DEBUG

	return S_OK;
}

HRESULT CDecal::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPos"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_VolumeMesh"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Blood_ARMT"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CDecal::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_PBR_Depth"), m_pShaderCom, "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	_float4x4 WorldMatrixInv = {};
	XMStoreFloat4x4(&WorldMatrixInv, m_pTransformCom->Get_WorldMatrix_Inverse());
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrixInv", &WorldMatrixInv)))
		return E_FAIL;

	_float4x4 ViewMatrixInv = {};
	XMStoreFloat4x4(&ViewMatrixInv, m_pGameInstance->Get_Transform_Matrix_Inv(D3DTS::VIEW));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrixInv", &ViewMatrixInv)))
		return E_FAIL;

	_float4x4 ProjMatrixInv = {};
	XMStoreFloat4x4(&ProjMatrixInv, m_pGameInstance->Get_Transform_Matrix_Inv(D3DTS::PROJ));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrixInv", &ProjMatrixInv)))
		return E_FAIL;

	return S_OK;
}

CDecal* CDecal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CDecal* pInstance = new CDecal(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CDecal");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDecal::Clone(void* pArg)
{
	CDecal* pInstance = new CDecal(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDecal");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CDecal::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
