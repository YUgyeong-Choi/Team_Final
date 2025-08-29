#include "GameInstance.h"

#include "DecalToolObject.h"

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
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	DECAL_DESC* pDesc = static_cast<DECAL_DESC*>(pArg);

	m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);
	m_bNormalOnly = pDesc->bNormalOnly;

	m_bHasLifeTime = pDesc->bHasLifeTime;
	m_fLifeTime = pDesc->fLifeTime;

	return S_OK;
}

void CDecal::Priority_Update(_float fTimeDelta)
{
}

void CDecal::Update(_float fTimeDelta)
{
	if (m_bHasLifeTime)
	{
		m_fLifeTime -= fTimeDelta;

		if (m_fLifeTime < 0.f)
			Set_bDead();
	}

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

	if (m_bNormalOnly)
	{
		//노말만 사용하는 데칼
		if (FAILED(m_pShaderCom->Begin(2)))
			return E_FAIL;
	}
	else
	{
		//기본 ARMT, N, BC 사용하는 데칼
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;
	}

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CDecal::Ready_Components(void* Arg)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Decal"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_VolumeMesh"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	/* For.Com_Texture_ARMT */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Texture_DefaultDecal"),
		TEXT("Com_Texture_ARMT"), reinterpret_cast<CComponent**>(&m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::ARMT)]))))
		return E_FAIL;

	/* For.Com_Texture_N */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Texture_DefaultDecal"),
		TEXT("Com_Texture_N"), reinterpret_cast<CComponent**>(&m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::N)]))))
		return E_FAIL;

	/* For.Com_Texture_BC */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Texture_DefaultDecal"),
		TEXT("Com_Texture_BC"), reinterpret_cast<CComponent**>(&m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::BC)]))))
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

	/* 
	노말만 적용하려면...AMRT의 알파를 0으로하고
	노말은 마스킹으로 처리해야하는데...
	*/

	//노말 바인드
	if (FAILED(m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::N)]->Bind_ShaderResource(m_pShaderCom, "g_N", 0)))
		return E_FAIL;

	if (m_bNormalOnly)
	{
		//마스킹 텍스쳐 바인드
		if (FAILED(m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::MASK)]->Bind_ShaderResource(m_pShaderCom, "g_MASK", 0)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::ARMT)]->Bind_ShaderResource(m_pShaderCom, "g_ARMT", 0)))
			return E_FAIL;

		if (FAILED(m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::BC)]->Bind_ShaderResource(m_pShaderCom, "g_BC", 0)))
			return E_FAIL;
	}

	_float4x4 ProjMatrixInv = {};
	XMStoreFloat4x4(&ProjMatrixInv, m_pGameInstance->Get_Transform_Matrix_Inv(D3DTS::PROJ));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrixInv", &ProjMatrixInv)))
		return E_FAIL;

	_float4x4 ViewWorldMatrixInv = {};
	XMStoreFloat4x4(&ViewWorldMatrixInv, m_pGameInstance->Get_Transform_Matrix_Inv(D3DTS::VIEW) * m_pTransformCom->Get_WorldMatrix_Inverse());
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewWorldMatrixInv", &ViewWorldMatrixInv)))
		return E_FAIL;
	

	return S_OK;
}

void CDecal::Free()
{
	__super::Free();

	for (_int i = 0; i < ENUM_CLASS(TEXTURE_TYPE::END); ++i)
	{
		Safe_Release(m_pTextureCom[i]);
	}

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
}
