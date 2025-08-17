#include "GameInstance.h"

#include "Static_Decal.h"

CStatic_Decal::CStatic_Decal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CDecal(pDevice, pContext)
{
}

CStatic_Decal::CStatic_Decal(const CStatic_Decal& Prototype)
	:CDecal(Prototype)
{
}

HRESULT CStatic_Decal::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CStatic_Decal::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	return S_OK;
}

void CStatic_Decal::Priority_Update(_float fTimeDelta)
{
}

void CStatic_Decal::Update(_float fTimeDelta)
{
}

void CStatic_Decal::Late_Update(_float fTimeDelta)
{
	if(m_pGameInstance->isIn_Frustum_WorldSpace(m_pTransformCom->Get_State(STATE::POSITION), 10.f))
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_DECAL, this);

}

HRESULT CStatic_Decal::Render()
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

HRESULT CStatic_Decal::Ready_Components(void* pArg)
{

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Decal"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_VolumeMesh"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	DECAL_DESC* pDesc = static_cast<DECAL_DESC*>(pArg);

	/* For.Com_Texture_ARMT */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), pDesc->PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::ARMT)],
		TEXT("Com_Texture_ARMT"), reinterpret_cast<CComponent**>(&m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::ARMT)]))))
		return E_FAIL;

	/* For.Com_Texture_N */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), pDesc->PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::N)],
		TEXT("Com_Texture_N"), reinterpret_cast<CComponent**>(&m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::N)]))))
		return E_FAIL;

	/* For.Com_Texture_BC */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), pDesc->PrototypeTag[ENUM_CLASS(TEXTURE_TYPE::BC)],
		TEXT("Com_Texture_BC"), reinterpret_cast<CComponent**>(&m_pTextureCom[ENUM_CLASS(TEXTURE_TYPE::BC)]))))
		return E_FAIL;

	return S_OK;
}

HRESULT CStatic_Decal::Bind_ShaderResources()
{
	return __super::Bind_ShaderResources();
}

CStatic_Decal* CStatic_Decal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CStatic_Decal* pInstance = new CStatic_Decal(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CStatic_Decal");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CStatic_Decal::Clone(void* pArg)
{
	CStatic_Decal* pInstance = new CStatic_Decal(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CStatic_Decal");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CStatic_Decal::Free()
{
	__super::Free();
}
